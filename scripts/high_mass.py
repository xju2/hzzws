#!/usr/bin/env python
import common
import glob
binning = "212, 140, 1200"
branch = "m4l_constrained, "+binning

###in workspace
obs_binning = binning

# key: category name
# value: TCut on mini-tree
categories = {"ggF_4mu_13TeV" : "(event_type ==0)", 
              "ggF_2mu2e_13TeV" :"((event_type==3 || event_type==2))",
              "ggF_4e_13TeV" : "(event_type==1)",

#              "VBF_13TeV" : "(prod_type==1)",
#              "VHLep_13TeV" : "(prod_type==2)" ,
#              "VHHad_13TeV" : "(prod_type==3)"
             }
#categories = {"all": "(1==1)"}

#sample information
#sig_samples_gt_400 = ["ggH", "VBFH"]
#sig_samples = ["ggH", "VBFH", "ZH", "WH"]
sig_samples_gt_400 = ["ggH"]
sig_samples = ["ggH"]
bkg_samples = ["qqZZ", 
               "ggZZ",
               "VVV",
              ]
samples = sig_samples + bkg_samples
samples_gt_400 = sig_samples_gt_400 + bkg_samples
samples_para = samples_gt_400

mass_points = 18
def get_mass(im):
    if im < 9:
        mass = 200 + im*100
    elif im < 16:
        mass = 1000 + (im-8)*200
    elif im == 17:
        mass = 3000
    else:
        mass = 2400 + (im-15)*400
    return mass

def get_signal_dict():
    tmp_dic = {}
    for im in range(mass_points):    
        mass = get_mass(im)
        tmp_dic[str(mass)] = get_sample_dict(mass) 
    return tmp_dic

def get_sample_dict(mass):
    tmp_res = {}
    if mass <= 400: sample_list = sig_samples
    else: sample_list = sig_samples_gt_400
    for sample_name in sample_list:
        pattern = common.minitree_dir+"*"+sample_name+str(mass)+"NW*.root"
        file_list = glob.glob(pattern)
        #print im,mass,len(file_list)
        if len(file_list) == 1:
            tmp_res[sample_name] = file_list[0]
    return tmp_res

samples_sig = get_signal_dict()
samples_bkg = {
    #"qqZZ":common.minitree_dir+"combined/mc15_qq2ZZ.root",
    "qqZZ":"/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc_15b/Nominal/mc15_qq2ZZ.root",
    "ggZZ":common.minitree_dir+"mc15_13TeV.361073.Sherpa_CT10_ggllll.root",
    "VVV":"/afs/cern.ch/user/x/xju/work/h4l/workspace/mc15_13TeV_v4/highmass/VVV.list",
}
samples_sig_scale = -999.0
samples_bkg_scale = 1.0
data = common.minitree_dir+"../../data15_grl_v73.root"
