/*
* The MIT License (MIT)
* Copyright (c) 2015 SK PLANET. All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
#include "util.h"
#include <binder/IMemory.h>
//#include <surfaceflinger/ISurfaceComposer.h>
//#include <surfaceflinger/Surface.h>
//#define JELLY_SUPPORT	1
#ifdef JELLY_SUPPORT
	#include <gui/SurfaceComposerClient.h>
#else
	#include <surfaceflinger/SurfaceComposerClient.h>
#endif
#include <cutils/properties.h>
#include <linux/input.h>


/*
 _Z     | N      | 5Thing  | 3foo | E          | v
 prefix | nested | `Thing` | `foo`| end nested | parameters: `void`

You can read the constructor names similarly, as below. Notice how the constructor "name" isn't given, but instead a C clause:

 _Z     | N      | 5Thing  | C1          | E          | i
 prefix | nested | `Thing` | Constructor | end nested | parameters: `int`

 <ctor-dtor-name> ::= C1   # complete object constructor
                   ::= C2   # base object constructor
                   ::= C3   # complete object allocating constructor
                   ::= D0   # deleting destructor
                   ::= D1   # complete object destructor
                   ::= D2   # base object destructor
static struct tagScreenshotClientFn {
    void (*ScreenshotClient)(void);
    int (*Update1)(void);
    int (&Update2)(unsigned int reqwidth, unsigned int reqheight);
    unsigned int (*getWidth)(void);
    unsigned int (*getHeight)(void)z
} gScreenshotClientFn;
 */

// for PowerManager
/*
#include "JNIHelp.h"
#include "jni.h"
#include <android_runtime/AndroidRuntime.h>
#include <ui/PowerManager.h>
*/
#define ABS_MT_SLOT	 0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse (48)*/
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) (49)*/
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse (50)*/
#define ABS_MT_POSITION_X	0x35	/* (53) */
#define ABS_MT_POSITION_Y	0x36	/* (54) */
#define ABS_MT_TRACKING_ID	0x39  	/* (57) */
#define ABS_MT_PRESSURE		0x3a    /* (58) */

using namespace android;

static int getMask(int length);
int check_screencap(struct fbinfo * pfbinfo);
void framebuffer_service(int fd, void *cookie);
int exit_framebuffer();
int init_framebuffer();
int read_rgb_framebuffer();
int read_rgb_framebuffer_to_jpeg(svcInfoPtr psvc);
int read_rgb_framebuffer_screencap();
int read_rgb_framebuffer_screencap_to_jpeg(svcInfoPtr psvc);
int read_rgb_framebuffer_fb0();
int read_rgb_framebuffer_fb0_to_jpeg(svcInfoPtr psvc);
int convert_to_rgb(unsigned char * x, int xsize, unsigned char * target);
int convert_to_jpeg(int quality);
void init_jpeg(int quality);
int exit_jpeg();


//global - for fb
int g_forcefbmode = 0;
int g_halfmode = 0;
struct fbinfo g_fbinfo;
unsigned char* g_fbbuffer = NULL;
//#define X_SIZE  640
unsigned char* g_x=NULL;
unsigned int     g_xsize = 0;

//global - for jpeg
static struct jpeg_destination_mgr jpegDstManager;
static int jpegError = FALSE;
static int jpegDstDataLen = 0;
static int jpegBufSize = 0;
//static char * jpegBuf = NULL;
static struct jpeg_compress_struct cinfo;
static struct jpeg_error_mgr jerr;
JSAMPROW rowPointer[1];
int fbc_output_current=0;
FBC_OUTPUT fbc_output[2];

// touch
#define FROYO_EVENT_PROTO 65536
#define ICS_EVENT_PROTO 65537

int touchfd = -1;
int touch_btn = 0;
int touchversion = ICS_EVENT_PROTO;
static int xmin, xmax;
static int ymin, ymax;



static int getMask(int length) {
   return (1 << length) - 1;
}

int is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}
#define PROPERTY_VALUE_MAX  92
//int property_get(const char *key, char *value, const char *default_value);
/*
 * 	FROYO					8	2.2
 * 	GINGERBREAD				9	2.3 / 2.3.1 / 2.3.2
 *	GINGERBREAD_MR1			10	2.3.3 / 2.3.4
 *	HONEYCOMB				11	3.0
 *	HONEYCOMB_MR1			12	3.1
 *	HONEYCOMB_MR2			13	3.2
 *	ICE_CREAM_SANDWICH		14	4.0 / 4.0.1 / 4.0.2
 *	ICE_CREAM_SANDWICH_MR1	15	4.0.3
 */
int get_android_version()
{
	char prop_value[PROPERTY_VALUE_MAX];
	int iver = 0;
	if( property_get("ro.build.version.sdk", prop_value, "0") )
	{
		Log("getprop(%s)\n", prop_value);
		iver = atoi(prop_value);
	}

	Log("ANDROID version(%d)\n",iver);
	return iver;
}

/*
#define FIND_CLASS(var, className) \
        var = env->FindClass(className); \
        LOG_FATAL_IF(! var, "Unable to find class " className); \
        var = jclass(env->NewGlobalRef(var));

#define GET_METHOD_ID(var, clazz, methodName, methodDescriptor) \
        var = env->GetMethodID(clazz, methodName, methodDescriptor); \
        LOG_FATAL_IF(! var, "Unable to find method " methodName);

#define GET_FIELD_ID(var, clazz, fieldName, fieldDescriptor) \
        var = env->GetFieldID(clazz, fieldName, fieldDescriptor); \
        LOG_FATAL_IF(! var, "Unable to find field " fieldName);

static struct {
    jclass clazz;

    jmethodID isScreenOn;
} gPowerManagerServiceClassInfo;

JNIEnv* create_vm() {
	JavaVM* jvm;
	JNIEnv* env;
	JavaVMInitArgs args;
	JavaVMOption options[1];

	args.version = JNI_VERSION_1_4; //  JNI_VERSION_1_2 is interchangeable for this example
	args.nOptions = 0;
	//options[0].optionString = "-Djava.class.path=c:\projects\local\inonit\classes";
	//args.options = options;
	args.ignoreUnrecognized = JNI_FALSE;
	JNI_CreateJavaVM(&jvm, &env, &args);
	//JNI_CreateJavaVM(&jvm, (void **)&env, NULL);
	return env;
}
*/
int get_screen_on()
{
	//AndroidRuntime * arun = AndroidRuntime::getRuntime();
	//JNIEnv* env = arun->getJNIEnv();
	/*
	//JNIEnv* env = AndroidRuntime::getJNIEnv();

	//jobject gPowerManagerServiceObj;
	//gPowerManagerServiceObj = env->NewGlobalRef(obj);
	//JNIEnv* env = create_vm();

    FIND_CLASS(gPowerManagerServiceClassInfo.clazz, "com/android/server/PowerManagerService");
    Log("isScreenOn clazz(%d)\n", (gPowerManagerServiceClassInfo.clazz == NULL));
    sleep(5);
    GET_METHOD_ID(gPowerManagerServiceClassInfo.isScreenOn, gPowerManagerServiceClassInfo.clazz,
            "isScreenOn", "()B");
    Log("isScreenOn method(%d)\n", (gPowerManagerServiceClassInfo.isScreenOn == NULL));
    sleep(5);
    int nRtn = env->CallBooleanMethod(gPowerManagerServiceClassInfo.clazz, gPowerManagerServiceClassInfo.isScreenOn);
    //checkAndClearExceptionFromCallback(env, "isScreenOn");
    Log("isScreenOn result(%d)\n", nRtn);

    sleep(5);
    return nRtn;
*/
	return 0;
}

/*
int get_dev_rotation()
{
	 int iOrientation = SurfaceComposerClient::getDisplayOrientation(0);

	//char prop_value[PROPERTY_VALUE_MAX];
    //
	//if( property_get("dev.rotation", prop_value, "0") )
	//{
	//	int iR = atoi(prop_value);
	//	return iR;
	//}

	//Log("check Screen Orientation(%d)\n", iOrientation);
	return iOrientation;
}
*/

int set_fbinfo(int f, int w, int h, struct fbinfo * pfbinfo)
{
    pfbinfo->version = ICS_SCREENCAP;
    switch(f) {
        case 1: /* RGBA_8888 */
            pfbinfo->bpp = 32;
            pfbinfo->size = w * h * 4;
            pfbinfo->width = w;
            pfbinfo->height = h;
            pfbinfo->red_offset = 0;
            pfbinfo->red_length = 8;
            pfbinfo->red_mask = getMask(pfbinfo->red_length);
            pfbinfo->blue_offset = 16;
            pfbinfo->blue_length = 8;
            pfbinfo->blue_mask = getMask(pfbinfo->blue_length);
            pfbinfo->green_offset = 8;
            pfbinfo->green_length = 8;
            pfbinfo->green_mask =  getMask(pfbinfo->green_length);
            pfbinfo->alpha_offset = 24;
            pfbinfo->alpha_length = 8;
            pfbinfo->alpha_mask =  getMask(pfbinfo->alpha_length);
            break;
        case 2: /* RGBX_8888 */
            pfbinfo->bpp = 32;
            pfbinfo->size = w * h * 4;
            pfbinfo->width = w;
            pfbinfo->height = h;
            pfbinfo->red_offset = 0;
            pfbinfo->red_length = 8;
            pfbinfo->red_mask = getMask(pfbinfo->red_length);
            pfbinfo->blue_offset = 16;
            pfbinfo->blue_length = 8;
            pfbinfo->blue_mask = getMask(pfbinfo->blue_length);
            pfbinfo->green_offset = 8;
            pfbinfo->green_length = 8;
            pfbinfo->green_mask =  getMask(pfbinfo->green_length);
            pfbinfo->alpha_offset = 24;
            pfbinfo->alpha_length = 0;
            pfbinfo->alpha_mask =  getMask(pfbinfo->alpha_length);
            break;
        case 3: /* RGB_888 */
            pfbinfo->bpp = 24;
            pfbinfo->size = w * h * 3;
            pfbinfo->width = w;
            pfbinfo->height = h;
            pfbinfo->red_offset = 0;
            pfbinfo->red_length = 8;
            pfbinfo->red_mask = getMask(pfbinfo->red_length);
            pfbinfo->blue_offset = 16;
            pfbinfo->blue_length = 8;
            pfbinfo->blue_mask = getMask(pfbinfo->blue_length);
            pfbinfo->green_offset = 8;
            pfbinfo->green_length = 8;
            pfbinfo->green_mask =  getMask(pfbinfo->green_length);
            pfbinfo->alpha_offset = 24;
            pfbinfo->alpha_length = 0;
            pfbinfo->alpha_mask =  getMask(pfbinfo->alpha_length);
            break;
        case 4: /* RGB_565 */
            pfbinfo->bpp = 16;
            pfbinfo->size = w * h * 2;
            pfbinfo->width = w;
            pfbinfo->height = h;
            pfbinfo->red_offset = 11;
            pfbinfo->red_length = 5;
            pfbinfo->red_mask = getMask(pfbinfo->red_length);
            pfbinfo->blue_offset = 0;
            pfbinfo->blue_length = 5;
            pfbinfo->blue_mask = getMask(pfbinfo->blue_length);
            pfbinfo->green_offset = 5;
            pfbinfo->green_length = 6;
            pfbinfo->green_mask =  getMask(pfbinfo->green_length);
            pfbinfo->alpha_offset = 0;
            pfbinfo->alpha_length = 0;
            pfbinfo->alpha_mask =  getMask(pfbinfo->alpha_length);
            break;
        case 5: /* BGRA_8888 */
            pfbinfo->bpp = 32;
            pfbinfo->size = w * h * 4;
            pfbinfo->width = w;
            pfbinfo->height = h;
            pfbinfo->red_offset = 15;
            pfbinfo->red_length = 8;
            pfbinfo->red_mask = getMask(pfbinfo->red_length);
            pfbinfo->blue_offset = 0;
            pfbinfo->blue_length = 8;
            pfbinfo->blue_mask = getMask(pfbinfo->blue_length);
            pfbinfo->green_offset = 8;
            pfbinfo->green_length = 8;
            pfbinfo->green_mask =  getMask(pfbinfo->green_length);
            pfbinfo->alpha_offset = 24;
            pfbinfo->alpha_length = 8;
            pfbinfo->alpha_mask =  getMask(pfbinfo->alpha_length);
            break;
        default:
        	Err("Unsupported format(%d)\r\n",f);
            goto done;
    }

    return 1;


done:

    return 0;
}
int exit_framebuffer()
{
	if(touchfd != -1)
	{
		close(touchfd);
	}

    if( g_fbbuffer != NULL )
    {
        free(g_fbbuffer);
    }

    if( g_x != NULL )
    {
        free(g_x);
    }
    

    if( fbc_output[0].jpegBuf != NULL )
    {
        free(fbc_output[0].jpegBuf);
        jpegBufSize = 0;
    }
    /*
    if( fbc_output[1].jpegBuf != NULL )
    {
        free(fbc_output[1].jpegBuf);
        jpegBufSize = 0;
    }
    */
    return 1;
}

/*
status_t SurfaceFlinger::captureScreen(DisplayID dpy,
        sp<IMemoryHeap>* heap,
        uint32_t* width, uint32_t* height, PixelFormat* format,
        uint32_t sw, uint32_t sh,
        uint32_t minLayerZ, uint32_t maxLayerZ)
*/

//#define test_bit(bit, array) (array [bit/8] & (1<< (bit %8)))
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define TEST_BIT(array, bit)    ((array[LONG(bit)] >> OFF(bit)) & 1)

int detect_touch_port(int checkloose)
{
	int i = 0, fd;
	unsigned long ev_bitmask[NBITS(EV_MAX)];
	char TEST_DEVICE[256];
	char name[80];
	char TOUCH_DEVICE[256];
	TOUCH_DEVICE[0] = 0;
	while( i<20 )
	{

		sprintf(TEST_DEVICE, "/dev/input/event%d",i++);

		if ((fd = open(TEST_DEVICE, O_RDWR)) == -1)
		{
			Err("cannot open test device %s\n", TEST_DEVICE);
			continue;
		}

		if( ioctl(fd, EVIOCGBIT(0, sizeof(ev_bitmask)), ev_bitmask) < 0 )
		{
			Err("cannot io test device %s\n", TEST_DEVICE);
			continue;
		}

		name[sizeof(name)-1] = 0;
		if(ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
			continue;
		}

		//Log("check device(%s)\n", name);

		if (ioctl(fd, EVIOCGVERSION, &touchversion)) {
			//Err("could not get driver version for (%d)\n", touchversion);
			continue;
		}


		touch_btn = 0;
		if( TEST_BIT(ev_bitmask, EV_KEY) )
		{
			touch_btn = 1;
		}

		Log("Device %d (%s:%s) button:%d\n", i, TEST_DEVICE, name,touch_btn);

		// detect TOUCH device
		if( touchversion ==  FROYO_EVENT_PROTO && touch_btn == 0)
		{
			//Log("FROYO but no touch_btn\n");
			continue;
		}

		if( TEST_BIT(ev_bitmask, EV_SYN)  && TEST_BIT(ev_bitmask, EV_ABS)  )
		{
			unsigned long abs_bitmask[NBITS(ABS_MAX)];
			if( ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0 )
			{

				//Log("get EV_ABS failed\n");
				continue;
			}



			//if( TEST_BIT(abs_bitmask, ABS_X) && TEST_BIT(abs_bitmask, ABS_Y) )
			{
				//Log("has ABS_X, ABSY\n");
				if( !TEST_BIT(abs_bitmask, ABS_MT_TOUCH_MAJOR))
				{
					//Log("ABS_MT_TOUCH_MAJOR failed\n");
					continue;
				}

				if( !checkloose && !TEST_BIT(abs_bitmask, ABS_MT_WIDTH_MAJOR))
				{
					//Log("ABS_MT_WIDTH_MAJOR failed\n");
					continue;
				}

				//if( !checkloose && !TEST_BIT(abs_bitmask, ABS_MT_SLOT))
				//{
				//	Log("ABS_MT_SLOT failed\n");
				//	continue;
				//}
				Log("found(%d):%s(%s)\n", touchversion,TEST_DEVICE, name);
				strcpy( TOUCH_DEVICE, TEST_DEVICE );
				break;
			}
			//else
			//{
			//	Log("ABS_X ABS_Y failed\n");
			//}
		}
		else
		{
			//Log("EV_SYN  EV_ABS failed\n");
		}
		close(fd);
	}

	if( TOUCH_DEVICE[0] == 0)
	{
		if( !checkloose)
		{
			Err("try loose check for touchfd\n");
			return detect_touch_port(1);
		}
		return 0;
	}

	// open

    struct input_absinfo info;
	if((touchfd = open(TOUCH_DEVICE, O_RDWR)) == -1)
	{
			Err("cannot open touch device %s\n", TOUCH_DEVICE);
			return 0;
	}

	unsigned long bits[NBITS(KEY_MAX)];
	ioctl (touchfd, EVIOCGBIT (EV_ABS, KEY_MAX), bits);

  if (!(TEST_BIT ( bits, ABS_MT_POSITION_X) &&
		  TEST_BIT (bits, ABS_MT_POSITION_Y)))
	{
		// Get the Range of X and Y
		if(ioctl(touchfd, EVIOCGABS(ABS_X), &info)) {
			Err("cannot get ABS_X info, %s\n", strerror(errno));
			return 0;
		}

		xmin = info.minimum;
		xmax = info.maximum;

		if(ioctl(touchfd, EVIOCGABS(ABS_Y), &info)) {
			Err("cannot get ABS_Y, %s\n", strerror(errno));
			return 0;
		}
		ymin = info.minimum;
		ymax = info.maximum;
    }
	else
	{
		Log("take MT position\n");
		if(ioctl(touchfd, EVIOCGABS(ABS_MT_POSITION_X), &info)) {
			Err("cannot get ABS_X info, %s\n", strerror(errno));
			return 0;
		}

		xmin = info.minimum;
		xmax = info.maximum;

		if(ioctl(touchfd, EVIOCGABS(ABS_MT_POSITION_Y), &info)) {
			Err("cannot get ABS_Y, %s\n", strerror(errno));
			return 0;
		}
		ymin = info.minimum;
		ymax = info.maximum;
	}
    Log("xmin(%d) xmax(%d)\n", xmin, xmax);
    Log("ymin(%d) ymax(%d)\n",ymin,ymax);

    return 1;
}

void write_touch_event(int type, int code, int value)
{
	struct input_event event;
	memset(&event, 0, sizeof(event));
	event.type = type;
	event.code = code;
	event.value = value;
	gettimeofday(&event.time,0);
	//usleep(1000);

	int ret = write(touchfd, &event, sizeof(event));
	if(ret < (int)sizeof(event))
	{
		Err("write event failed\n");
		return;
	}
}


void injectTouchEvent(int mode, int x, int y)
{
	if( touchfd == -1 )
		return;

    if( xmax == 0)
    	xmax = g_fbinfo.width;
    if( ymax == 0 )
    	ymax = g_fbinfo.height;

    //Log("Inject Touch:(%d,%d), xmax(%d), ymax(%d), g_width(%d), g_height(%d)\n", x,y, xmax, ymax, g_fbinfo.width, g_fbinfo.height);
	x = xmin + (x * (xmax - xmin)) / (g_fbinfo.width);
	y = ymin + (y * (ymax - ymin)) / (g_fbinfo.height);

	//Log("final(%02x,%02x\n", x,y);
	static int tid = 0;
	if( mode == 1 ) // down
	{
	  if (touchversion == ICS_EVENT_PROTO)
	  {
		  write_touch_event(3,ABS_MT_TRACKING_ID,tid);
		  if( touch_btn )
			  write_touch_event(1,0x14a,1);	// EV_KEY BTN_TOUCH
		  /*
		  write_touch_event(3,ABS_MT_POSITION_X,x);
		  write_touch_event(3,ABS_MT_POSITION_Y,y);
		  write_touch_event(3,ABS_MT_TOUCH_MAJOR,12);	// diameter
		  write_touch_event(3,ABS_MT_TOUCH_MINOR,7);	// diameter
		  write_touch_event(3,ABS_MT_WIDTH_MAJOR,12);	// diameter
		  write_touch_event(3,ABS_MT_PRESSURE,90);
		  write_touch_event(0,0,0);
		  */
		struct input_event event[7];
		//memset(&event[0], 0, sizeof(event)*7);
		event[0].type = 3; event[0].code = ABS_MT_POSITION_X; 	event[0].value = x; 	gettimeofday(&event[0].time,0);
		event[1].type = 3; event[1].code = ABS_MT_POSITION_Y; 	event[1].value = y; 	event[1].time = event[0].time;
		event[2].type = 3; event[2].code = ABS_MT_TOUCH_MAJOR; 	event[2].value = 12; 	event[2].time = event[0].time;
		event[3].type = 3; event[3].code = ABS_MT_TOUCH_MINOR; 	event[3].value = 7; 	event[3].time = event[0].time;
		event[4].type = 3; event[4].code = ABS_MT_WIDTH_MAJOR; 	event[4].value = 12; 	event[4].time = event[0].time;
		event[5].type = 3; event[5].code = ABS_MT_PRESSURE; 	event[5].value = 90; 	event[5].time = event[0].time;
		event[6].type = 0; event[6].code = 0; 					event[6].value = 0; 	event[6].time = event[0].time;

		write(touchfd, &event, sizeof(event)*7);
	  }
	  else
	  {
		  write_touch_event(3,50,9);	// diameter
		  write_touch_event(3,53,x);
		  write_touch_event(3,54,y);
		  write_touch_event(0,2,0);
		  write_touch_event(0,0,0);
	  }
	}
	else if( mode == 2) // move
	{
	  if (touchversion == ICS_EVENT_PROTO)
	  {
	  		  /*
	  		  		  #define ABS_MT_TOUCH_MAJOR	0x30	(48)
	  		  		  #define ABS_MT_TOUCH_MINOR	0x31	(49)
	  		  		  #define ABS_MT_WIDTH_MAJOR	0x32	(50)
	  		  		  #define ABS_MT_POSITION_X	0x35	(53)
	  		  		  #define ABS_MT_POSITION_Y	0x36	(54)
	  		  		  #define ABS_MT_TRACKING_ID	0x39  	(57)
	  		  		  #define ABS_MT_PRESSURE		0x3a    (58)
	  		  		   */
	  		  		  /*
	  		  		  write_touch_event(3,ABS_MT_POSITION_X,x);	// pos x
	  		  		  write_touch_event(3,ABS_MT_POSITION_Y,y);	// pos y
	  		  		  write_touch_event(3,ABS_MT_TOUCH_MAJOR,7); 	// touch major
	  		  		  write_touch_event(3,ABS_MT_WIDTH_MAJOR,7);	// diameter
	  		  		  write_touch_event(3,ABS_MT_PRESSURE,90);	// ABS_MT_PRESSURE 0x3a
	  		  		  write_touch_event(0,0,0);
	  		  		  */
	  		  			struct input_event event[6];
	  		  			//memset(&event[0], 0, sizeof(event)*6);
	  		  			event[0].type = 3; event[0].code = ABS_MT_POSITION_X; 	event[0].value = x; 	gettimeofday(&event[0].time,0);
	  		  			event[1].type = 3; event[1].code = ABS_MT_POSITION_Y; 	event[1].value = y; 	event[1].time = event[0].time;
	  		  			event[2].type = 3; event[2].code = ABS_MT_TOUCH_MAJOR; 	event[2].value = 7; 	event[2].time = event[0].time;
	  		  			event[3].type = 3; event[3].code = ABS_MT_WIDTH_MAJOR; 	event[3].value = 7; 	event[3].time = event[0].time;
	  		  			event[4].type = 3; event[4].code = ABS_MT_PRESSURE; 	event[4].value = 90; 	event[4].time = event[0].time;
	  		  			event[5].type = 0; event[5].code = 0; 					event[5].value = 0; 	event[5].time = event[0].time;

	  		  			write(touchfd, &event, sizeof(event)*6);
	  	  }
	  else
	  {
		  write_touch_event(3,50,15);	// diameter
		  write_touch_event(3,53,x);
		  write_touch_event(3,54,y);
		  write_touch_event(0,2,0);
		  write_touch_event(0,0,0);
	  }
	}
	else // up
	{
	  if (touchversion == ICS_EVENT_PROTO)
	  {
		  write_touch_event(3,57,-1);
		  tid++;
		  if(touch_btn)
			  write_touch_event(1,0x14a,0);	// EV_KEY BTN_TOUCH
		  write_touch_event(0,0,0);
	  }
	  else
	  {
		  write_touch_event(0,2,0);
		  write_touch_event(0,0,0);
	  }
	}

}

void onTouch(svcInfoPtr psvc, PFBCCMD msg)
{
	UINT mode;
	memcpy(&mode, msg->data, sizeof(UINT));
	mode = ntohl(mode);

	USHORT x,y;

	memcpy(&x, msg->data+4, sizeof(USHORT));
	x = ntohs(x);

	memcpy(&y, msg->data+6, sizeof(USHORT));
	y = ntohs(y);

	//Log("onTouch:%d (%d-%d)\n", mode, x, y);
	injectTouchEvent((int)mode, (int)x, (int)y);
}

int init_framebuffer()
{
    struct fb_var_screeninfo vinfo;
    int fb = -1;

    struct fbinfo fbinfo;
    unsigned i, bytespp;
    //int w,h,f;
    //void const* base = 0;
    //size_t size = 0;

    int iversion = get_android_version();
    int busefb = 1;
    if( !g_forcefbmode && iversion >= 10 /* ICE_CREAM_SANDWICH */) {

		uint32_t w=0, h=0;
		PixelFormat fmt=0;
		int chk = 0;

        ScreenshotClient sc;
        sc.update();
        w = sc.getWidth();
        h = sc.getHeight();
        fmt = sc.getFormat();
        sc.release();

/*
		sp<IMemoryHeap> heap;

		sp<ISurfaceComposer> sf(ComposerService::getComposerService());

		sf->captureScreen(0, &heap, &w, &h, &fmt, 0, 0, 0,     0x7fffffff);
*/
        Log("fmt(%d), w(%d), h(%d)\n", fmt, w, h);
		if( set_fbinfo(fmt,w,h, &fbinfo) != 0 )
			busefb = 0;
    }

    if( busefb )
    { // not support screencap
            fb = open("/dev/graphics/fb0", O_RDONLY);
            if(fb < 0) {
                Err("Error:fb open error.\n");
                return 0;
            }

            if(ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0) return 0;
            fcntl(fb, F_SETFD, FD_CLOEXEC);

            bytespp = vinfo.bits_per_pixel / 8;

            fbinfo.version = BEFORE_ICS;
            fbinfo.bpp = vinfo.bits_per_pixel;
            fbinfo.size = vinfo.xres * vinfo.yres * bytespp;
            fbinfo.width = vinfo.xres;
            fbinfo.height = vinfo.yres;
            fbinfo.red_offset = vinfo.red.offset;
            if( vinfo.red.msb_right !=0)
            {
            	Log("red MSB is on right\n");
            	fbinfo.red_offset = vinfo.bits_per_pixel - vinfo.red.offset - vinfo.red.length;
            }
            fbinfo.red_length = vinfo.red.length;
            fbinfo.red_mask = getMask(vinfo.red.length);

            fbinfo.green_offset = vinfo.green.offset;
            if( vinfo.green.msb_right != 0)
            {
            	Log("green MSB is on right\n");
            	fbinfo.green_offset = vinfo.bits_per_pixel - vinfo.green.offset - vinfo.green.length;
            }
            fbinfo.green_length = vinfo.green.length;
            fbinfo.green_mask = getMask(vinfo.green.length);

            fbinfo.blue_offset = vinfo.blue.offset;
            if( vinfo.blue.msb_right != 0)
            {
            	Log("blue MSB is on right\n");
            	fbinfo.blue_offset = vinfo.bits_per_pixel - vinfo.blue.offset - vinfo.blue.length;
            }
            fbinfo.blue_length = vinfo.blue.length;
            fbinfo.blue_mask = getMask(vinfo.blue.length);
            fbinfo.alpha_offset = vinfo.transp.offset;
            fbinfo.alpha_length = vinfo.transp.length;
            fbinfo.alpha_mask = getMask(vinfo.transp.length);

            close(fb);
    }

    fbinfo.orientation = 0; //get_dev_rotation();
    memcpy(&g_fbinfo, &fbinfo, sizeof(struct fbinfo));

    g_fbbuffer = (unsigned char*)malloc( fbinfo.width * fbinfo.height * 32/8);
    g_xsize = fbinfo.width * fbinfo.bpp / 8;
    g_x = (unsigned char*)malloc( fbinfo.width * 4);
    jpegBufSize = fbinfo.width * fbinfo.height * 32/8 * 2 / 3;
    fbc_output[0].jpegBuf = (char*)malloc( jpegBufSize );
#ifdef USE_XOR_MODE
    fbc_output[1].jpegBuf = (char*)malloc( jpegBufSize );
    memset(fbc_output[1].jpegBuf , 0, jpegBufSize);
#endif
    fbc_output_current = 0;

    int isBigEndian = is_big_endian();
    Log("xres=%d, yres=%d, bpp=%d red(off:%d,len:%d), green(%d,%d), blue(%d,%d)\n\
isBigEndian(%d), half(%d), forcefb(%d)\n",
      (int)fbinfo.width, (int)fbinfo.height,
      (int)fbinfo.bpp,
      (int) fbinfo.red_offset, (int)fbinfo.red_length,
      (int) fbinfo.green_offset, (int)fbinfo.green_length,
      (int) fbinfo.blue_offset, (int)fbinfo.blue_length,
      isBigEndian, g_halfmode, g_forcefbmode
      );
    if( fbinfo.version == ICS_SCREENCAP)
    	Log("Capturing Method : ICS_SCREENCAP\n");
    else
    	Log("Capturing Method : FB_ACCESS\n");

    int tresult = detect_touch_port(0);

    Log("Detect touch port : %d\n", tresult);

    return 1;
}

int read_rgb_framebuffer_to_jpeg(svcInfoPtr psvc)
{
    if( g_fbinfo.version == ICS_SCREENCAP )
        return read_rgb_framebuffer_screencap_to_jpeg(psvc);
    else
        return read_rgb_framebuffer_fb0_to_jpeg(psvc);

}

int read_rgb_framebuffer_screencap_to_jpeg(svcInfoPtr psvc)
{
#ifdef PERFORMANCE_REPORT
	struct timeval tS, tE;
	gettimeofday(&tS,NULL);
#endif
    void const* mapbase = 0;
    size_t size = 0;
    int nRtn;

    /*
    if (pscreenshot->update() == NO_ERROR) {
    	mapbase = pscreenshot->getPixels();
        //w = pscreenshot->getWidth();
        //h = pscreenshot->getHeight();
        //f = pscreenshot->getFormat();
        size = pscreenshot->getSize();
    }
    else {
    	Err("Error:ScreenshotClient init failed.\n");
    	return 0;
    }*/

    unsigned int sw, sh, xsize, gsize;
    sw = g_fbinfo.width;
    sh = g_fbinfo.height;
    xsize = g_xsize;
    gsize = g_fbinfo.size;
    if( g_halfmode) {
    	sw /= 2;
    	sh /= 2;
    	xsize /= 2;
    	gsize /= 4;
    }
        uint32_t w=0, h=0;
        PixelFormat fmt=0;

        //Log("w(%d) h(%d) sw(%d) sh(%d) xsize(%d) gsize(%d)\n", w, h, sw, sh, xsize, gsize);

        ScreenshotClient sc;
        sc.update(sw,sh);
        /*
        sp<IMemoryHeap> heap;
        sp<ISurfaceComposer> sf(ComposerService::getComposerService());
        sf->captureScreen(0, &heap, &w, &h, &fmt, sw, sh, 0,     0x7fffffff);
        mapbase = heap->getBase();
        */

        mapbase = sc.getPixels();

        //Log("w(%d) h(%d) sw(%d) sh(%d) xsize(%d) gsize(%d)\n", w, h, sw, sh, xsize, gsize);
    if( mapbase != NULL) {
        unsigned char* base = (unsigned char*)((char const *)mapbase);
        unsigned char* p = g_fbbuffer;
        unsigned int i,j;

#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_capture += ELAPSED(tS, tE);
	tS = tE;
#endif
                init_jpeg(psvc->jpegquality);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_compress += ELAPSED(tS, tE);
	tS = tE;
#endif
                for(i=0; i< gsize; i += xsize) {
#ifdef USE_XOR_MODE
                	for(unsigned int t=0; t< xsize; t++)
                	{
                		*(g_x + t) = (*(fbc_output[1].jpegBuf + i + t) ^ *(base + t));
                	}
#else
				    memcpy(g_x, base, xsize);
#endif

#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_capture += ELAPSED(tS, tE);
	tS = tE;
#endif
              	  	nRtn = convert_to_rgb(g_x, xsize, p);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_colorspace += ELAPSED(tS, tE);
	tS = tE;
#endif
                    rowPointer[0] = p;
                    jpeg_write_scanlines(&cinfo, rowPointer, 1);
                    p += nRtn;
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_compress += ELAPSED(tS, tE);
	tS = tE;
#endif
                    base += xsize;

                }

                if( gsize % xsize > 0 ) {
                	memcpy(g_x, base, gsize % xsize );

#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_capture += ELAPSED(tS, tE);
	tS = tE;
#endif
                    nRtn = convert_to_rgb(g_x, xsize, p);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_colorspace += ELAPSED(tS, tE);
	tS = tE;
#endif
					rowPointer[0] = p;
                    jpeg_write_scanlines(&cinfo, rowPointer, 1);
                    p += nRtn; 
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_compress += ELAPSED(tS, tE);
	tS = tE;
#endif
                }
#ifdef USE_XOR_MODE
            	memcpy(fbc_output[1].jpegBuf , (unsigned char*)((char const *)mapbase), gsize);
#endif

                int nSize = exit_jpeg();

                // release sc
                sc.release();

                //Log("jpeg size:%d - ", nSize);
                if( nSize > 0 ) {
                	//FBCCMD res;

                	fbc_output[fbc_output_current].cmd = g_halfmode ? 'h' : 'u';
                	fbc_output[fbc_output_current].size = htonl(FBCCMD_HEADER + nSize);

                	LOCK(psvc->output_mutex);
                	int r1,r2;
                	r1 = WriteExact(psvc, (const char*)&fbc_output[fbc_output_current], FBCCMD_HEADER);
                	// actual data
                	//char * test = (char*)&fbc_output[fbc_output_current];
                	r2 = WriteExact(psvc, fbc_output[fbc_output_current].jpegBuf, nSize);
                	//Log("W1:%d W2:%d (%02x %02x %02x %02x %02x %02x %02x %02x\n", r1, r2,
                	//		test[0], test[1], test[2], test[3], test[4], test[5], test[6], test[7] );
                	UNLOCK(psvc->output_mutex);
                }

#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_tx += ELAPSED(tS, tE);
	tS = tE;
#endif
                return nSize;

            }
            else {
                Err("map failed!!!\n");
            }
    return 0;
}

int read_rgb_framebuffer_fb0_to_jpeg(svcInfoPtr psvc)
{
    const char* fbpath = "/dev/graphics/fb0";

#ifdef PERFORMANCE_REPORT
	struct timeval tS, tE;
	gettimeofday(&tS,NULL);
#endif

    int fb = open(fbpath, O_RDONLY);
    if (fb >= 0) {
        struct fb_var_screeninfo vinfo;
        if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == 0) {
            fcntl(fb, F_SETFD, FD_CLOEXEC);
            size_t offset = (vinfo.xoffset + vinfo.yoffset*vinfo.xres) * g_fbinfo.bpp / 8;
            size_t size = vinfo.xres*vinfo.yres*(vinfo.bits_per_pixel/8);
            ssize_t mapsize = offset + size;
            //Log("mapbase:size(%d) offset(%d) xoffset(%d) yoffset(%d) xres(%d) yres(%d) bpp(%d)\n", size, offset,
            //		vinfo.xoffset, vinfo.yoffset, vinfo.xres, vinfo.yres, g_fbinfo.bpp);
            //void const* mapbase = mmap(0, mapsize, PROT_READ, MAP_PRIVATE, fb, 0);
            void const* mapbase = mmap(0, mapsize, PROT_READ, MAP_SHARED, fb, 0);
            close(fb);
            int nRtn;
            if (mapbase != MAP_FAILED) {
                unsigned char* base = (unsigned char*)((char const *)mapbase + offset);
                unsigned char* p = g_fbbuffer;
                unsigned int i,j;
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_capture += ELAPSED(tS, tE);
	tS = tE;
#endif
                init_jpeg(psvc->jpegquality);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_compress += ELAPSED(tS, tE);
	tS = tE;
#endif


                for(i=0; i< g_fbinfo.size; i += g_xsize) {

				    memcpy(g_x, base, g_xsize);

            		if( g_halfmode ) {
            			int bpp = g_fbinfo.bpp / 8;
            			for(j =0; j< g_xsize/2;) {
            				memcpy(g_x+j, g_x+j*2, bpp);
            				j+=bpp;
            			}
            		}
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_capture += ELAPSED(tS, tE);
	tS = tE;
#endif
              	  	nRtn = convert_to_rgb(g_x, g_halfmode ? g_xsize/2 : g_xsize, p);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_colorspace += ELAPSED(tS, tE);
	tS = tE;
#endif
                    rowPointer[0] = p;
                    jpeg_write_scanlines(&cinfo, rowPointer, 1);
                    p += nRtn;
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_compress += ELAPSED(tS, tE);
	tS = tE;
#endif
                    base += g_xsize;
                    if( g_halfmode) {
                        base += g_xsize;
                        i += g_xsize;
                    }
                }

                if( g_fbinfo.size % g_xsize > 0 ) {
                	memcpy(g_x, base, g_fbinfo.size % g_xsize );
            		if( g_halfmode ) {
            			int bpp = g_fbinfo.bpp / 8;
            			for(j =0; j< g_fbinfo.size % g_xsize/2;) {
            				memcpy(g_x+j, g_x+j*2, bpp);
            				j+=bpp;
            			}
            		}
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_capture += ELAPSED(tS, tE);
	tS = tE;
#endif
                    nRtn = convert_to_rgb(g_x, g_halfmode ?  g_fbinfo.size % g_xsize/2 : g_fbinfo.size % g_xsize, p);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_colorspace += ELAPSED(tS, tE);
	tS = tE;
#endif
					rowPointer[0] = p;
                    jpeg_write_scanlines(&cinfo, rowPointer, 1);
                    p += nRtn;
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_compress += ELAPSED(tS, tE);
	tS = tE;
#endif
                }
                munmap((void*)mapbase,mapsize);

                int nSize = exit_jpeg();
                if( nSize > 0 ) {

                	//FBCCMD res;
                	fbc_output[fbc_output_current].cmd = g_halfmode ? 'h' : 'u';
                	fbc_output[fbc_output_current].size = htonl(FBCCMD_HEADER + nSize);

                	LOCK(psvc->output_mutex);
                	WriteExact(psvc, (const char*)&fbc_output[fbc_output_current], FBCCMD_HEADER);

                	// actual data
                	WriteExact(psvc, fbc_output[fbc_output_current].jpegBuf, nSize);
                	UNLOCK(psvc->output_mutex);

                	//Log("jpeg:%dbytes\n" ,nSize);
                }

#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_tx += ELAPSED(tS, tE);
	tS = tE;
#endif
                return nSize;

            }
            else {
                Err("map failed!!!\n");
            }
        }
        else {
            Err("FBIOGET_VSCREENINFO failed!!!\n");
        }
        close(fb);
    }
    else {
        Err("fb0 open failed!!!\n");
    }

    return 0;
}


int convert_to_rgb(unsigned char * x, int xsize, unsigned char * target)
{
    int value;
    int bpp = g_fbinfo.bpp;
    unsigned char *p;
    int j;

    p = target;
    for(j=0; j< xsize; ) {
        if (bpp == 16) {
            value = x[j++] & 0x00FF;
            value |= (x[j++] << 8) & 0x0FF00;
            *p++ = ((value >> g_fbinfo.red_offset) & g_fbinfo.red_mask) << (8 - g_fbinfo.red_length);
            *p++ = ((value >> g_fbinfo.green_offset) & g_fbinfo.green_mask) << (8 - g_fbinfo.green_length);
            *p++ = ((value >> g_fbinfo.blue_offset) & g_fbinfo.blue_mask) << (8 - g_fbinfo.blue_length);
        } else if (bpp == 24) {
        	/*
            value = x[j++] & 0x000000FF;
            value |= (x[j++] & 0x000000FF) << 8;
            value |= (x[j++] & 0x000000FF) << 16;
            */
        	*p++ = x[j+g_fbinfo.red_offset/8];
        	*p++ = x[j+g_fbinfo.green_offset/8];
        	*p++ = x[j+g_fbinfo.blue_offset/8];
        	j += 3;
        } else if (bpp == 32) {
        	/*
            value  =  (x[j++]    )	& 0x000000FF;
            value |= (x[j++] << 8)	& 0x0000FF00;
            value |= (x[j++] <<16)	& 0x00FF0000;
            value |= (x[j++] <<24)	& 0xFF000000;
        	*/
        	/*
        	*p++ = x[j+0];
        	*p++ = x[j+1];
        	*p++ = x[j+2];
        	*/
        	*p++ = x[j+g_fbinfo.red_offset/8];
        	*p++ = x[j+g_fbinfo.green_offset/8];
        	*p++ = x[j+g_fbinfo.blue_offset/8];
        	j += 4;
        }
        else {
            Err("Only support 16 / 24 / 32 bit color depth\n");
            return 0;
        }

    }

    return (p - target);
}
//--------------------------- JPEG
static void
JpegInitDestination(j_compress_ptr cinfo)
{
    jpegError = FALSE;
    jpegDstManager.next_output_byte = (JOCTET *)fbc_output[fbc_output_current].jpegBuf;
    jpegDstManager.free_in_buffer = (size_t)jpegBufSize;
}

static boolean
JpegEmptyOutputBuffer(j_compress_ptr cinfo)
{
    jpegError = TRUE;
    jpegDstManager.next_output_byte = (JOCTET *)fbc_output[fbc_output_current].jpegBuf;
    jpegDstManager.free_in_buffer = (size_t)jpegBufSize;

    return TRUE;
}

static void
JpegTermDestination(j_compress_ptr cinfo)
{
    jpegDstDataLen = jpegBufSize - jpegDstManager.free_in_buffer;
}

static void
JpegSetDstManager(j_compress_ptr cinfo)
{
    jpegDstManager.init_destination = JpegInitDestination;
    jpegDstManager.empty_output_buffer = JpegEmptyOutputBuffer;
    jpegDstManager.term_destination = JpegTermDestination;
    cinfo->dest = &jpegDstManager;
}

void init_jpeg(int quality)
{
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    if( g_halfmode ) {
		cinfo.image_width = g_fbinfo.width / 2;
		cinfo.image_height = g_fbinfo.height / 2;
    } else {
		cinfo.image_width = g_fbinfo.width;
		cinfo.image_height = g_fbinfo.height;
    }
    cinfo.input_components = 3;

    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    //cinfo.desired_number_of_colors = 256;
    cinfo.dct_method = JDCT_FASTEST;
    /*
    if( g_halfmode)
    {
    	cinfo.scale_num = 2;
    }
    */
    //cinfo.scale_num = 3;
    //cinfo.scale_denom = 5;
    jpeg_set_quality(&cinfo, quality, TRUE);

    JpegSetDstManager(&cinfo);

    jpeg_start_compress(&cinfo, TRUE);
}

int exit_jpeg()
{
    if( !jpegError )
        jpeg_finish_compress(&cinfo);
    else {
    	Log("exit_jpeg : error len(%d)\n", jpegDstDataLen);
    }
    jpeg_destroy_compress(&cinfo);

    return jpegDstDataLen;
}

int convert_to_jpeg(int quality)
{

    init_jpeg(quality);


    int row_stride = cinfo.image_width * 3;

    while(cinfo.next_scanline < cinfo.image_height) {

        rowPointer[0] = &g_fbbuffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, rowPointer, 1);
    }

    return exit_jpeg();

}


#ifdef CAMEL_REMOVED
int main(int argc, char * argv[])
{
    int nSize;
    FILE* outfile;
	g_forcefbmode = false;
	if( argc > 1 && strcmp(argv[1], "fb")==0)
		g_forcefbmode = true;

    //--- fb initialize
    if( init_framebuffer() == 0 ) {
        Err("init framebuffer failed\n");
        return 0;
    }
    Log("Framebuffer initialized.\n");

    char fname[256];
    int i;
    struct timeval tS, tE;
    double utime, mtime, seconds, useconds, average;
    int ntest = 10;
    int jpegquality = 40;
    svcInfo svc;
    svc.sock = -1;
    svc.port=6900;
    svc.socketState=SOCKET_INIT;
    svc.host[0] = 0;
    svc.jpegquality = 40;


    gettimeofday(&tS, NULL);

    for(i=0; i<ntest;i++) {
    //--- rgb framebuffer capture
#ifndef CAMEL_TEST
//    	read_rgb_framebuffer();


     	int fd;
        sprintf(fname,"/data/local/tmp/fb%02d.jpg",i);
        fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        svc.sock = fd;
        nSize = read_rgb_framebuffer_to_jpeg(&svc);
        close(fd);
        Log("%d:read_rgb_framebuffer_to_jpeg(%d)\n", i, nSize);

        /*
    nSize = read_rgb_framebuffer();
    Log("rgb framebuffer captured(%d)\n", nSize);
    */
    /*
    sprintf(fname,"fb%02d.raw",i);
    outfile = fopen(fname, "wb");
    fwrite(g_fbbuffer, 1, nSize, outfile);
    fclose(outfile);
    Log("rgb framebuffer captured(%d:%s)\n", nSize, "fb.rgb");
	*/
/*
    //--- jpeg compression
    nSize = convert_to_jpeg(jpegquality);
    sprintf(fname,"fb%02d.jpg",i);
    outfile = fopen(fname, "wb");
    fwrite(jpegBuf, 1, jpegDstDataLen, outfile);
    fclose(outfile);
    Log("%d:jpeg converted(%d)\n", i+1, jpegDstDataLen);
*/
#else
    
    sprintf(fname,"fb%02d.jpg",i);
    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    int nSize = read_rgb_framebuffer_to_jpeg(fd, jpegquality);
    close(fd);
    Log("%d:jpeg converted(%d)\n", i+1, nSize);
#endif /* CAEML_TEST */

    usleep(1);
    }

    gettimeofday(&tE, NULL);
    seconds = tE.tv_sec - tS.tv_sec;
    useconds= tE.tv_usec- tS.tv_usec;

    utime = seconds * 1000000 + useconds;
    average = (double)ntest * 1000000 / utime ;
    Log("Elapsed:%.2f, Average:%.2f\n", utime/1000000, average);


    //--- exit
    if( exit_framebuffer() == 0 ) {
        Err("exit framebuffer failed\n");
        return 0;
    }

    Log("Framebuffer Exit.\n");

    return 1;

}
#endif /* CAMEL_REMOVED */

