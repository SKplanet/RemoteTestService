#!/bin/bash
cd ~/WORKING_JB.4.2.2
source build/envsetup.sh
cd ~/WORKING_JB.4.2.2/frameworks/base/cmds/ruicapsvcj2
touch *.cpp
export TARGET_AML=1
mm
cp ~/WORKING_JB.4.2.2/out/target/product/generic/system/bin/amlscmj2 /mnt/hgfs/camel/work/skrc/trunk/TestAgent/TA/ruicmd/amlscmj2
touch *.cpp
export TARGET_AML=0
mm
cp ~/WORKING_JB.4.2.2/out/target/product/generic/system/bin/ruicapsvcj2 /mnt/hgfs/camel/work/skrc/trunk/TestAgent/TA/ruicmd/ruicapsvcj2

