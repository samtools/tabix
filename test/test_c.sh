## 2D
bin/pairix -f -s2 -d6 -b3 -e3 -u7 samples/merged_nodup.tab.chrblock_sorted.txt.gz
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20:50000000-60000000'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '1:1-10000000|20:50000000-60000000' '3:5000000-9000000|X:70000000-90000000'

## 1D
bin/pairix -s1 -b2 -e2 -f samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
bin/pairix samples/SRR1171591.variants.snp.vqsr.p.vcf.gz chr10:1-4000000

## 2D, space-delimited
bin/pairix -f -s2 -d6 -b3 -e3 -u7 -T samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz
bin/pairix samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz '10:1-1000000|20'

## pairs_merger
bin/pairs_merger samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz | bin/bgzip -c > out.gz
bin/pairix -f -s2 -d6 -b3 -e3 -u7 -T out.gz
# compare with the approach of concatenating and resorting.
bin/merge-pairs . out2 samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz
gunzip -f out2.bsorted.pairs.gz 
gunzip -f out.gz
if [ -z $(diff out out2.bsorted.pairs)]; then echo "pairs_merger successful!"; fi
rm -f out out2.bsorted.pairs out2.pairs
