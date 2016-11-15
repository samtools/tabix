# merge-pairs

This script merges two or more chromosome-block-sorted pairs files into a single merged pairs file.
The input files are gzipped and the output file will also be bgzipped and pairix-indexed.

# Requirements
* bgzip, pairix 

# Warning
* This subdirectory is currently under development. It does what it's supposed to do, but it will be improved in the future in the following aspects.
    * Checking header for sorting mechanism.
    * Using the pairix index for faster merging. (Curretnly it does not use the index files.)

