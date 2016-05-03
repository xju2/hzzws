#!/usr/bin/env python
import common
import os
class Sample:
    """
    define samples
    """
    def __init__(self, name, nick, samples, rho=1):
        self.name = name
        self.nick = nick
        self.samples = samples
        self.rho = rho  # smoothing parameter

    def __eq__(self, other):
        return (isinstance(other, self.__class__)\
                and self.name == other.name)

    def __repr__(self):
        return "{__class__.__name__}(name={name!r}, samples={samples!r}, nick={nick!r}, rho={rho!r})".\
                format(__class__=self.__class__, **self.__dict__)

    def __str__(self):
        return "{name}".format(**self.__dict__)

    def get_file(self, mass, do_base=False):
        """
        get file path for specific mass point, in case of background, it's -1
        """
        sample_ = ""
        try:
            sample_ = self.samples[mass]
        except KeyError:
            print mass,"not in", self.name
            if do_base:
                print "try mass -1"
                sample_ = self.get_file(-1, False)
        return sample_

    def check(self):
        """
        check if files are exists
        """
        print "Checking", self.name
        bad_files = []
        for value, file_ in self.samples.iteritems():
            if not os.path.exists(file_):
                print value,"does not exist!"
                bad_files.append(file_)
        if len(bad_files) > 0:
            print "Here are bad files"
            print "\n".join([a for a in bad_files])
        else:
            print "all files exist"
        return len(bad_files)

base = common.minitree_dir
qqZZ_Low = Sample(name="qqZZ", nick="ATLAS_Bkg_qqZZ",
                  samples={-1:"/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc_15b/Nominal/mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.root"}
                 )
ggZZ = Sample(name="ggZZ", nick="ATLAS_Bkg_ggZZ",
              samples={-1:base+"mc15_gg2ZZ_low.root"}
             )
Zjets = Sample(name="Zjets", nick="ATLAS_Bkg_Zjets",
               samples={-1:"/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v01/mc/Nominal/combined/mc15_redBkg_filtered.root"}
              )

ggH_dict = {
    124:base+"mc15_13TeV.341504.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH124_ZZ4lep_noTau.root",
    125:base+"mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.root",
    126:base+"mc15_13TeV.341506.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH126_ZZ4lep_noTau.root",
}
ggH = Sample(name="ggH", nick="ATLAS_Signal_ggH", samples=ggH_dict)

# VBF samples
VBF_dict = {
    124:base+"mc15_13TeV.341515.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH122_ZZ4lep_noTau.root",
    125:base+"mc15_13TeV.341518.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_ZZ4lep_noTau.root",
    126:base+"mc15_13TeV.341519.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH126_ZZ4lep_noTau.root"
}
VBF = Sample(name="VBFH", nick="ATLAS_Signal_VBFH", samples=VBF_dict)

# WH samples
WH_dict = {
    124:base+"mc15_13TeV.341963.Pythia8EvtGen_A14NNPDF23LO_WH124_ZZ4l.root",
    125:base+"mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.root",
    126:base+"mc15_13TeV.341965.Pythia8EvtGen_A14NNPDF23LO_WH126_ZZ4l.root"
}
WH = Sample(name="WH", nick="ATLAS_Signal_WH", samples=WH_dict)

# ZH samples
ZH_dict = {
    124:base+"mc15_13TeV.341946.Pythia8EvtGen_A14NNPDF23LO_ZH124_ZZ4l.root",
    125:base+"mc15_13TeV.341947.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZ4l.root",
    126:base+"mc15_13TeV.341948.Pythia8EvtGen_A14NNPDF23LO_ZH126_ZZ4l.root"
}
ZH = Sample(name="ZH", nick="ATLAS_Signal_ZH", samples=ZH_dict)

# ttH samples
ttH_dict = {
    124:base+"mc15_13TeV.342565.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH124_4l.root",
    125:base+"mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.root",
    126:base+"mc15_13TeV.342566.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH126_4l.root",
}
ttH = Sample(name="ttH", nick="ATLAS_Signal_ttH", samples=ttH_dict)

if __name__ == "__main__":
    print ggH
    ggH.check()
