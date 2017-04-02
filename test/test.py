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

Note: tests are run to anticipate either juicer-formatted pairs files or 4DN-
formatted pairs files.
The columns (given in form <attribute [col#]):
Juicer: chr1[1] pos1[2] chr2[5] pos2[6]
4DN: chr1[1] pos1[2] chr2[3] pos2[4]
"""


import unittest
import gzip
import sys
import pypairix
import warnings

TEST_FILE_2D = 'samples/merged_nodup.tab.chrblock_sorted.txt.gz'
TEST_FILE_2D_4DN = 'samples/4dn.bsorted.chr21_22_only.pairs.gz'
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


def find_pairs_type(filename, delimiter='\t'):
    """Attempt to determine if input pairs file is of type: juicer, 4DN,
    or undetermined. Do this by testing string values of """
    is_juicer = False
    is_4DN = False
    for line in gzip.open(filename):
        try:
            line = line.decode('utf-8')
        except AttributeError:
            pass
        fields = line.rstrip().split(delimiter)
        if is_str(fields[2]) and is_str(fields[6]):
            is_juicer = True
        if is_str(fields[2]) and is_str(fields[4]):
            is_4DN = True
        if not is_juicer and is_4DN:
            return '4DN'
        elif is_juicer:
            return 'juicer'
    return 'undetermined'


def is_str(s):
    """Helper function to see if a string is an int. Return True if so"""
    try:
        int(s)
        return True
    except ValueError:
        return False


def read_pairs(filename, file_type='undetermined', delimiter='\t'):
    """Read a pairs file and return a list of [chrom1, start1, end1, chrom2, start2, end2] items."""
    # handle this a different way?
    if file_type == 'undetermined':
        return []
    retval = []
    for line in gzip.open(filename):
        try:
            line = line.decode('utf-8')
        except AttributeError:
            pass
        fields = line.rstrip().split(delimiter)
        if file_type == 'juicer':
            chrom1 = fields[1]
            start1 = fields[2]
            chrom2 = fields[5]
            start2 = fields[6]
        elif file_type == '4DN':
            chrom1 = fields[1]
            start1 = fields[2]
            chrom2 = fields[3]
            start2 = fields[4]
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


def build_it_result(it, f_type):
    """Build results using the pairix iterator based on the filetype"""
    if f_type == 'juicer':
        pr_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
    elif f_type == '4DN':
        pr_result = [[x[1], x[2], x[2], x[3], x[4], x[4]] for x in it]
    elif f_type == 'undetermined':
        pr_result = []
    return pr_result


## 1D query on 1D indexed file
class PairixTest(unittest.TestCase):
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
class PairixTest_2(unittest.TestCase):
    f_type = find_pairs_type(TEST_FILE_2D)
    regions = read_pairs(TEST_FILE_2D, f_type)
    chrom = '10'
    start = 25944
    end = 27000000
    chrom2 = '20'
    result = get_result_2D(regions, chrom, start, end, chrom2, 0, sys.maxsize)
    pr = pypairix.open(TEST_FILE_2D)

    def test_querys(self):
        query = '{}:{}-{}|{}'.format(self.chrom, self.start, self.end, self.chrom2)
        it = self.pr.querys2D(query)
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)


## 2D query on 2D indexed file
class PairixTest2D(unittest.TestCase):
    f_type = find_pairs_type(TEST_FILE_2D)
    regions = read_pairs(TEST_FILE_2D, f_type)
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
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)

    def test_querys_2(self):
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        it = self.pr.querys2D(query)
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)

    def test_querys_2_bad_order(self):
        # build the query with coordinates in the wrong order
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.end, self.start, self.chrom2, self.start2, self.end2)
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            # trigger a warning
            it = self.pr.querys2D(query)
            # verify some things about the warning
            assert len(w) == 1
            assert issubclass(w[-1].category, pypairix.PairixWarning)


## 2D query on 2D indexed file with chromosomes input in reverse order
class PairixTest2D_reverse(unittest.TestCase):
    f_type = find_pairs_type(TEST_FILE_2D)
    regions = read_pairs(TEST_FILE_2D, f_type)
    chrom2 = '10'
    start2 = 1
    end2 = 1000000
    chrom = '20'
    start = 50000000
    end = 60000000
    # reverse reversed results to get them in the required order here
    result = get_result_2D(regions, chrom2, start2, end2, chrom, start, end)
    pr = pypairix.open(TEST_FILE_2D)

    def test_query2_rev(self):
        # 1 is included as last argument to test flipping chromosome order
        it = self.pr.query2D(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2, 1)
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)

    def test_querys_2_rev(self):
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        # 1 is included as last argument to test flipping chromosome order
        it = self.pr.querys2D(query, 1)
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)

    def test_query2_rev_fail(self):
        # do not include 1 to test flipped order of chrs; expect this to hit a PairixWarning
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            # trigger a warning
            it = self.pr.query2D(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
            # verify some things about the warning
            assert len(w) == 1
            assert issubclass(w[-1].category, pypairix.PairixWarning)


## 2D query on 2D indexed file with chromosomes using a 4DN pairs file
class PairixTest2D_4DN(unittest.TestCase):
    f_type = find_pairs_type(TEST_FILE_2D_4DN)
    regions = read_pairs(TEST_FILE_2D_4DN, f_type)
    chrom = 'chr21'
    start = 1
    end = 48129895
    chrom2 = 'chr22'
    start2 = 1
    end2 = 51304566
    # reverse reversed results to get them in the required order here
    result = get_result_2D(regions, chrom, start, end, chrom2, start2, end2)
    pr = pypairix.open(TEST_FILE_2D_4DN)

    def test_query2_4dn(self):
        it = self.pr.query2D(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)

    def test_querys_2_4dn(self):
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        it = self.pr.querys2D(query)
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)


## 2D query on 2D indexed space-delimited file
class PairixTest2DSpace(unittest.TestCase):
    f_type = find_pairs_type(TEST_FILE_2D_SPACE, ' ')
    regions = read_pairs(TEST_FILE_2D_SPACE, f_type, ' ')
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
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)

    def test_querys_2(self):
        query = '{}:{}-{}|{}:{}-{}'.format(self.chrom, self.start, self.end, self.chrom2, self.start2, self.end2)
        it = self.pr.querys2D(query)
        pr_result = build_it_result(it, self.f_type)
        self.assertEqual(self.result, pr_result)


class PairixTestBlocknames(unittest.TestCase):

    def test_blocknames(self):

        # block list obtained from get_blocknames()
        pr = pypairix.open(TEST_FILE_2D)
        retrieved_blocklist = pr.get_blocknames()
        retrieved_blocklist.sort()

        # true block list
        blocklist=[]
        f_type = find_pairs_type(TEST_FILE_2D)
        regions = read_pairs(TEST_FILE_2D, f_type)
        for a in regions:
            blocklist.append(a[0] + '|' + a[3])
        blocklist_uniq = list(set(blocklist))
        blocklist_uniq.sort()

        self.assertEqual(retrieved_blocklist, blocklist_uniq)


class PairixTestGetColumnIndex(unittest.TestCase):

    def test_columnindex(self):
        pr = pypairix.open(TEST_FILE_2D)
        pr2 = pypairix.open(TEST_FILE_2D_4DN)

        self.assertEqual(pr.get_chr1_col(),1)
        self.assertEqual(pr.get_chr2_col(),5)
        self.assertEqual(pr.get_startpos1_col(),2)
        self.assertEqual(pr.get_startpos2_col(),6)
        self.assertEqual(pr.get_endpos1_col(),2)
        self.assertEqual(pr.get_endpos2_col(),6)

        self.assertEqual(pr2.get_chr1_col(),1)
        self.assertEqual(pr2.get_chr2_col(),3)
        self.assertEqual(pr2.get_startpos1_col(),2)
        self.assertEqual(pr2.get_startpos2_col(),4)
        self.assertEqual(pr2.get_endpos1_col(),2)
        self.assertEqual(pr2.get_endpos2_col(),4)


class PairixTestExists(unittest.TestCase):

    def test_exists(self):
        pr = pypairix.open(TEST_FILE_2D_4DN)
        self.assertEqual(pr.exists("chr21|chr21"),1)
        self.assertEqual(pr.exists("chr21|chr22"),1)
        self.assertEqual(pr.exists("chr22|chr22"),1)
        self.assertEqual(pr.exists("chr22|chr21"),0)
        self.assertEqual(pr.exists("chr1|chr2"),0)
        self.assertEqual(pr.exists("chr21"),0)
        self.assertEqual(pr.exists("1|2"),0)


if __name__ == '__main__':
    unittest.main()
