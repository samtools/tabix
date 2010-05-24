#!/usr/bin/env python

from ctypes import *
from ctypes.util import find_library
import glob, platform

def load_shared_library(lib, _path='.', ver='*'):
    # find from the system path
    path = find_library(lib)
    if (path == None): # if fail, search in the custom directory
        s = platform.system()
        if (s == 'Darwin'):
            suf = ver+'.dylib'
        elif (s == 'Linux'):
            suf = '.so'+ver
        candidates = glob.glob(_path+'/lib'+lib+suf);
        if (len(candidates) == 1):
            path = candidates[0]
        else:
            return None
    cdll.LoadLibrary(path)
    return CDLL(path)

def tabix_init():
    tabix = load_shared_library('tabix')
    if (tabix == None):
        return None
    tabix.ti_read.restype = c_char_p
    # on Mac OS X 10.6, the following declarations are required.
    tabix.ti_open.restype = c_void_p
    tabix.ti_querys.argtypes = [c_void_p, c_char_p]
    tabix.ti_querys.restype = c_void_p
    tabix.ti_read.argtypes = [c_void_p, c_void_p, c_void_p]
    tabix.ti_iter_destroy.argtypes = [c_void_p]
    tabix.ti_close.argtypes = [c_void_p]
    # FIXME: explicit declarations for APIs not used in this script
    return tabix

# command-line interface

import sys

def tabix_cmd(fn, reg=None, fnidx=0):
    tabix = tabix_init()
    if (tabix == None):
        print "ERROR: Please make sure the shared library is compiled and available."
        sys.exit(1)
    t = tabix.ti_open(fn, fnidx) # open file
    iter = tabix.ti_querys(t, reg) # query
    if (iter == None):
        print "ERROR: Fail to locate the region."
        sys.exit(1)
    while (1):
        s = tabix.ti_read(t, iter, 0)
        if (s == None):
            break
        print s    
    tabix.ti_iter_destroy(iter) # deallocate
    tabix.ti_close(t) # close file
    
if (len(sys.argv) < 3):
    print "Usage: tabix.py <in.gz> <reg>"
    sys.exit(1)
tabix_cmd(sys.argv[1], sys.argv[2])
