#!/usr/bin/env python

import os
import sys


def glue_contigs(outdir):
    res = outdir + "/final_metaplasmid.fasta"
    res_f = open(res, "w")
    for file in os.listdir(outdir):
        farr = file.split('.')
        if farr[-1] != "fasta":
            continue
        if farr[-2] != "circular":
            continue
        arr = farr[-3].split("_")
        if len(arr) < 2:
            continue
        cov = arr[-1]

  #  for line in open(os.path.join(dir,file), "r"):
        for line in open(os.path.join(outdir,file), "r"):
            line = line.strip()
            if len(line) > 0 and line[0] == ">":
                line += "_cutoff_" + cov
            res_f.write(line+ "\n")

