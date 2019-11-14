#!/bin/bash
f=$1  # input unsorted pairs file (gzipped)
outprefix=$2

mkfifo pp

# header
gunzip -c $f | grep "^#" > pp
    
# sorting 
gunzip -c $f | grep -v '^#' | sort -k2,2 -k4,4 -k3,3g -k5,5g >> pp
    
# compressing
bgzip -c pp > $outprefix.pairs.gz
    
# indexing
pairix -f $outprefix.pairs.gz

# cleanup
rm pp
