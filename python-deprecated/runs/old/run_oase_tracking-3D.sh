#!/bin/bash

if [ "X$1" = "X" ]; then
    echo "run_oase_tracking-3D.sh <path to directory containing composite files> <scale> [resume]"
    echo "Creates a python script for cluster creation and tracking and runs it in Visit"
    exit 0
fi

if [ "X$2" = "X" ]; then
    echo "run_oase_tracking-3D.sh <path to directory containing composite files> <scale> [resume]"
    echo "Creates a python script for cluster creation and tracking and runs it in Visit"
    exit 0
fi

if [ "X${VISIT_EXECUTABLE}" = "X" ]; then
    echo "Please set environment variable VISIT_EXECUTABLE"
    exit 0
fi

if [ "X${MEANIE3D_HOME}" = "X" ]; then
    echo "Please set environment variable MEANIE3D_HOME"
    exit 0
fi

resume="NO"
if [ "$3" = "resume" ]; then
    resume="YES"
fi

SCRIPTFILE="/tmp/tracking-$RANDOM.py"
ESCAPED_SOURCE_DIR=$(echo $1 | sed -e "s/\//\\\\\//g")
ESCAPED_MEANIE3D_HOME=$(echo $MEANIE3D_HOME | sed -e "s/\//\\\\\//g")

cat $MEANIE3D_HOME/visit/tracking/run_oase_tracking-3D.py | sed -e "s/P_SOURCE_DIR/$ESCAPED_SOURCE_DIR/g" | sed -e "s/P_M3D_HOME/$ESCAPED_MEANIE3D_HOME/g" | sed -e "s/P_SCALE/$2/g" | sed -e "s/P_RESUME/$resume/g" > $SCRIPTFILE

python ${SCRIPTFILE}
