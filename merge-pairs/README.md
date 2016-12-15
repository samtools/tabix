# merge-pairs
**It is strongly recommended to use pairs_merger instead (part of this repo). Merge-pairs is used only for testing pairs_merger.**
This script merges two or more chromosome-block-sorted pairs files into a single merged pairs file.
The input files are gzipped and the output file will also be bgzipped and pairix-indexed.

# Requirements
* bgzip, pairix 

# Usage
```
merge-pairs outdir outprefix in1.gz in2.gz in3.gz ...
```
