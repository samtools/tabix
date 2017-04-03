#!/bin/bash
# usage: process_old_merged_nodup.sh merged_nodups.txt (older version with readID as the first column)

input=$1
sort -t' ' -k3,3 -k7,7 -k4,4n -k8,8n $input | bgzip -cf > $input.bsorted.gz
pairix -f -p old_merged_nodups $input.bsorted.gz
