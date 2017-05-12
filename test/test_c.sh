PATH=./bin:$PATH

## 2D
pairix -f -s2 -d6 -b3 -e3 -u7 samples/merged_nodup.tab.chrblock_sorted.txt.gz
pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20' > log1
gunzip -c samples/merged_nodup.tab.chrblock_sorted.txt.gz | awk '$2=="10" && $3>=1 && $3<=1000000 && $6=="20"' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 1 failed"
  return 1;
fi

pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20:50000000-60000000' > log1
gunzip -c samples/merged_nodup.tab.chrblock_sorted.txt.gz | awk '$2=="10" && $3>=1 && $3<=1000000 && $6=="20" && $7>=50000000 && $7<=60000000' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 2 failed"
  return 1;
fi

pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '1:1-10000000|20:50000000-60000000' '3:5000000-9000000|X:70000000-90000000' > log1
gunzip -c samples/merged_nodup.tab.chrblock_sorted.txt.gz | awk '$2=="1" && $3>=1 && $3<=10000000 && $6=="20" && $7>=50000000 && $7<=60000000' > log2
gunzip -c samples/merged_nodup.tab.chrblock_sorted.txt.gz | awk '$2=="3" && $3>=5000000 && $3<=9000000 && $6=="X" && $7>=70000000 && $7<=90000000' >> log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 3 failed"
  return 1;
fi

pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '*|1:0-100000' > log1
gunzip -c samples/merged_nodup.tab.chrblock_sorted.txt.gz | awk '$6=="1" && $7>=0 && $7<=100000' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 4 failed"
  return 1;
fi

pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '1:0-100000|*' > log1
gunzip -c samples/merged_nodup.tab.chrblock_sorted.txt.gz | awk '$2=="1" && $3>=0 && $3<=100000' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 5 failed"
  return 1;
fi


## 1D
pairix -s1 -b2 -e2 -f samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
pairix samples/SRR1171591.variants.snp.vqsr.p.vcf.gz chr10:1-4000000 > log1
gunzip -c samples/SRR1171591.variants.snp.vqsr.p.vcf.gz | awk '$1=="chr10" && $2>=1 && $2<=4000000' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 6 failed"
  return 1;
fi


## 2D, space-delimited
pairix -f -s2 -d6 -b3 -e3 -u7 -T samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz
pairix samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz '10:1-1000000|20' > log1
gunzip -c samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz | awk '$2=="10" && $3>=1 && $3<=1000000 && $6=="20"' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 7 failed"
  return 1;
fi


## preset for pairs.gz
pairix -f samples/test_4dn.pairs.gz
pairix samples/test_4dn.pairs.gz 'chr10|chr20' > log1
gunzip -c samples/test_4dn.pairs.gz | awk '$2=="chr10" && $4=="chr20"' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 8 failed"
  return 1;
fi


## process merged_nodups
source util/process_merged_nodup.sh samples/test_merged_nodups.txt
pairix samples/test_merged_nodups.txt.bsorted.gz '10|20' > log1
awk '$2=="10" && $6=="20"' samples/test_merged_nodups.txt > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 9 failed"
  return 1;
fi

## process old merged_nodups
source util/process_old_merged_nodup.sh samples/test_old_merged_nodups.txt
pairix samples/test_old_merged_nodups.txt.bsorted.gz '10|20' > log1
awk '$3=="10" && $7=="20"' samples/test_old_merged_nodups.txt > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 10 failed"
  return 1;
fi

## merged_nodups2pairs.pl
gunzip -c samples/test_merged_nodups.txt.bsorted.gz | perl util/merged_nodup2pairs.pl - samples/hg19.chrom.sizes.-chr samples/test_merged_nodups
pairix -f samples/test_merged_nodups.bsorted.pairs.gz
pairix samples/test_merged_nodups.bsorted.pairs.gz 'X|8' | cut -f2,3,4,5,8,9 > log1
gunzip -c samples/test_merged_nodups.bsorted.pairs.gz | awk '$2=="X" && $4=="8" {print $2"\t"$3"\t"$4"\t"$5"\t"$8"\t"$9 }' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 11 failed"
  return 1;
fi

## oldmerged_nodups2pairs.pl
gunzip -c samples/test_old_merged_nodups.txt.bsorted.gz | perl util/oldmerged_nodup2pairs.pl - samples/hg19.chrom.sizes.-chr samples/test_old_merged_nodups
pairix -f samples/test_old_merged_nodups.bsorted.pairs.gz
pairix samples/test_old_merged_nodups.bsorted.pairs.gz 'X|8' | cut -f2,3,4,5,8,9 > log1
gunzip -c samples/test_old_merged_nodups.bsorted.pairs.gz | awk '$2=="X" && $4=="8" {print $2"\t"$3"\t"$4"\t"$5"\t"$8"\t"$9 }' > log2
if [ ! -z "$(diff log1 log2)" ]; then
  echo "test 12 failed"
  return 1;
fi

## pairs_merger
bin/pairs_merger samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz | bgzip -c > out.gz
pairix -f -s2 -d6 -b3 -e3 -u7 -T out.gz
# compare with the approach of concatenating and resorting.
chmod +x test/inefficient-merger-for-testing
test/inefficient-merger-for-testing . out2 merged_nodups samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz
gunzip -f out2.bsorted.pairs.gz 
gunzip -f out.gz
if [ ! -z "$(diff out out2.bsorted.pairs)" ]; then 
  echo "test 13 failed"
  return 1;
fi
rm -f out out2.bsorted.pairs out2.pairs out.gz.px2 out2.bsorted.pairs.gz.px2


## streamer_1d
bin/streamer_1d samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz | bgzip -c > out.1d.pairs.gz
gunzip -c samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz | sort -t' ' -k2,2 -k3,3g | bgzip -c > out2.1d.pairs.gz
gunzip -f out.1d.pairs.gz
gunzip -f out2.1d.pairs.gz
if [ ! -z "$(diff out.1d.pairs out2.1d.pairs)" ]; then
  echo "test 14 failed"
  return 1;
fi
rm -f out.1d.pairs out2.1d.pairs

