#!/bin/bash
# usage: process_merged_nodup.sh merged_nodups.txt(newer version with strand1 as the first column)

input=$1
sort -t' ' -k2,2 -k6,6 -k3,3n -k7,7n $input | bgzip -c > $input.bsorted.gz
pairix -f -p merged_nodups $input.bsorted.gz
