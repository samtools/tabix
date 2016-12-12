#!/usr/bin/env python
"""
test.py
Used to run tests on the test files found in /samples/
From root, execute using `python test/test.py`
First, ensure you have fully installed the pypairix package:
`pip install pypairix --user`
OR
`sudo python setup.py install`

If you're having trouble running this file, try installing
python-dev and zlib1g-dev.
"""


import unittest
import gzip
import sys
import pypairix

TEST_FILE_2D = 'samples/merged_nodup.tab.chrblock_sorted.txt.gz'
TEST_FILE_1D = 'samples/SRR1171591.variants.snp.vqsr.p.vcf.gz'
TEST_FILE_2D_SPACE = 'samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz'

def read_vcf(filename):
    """Read a VCF file and return a list of [chrom, start, end] items."""
    retval = []
    for line in gzip.open(filename):
        try:
            line = line.decode('utf-8')
        except AttributeError:
            pass
        fields = line.rstrip().split('\t')
        chrom = fields[0]
        start = fields[1]
        end = fields[1]
        retval.append([chrom, start, end])
    return retval


def read_pairs(filename, delimiter='\t'):
    """Read a pairs file and return a list of [chrom1, start1, end1, chrom2, start2, end2] items."""
    retval = []
    for line in gzip.open(filename):
        try:
            line = line.decode('utf-8')
        except AttributeError:
            pass
        fields = line.rstrip().split(delimiter)
        chrom1 = fields[1]
        start1 = fields[2]
        chrom2 = fields[5]
        start2 = fields[6]
        retval.append([chrom1, start1, start1, chrom2, start2, start2])
    return retval


def overlap1(a0, a1, b0, b1):
    return int(a0) <= int(b1) and int(a1) >= int(b0)


def get_result(regions, chrom, start, end):
    retval = []
    for r in regions:
        if r[0] == chrom and overlap1(r[1], r[2], start, end):
            retval.append(r)
    return retval

def get_result_2D(regions, chrom, start, end, chrom2, start2, end2):
    retval = []
    for r in regions:
        if r[0] == chrom and overlap1(r[1], r[2], start, end) and r[3] == chrom2 and overlap1(r[4], r[5], start2, end2):
            retval.append(r)
    return retval


## 1D query on 1D indexed file
class TabixTest(unittest.TestCase):
    regions = read_vcf(TEST_FILE_1D)
    chrom = 'chr10'
    start = 25944
    end = 27000000
    result = get_result(regions, chrom, start, end)
    pr = pypairix.open(TEST_FILE_1D)

    def test_query(self):
        it = self.pr.query(self.chrom, self.start, self.end)
        pr_result = [[x[0], x[1], x[1]] for x in it]
        self.assertEqual(self.result, pr_result)

    def test_querys(self):
        query = '{}:{}-{}'.format(self.chrom, self.start, self.end)
        it = self.pr.querys(query)
        pr_result = [[x[0], x[1], x[1]] for x in it]
        self.assertEqual(self.result, pr_result)



## semi-2D query on 2D indexed file
class TabixTest_2(unittest.TestCase):
    regions = read_pairs(TEST_FILE_2D)
    chrom = '10'
    start = 25944
    end = 27000000
    chrom2 = '20'
    result = get_result_2D(regions, chrom, start, end, chrom2, 0, sys.maxsize)
    pr = pypairix.open(TEST_FILE_2D)

    def test_querys(self):
        query = '{}:{}-{}|{}'.format(self.chrom, self.start, self.end, self.chrom2)
        it = self.pr.querys2D(query)
        pr_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, pr_result)


## 2D query on 2D indexed file
class TabixTest2D(unittest.TestCase):
    regions = read_pairs(TEST_FILE_2D)
    chrom = '10'
    start = 1
    end = 1000000
    chrom2 = '20'
    start2 = 50000000
    end2 = 60000000
    result = get_result_2D(regions, chrom, start, end, chrom2, start2, end2)
    pr = pypairix.open(TEST_FILE_2D)

    def test_query2(self):
        it = self.pr.query2D(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        pr_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, pr_result)

    def test_querys_2(self):
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        it = self.pr.querys2D(query)
        pr_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, pr_result)


## 2D query on 2D indexed space-delimited file
class TabixTest2DSpace(unittest.TestCase):
    regions = read_pairs(TEST_FILE_2D_SPACE, ' ')
    chrom = '10'
    start = 1
    end = 1000000
    chrom2 = '20'
    start2 = 50000000
    end2 = 60000000
    result = get_result_2D(regions, chrom, start, end, chrom2, start2, end2)
    pr = pypairix.open(TEST_FILE_2D_SPACE)

    def test_query2(self):
        it = self.pr.query2D(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        pr_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, pr_result)

    def test_querys_2(self):
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        it = self.pr.querys2D(query)
        pr_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, pr_result)


class TabixTestBlocknames(unittest.TestCase):
    pr = pypairix.open(TEST_FILE_2D)

    def test_blocknames(self):
      print (str(self.pr.get_blocknames()))


if __name__ == '__main__':
    unittest.main()
