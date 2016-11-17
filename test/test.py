#!/usr/bin/env python
"""
test.py
Used to run tests on the test files found in /samples/
From root, execute using `python test/test.py`
First, ensure you have fully installed the pypairix package:
`pip install pypairix --user`
OR
`sudo python setup.py install`
"""


import unittest
import gzip
import sys
import pairix

TEST_FILE_2D = 'samples/merged_nodup.tab.chrblock_sorted.txt.gz'
TEST_FILE_1D = 'samples/SRR1171591.variants.snp.vqsr.p.vcf.gz'


def read_vcf(filename):
    """Read a VCF file and return a list of [chrom, start, end] items."""
    retval = []
    for line in gzip.open(filename):
        fields = line.rstrip().split('\t')
        chrom = fields[0]
        start = fields[1]
        end = fields[1]
        retval.append([chrom, start, end])
    return retval


def read_pairs(filename):
    """Read a pairs file and return a list of [chrom1, start1, end1, chrom2, start2, end2] items."""
    retval = []
    for line in gzip.open(filename):
        fields = line.rstrip().split('\t')
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
    tb = pairix.open(TEST_FILE_1D)

    def test_query(self):
        it = self.tb.query(self.chrom, self.start, self.end)
        tb_result = [[x[0], x[1], x[1]] for x in it]
        self.assertEqual(self.result, tb_result)

    def test_querys(self):
        query = '{}:{}-{}'.format(self.chrom, self.start, self.end)
        it = self.tb.querys(query)
        tb_result = [[x[0], x[1], x[1]] for x in it]
        self.assertEqual(self.result, tb_result)

    def test_remote_file(self):
        file1 = "ftp://ftp.1000genomes.ebi.ac.uk/vol1/ftp/release/20100804/" \
                "ALL.2of4intersection.20100804.genotypes.vcf.gz"
        pairix.open(file1)

    def test_remote_file_bad_url(self):
        file1 = "ftp://badurl"
        with self.assertRaises(pairix.TabixError):
            pairix.open(file1)


## semi-2D query on 2D indexed file
class TabixTest_2(unittest.TestCase):
    regions = read_pairs(TEST_FILE_2D)
    chrom = '10'
    start = 25944
    end = 27000000
    chrom2 = '20'
    result = get_result_2D(regions, chrom, start, end, chrom2, 0, sys.maxint)
    tb = pairix.open(TEST_FILE_2D)

    def test_querys(self):
        query = '{}:{}-{}|{}'.format(self.chrom, self.start, self.end, self.chrom2)
        it = self.tb.querys2D(query)
        tb_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, tb_result)


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
    tb = pairix.open(TEST_FILE_2D)

    def test_query2(self):
        it = self.tb.query2D(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        tb_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, tb_result)

    def test_querys_2(self):
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        it = self.tb.querys2D(query)
        tb_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
        self.assertEqual(self.result, tb_result)

if __name__ == '__main__':
    unittest.main()
