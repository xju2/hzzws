#!/usr/bin/env python

import ROOT
ROOT.gROOT.SetBatch()
import AtlasStyle
import low_mass
ROOT.gROOT.LoadMacro("/afs/cern.ch/user/x/xju/tool/loader.c") 
#ROOT.gROOT.LoadMacro("/afs/cern.ch/user/x/xju/tool/AtlasUtils.C") 

br = ROOT.BranchInfo()
br.name_ = "m4l_constrained"
br.n_ = 180
br.low_ = 110.0
br.high_ = 200.0
tree_name = "tree_incl_all"
def draw_m4l_for_channels(file_name):
    chain = ROOT.TChain(tree_name, tree_name)
    chain.Add(file_name)
    canvas = ROOT.TCanvas("canvas", "canvas", 600, 600)
    canvas.SetLogy()
    cut1 = ROOT.TCut("weight*(event_type==0)")
    cut2 = ROOT.TCut("weight*(event_type==1)")
    cut3 = ROOT.TCut("weight*(event_type==2)")
    cut4 = ROOT.TCut("weight*(event_type==3)")
    h1 = ROOT.draw_hist_from_chain(chain, cut1, "h_4mu", br)
    h2 = ROOT.draw_hist_from_chain(chain, cut2, "h_4e", br)
    h3 = ROOT.draw_hist_from_chain(chain, cut3, "h_2mu2e", br)
    h4 = ROOT.draw_hist_from_chain(chain, cut4, "h_2e2mu", br)
    h1.Scale(1./h1.Integral())
    h2.Scale(1./h2.Integral())
    h3.Scale(1./h3.Integral())
    h4.Scale(1./h4.Integral())
    h1.SetXTitle(br.name_)
    #h1.GetYaxis().SetRangeUser(0., 0.05)
    h1.GetYaxis().SetRangeUser(1e-4, 1e2)
    color = 2
    h1.SetLineColor(color)
    color += 1
    h2.SetLineColor(color)
    h2.SetLineStyle(2)
    color += 1
    h3.SetLineColor(color)
    color += 2
    h4.SetLineColor(color)
    h4.SetLineStyle(2)
    legend = ROOT.myLegend(0.6, 0.7, 0.9, 0.9)
    legend.AddEntry(h1, h1.GetName(), "F")
    legend.AddEntry(h2, h2.GetName(), "F")
    legend.AddEntry(h3, h3.GetName(), "F")
    legend.AddEntry(h4, h4.GetName(), "F")
    h1.Draw()
    h2.Draw("same")
    h3.Draw("same")
    h4.Draw("same")
    legend.Draw()
    canvas.SaveAs("test_m4l.eps")

def draw_m4l_for_files(file_list):
    canvas = ROOT.TCanvas("canvas", "canvas", 600, 600)
    template = ROOT.TH1F("template", "m4l template", 600, 110, 140)
    hist_list = []
    color = 2
    for f in file_list:
        chain = ROOT.TChain(tree_name, tree_name)
        chain.Add(f)
        name = f.split('.')[0]
        hist = template.Clone("h_"+name)
        chain.Draw("m4l_constrained>>"+hist.GetName(), "weight")
        hist.SetLineColor(color)
        hist_list.append(hist)
        color += 1

    maxy = -1
    print len(hist_list)," histograms"
    for hist in hist_list:
        if maxy < hist.GetMaximum(): maxy = hist.GetMaximum()

    h1 = hist_list[0]
    h1.GetYaxis().SetRangeUser(0, maxy*1.2)
    h1.SetXTitle("m_{4L} constrained")
    ncount = 0
    for hist in hist_list:
        if ncount == 0:
            hist.Draw()
        else:
            hist.Draw('same')
        ncount += 1
    canvas.SaveAs("m4l_shifted.pdf")


def compare_two_list(l1, l2):
    for h1,h2 in zip(l1, l2):
        ROOT.compare_hists(h1,h2,"m_{4l}", "parametrized","shifted", True)

def draw_m4l_from_parametrized():
    """f1: parametrized workspace, f2: shifted histogram"""
    f1_name = "/afs/cern.ch/user/x/xju/work/h4l/workspace/mc15_13TeV_v3/lowmass_mH/combined.root"
    #f2_name = "test_all_125.09.root"
    f2_name = "test_ggH_125p9.root"
    f1 = ROOT.TFile.Open(f1_name,'read')
    f2 = ROOT.TFile.Open(f2_name, "read")
    ws = f1.Get("combined")
    simPdf = ws.obj("simPdf")
    m4l = ws.obj("m4l")
    category = simPdf.indexCat()
    mH = ws.var('mH')
    mH.setVal(125.09)
    cat_iter = ROOT.TIter(category.typeIterator())
    obj = cat_iter()

    hist_list = []
    hist2_list = []
    cat_name_list = []
    while obj:
        cat_name = obj.GetName()
        new_name = cat_name.replace("Cat","")
        pdf = simPdf.getPdf(cat_name)
        ggH_pdf_name = "ATLAS_Signal_ggH_"+new_name+"_Para"
        pdf = ws.obj(ggH_pdf_name)
        h_gen = pdf.createHistogram("h_"+new_name, m4l, ROOT.RooFit.Binning(60, 110, 140));
        hist_list.append(h_gen)
        hist_name = "m4l_"+new_name
        print hist_name
        h2 = f2.Get(hist_name)
        hist2_list.append(h2)
        cat_name_list.append(new_name)
        obj = cat_iter()
    compare_two_list(hist_list, hist2_list)


if __name__ == "__main__":
    #draw_m4l_for_channels(low_mass.samples_bkg["qqZZ"])
    #draw_m4l_for_files(["signal_mH125.root", "signal_mH125_09.root"])
    #draw_m4l_for_channels(low_mass.samples_bkg["ggZZ"])
    draw_m4l_from_parametrized()
