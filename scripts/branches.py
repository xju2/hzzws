#!/usr/bin/env python
"""
define common branches, used for plot
"""
from array import array
import ROOT

class Branch:
    """definition of branch"""
    def __init__(self, name, x_title, binning):
        self.name = name
        self.x_title = x_title
        self.binning = binning

    def __repr__(self):
        return "{__class__.__name__}(name={name!r}, x_title={x_title!r}, binning={binning!r})".\
                format(__class__=self.__class__, **self.__dict__)

    def __str__(self):
        return "{name}".format(**self.__dict__)

    def create_hist(self, hist_name=""):
        bin_type = type(self.binning)
        num_bins = len(self.binning)
        if not hist_name:
            hist_name = self.name

        if bin_type == list:
            hist = ROOT.TH1F(hist_name, hist_name,
                                  num_bins, array('f', self.binning))
        elif bin_type == tuple:
            if num_bins != 3:
                print self.binning, " not valid!"
                return None
            nbins, low, hi = self.binning
            hist = ROOT.TH1F(hist_name, hist_name, nbins, low, hi)
        else:
            print "Don't understand binning", self.binning, " in ", self.name
            return None
        return hist

M4L = Branch(name="m4l_constrained", x_title="m_{4L} [GeV]",
               binning=(60, 110, 140))

if __name__ == "__main__":
    print BR_M4L
