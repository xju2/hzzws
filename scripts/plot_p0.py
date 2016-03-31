#!/usr/bin/env python

import ROOT
import AtlasStyle
from array import array

ROOT.gROOT.SetBatch()
if not hasattr(ROOT, "myText"):
    ROOT.gROOT.LoadMacro("$HZZWSCODEDIR/scripts/loader.c") 

def get_graph(file_name):
    mass_list = []
    obs_list = []
    exp_list = []
    gr_obs = None
    gr_exp = None
    with open(file_name, 'r') as f:
        for line in f:
            items = line[:-1].split()
            if len(items) < 2: 
                print line[:-1]," not valid, skipped"
                continue
            mass_list.append(float(items[0]))
            obs_list.append(float(items[1]))
            if len(items) > 2:
                exp = float(items[2])
                if exp > 0:
                    exp_list.append(exp)
    
        if len(obs_list) == len(mass_list):
            gr_obs = ROOT.TGraph(len(mass_list), array('f', mass_list),
                         array('f', obs_list))

        if len(mass_list) == len(exp_list):
            gr_exp = ROOT.TGraph(len(mass_list), array('f', mass_list),
                             array('f', exp_list))

    return gr_obs, gr_exp

def plot():
    #f1_name = "scalar_pvalue_0.01.txt"
    f1_name = "scalar_pvalue_0.01_fine_steps.txt"
    f2_name = "scalar_pvalue_hongtao_0.01.txt"
     
    gr_obs1,_ = get_graph(f1_name)
    gr_obs2,_ = get_graph(f2_name)
    
    canvas = ROOT.TCanvas("canvas", "canvas", 600, 600)
    canvas.SetLogy()
    gr_obs1.Draw("LA")
    gr_obs2.Draw("L")
    gr_obs1.SetLineColor(2)
    gr_obs2.SetLineColor(4)
    gr_obs1.SetLineWidth(2)
    gr_obs2.SetLineWidth(2)
    x_title = "m_{X} [GeV]"
    gr_obs1.GetXaxis().SetTitle(x_title)
    gr_obs1.GetYaxis().SetTitle("Local #it{p}_{0}")
    gr_obs1.GetYaxis().SetRangeUser(1E-7, 200.)
    gr_obs1.GetXaxis().SetRangeUser(200, 2000.)
    gr_obs1.GetXaxis().SetNdivisions(506)

    lumi = 3.2
    x_off_title = 0.185
    ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Internal")
    #ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Preliminary")
    ROOT.myText(x_off_title, 0.80, 1, "13 TeV, {:.2f} fb^{{-1}}".format(lumi))
    tag_name = "X#rightarrow#gamma#gamma, #kappa=0.01"
    ROOT.myText(x_off_title+0.4, 0.85, 1, tag_name)
    
    tag1_name = "XY"
    tag2_name = "HT"
    legend = ROOT.myLegend(0.62, 0.40, 0.83, 0.50)
    legend.AddEntry(gr_obs1, tag1_name, "l")
    legend.AddEntry(gr_obs2, tag2_name, "l")
    legend.Draw()
    
    x_axis = gr_obs1.GetXaxis()
    _1s = ROOT.RooStats.SignificanceToPValue(1.)
    _2s = ROOT.RooStats.SignificanceToPValue(2.)
    _3s = ROOT.RooStats.SignificanceToPValue(3.)
    _4s = ROOT.RooStats.SignificanceToPValue(4.)
    _5s = ROOT.RooStats.SignificanceToPValue(5.)
    ROOT.AddLine(x_axis, _1s)
    ROOT.AddLine(x_axis, _2s)
    ROOT.AddLine(x_axis, _3s)
    ROOT.AddLine(x_axis, _4s)
    ROOT.AddLine(x_axis, _5s)

    latex = ROOT.TLatex()
    latex.SetTextSize(0.04)
    latex.SetTextFont(42)
    latex.SetNDC(0)
    latex.DrawLatex(x_axis.GetBinLowEdge(x_axis.GetNbins()+1)+0.01, _1s, "1#sigma")
    latex.DrawLatex(x_axis.GetBinLowEdge(x_axis.GetNbins()+1)+0.01, _2s, "2#sigma")
    latex.DrawLatex(x_axis.GetBinLowEdge(x_axis.GetNbins()+1)+0.01, _3s, "3#sigma")
    latex.DrawLatex(x_axis.GetBinLowEdge(x_axis.GetNbins()+1)+0.01, _4s, "4#sigma")
    latex.DrawLatex(x_axis.GetBinLowEdge(x_axis.GetNbins()+1)+0.01, _5s, "5#sigma")

    canvas.SaveAs("test.pdf")

if __name__ == "__main__":
    plot()
