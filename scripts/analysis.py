#!/usr/bin/env python
"""
define a analysis
"""
import sample
import common
import category
class Analysis:
    def __init__(self, name, categories, signal, background):
        self.name = name
        self.categories = categories
        self.signal = signal
        self.background = background
        self.samples = signal + background 

    def __str__(self):
        return "{} {} categories and {} samples".\
                format(self.name, len(self.categories), len(self.samples))

LOWMASS = Analysis(
    name="LowMass",
    categories = [
        category.ggF_4mu_13TeV,
        category.ggF_2mu2e_13TeV,
        category.ggF_2e2mu_13TeV,
        category.ggF_4e_13TeV,
    ],
    signal = [sample.ggH, sample.VBF, sample.WH, sample.ZH, sample.ttH],
    background = [sample.qqZZ_Low, sample.ggZZ, sample.Zjets],
)

if __name__ == "__main__":
    print LOWMASS
    print ",".join([str(x) for x in LOWMASS.categories])
