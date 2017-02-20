## 2D
bin/pairix -f -s2 -d6 -b3 -e3 -u7 samples/merged_nodup.tab.chrblock_sorted.txt.gz
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20:50000000-60000000'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '1:1-10000000|20:50000000-60000000' '3:5000000-9000000|X:70000000-90000000'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '*|1:0-100000'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '1:0-100000|*'

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
chmod +x test/inefficient-merger-for-testing
test/inefficient-merger-for-testing . out2 samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz
gunzip -f out2.bsorted.pairs.gz 
gunzip -f out.gz
if [ -z $(diff out out2.bsorted.pairs)]; then echo "pairs_merger successful!"; fi
rm -f out out2.bsorted.pairs out2.pairs out.gz.px2 out2.bsorted.pairs.gz.px2


## streamer_1d
bin/streamer_1d samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz | bin/bgzip -c > out.1d.pairs.gz
gunzip -c samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz | sort -t' ' -k2,2 -k3,3g | bin/bgzip -c > out2.1d.pairs.gz
gunzip -f out.1d.pairs.gz
gunzip -f out2.1d.pairs.gz
if [ -z $(diff out.1d.pairs out2.1d.pairs)]; then echo "streamer_1d successful!"; fi 
rm -f out.1d.pairs out2.1d.pairs
