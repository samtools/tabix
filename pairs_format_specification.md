## Pairs file format specification draft 1.0
<p> 4D Nucleome DCIC and Omics Working Group </p>

<br>

<p>
This document describes the specification for the text contact list file format for chromosome conformation experiments. This type of file is created in practically all HiC pipelines after filtering of aligned reads, and is used for downstream analyses including building matrices, QC, and other high resolution validation. Ideally, the files should be light-weight, easy-to-use and minimally different from the various formats already being used by major Hi-C analysis tools. With these criteria in mind, members of omics WG from the DCIC and authors of the cooler and juicer pipelines have come up with the following specification. 
</p>

<p>
The document begins with a summary of the specification, and later sections contain brief introduction of the tools that can be used along with the format.
</p>

***

### Table of Contents
* [Pairs file](#pairs-file)
* [Standard format](#standard-format)
* [Standard sorting and indexing](#standard-sorting-and-indexing)
* [Example optional columns](#example-optional-columns)
* [Sorting mechanisms](#sorting-mechanisms)
* [Tools for pairs file](#tools-for-pairs-file)

***

### Pairs file
* File containing a list of contacts (e.g. a contact : pair of genomic loci that is represented by a valid read pair)
* Similar to bam, but lossy (filtered) and minimal in information.
* Similar to matrix, but at read pair resolution.
* Analogous to the formats used by hiclib, cooler and juicer pipelines (cooler and juicer have been modified to accommodate the new format)

***

### Standard format
* Content
  * Contacts must be nonredundant.
    * The requirement is that each pair appears only once and in a consistent ordering of the two mates; either in an upper-triangle (mate2 coordinate is larger than mate1) or a lower-triangle (mate1 coordinate is larger than mate2).
  * A text file with 7 reserved  columns and ? optional columns.
  * Reserved columns: `readID, chr1, chr2, pos1, pos2, strand1, strand2` 
    * Preserving READ ID allows retrieving corresponding reads from a bam file.
  * Required columns: `chr1, chr2, pos1, pos2` (i.e. optionally, readID and strands can be blank (‘.’))
  * Missing values for required columns
    * a single-character dummy (‘.’)
  * Optional columns can be added (e.g. for triplets, quadruplets, mapq, sequence/mismatch information (bowtie-style e.g. 10:A>G))
  * Reserved optional column names (column positions are not reserved): `frag1`, `frag2` (restriction enzyme fragmenet index used by juicer)
* Header
  * Required
    * First line: `## pairs format v1.0`
    * column contents and ordering: `#columns: readID chr1 position1 chr2 position2 strand1 strand2`
  * Optional, but reserved (header key is reserved)
    * Sorting mechanism: `#sorted: chr1-chr2-pos1-pos2`, `#sorted: chr1-pos1`, `#sorted: none`, or other custom sorting mechanisms
    * Upper triangle vs lower triangle: `#shape: upper triangle`
    * chromosome mate order: `#chromosomes: chr1 chr2 chr3 chr4 chr5 chr6 chr7 chr8 chr9 chr10 chr11 chr12 chr13 chr14 chr15 chr16 chr17 chr18 chr19 chr20 chr21 chr22 chrX chrY chrM`
      * Chromosome order for defining upper triangle (mate1 < mate2) is defined by this header.
      * Chromosome order for rows is not defined (UNIX sort or any other sort can be used)
      * The style in the header must match the actual chromosomes in the file:
        * ENSEMBL-style (1,2,.. ) vs USCS-style (chr1, chr2)
  * Command used to generate the pairs file: `#command: bam2pairs mysample.bam mysample`
  * Other example optional headers (not parsed by software tools but for human)
    * Genome assembly may be included in the header (see below).
    * Filtering information (commands used to generate the file) can be reported in the header (optional). 

***

### Standard sorting and indexing
* Bgzipped, tab-delimited text file with an index for random access
* File extension: `.pairs.gz` (with index `.pairs.gz.px2`)
* Mate sorting
  * Upper triangle (mate1 coordinate is lower than mate2 coordinate) by default; optionally, lower triangle.
* Row sorting
  * Sort by (chr1, chr2, pos1, pos2) ('Chromosome-pair-block-sorted') by default (i.e all chr1:chr1 pairs appear before chr1:chr2); optionally, 1D-position-sorted (chr1, pos1)
* Compressing
  * Compressing by bgzip : https://github.com/4dn-dcic/pairix (forked) or https://github.com/samtools/tabix (original) (The two are identical)
* Indexing and querying
  * Indexing and querying by Pairix : https://github.com/4dn-dcic/pairix

***

### Example optional columns
* MateIDs: mate1, mate2,
* Map quality: mapq1, mapq2
* Sequence / mismatch information (for phasing)
* 3rd and 4th contact for triplets and quadraplets (or more)

#### Sequence information for phasing
* Optionally, full sequence can be added.
* CIGAR and mismatch information
* Bowtie format : 10:A>G,13:C>G
  *  10:A>G,13:C>G means : 10th position A is substituted by G on the read, 13th position C is substituted by G on the read.
* BWA format : 6G11A56
  * 6G11A56 means : 6 matches followed by a mismatch (G -> something) followed by 11 matches followed by a mismatch (A -> something) then 56 matches.
  * Note: BWA format does not record the base on the read but the reference, and therefore must accompany read sequence.
* Add CIGAR string 
  * For soft- or hard-clipped cases, we need to know where the MD tag begins.
 
#### Genome assembly
* Genome assembly can be optionally specified in the header
```
#genome_assembly: hg38
```

***

### Sorting mechanisms
#### Examples of the two sorting mechanisms

```
# Sorted: chr1-chr2-pos1-pos2
chr1 10000 chr1 20000
chr1 50000 chr1 70000
chr1 60000 chr2 10000
chr1 30000 chr3 40000
```
```
# Sorted: chr1-pos1
chr1 10000 chr1 20000
chr1 30000 chr3 40000
chr1 50000 chr1 70000
chr1 60000 chr2 10000
```

***

### Example pairs files 
#### A simple example with only reserved fields
``` 
## pairs format v1.0
#sorted: chr1-chr2-pos1-pos2
#shape: upper triangle
#chromosomes: chr1 chr2 chr3 chr4 chr5 chr6 chr7 chr8 chr9 chr10 chr11 chr12 chr13 chr14 chr15 chr16 chr17 chr18 chr19 chr20 chr21 chr22 chrX chrY chrM
#genome_assembly: hg38
#columns: readID chr1 position1 chr2 position2 strand1 strand2
EAS139:136:FC706VJ:2:2104:23462:197393 chr1 10000 chr1 20000 + +
EAS139:136:FC706VJ:2:8762:23765:128766 chr1 50000 chr1 70000 + +
EAS139:136:FC706VJ:2:2342:15343:9863 chr1 60000 chr2 10000 + + 
EAS139:136:FC706VJ:2:1286:25:275154 chr1 30000 chr3 40000 + -
```
 
#### A complex example with optional fields and/or missing fields
<p>
(not DCIC-provided file, but a user could generate a file like this)
</p>

``` 
## pairs format v1.0
#sorted: none
#shape: upper triangle
#chromosomes: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 X Y
#genome_assembly: GRCh37
#columns: readID chr1 position1 chr2 position2 strand1 strand2 chr3 position3 strand3 mismatch_str1 mismatch_str2 mismatch_str3
.  1 60000 2 10000 + +
.  1 10000 1 20000 + +     10:A>G,13:C>G 4:G>T,6:C>G
.  1 30000 3 40000 + -      8:T>A
.  1 50000 1 70000 + + chr5 80000 - 2:T>G
```
 
***

### Tools for pairs file
* Pairix
  * Indexing on bgzipped file that is sorted by chr1-chr2-pos1-pos2 and querying on the indexed file.
  * https://github.com/4dn-dcic/pairix
  * Although Pairix is developed on top of tabix, the two indices are not mutually compatible.
* Pypairix
  * A python binder for pairix is available through pip install.
  * `pip install pypairix`
* Rpairix
  * An R binder is available as well.
  * https://github.com/4dn-dcic/pairix
* bam2pairs
  * Converter from bam to pairs
  * https://github.com/4dn-dcic/bam2pairs
* utils for converting juicer's merged_nodups.txt formats to a pairs format can be found as part of the pairix package.
  * https://github.com/4dn-dcic/pairix/tree/master/util
