### Pairs file format specification draft 1.0
<p>(document currently under developemt)</p>

#### Authors
Soo Lee, Burak Alver, Nezar Abdennur, Neva Durand, Peter Kerpedjiev, Anton Goloborodko, Hakan Özadam, Lixing Yang, Nils Gehlenborg, Andy Shroeder, Bing Ren, Erez Lieberman Aiden, 4D Nucleome Omics Working Group

<p>
This document describes the proposed specification for the text contact list file format for chromosome conformation experiments. This type of file is created in practically all HiC pipelines after filtering of aligned reads, and is used for downstream analyses including building matrices, QC, and other high resolution validation. However, there is currently no standard for these contact list files. Ideally, the files should be light-weight, easy-to-use and minimally different from the various formats already being used by major Hi-C analysis tools. With these criteria in mind, members of omics WG from the DCIC and authors of the cooler and juicer pipelines have come up with the following proposal. 
</p>

<p>
The document begins with a summary of the proposal, and later sections contain discussions about alternative approaches that have been considered and the rationale underlying the  choices.
</p>

#### Pairs file
* File containing a list of contacts (e.g. a contact : pair of genomic loci that is represented by a valid read pair)
* Similar to bam, but lossy (filtered) and minimal in information.
* Similar to matrix, but at read pair resolution.
* Analogous to the formats used by hiclib, cooler and juicer pipelines (these pipelines can be modified to accommodate the new format)

#### Standard format
 
* A text file with 7 reserved  columns and ? optional columns.
* Reserved columns: readID, chr1, chr2, pos1, pos2, strand1, strand2 
* Optionally, readID and strands can be blank (‘.’) : DCIC provides both readID and strands.
* Positions are 5’end of reads.
* Optional columns for triplets, quadruplets, mapq, sequence/mismatch information (bowtie-style e.g. 10:A>G) 
* Contacts must be nonredundant.
* Header
  * Required
    * First line: `## pairs format v1.0`
  * Optional
    * Sorting mechanism, chromosome order, genome assembly and column contents are included in the header (see below).
    * Filtering information (commands used to generate the file) can be reported in the header (optional). 
    * Chromosome order for rows is not defined (UNIX sort or any other sort can be used)
    * Chromosome order for defining upper triangle (mate1 < mate2) is defined.

#### Standard sorting and indexing for files provided by DCIC
* Gzipped (bgzf), tab-delimited text file with an index for random access
* Can be filtered and lossy.
* Upper triangle (mate1 coordinate is lower than mate2 coordinate) by default; optionally, lower triangle.
* Chromosome-pair-block-sorted 
* Sort by (chr1, chr2, pos1, pos2) by default;
  * i.e all chr1:chr1 pairs appear before chr1:chr2
* Optionally, 1D-position-sorted (chr1, pos1, chr2, pos2)
* Provide a 2d indexer based on tabix (tabix indexing uses a single sequence name (e.g. chromosome) and start and end positions (in our case start = end).
* Pairix : https://github.com/hms-dbmi/pairix : indexes on both chromosomes.
* The index is modified from the tabix index (not backward compatible).
* Python binder (pypairix) is available for pip install.
* A compromise between simplicity of format (text, simple sort on first mate) vs. very-fast query (query on both pos1 and pos2)
* Provide a fast, linear-time sorter (converter) between the two sorting.
 

#### Example of DCIC-provided pairs file 
##### A simple example with only reserved fields
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
 
##### A complex example with optional fields and/or missing fields
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
 
#### Details on each proposed component

##### Mate ordering / arrangement
* Proposed: upper-triangle
* Discussed options
  * Unique-contacts vs whole matrix
  * Unique-contacts : upper-triangle (more commonly used), lower triangle
  * Whole matrix (bam style): Each contact occurs twice. 
    * Pros: Good for virtual 4C type query. (extracting an entire row, i.e. retrieving all contacts with a specific genomic location)
    * Cons: Risk of double-counting, especially along the diagonal. Double size.

##### General file type
* Proposed: tab-delimited text, bgzipped. 
<p>
bgzip (.gz) allows indexing and random-access by tabix & pairix
Works with regular gunzip
</p>

* Discussed options
  * Alternative: hdf5 
  * Hdf5 needs special tools to access, less generic.
  * The pairs files would most likely be used for quick examination of a specific region of a matrix without going through the bam file; Light-weight, easy-to-use file type that allows fast queries would be best.
 
##### Reserved columns
* column1-7 are reserved
```
readID chr1 chr2 pos1 pos2 strand1 strand2
```
<p>
Preserving READ ID allows retrieving corresponding reads from a bam file.
</p>

##### Required columns
* 4 columns
````
chr1 pos1 chr2 pos2
````

##### Read ID
* Preserving READ ID allows retrieving corresponding reads from a bam file.

##### Optional columns
* MateIDs: mate1, mate2,
* Map quality: mapq1, mapq2
* Sequence / mismatch information (for phasing)
* 3rd and 4th contact for triplets and quadraplets (or more)
* Restriction enzyme fragment number (as in juicer pairs format)
 
##### Missing values for reserved columns
* a single-character dummy (‘.’)
 
##### Sequence information for phasing
* Full sequence vs difference only
* What format if difference only
* Bowtie format (proposed): 10:A>G,13:C>G
  *  10:A>G,13:C>G means : 10th position A is substituted by G on the read, 13th position C is substituted by G on the read.
* BWA format : 6G11A56
  * 6G11A56 means : 6 matches followed by a mismatch (G -> something) followed by 11 matches followed by a mismatch (A -> something) then 56 matches.
  * problem : it does not record the base on the read but the reference.
* Add CIGAR string 
  * For soft- or hard-clipped cases, we need to know where the MD tag begins.
 
##### Genome assembly
* Optionally specified in the header
```
#genome_assembly: hg38
```

##### Chromosome ordering
* Chromosome order is optionally specified in the header.
```
#chromosomes: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 X Y Mt
```
* The style in the header must match the actual chromosomes in the file:
  * ENSEMBL-style (1,2,.. ) vs USCS-style (chr1, chr2)
 
##### Sorting mechanism
* Mark the sorting mechanism in the header, so that the user knows how the file is sorted without having to look into it.
 
``` 
# Sorted: chr1-chr2-pos1-pos2
```
or
```
# Sorted: chr1-pos1
```
or
```
# Sorted: none
```
or other custom sorting mechanisms (e.g. sorting that takes into account triplets, quadraplets, … )

##### Comparison of two sorting mechanisms
* Three possible sorting options  being considered
  * 1D-position-sorted : sort by chr1 pos1 chr2 pos2 (can be used with tabix as is)
  * chromosome-pair-block sorted: sort by chr1 chr2 pos1 pos2 (proposed)
  * z-index (fractal 2d sorting) : (Would allow very fast 2D queries, too complex for a text file which will be also used by other tools to read the whole genome.)
 
###### Examples
* position-sorted (Up)
```
chr1 10000 chr1 20000
chr1 30000 chr3 40000
chr1 50000 chr1 70000
chr1 60000 chr2 10000
```

* chromosome-block-sorted (Uc)
```
chr1 10000 chr1 20000
chr1 50000 chr1 70000
chr1 60000 chr2 10000
chr1 30000 chr3 40000
```

##### Comparison based on Query operations
* Uc uses smaller search space (faster) for intra- and inter-chromosomal submatrix query and a virtual 4C (row-extraction) query. Up also adds time for checking chr2 for every retrieved entry.

##### Other comparisons
 
| | 1D-Position-sorted | Chromosome-pair-block-sorted |
-------------------------------------------------------
| Cost of resorting (to the other one)   | Linear,                                       | Linear,                          |
| (suggested algorithms described below) | Multiple temporary files (total 2x space, IO) | With 2d index and random access. |
| Indexing and random access | BGZF and tabix or pairix | BGZF and pairix |
| Example application type that can benefit from the type of sorting | Treating whole-genome (all chromosomes concatenated) | Per-chromosome-pair analysis (e.g. specific intra-chromosomal or inter-chromosomal pairs), virtual 4C.
(* Probably, more analyses would be this type than the other.) |
| Examples of currently available tools | Cooler, Hi-Glass | Juicer, Cooler* |
 
<p>* not available at the time of discussion </p>
 
 
##### sorting algorithms
* 1D-position-sorted -> chr-pair-block-sorted
  * 1) scan original file and for each line, write to a temporary file corresponding to the current chromosome pair.
  * 2) Concatenate the temporary files in the right order
  * Note: you need to write to 23*23/2 temporary files simultaneously.
 
* Chr-pair-block-sorted -> 1D-position-sorted
  * 1) Given a chromosome (e.g. chr1), prepare an array of pointers each pointing to the beginning of a chromosome pair block. (e.g. chr1:chr1, chr1:chr2, … chr1:chrY) : (maximum length = number of chromosomes = 23)
  * 2) Sort the values and put them into a linked list S
  * 3) Write the minimum into the final output, delete the entry from S, move the pointer of that chromosome pair to the next entry and insert it in the right place in S.
  * 4) repeat 3 until all the entries are covered.
  * 5) repeat 1-3 for every chromosome.
  * Note : this algorithm is in theory linear to number_of_contacts * number of chromosomes, in a vast majority of cases, it will be linear to the number of contacts, because most of the cases, there will be only 1~2 comparisons.
  * Note : merge-sort turned out to be much faster. 
 
##### Indexing
* Provide a 2d indexer based on tabix (pairix) (proposed).
  * Pairix : https://github.com/hms-dbmi/pairix : indexes on both chromosomes (two-columns).
  * The index is modified from the tabix index (not backward compatible).
  * Python binder (pypairix) is available through pip install.
  * R binder (Rpairix) is available as well.
  * A compromise between simplicity of format (text, simple sort on first mate) vs. very-fast query (query on both pos1 and pos2)
  * Any optional column can be added to identify blocks if the user wants to use tabix instead. 
* Alternative:
  * Using tabix (single-column 1d indexing) as it is, add an additional column that corresponds to chromosome pair (e.g. chr1:chr2 or a simpler identifier whose mapping is defined in the header)
    * Pro: 
      * Tabix has been used stably, there is a java library written around it already. 
      * No maintenance burden.
      * Can be extended to multi-way interactions.
    * Con: 
      * File size increase due to additional column, redundant information.
      * Need a wrapper tool and a mapping between the chromosome names and block identifiers for 2D querying
  * Z-index (fractal 2d sorting) : 
    * Pro: ‘true’ 2d indexing.
    * Con: sorting can be confusing to general users. (given this is a text file)

