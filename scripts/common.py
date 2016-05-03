#!/usr/bin/env python
import glob
import os
import ROOT

minitree_dir = \
        "/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc/Nominal/"
minitree_sys = \
        "/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc/Systematics/"
minitree_bkg = \
        "/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc/Background/CR/"

data = minitree_dir+"../../data15_grl_v73.root"
tree_name = "tree_incl_all"

#new minitree for shape systematics 
#e.g. __1down, __1up
shape_sys_list = [
    ("EG_RESOLUTION_ALL","ATLAS_EL_RES"), 
    ("EG_SCALE_ALL", "ATLAS_EL_ESCALE"),
    ("JET_GroupedNP_1", "ATLAS_JES_G1"),
    ("JET_GroupedNP_2", "ATLAS_JES_G2"),
    ("JET_GroupedNP_3", "ATLAS_JES_G3"),
    ("MUONS_ID", "ATLAS_MU_MS_RES_ID"),
    ("MUONS_MS", "ATLAS_MU_MS_RES_MS"),
    ("MUONS_SCALE", "ATLAS_MU_ESCALE"),
    ("JET_JER_SINGLE_NP", "ATLAS_JER"),
    #("MET_SoftTrk_Scale","ATLAS_MET_SoftTrk_Scale"),
    #("MET_SoftTrk_Reso","ATLAS_MET_SoftTrk_Reso"),
]

## addtional weight for normalization systematics
## e.g. weight_MUON_EFF_SYS__1up, weight_MUON_EFF_SYS__1down 
norm_sys_list = [
    ("EL_EFF_ID_TotalCorrUncertainty", "ATLAS_EL_EFF_ID"),
    ("EL_EFF_Reco_TotalCorrUncertainty", "ATLAS_EL_EFF_RECO"),
    ("EL_EFF_Iso_TotalCorrUncertainty", "ATLAS_EL_EFF_ISO"),
    ("MUON_EFF_STAT", "ATLAS_MU_EFF_STAT"),
    ("MUON_EFF_SYS", "ATLAS_MU_EFF_SYS"),
    ("MUON_EFF_STAT_LOWPT", "ATLAS_MU_EFF_STAT_LOWPT"),
    ("MUON_EFF_SYS_LOWPT", "ATLAS_MU_EFF_SYS_LOWPT"),
    ("MUON_ISO_STAT", "ATLAS_MU_ISO_STAT"),
    ("MUON_ISO_SYS", "ATLAS_MU_ISO_SYS"),
    ("MUON_TTVA_STAT", "ATLAS_MU_TTVA_STAT"),
    ("MUON_TTVA_SYS", "ATLAS_MU_TTVA_SYS"),
    ("PRW_DATASF", "ATLAS_PRW_DATASF"),
    #("HOEW_syst", "ATLAS_HOEW"),
]

met_sys_list = [
    ("met_et_MET_SoftTrk_Reso", "ATLAS_MET_SoftTrk_Reso"),
    ("met_et_MET_SoftTrk_Scale", "ATLAS_MET_SoftTrk_Scale"),
]

#smooth_cmd = os.getenv("HZZWSCODEDIR")+"/bin/smooth"
#pvalue_cmd = os.getenv("HZZWSCODEDIR")+"/bin/pvalue"

# key: sample name
# value: 1, pattern for searching mini-trees
#        2, tag for if it's signal 
#        3, nick name used for pdf
samples = {
       "ggH" : ["ggH", True, "ATLAS_Signal_ggH"], 
       "VBFH" : ["VBFH", True, "ATLAS_Signal_VBFH"],
       "WH" :  ["WH", True, "ATLAS_Signal_WH"],
       "ZH" :  ["ZH", True, "ATLAS_Signal_ZH"],
       "ttH" :  ["ttH", True, "ATLAS_Signal_ttH"],
       "qqZZ" : ["_ZZ_", False, "ATLAS_Bkg_qqZZ"],
       "ggZZ" : ["ggllll", False, "ATLAS_Bkg_ggZZ"],
       "Zjets" : ["redBkg", False, "ATLAS_Bkg_Zjets"],
       "VVV": ["VVV", False, "ATLAS_Bkg_VVV"], 
      }

samples_monoH = {
       "ggH" : ["ggH", False, "ATLAS_Signal_ggH"], 
       "VBFH" : ["VBFH", False, "ATLAS_Signal_VBFH"],
       "WH" :  ["WH", False, "ATLAS_Signal_WH"],
       "ZH" :  ["ZH", False, "ATLAS_Signal_ZH"],
       "ttH" :  ["ttH", False, "ATLAS_Signal_ttH"],
       "qqZZ" : ["_ZZ_", False, "ATLAS_Bkg_qqZZ"],
       "ggZZ" : ["ggllll", False, "ATLAS_Bkg_ggZZ"],
       "Zjets" : ["redBkg", False, "ATLAS_Bkg_Zjets"],
        "ZHlvlv" : ["ZHlvlv", False, "ATLAS_Signal_ZHlvlv"],
        "ZHllvv" : ["ZHllvv", False, "ATLAS_Signal_ZHllvv"],
        "zphxx" : ["zphxx", True, "ATLAS_BSM_zp"],
        "shxx" : ["shxx", True, "ATLAS_BSM_scalar"]
      }

def get_shape_downup(file_name, shape_sys):
    global minitree_sys
    if not os.path.isfile(file_name):
        print file_name,"not found"
        return None
    base_name = os.path.basename(file_name)
    file_down = minitree_sys+shape_sys+"__1down/"+base_name
    file_up = minitree_sys+shape_sys+"__1up/"+base_name
    if "MET_SoftTrk_Scale" in shape_sys:
        file_down = minitree_sys+shape_sys+"Down/"+base_name
        file_up = minitree_sys+shape_sys+"Up/"+base_name
    elif "MET_SoftTrk_Reso" in shape_sys:
        file_down = minitree_sys+shape_sys+"Para/"+base_name
        file_up = minitree_sys+shape_sys+"Perp/"+base_name
    return (file_down, file_up)

if __name__ == "__main__":
    print minitree_dir
