#!/usr/bin/env python
import ROOT
import low_mass

qqZZ_yields = {
    "ggF_2e2mu_13TeV": 6.14E-01,
    "ggF_2mu2e_13TeV": 4.85E-01,
    "ggF_4e_13TeV": 4.34E-01,
    "ggF_4mu_13TeV": 9.39E-01
}

def get_yield_dic(yield_txt, sample_name):
    res_dict = {}
    with open(yield_txt, 'r') as f:
        iline = 0
        for line in f:
            if iline == 0:
                key_list = map(str.strip, line[:-1].split('&'))
                iline += 1
            elif sample_name in line:
                item_list = line[:-1].split('&')
                icount = 1
                for key in key_list:
                    res_dict[key] = float(item_list[icount])
                    icount += 1
    return res_dict



def add_shape(file_name, sample_name, config):
    f1 = ROOT.TFile.Open(file_name, "read")
    icount = 0
    yields = get_yield_dic("yields_13TeV.txt", sample_name)
    for cat_name in config.categories.keys():
        hist_name = "m4l_"+cat_name
        h1 = f1.Get(hist_name)
        h1.Scale(yields[cat_name]/h1.Integral())
        if icount == 0:
            h_total = h1.Clone("m4l_inclusive")
            icount += 1
        else:
            h_total.Add(h1)
    fout = ROOT.TFile.Open(file_name.replace("test","inclusive"), 
                           "recreate")
    print sample_name,h_total.Integral()
    h_total.Scale(1./h_total.Integral())
    h_total.Write()
    fout.Close()
    f1.Close()

if __name__ == "__main__":
    add_shape("test_qqZZ.root", "qqZZ", low_mass)
    add_shape("test_all_125.09.root", "all", low_mass)
    add_shape("test_Zjets.root", "Zjets", low_mass)
