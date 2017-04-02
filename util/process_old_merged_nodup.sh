#!/bin/bash
# usage: process_old_merged_nodup.sh merged_nodups.txt (older version with readID as the first column)

input=$1
cp /home/sw229/Storage/HiC_SV/data/HiC_data/GM12878/pairFile/GM12878_insitu_DpnII_merged_nodups.tab.chrblock_sorted.txt $PAIRS/
sort -k3,3 -k7,7 -k4,4 -k8,8 $input > sort -k3,3 -k7,7 -k4,4 -k8,8 $input.bsorted
bgzip -f $input.bsorted
pairix -f -s3 -d7 -b4 -e4 -u8 -T $input.bsorted.gz
