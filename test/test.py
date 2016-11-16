#!/usr/bin/env python
"""
test_tabix.py
Hyeshik Chang <hyeshik@snu.ac.kr>
Kamil Slowikowski <slowikow@broadinstitute.org>
March 21, 2015
The MIT License
Copyright (c) 2011 Seoul National University.
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""


import unittest
import random
import gzip
import tabix

# TEST_FILE = 'samples/merged_nodup.tab.chrblock_sorted.txt.gz'
TEST_FILE = 'test/example.gtf.gz'


def read_gtf(filename):
    """Read a GTF file and return a list of [chrom, start, end] items."""
    retval = []
    for line in gzip.open(filename):
        fields = line.rstrip().split('\t')
        chrom = fields[0]
        start = fields[3]
        end = fields[4]
        retval.append([chrom, start, end])
    return retval


def overlap1(a0, a1, b0, b1):
    """Check if two 1-based intervals overlap."""
    return int(a0) <= int(b1) and int(a1) >= int(b0)


def get_result(regions, chrom, start, end):
    retval = []
    for r in regions:
        if r[0] == chrom and overlap1(r[1], r[2], start, end):
            retval.append(r)
    return retval


class TabixTest(unittest.TestCase):
    regions = read_gtf(TEST_FILE)
    chrom = 'chr1'
    start = 25944
    end = 27000
    result = get_result(regions, chrom, start, end)
    tb = tabix.open(TEST_FILE)

    def test_query(self):
        it = self.tb.query(self.chrom, self.start, self.end)
        tb_result = [ [x[0], x[3], x[4]] for x in it ]
        self.assertEqual(self.result, tb_result)

    def test_querys(self):
        query = '{}:{}-{}'.format(self.chrom, self.start, self.end)
        it = self.tb.querys(query)
        tb_result = [ [x[0], x[3], x[4]] for x in it ]
        self.assertEqual(self.result, tb_result)

    def test_remote_file(self):
        file1 = "ftp://ftp.1000genomes.ebi.ac.uk/vol1/ftp/release/20100804/" \
                "ALL.2of4intersection.20100804.genotypes.vcf.gz"
        tabix.open(file1)

    def test_remote_file_bad_url(self):
        file1 = "ftp://badurl"
        with self.assertRaises(tabix.TabixError):
            tabix.open(file1)


if __name__ == '__main__':
    unittest.main()
