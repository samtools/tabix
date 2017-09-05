#!/bin/bash
ls -1 samples/*pairs.gz |xargs -I{} pairix -f {}
pairix -f samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
pairix -f -p merged_nodups samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz
pairix -f -p merged_nodups samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz
pairix -f -p merged_nodups samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz
pairix -f -p merged_nodups samples/test_merged_nodups.txt.bsorted.gz
pairix -f -p old_merged_nodups samples/test_old_merged_nodups.txt.bsorted.gz
pairix -f -s2 -d6 -b3 -e3 -u7 samples/merged_nodup.tab.chrblock_sorted.txt.gz

