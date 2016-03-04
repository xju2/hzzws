#!/usr/bin/env python
import os
import ROOT
ROOT.gROOT.SetBatch()
ROOT.gROOT.LoadMacro("/afs/cern.ch/user/x/xju/tool/loader.c") 

import low_mass
import monoH

config = monoH
def change(file_name):
    global config
    f1 = ROOT.TFile.Open(file_name, "read")
    h1 = f1.Get('Reducible_bkg_2D_all_m4l__m4l_constrained')
    hist_list = []
    for cat_name in config.categories.keys():
        hist_list.append(h1.Clone("m4l_"+cat_name))

    fout = ROOT.TFile.Open("test_Zjets.root", "recreate")
    for hist in hist_list:
        hist.Write()
    fout.Close()
    f1.Close()

def change_sys(f_low_name, f_up_name):
    global config
    f_low = ROOT.TFile.Open(f_low_name, "read")
    f_up = ROOT.TFile.Open(f_up_name, "read")
    h_low = f_low.Get('Reducible_bkg_2D_all_m4l__m4l_constrained')
    h_up = f_up.Get('Reducible_bkg_2D_all_m4l__m4l_constrained')
    hist_list = []
    for cat_name in config.categories.keys():
        hist_list.append(h_low.Clone('m4l-ATLAS_Shape_Zjet-'+cat_name+'-down'))
        hist_list.append(h_up.Clone('m4l-ATLAS_Shape_Zjet-'+cat_name+'-up'))

    fout = ROOT.TFile.Open("Zjets_Low_Shape.root", 'recreate')
    for hist in hist_list:
        hist.Write()
    fout.Close()
    f_low.Close()
    f_up.Close()

def change_all(fname):
    global config
    fall = ROOT.TFile.Open(fname, "read")
    h_low = fall.Get('Reducible_bkg_2D_all_m4l_syst_down__m4l_constrained')
    h_up = fall.Get('Reducible_bkg_2D_all_m4l_syst_up__m4l_constrained')
    h_nom = fall.Get("Reducible_bkg_2D_all_m4l__m4l_constrained")
    h_low.Scale(1./h_low.Integral())
    h_up.Scale(1./h_up.Integral())
    h_nom.Scale(1./h_nom.Integral())

    hist_list = []
    for cat_name in config.categories.keys():
        hist_down = h_low.Clone('m4l-ATLAS_Shape_Zjet-'+cat_name+'-down')
        hist_down.Divide(h_nom)
        hist_up = h_up.Clone('m4l-ATLAS_Shape_Zjet-'+cat_name+'-up')
        hist_up.Divide(h_nom)
        hist_list.append(hist_down)
        hist_list.append(hist_up)

    fout = ROOT.TFile.Open("Zjets_Low_Shape.root", 'recreate')
    for hist in hist_list:
        hist.Write()
    fout.Close()
    fall.Close()


def change_shape(f_name):
    f_in = ROOT.TFile.Open(f_name, "read")
    f_out_name = os.path.basename(f_name).replace("Low", "MonoH")
    hist_list = []
    cat_name = "ggF_4l_13TeV"
    for key in f_in.GetListOfKeys():
        h1 = key.ReadObj()
        hist_name = key.GetName()
        if cat_name in hist_name and "Nominal" not in hist_name:
            hist_list.append(h1.Clone(hist_name.replace(cat_name,"hi_met")))
            hist_list.append(h1.Clone(hist_name.replace(cat_name,"low_met")))

    f_out = ROOT.TFile.Open(f_out_name, "recreate")
    for hist in hist_list:
        hist.Write()
    f_out.Close()
    f_in.Close()

def change_shape_monoH():
    sys_dir = "/afs/cern.ch/user/x/xizhao/work/public/Results_2015-11-17_16-21/Low/"
    change_shape(sys_dir+"qqZZ_Low_Shape.root")
    change_shape(sys_dir+"ZH125_Low_Shape.root")
    change_shape(sys_dir+"WH125_Low_Shape.root")
    change_shape(sys_dir+"ggH125_Low_Shape.root")
    change_shape(sys_dir+"VBFH125_Low_Shape.root")

def change_qqZZ():
    file_name = "test_qqZZ.root"
    f_in = ROOT.TFile.Open(file_name, "read")
    h1 = f_in.Get("m4l_hi_met")
    h2 = h1.Clone("m4l_low_met")
    fout = ROOT.TFile.Open("test.root", "recreate")
    h1.Write()
    h2.Write()
    fout.Close()
    f_in.Close()

def get_qqZZ():
    f1 = "/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc_15b/Nominal/mc15_13TeV.342556.PowhegPy8EG_CT10nloME_AZNLOCTEQ6L1_ZZllll_mll4_m4l_100_150.root"
    chain = ROOT.loader(f1, "tree_incl_all")
    h1 = ROOT.TH1F("m4l_hi_met", "m4l", 60, 110, 140)
    chain.Draw("m4l_constrained>>m4l_hi_met", "weight*(m4l_constrained > 110 &&\
               m4l_constrained < 140)")
    h2 = h1.Clone("m4l_low_met")
    fout = ROOT.TFile.Open("test.root", "recreate")
    h1.Write()
    h2.Write()
    fout.Close()

if __name__ == "__main__":
    #config = monoH
    config = low_mass
    #change_sys('redBkg_Zfilt_relCR_low_shapesConstrained_ma_1.300000.root',
    #           'redBkg_Zfilt_relCR_high_shapesConstrained_ma_1.300000.root')
    #change_sys('redBkg_Zfilt_relCR_low_shapesConstrained_ma_1.300000.root',
    #           'redBkg_Zfilt_relCR_high_shapesConstrained_ma_1.300000.root')
    #change('redBkg_Zfilt_relCR_nom_shapesConstrained_ma_1.300000.root')
    #change('redBkg_shapes.root')
    change('minitree_AllBkg_shapes.root')
    #change_shape_monoH()
    #change_qqZZ()
    #get_qqZZ()
    #change_all("minitree_AllBkg_shapes.root")
