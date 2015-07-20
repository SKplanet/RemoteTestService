#!/bin/bash
cd ~/WORKING_GINGER_2.3.3_r1
source build/envsetup.sh
cd ~/WORKING_GINGER_2.3.3_r1/frameworks/base/cmds/ruicapsvc
touch *.cpp
export TARGET_AML=1
mm
cp ~/WORKING_GINGER_2.3.3_r1/out/target/product/generic/system/bin/amlscm /mnt/hgfs/camel/work/skrc/trunk/TestAgent/TA/ruicmd/amlscm
touch *.cpp
export TARGET_AML=0
mm
cp ~/WORKING_GINGER_2.3.3_r1/out/target/product/generic/system/bin/ruicapsvc /mnt/hgfs/camel/work/skrc/trunk/TestAgent/TA/ruicmd/ruicapsvc

