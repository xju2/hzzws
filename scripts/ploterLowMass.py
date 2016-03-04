#!/usr/bin/env python

import ROOT
import AtlasStyle
if not hasattr(ROOT, 'loader'):
    ROOT.gROOT.LoadMacro("/afs/cern.ch/user/x/xju/tool/loader.c") 

class Ploter:
    def __init__(self):
        print "I can help with ploting"

    def plot_mu_scan(self, file_name, tree_name="physics");
        chain = ROOT.loader(file_name, tree_name)        

def compare_
