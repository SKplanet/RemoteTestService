/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <binder/IMemory.h>
#include <surfaceflinger/SurfaceComposerClient.h>

//#include <SkImageEncoder.h>
//#include <SkBitmap.h>
//#include <SkData.h>
//#include <SkStream.h>

using namespace android;

static void usage(const char* pname)
{
    fprintf(stderr,
            "usage: %s [-hp] [FILENAME]\n"
            "   -h: this message\n"
            "   -p: save the file as a png.\n"
            "If FILENAME ends with .png it will be saved as a png.\n"
            "If FILENAME is not given, the results will be printed to stdout.\n",
            pname
    );
}

/*
static SkBitmap::Config flinger2skia(PixelFormat f)
{
    switch (f) {
        case PIXEL_FORMAT_A_8:
        case PIXEL_FORMAT_L_8:
            return SkBitmap::kA8_Config;
        case PIXEL_FORMAT_RGB_565:
            return SkBitmap::kRGB_565_Config;
        case PIXEL_FORMAT_RGBA_4444:
            return SkBitmap::kARGB_4444_Config;
        default:
            return SkBitmap::kARGB_8888_Config;
    }
}
*/

static status_t vinfoToPixelFormat(const fb_var_screeninfo& vinfo,
        uint32_t* bytespp, uint32_t* f)
{

    switch (vinfo.bits_per_pixel) {
        case 16:
            *f = PIXEL_FORMAT_RGB_565;
            *bytespp = 2;
            break;
        case 24:
            *f = PIXEL_FORMAT_RGB_888;
            *bytespp = 3;
            break;
        case 32:
            // TODO: do better decoding of vinfo here
            *f = PIXEL_FORMAT_RGBX_8888;
            *bytespp = 4;
            break;
        default:
            return BAD_VALUE;
    }
    return NO_ERROR;
}

int main(int argc, char** argv)
{
    const char* pname = argv[0];
    bool png = false;
    int c;
    while ((c = getopt(argc, argv, "ph")) != -1) {
        switch (c) {
            case 'p':
                png = true;
                break;
            case '?':
            case 'h':
                usage(pname);
                return 1;
        }
    }
    argc -= optind;
    argv += optind;

    int fd = -1;
    /*
    if (argc == 0) {
        fd = dup(STDOUT_FILENO);
    } else if (argc == 1) {
        const char* fn = argv[0];
        fd = open(fn, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        if (fd == -1) {
            fprintf(stderr, "Error opening file: %s (%s)\n", fn, strerror(errno));
            return 1;
        }
        const int len = strlen(fn);
        if (len >= 4 && 0 == strcmp(fn+len-4, ".png")) {
            png = true;
        }
    }
    
    if (fd == -1) {
        usage(pname);
        return 1;
    }
    */

    void const* mapbase = MAP_FAILED;
    ssize_t mapsize = -1;

    void const* base = 0;
    uint32_t w, h, f;
    size_t size = 0;
    int i = 0;
    int ntest = 100;
    char fname[256];
    struct timeval tS, tE;
    double utime, mtime, seconds, useconds, average;



   gettimeofday(&tS, NULL);
    ScreenshotClient screenshot;
    for(i=0; i< ntest; i++) {

    sprintf(fname,"fb%02d.raw",i);
    fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if (screenshot.update() == NO_ERROR) {
        base = screenshot.getPixels();
        w = screenshot.getWidth();
        h = screenshot.getHeight();
        f = screenshot.getFormat();
        size = screenshot.getSize();
    } else {
        printf("screenshot update failed - 2 \n");
        /*
        const char* fbpath = "/dev/graphics/fb0";
        int fb = open(fbpath, O_RDONLY);
        if (fb >= 0) {
            struct fb_var_screeninfo vinfo;
            if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == 0) {
                uint32_t bytespp;
                if (vinfoToPixelFormat(vinfo, &bytespp, &f) == NO_ERROR) {
                    size_t offset = (vinfo.xoffset + vinfo.yoffset*vinfo.xres) * bytespp;
                    w = vinfo.xres;
                    h = vinfo.yres;
                    size = w*h*bytespp;
                    mapsize = offset + size;
                    mapbase = mmap(0, mapsize, PROT_READ, MAP_PRIVATE, fb, 0);
                    if (mapbase != MAP_FAILED) {
                        base = (void const *)((char const *)mapbase + offset);
                    }
                }
            }
            close(fb);
        }
        */
    }

    if (base) {
        /*
        if (png) {
            SkBitmap b;
            b.setConfig(flinger2skia(f), w, h);
            b.setPixels((void*)base);
            SkDynamicMemoryWStream stream;
            SkImageEncoder::EncodeStream(&stream, b,
                    SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);
            SkData* streamData = stream.copyToData();
            write(fd, streamData->data(), streamData->size());
            streamData->unref();
        } else 
        */
        {
            write(fd, &w, 4);
            write(fd, &h, 4);
            write(fd, &f, 4);
            write(fd, base, size);
        }
    }
    close(fd);
    if (mapbase != MAP_FAILED) {
        munmap((void *)mapbase, mapsize);
    }

    screenshot.release();
    printf("%d:%s\n", i+1, fname);
    gettimeofday(&tE, NULL);
    seconds = tE.tv_sec - tS.tv_sec;
    useconds= tE.tv_usec- tS.tv_usec;

    utime = seconds * 1000000 + useconds;
    printf("Elapsed:%.2f\n", utime/1000000 );
    usleep(1);
    }
    average = (double)ntest * 1000000 / utime ;
    printf("Elapsed:%.2f, Average:%.2f\n", utime/1000000, average);
    return 0;
}
