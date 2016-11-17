#!/usr/bin/env python
# Written by Soo Lee and Carl Vitzthum
# This code is based on the following open-source projects:
# pytabix (https://github.com/slowkow/pytabix)
# pysam (https://github.com/pysam-developers)
# The Github repo for this project is:
# https://github.com/4dn-dcic/pairix

from setuptools import setup, find_packages, Extension

EXT_MODULES = [
    Extension("pairix",
        sources=[
            "bgzf.c", "bgzip.c", "index.c",
            "knetfile.c", "kstring.c",
            "python/pairixmodule.c"
        ],
        include_dirs=["python"],
        libraries=["z"],
        define_macros=[("_FILE_OFFSET_BITS", 64), ("_USE_KNETFILE", 1)]
    )
]

setup(
    name = "pypairix",
    version = "0.0.1",
    description = "Python interface for pairix",
    url = "https://github.com/4dn-dcic/pairix",
    download_url = "https://github.com/4dn-dcic/pairix/tarball/0.0.1",
    author = "Soo Lee",
    author_email = "duplexa@gmail.com",
    license = "...",
    keywords = ["pairix","tabix", "bgzip", "bioinformatics", "genomics","hi-c"],
    packages = find_packages(),
    package_data = { "": ["*.gz", "*.gz.px2"] },
    ext_modules = EXT_MODULES,
    test_suite = "test",
    classifiers = [
        "Programming Language :: Python",
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: C",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Scientific/Engineering :: Bio-Informatics"
    ]
)
