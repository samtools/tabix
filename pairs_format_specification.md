## Pairs file format specification draft 1.0
<p> 4D Nucleome Omics Data Standards Working Group </p>

<br>

<p>
This document describes the specification for the text contact list file format for chromosome conformation experiments. This type of file is created in practically all HiC pipelines after filtering of aligned reads, and is used for downstream analyses including building matrices, QC, and other high resolution validation. Ideally, the files should be light-weight, easy-to-use and minimally different from the various formats already being used by major Hi-C analysis tools. With these criteria in mind, members of 4D Nucleome Omics Data Standards Working Group have come up with the following specification.
</p>

<p>
The document begins with a summary of the specification. Later sections introduce the pairix tools developed by the 4DN DCIC that provide various utilities for indexing and querying pairs-type files.
</p>

***

### Table of Contents
* [Pairs file](#pairs-file)
* [Example pairs file](#example-pairs-file)
* [Standard format](#standard-format)
* [Standard sorting and indexing](#standard-sorting-and-indexing)
* [Example optional columns](#example-optional-columns)
* [More examples](#more-examples)
* [Tools for pairs file](#tools-for-pairs-file)
* [Contributors](#contributors)

***

### Pairs file
* Pairs format is a standard text format for pairs of genomic loci given as 1bp point positions

***

### Example pairs file
```
## pairs format v1.0
#sorted: chr1-chr2-pos1-pos2
#shape: upper triangle
#genome_assembly: hg38
#chromsize: chr1 249250621
#chromsize: chr2 243199373
#chromsize: chr3 198022430
...
#columns: readID chr1 pos1 chr2 pos2 strand1 strand2
EAS139:136:FC706VJ:2:2104:23462:197393 chr1 10000 chr1 20000 + +
EAS139:136:FC706VJ:2:8762:23765:128766 chr1 50000 chr1 70000 + +
EAS139:136:FC706VJ:2:2342:15343:9863 chr1 60000 chr2 10000 + +
EAS139:136:FC706VJ:2:1286:25:275154 chr1 30000 chr3 40000 + -
```

### Standard format
* Data columns
  * 7 reserved; 4 required; 9 reserved column names; Any number of optional columns can be added.
    * Reserved columns (columns 1-7): The positions of these columns are reserved.
      * `readID, chr1, pos1, chr2, pos2, strand1, strand2`
    * Required columns (columns 2-5): Required columns cannot have a missing value.
      * `chr1, pos1, chr2, pos2`
      * Missing values for reserved columns: a single-character dummy (‘.’)
    * Reserved optional column names: The positions of these columns are not reserved.
      * `frag1`, `frag2` (restriction enzyme fragmenet index used by juicer)
* Header
  * Header lines begin with '#' and must appear before the data entries. Relative positions of header lines are not determined, except for the first line that specifies the format.
  * Required
    * First line: `## pairs format v1.0`
    * column contents and ordering:
      * `#columns: readID chr1 pos1 chr2 pos2 strand1 strand2 <column_name> <column_name> ...`
    * chromosome names and their size in bp, one chromosome per line, in the same order that defines ordering between mates.
      * `#chromsize: <chromosome_name> <chromosome_size>` 
      * Chromosome order for defining upper triangle (mate1 < mate2) is defined by this header.
      * Chromosome order for rows is not defined (UNIX sort or any other sort can be used)
      * The style in the header must match the actual chromosomes in the file:
        * ENSEMBL-style (1,2,.. ) vs USCS-style (chr1, chr2)
  * Optional lines with reserved header keys (`sorted`, `chromosomes`, `shape`, `command`, `genome_assembly`)
    * Sorting mechanism: `chr1-chr2-pos1-pos2`, `chr1-pos1`, `none` are reserved.
      * `#sorted: chr1-chr2-pos1-pos2`, `#sorted: chr1-pos1`, `#sorted: none`, or other custom sorting mechanisms
    * Upper triangle vs lower triangle: `upper triangle`, `lower triangle` are reserved.
      * `#shape: upper triangle` or `#shape: lower triangle`
    * Command used to generate the pairs file:
      * `#command: bam2pairs mysample.bam mysample`
      * e.g.) Filtering information can be reported this way.
    * Genome assembly:
      * `#genome_assembly: hg38`
  * Other example optional headers (see below for more examples)


***

### Standard sorting and indexing
* Bgzipped, tab-delimited text file with an index for random access
* File extension: `.pairs.gz` (with index `.pairs.gz.px2`)
* Mate sorting
  * By default, `upper triangle` (mate1 coordinate is lower than mate2 coordinate); optionally, `lower triangle`.
* Row sorting
  * By default, `chr1-chr2-pos1-pos2`; optionally, `chr1-pos1`. Examples are shown below.
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
* Redundancy
  * Contacts must be nonredundant.
    * Each entry appears only once and in a consistent ordering of the two mates; either in an `upper triangle` (mate2 coordinate is larger than mate1) or a `lower triangle` (mate1 coordinate is larger than mate2). For example, the following two rows are redundant and only one of them must be kept. This order must be consistent with the `#shape` and `#chromoosmes` header lines if they are present.
      ```
      EAS139:136:FC706VJ:2:1286:25:275154 chr1 10000 chr2 2000 + +
      EAS139:136:FC706VJ:2:1286:25:275154 chr2 2000 chr1 10000 + +
      ```   
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

***

### More examples
#### A complex example with optional fields and/or missing fields
```
## pairs format v1.0
#sorted: none
#shape: upper triangle
#chromsize: 1 249250621
#chromsize: 2 243199373
...
#genome_assembly: GRCh37
#columns: readID chr1 pos1 chr2 pos2 strand1 strand2 chr3 pos3 strand3 mismatch_str1 mismatch_str2 mismatch_str3
.  1 60000 2 10000 + +
.  1 10000 1 20000 + +     10:A>G,13:C>G 4:G>T,6:C>G
.  1 30000 3 40000 + -      8:T>A
.  1 50000 1 70000 + + chr5 80000 - 2:T>G
```

***

### Tools for pairs file
* Pairix
  * Indexing on bgzipped file that is sorted by `chr1-chr2-pos1-pos2` and querying on the indexed file.
  * https://github.com/4dn-dcic/pairix
  * example usage
    ```
    pairix sample.pairs.gz # indexing
    pairix sample.pairs.gz 'chr1:10000-20000|chr2:20000-30000' # querying
    ```
* Pypairix
  * A python binder for pairix is available through pip install.
  * `pip install pypairix`
  * https://github.com/4dn-dcic/pairix
  * example usage
    ```
    pypairix.build_index("sample.pairs.gz") #indexing
    pr = pypairix.open("sample.pairs.gz")
    iter = pr.querys2D('chr1:10000-20000|chr2:20000-30000') # querying; returns an iterator
    for line in iter:
	   print line
    ```
* Rpairix
  * An R binder is available as well.
  * https://github.com/4dn-dcic/pairix
  * example usage
    ```
    px_build_index("sample.pairs.gz") # indexing
    px_query("sample.pairs.gz",'chr1:10000-20000|chr2:20000-30000') # querying
    ```
* bam2pairs
  * Converter from bam to pairs
  * https://github.com/4dn-dcic/pairix/tree/master/util/bam2pairs
* utils for converting juicer's merged_nodups.txt formats to a pairs format can be found as part of the pairix package.
  * https://github.com/4dn-dcic/pairix/tree/master/util

***

### Contributors
Soohyun Lee, Nezar Abdennur, Neva Durand, Anton Goloborodko, Hakan Özadam, Peter Kerpedjiev, Lixing Yang, Nils Gehlenborg, Andy Shroeder, Bing Ren, Erez Lieberman Aiden, Peter Park, Burak Alver, and other members of the 4D Nucleome Omics Data Standards Working Group.
