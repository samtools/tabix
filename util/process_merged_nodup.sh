#!/bin/bash
# usage: process_merged_nodup.sh merged_nodups.txt (newer version with strand1 as the first column)

input=$1
cp /home/sw229/Storage/HiC_SV/data/HiC_data/GM12878/pairFile/GM12878_insitu_DpnII_merged_nodups.tab.chrblock_sorted.txt $PAIRS/
sort -k2,2 -k6,6 -k3,3 -k7,7 $input > sort -k2,2 -k6,6 -k3,3 -k7,7 $input.bsorted
bgzip -f $input.bsorted
pairix -f -s2 -d6 -b3 -e3 -u7 -T $input.bsorted.gz
