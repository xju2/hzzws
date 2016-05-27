#!/usr/bin/env python
import common
import glob

name = "Low"
binning = "60, 110, 140"
branch = "m4l_constrained, "+binning

###in workspace
obs_binning = binning

# key: category name
# value: TCut on mini-tree
categories = {
    "ggF_4mu_13TeV" : "(event_type==0)",
    "ggF_2mu2e_13TeV" : "(event_type==2)",
    "ggF_2e2mu_13TeV" : "(event_type==3)",
    "ggF_4e_13TeV" : "(event_type==1)",
}

sig_samples = ["ggH", "VBFH", "ZH", "WH", "ttH"]
bkg_samples = ["qqZZ", "Zjets", "ggZZ"]
samples = sig_samples + bkg_samples
samples_para = samples

samples_lowmass_sig125 = {
    "ggH":common.minitree_dir+"mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.root",
    "VBFH":common.minitree_dir+"mc15_13TeV.341518.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_ZZ4lep_noTau.root",
    "WH":common.minitree_dir+"mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.root",
    "ZH":common.minitree_dir+"mc15_13TeV.341947.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZ4l.root",
    "ttH":common.minitree_dir+"mc15_13TeV.342561.aMcAtNloHerwigppEvtGen_UEEE5_CTEQ6L1_CT10ME_ttH125_4l.root",
}

masses = [124, 125, 126]
#masses = [125]
mass_points = len(masses)
def get_mass(im):
    return masses[im]

def get_sample_dict(mass):
    tmp_res = {}
    sample_list = sig_samples
    for sample_name in sample_list:
        pattern = common.minitree_dir+"*"+sample_name+str(mass)+"_*4l*.root"
        file_list = glob.glob(pattern)
        #print mass,len(file_list), file_list
        if len(file_list) == 1:
            tmp_res[sample_name] = file_list[0]
        elif len(file_list) == 2:
            for ff in file_list:
                if "noTau" in ff:
                    tmp_res[sample_name] = ff
    return tmp_res

def get_signal_dict():
    tmp_dic = {}
    for im in range(mass_points):
        mass = get_mass(im)
        tmp_dic[str(mass)] = get_sample_dict(mass)
    return tmp_dic 

samples_sig = get_signal_dict()
samples_bkg = {
    "qqZZ":"/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc_15b/Nominal/mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.root",
    "Zjets":"/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v01/mc/Nominal/combined/mc15_redBkg_filtered.root",
    "ggZZ":common.minitree_dir+"mc15_gg2ZZ_low.root",
}

def print_samples():
    for sample,add in samples_bkg.iteritems():
        print sample,add

    for sample,add in samples_sig["125"].iteritems():
        print sample,add

#print_samples()
samples_sig_scale = 1.0
samples_bkg_scale = 1.0

data = common.minitree_dir+"../../data15_grl_v73.root"
if __name__ == "__main__":
    print_samples()
