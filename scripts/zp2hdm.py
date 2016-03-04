#!/usr/bin/env python
import common
import glob

binning = "60, 110, 140"
branch = "m4l_constrained, "+binning

###in workspace
obs_binning = binning

# key: category name
# value: TCut on mini-tree
met_cut = 100
#categories = {"ggF_4mu_13TeV" : "(event_type ==0 && met_et > "+str(met_cut)+")", 
#              "ggF_2mu2e_13TeV" : "(event_type==1 && met_et >"+str(met_cut)+")",
#              "ggF_2e2mu_13TeV" : "(event_type==2)&& met_et >"+str(met_cut)+")",
#              "ggF_4e_13TeV" : "(event_type==3)&& met_et >"+str(met_cut)+")",
#             }
categories = {
    "hi_met" : "(met_et > 100 && m4l_constrained > 110 && m4l_constrained < 140)",
    "low_met" : "(met_et <= 100&& m4l_constrained > 110 && m4l_constrained < 140)"
             }

samples_lowmass_sig125 = {
}

masses = [600, 800, 1400]
dm_list = [300, 400, 500, 600, 700, 800, ]

mass_points = len(masses)
def get_mass(im):
    return masses[im]


m_dm = 1
dm_points = len(dm_list)
def get_dm(im):
    return dm_list[im]


sig_samples = ["zp2hdm"]
pp = "mzp" #mzp ms
xx = "mA"

bkg_samples = ["ggH", "VBFH", "ZH", "WH",
               "ZHlvlv", "ZHllvv", "qqZZ"
              ]
samples = sig_samples + bkg_samples
def get_sample_dict(mass):
    tmp_res = {}
    sample_list = sig_samples
    for sample_name in sample_list:
        pattern = common.minitree_dir+"*"+sample_name+"*"+pp+str(mass)+"_"+xx+str(m_dm)+".root"
        #print pattern
        file_list = glob.glob(pattern)
        #print mass,len(file_list)
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
    "ggH":common.minitree_dir+"mc15_13TeV.341505.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_ZZ4lep_noTau.root", 
    "VBFH":common.minitree_dir+"mc15_13TeV.341518.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_ZZ4lep_noTau.root", 
    "WH":common.minitree_dir+"mc15_13TeV.341964.Pythia8EvtGen_A14NNPDF23LO_WH125_ZZ4l.root", 
    "ZH":common.minitree_dir+"mc15_13TeV.341947.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZ4l.root", 
    "ZHlvlv": "/afs/cern.ch/work/x/xju/h4l/workspace/mc15_13TeV_v2/monoH/combined_ZHlvlv.root",
    "ZHllvv":common.minitree_dir+"mc15_13TeV.341974.Pythia8EvtGen_A14NNPDF23LO_ZH125_ZZllvv.root",
    "qqZZ":common.minitree_dir+"mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.root", 
    #"ggZZ":None
}

def print_samples():
    for sample,add in samples_bkg.iteritems():
        print sample,add

    for im in range(mass_points):
        mass = get_mass(im)
        for sample,add in samples_sig[str(mass)].iteritems():
            print sample,add

samples_sig_scale = 1.0
samples_bkg_scale = 1.0
#data = "/afs/cern.ch/user/x/xju/work/monoH4l/shape/mc15_v2/data.list"
data = "/afs/cern.ch/user/x/xju/work/h4l/workspace/mc15_13TeV_v2/monoH/data_1.7fb.root"
if __name__ == "__main__":
    print_samples() 
