#!/bin/bash
cd ~/WORKING_GINGER_2.3.3_r1
source build/envsetup.sh
cd ~/WORKING_GINGER_2.3.3_r1/frameworks/base/cmds/ruicapsvc
mm
cp ~/WORKING_GINGER_2.3.3_r1/out/target/product/generic/system/bin/ruicapsvc /mnt/hgfs/camel/rui/
