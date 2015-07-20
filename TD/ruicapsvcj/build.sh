#!/bin/bash
cd ~/WORKING_JELLYBEAN
source build/envsetup.sh
cd ~/WORKING_JELLYBEAN/frameworks/base/cmds/ruicapsvcj
touch *.cpp
export TARGET_AML=1
mm
cp ~/WORKING_JELLYBEAN/out/target/product/generic/system/bin/amlscmj /mnt/hgfs/camel/work/skrc/trunk/TestAgent/TA/ruicmd/amlscmj1
touch *.cpp
export TARGET_AML=0
mm
cp ~/WORKING_JELLYBEAN/out/target/product/generic/system/bin/ruicapsvcj /mnt/hgfs/camel/work/skrc/trunk/TestAgent/TA/ruicmd/ruicapsvcj1

