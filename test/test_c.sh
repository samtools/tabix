bin/pairix -f -s2 -d6 -b3 -e3 -u7 samples/merged_nodup.tab.chrblock_sorted.txt.gz
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20:50000000-60000000'
bin/pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '1:1-10000000|20:50000000-60000000' '3:5000000-9000000|X:70000000-90000000'
bin/pairix -s1 -b2 -e2 -f samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
bin/pairix samples/SRR1171591.variants.snp.vqsr.p.vcf.gz chr10:1-4000000

