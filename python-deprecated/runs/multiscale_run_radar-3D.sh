#!/bin/bash

if [ "X$1" = "X" ]; then
    echo "multiscale_run_oase-3D.sh <path to directory containing composite files> [resume]"
    echo "Runs the clustering for a whole set of scales"
    exit 0
fi

if [ "X${MEANIE3D_HOME}" = "X" ]
then
    echo "Please set environment variable MEANIE3D_HOME"
    exit 0
fi

resume="NO"
if [ "$2" = "resume"]; then
    resume="YES"
fi

scales=( 10 25 50 100 250 )
echo "Running complete sets on scales ${scales[@]}"

pids=()

#
# Iterate over scales
#
for scale in ${scales[@]}; do

    dest="scale${scale}"

    if [ ! -d $dest ]; then
        echo "Creating directory ${dest}"
        mkdir ${dest}
    else
        echo "Directory exists. Removing content"
        rm -rf ${dest}/*
    fi

    cd ${dest}
    ${MEANIE3D_HOME}/visit/tracking/run_radar_tracking-3D.sh "$1" "$scale" "$resume" &

    pid=$!
    pids=( "${pids[@]}" "${pid}" )
    cd ..

done

# Poll until all jobs are finished


num_finished=0
num_processes=${#pids[@]}
echo "Polling for pids ${pids[@]} to finish ($num_processes processes)"

while [ ! $num_processes = $num_finished ]; do
    finished_pids=()
    for pid in $pids; do
        if [ "X`ps | awk '{print $1}' | grep ${pid}`" = "X" ]; then
            echo "Process $pid finished."
            finished_pids=( "${finished_pids[@]}" "${pid}" )
        fi
    done
    num_finished=${#finished_pids[@]}
    sleep 1
done

#
# Collate stats
#
${MEANIE3D_HOME}/visit/statistics/trackstats.sh
${MEANIE3D_HOME}/visit/statistics/plot_scale_comparison.sh
