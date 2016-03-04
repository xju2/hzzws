#!/usr/bin/env python
import ROOT
import common
import high_mass
import os

cats =  high_mass.categories.keys()
def get_m4l(file_name):
    res = []
    f1 = ROOT.TFile.Open(file_name, "read")
    for cat in cats:
        hist_name = "m4l_"+cat
        h1 = f1.Get(hist_name)
        h1.SetDirectory(0)
        h1.Scale(1./h1.Integral())
        #print h1.GetName()
        res.append(h1)
    #print "total: ", len(res)
    f1.Close()
    return res

def compare_highmass_signal(sample):
    ROOT.gROOT.SetBatch()
    all_hists = []
    for im in range(common.highmass_points):
        mass = common.get_highmass(im)
        file_name = "test_"+sample+"_"+str(mass)+".root"
        if not os.path.isfile(file_name):
            continue
        #print file_name," not exist"
        hists = get_m4l(file_name)
        all_hists.append(hists)

    #start to plot
    ncount = 0
    for cat in cats:
        canvas = ROOT.TCanvas("c"+cat, "c1", 600, 600)
        max_y = 1. 
        nhist = 0
        color = 2
        for hist_list in all_hists:
            h1 = hist_list[ncount]
            if not h1:
                continue
            h1.SetLineColor(color)
            color += 1
            if nhist == 0:
                nhist += 1
                h1.GetYaxis().SetRangeUser(0,max_y)
                h1.Draw()
            else:
                h1.Draw("same")
        canvas.SaveAs("pdf/"+sample+"_"+cat+".pdf")
        ncount += 1

if __name__ == '__main__':
    for sample in common.sig_sample_names:
        compare_highmass_signal(sample)
