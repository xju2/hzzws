#!/usr/bin/env python
import os
import AtlasStyle
import ROOT
from array import array

ROOT.gROOT.SetBatch()
if not hasattr(ROOT, "loader"):
    ROOT.gROOT.LoadMacro("/afs/cern.ch/user/x/xju/tool/loader.c") 

import monoH
import common
import dump_yields
import branch_info

font = 42
t_size = 0.04
###////////////
###// define the color
###//////////////
color_list_int = [
    ROOT.kRed, 
    ROOT.kOrange-2, ROOT.kGreen+1, ROOT.kViolet-3, ROOT.kOrange+6, 
    ROOT.kViolet-9, ROOT.kViolet-4, 
    ROOT.kAzure+2, ROOT.kAzure+5, ROOT.kAzure+6
]

higgs_Lcolor = ROOT.TColor.GetColor("#487D9E")
higgs_Fcolor = ROOT.TColor.GetColor("#74CCFF");
qqZZ_Lcolor = ROOT.TColor.GetColor("#A10000");
qqZZ_Fcolor = ROOT.TColor.GetColor("#ee0000");
zjets_Lcolor = ROOT.kGreen + 3
zjets_Fcolor = ROOT.kGreen + 1
ttbar_Lcolor = ROOT.kOrange + 4
ttbar_Fcolor = ROOT.kOrange - 3

overall_cut = ROOT.TCut("weight*(m4l_constrained > 110 && m4l_constrained < 140)")
#overall_cut = ROOT.TCut("weight")

br_info =  branch_info.met_br
#br_info = hpt_br
#br_info = m4l_br 
#br_info = rpt_br 
#br_info = dphi_br 

class Ploter:
    def __init__(self, config):
        print "Ploter is created"
        self.config = config
        self.with_data = False
        self.path =  common.minitree_dir
        self.tree_name = common.tree_name
    def scale_by_bin(self, h1):
        for ibin in range(h1.GetNbinsX()):
            width = h1.GetBinWidth(ibin+1)
            scale = width/10
            content = h1.GetBinContent(ibin+1)
            h1.SetBinContent(ibin+1, content/scale) 
    def make_hist(self, file_in, hist_name, color):
        global overall_cut
        global br_info
        file_name = file_in
        chain = ROOT.loader(file_name, self.tree_name)
        #bin_list = [0,10,20,30,40,50,60,70,80,90,100,150,200,250,300]
        #bin_list = [0,10,20,30,40,50,60,70,80,90,100,120,140,160,180,200,220,240,260,280,300]
        bin_list = [0,10,20,30,40,50,60,70,80,90,100,120,140,160,180,200,250,300]
        n_met_bins = len(bin_list)-1
        h1 = ROOT.TH1F(hist_name, hist_name, n_met_bins, array('f',bin_list))
        chain.Draw("met_et>>"+hist_name, overall_cut)
        self.scale_by_bin(h1)
        #h1 = ROOT.draw_hist_from_chain(chain, overall_cut, 
        #                               hist_name, br_info);
        if type(color) is list:
            h1.SetLineColor(color[0])
            h1.SetFillColor(color[1])
        else:
            h1.SetLineColor(color)
            h1.SetFillColor(color)
        return h1

    def get_bkg_hist(self, bkg_name):
        bkg_dir = self.config.samples_bkg[bkg_name]
        return self.make_hist(bkg_dir, "h_bkg_"+bkg_name, 1)
    
    def get_reducible_hist(self,hist_name):
        global overall_cut
        global br_info
        if "Zjets" in hist_name:
            file_name = "minitrees/mc15_Zjets_CR.root"
            tree_name = "tree_ss"
            n_exp = 0.71927
        elif "ttbar" in hist_name:
            file_name = "minitrees/mc15_ttbar_CR.root"
            tree_name = "tree_emu"
            n_exp = 0.23*0.4466
        chain = ROOT.loader(file_name, tree_name)
        bin_list = [0,10,20,30,40,50,60,70,80,90,100,120,140,160,180,200,250,300]
        n_met_bins = len(bin_list)-1
        h1 = ROOT.TH1F(hist_name, hist_name, n_met_bins, array('f',bin_list))
        chain.Draw("met_et>>"+hist_name, overall_cut)
        #h1 = ROOT.draw_hist_from_chain(
        #    chain, overall_cut, hist_name, br_info)
        h1.Scale(n_exp/h1.Integral())
        return h1

    def draw_signal_conf(self):
        add_overflow = True
        color_list = color_list_int
        lumi_weight = 3.21
        print "total lumi: ",lumi_weight
        canvas = ROOT.TCanvas("canvas", "canvas", 600, 600);
        canvas.SetLogy();
        global br_info

        if self.with_data:
            h_data = self.make_hist(self.config.data, "h_data", [1,1])
            if add_overflow:
                h_data = ROOT.DrawOverflow(h_data)

        mass = "200" 
        mass2 = "300"
        sig_sample_name = self.config.sig_samples[0]
        sig_sample_dir = common.minitree_dir+"mc15_13TeV.341748.MadGraphPythia8EvtGen_A14NNPDF23LO_zphxx_ZZ4l_mzp200_mx1.root"
        sig_sample_dir2 = common.minitree_dir+"mc15_13TeV.341784.MadGraphPythia8EvtGen_A14NNPDF23LO_shxx_ZZ4l_ms300_mx1.root"
        print sig_sample_dir
        h_sig = self.make_hist(sig_sample_dir, "h_sig_"+mass, color_list[0])
        h_sig2 = self.make_hist(sig_sample_dir2, "h_sig_"+mass2, color_list[0])
        if add_overflow:
            h_sig = ROOT.DrawOverflow(h_sig)
            h_sig2 = ROOT.DrawOverflow(h_sig2)
        #option = 'width'
        option = ''
        h_sig.Scale(1.25e-4, option)
        h_sig2.Scale(1.25e-4, option)
        h_sig.SetFillColor(0)
        h_sig2.SetFillColor(0)
        h_sig2.SetLineStyle(2)
        h_bkgs_list = ROOT.TList() 
        h_bkgs_list_no_order = ROOT.TList() 

        ncount = 1
        h_ggZZ = self.get_bkg_hist("ggZZ")
        h_qqZZ = self.get_bkg_hist("qqZZ")
        h_ZZ = h_ggZZ.Clone("h_ZZ")
        h_ZZ.Add(h_qqZZ)
        h_ZZ.SetLineColor(qqZZ_Lcolor)
        h_ZZ.SetFillColor(qqZZ_Fcolor)

        h_ggH = self.get_bkg_hist("ggH")
        h_VBFH = self.get_bkg_hist("VBFH")
        h_ZH = self.get_bkg_hist("ZH")
        h_WH = self.get_bkg_hist("WH")
        h_ttH = self.get_bkg_hist("ttH")
        h_H4l = h_ggH.Clone("h_H4l")
        h_H4l.Add(h_VBFH)
        h_H4l.Add(h_ZH)
        h_H4l.Add(h_WH)
        h_H4l.Add(h_ttH)
        h_H4l.SetLineColor(higgs_Lcolor)
        h_H4l.SetFillColor(higgs_Fcolor)

        h_ZHlvlv = self.get_bkg_hist("ZHlvlv")
        h_ZHlvlv.SetLineColor(ROOT.kViolet+5)
        h_ZHlvlv.SetFillColor(ROOT.kViolet+1)

        h_ZHllvv = self.get_bkg_hist("ZHllvv")
        h_ZHllvv.SetLineColor(ROOT.kGray+3)
        h_ZHllvv.SetFillColor(ROOT.kGray)

        h_zjets = self.get_reducible_hist("h_Zjets")
        h_zjets.SetLineColor(zjets_Lcolor)
        h_zjets.SetFillColor(zjets_Fcolor)

        h_ttbar = self.get_reducible_hist("h_ttbar")
        h_ttbar.SetLineColor(ttbar_Lcolor)
        h_ttbar.SetFillColor(ttbar_Fcolor)
    
        if add_overflow:
            h_ZZ = ROOT.DrawOverflow(h_ZZ)
            h_H4l = ROOT.DrawOverflow(h_H4l)
            h_ZHlvlv = ROOT.DrawOverflow(h_ZHlvlv)
            h_ZHllvv = ROOT.DrawOverflow(h_ZHllvv)
            h_zjets = ROOT.DrawOverflow(h_zjets)
            h_ttbar = ROOT.DrawOverflow(h_ttbar)
        ROOT.add_hist(h_bkgs_list, h_ZZ)
        ROOT.add_hist(h_bkgs_list, h_H4l)
        ROOT.add_hist(h_bkgs_list, h_ZHlvlv)
        ROOT.add_hist(h_bkgs_list, h_ZHllvv)
        ROOT.add_hist(h_bkgs_list, h_zjets)
        ROOT.add_hist(h_bkgs_list, h_ttbar)
        h_bkgs_list_no_order.Add(h_ZZ)
        h_bkgs_list_no_order.Add(h_H4l)
        h_bkgs_list_no_order.Add(h_ZHlvlv)
        h_bkgs_list_no_order.Add(h_ZHllvv)
        h_bkgs_list_no_order.Add(h_zjets)
        h_bkgs_list_no_order.Add(h_ttbar)

        sys_list = ROOT.std.vector('double')()
        sys_list += [0.554, 0.611, 0.152, 0.153, 0.28, 0.28]
        only_sys = True
        hist_sys = ROOT.SumHistsWithSysUncertainties(h_bkgs_list_no_order,
                                                     sys_list, only_sys);
        hist_sys.SetMarkerStyle(0)
        hist_sys.SetLineColor(0)
        hist_sys.SetFillColor(1)
        hist_sys.SetFillStyle(3254)
        hist_sys.Scale(1.0, option)
        hist_stack = ROOT.THStack("hs", "")
        icount = 0
        for hist in h_bkgs_list:
            hist.Scale(1.0, option)
            hist_stack.Add(hist)
            icount += 1

        h_sig.SetXTitle(br_info.x_title_)
        h_sig.SetYTitle("Events/10 GeV")
        max_y = h_sig.GetMaximum()
        if hist_stack.GetMaximum() > max_y: max_y = hist_stack.GetMaximum()
        h_sig.GetYaxis().SetRangeUser(1e-7, max_y*1e7);
        h_sig.Draw()
        hist_stack.Draw("HIST SAME")
        h_sig.Draw("SAME")
        h_sig2.Draw("SAME")
        hist_sys.Draw("E2 SAME")
        h_sig.Draw("AXIS SAME")
        legend = ROOT.myLegend(0.63, 0.54, 0.9, 0.94)
        legend.SetTextSize(0.03)
        if self.with_data: 
            h_data.Scale(1.0, option)
            h_data.SetMarkerSize(1.0)
            h_data.Draw("EP SAME")
            legend.AddEntry(h_data, "Data", "PL")

        #legend.AddEntry(h_sig, "m_{zp} = "+mass+" GeV", "L")
        #legend.AddEntry(h_sig2, "m_{zp} = "+mass2+" GeV", "L")
        legend.AddEntry(h_sig, "Vector: 200 GeV", "L")
        legend.AddEntry(h_sig2, "Scalar: 300 GeV", "L")
        legend.AddEntry(h_H4l, "H#rightarrow4l", "F")
        legend.AddEntry(h_ZZ, "ZZ*", "F")
        legend.AddEntry(h_zjets, "Z+jets", "F")
        legend.AddEntry(h_ttbar, "t#bar{t}", "F")
        legend.AddEntry(h_ZHlvlv, "ZHl#nul#nu", "F")
        legend.AddEntry(h_ZHllvv, "ZHll#nu#nu", "F")
        legend.Draw()

        x_off_title = 0.200
        #ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Internal")
        ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Preliminary")
        ROOT.myText(x_off_title, 0.80, 1, "m_{#chi} = 1 GeV")
        #ROOT.myText(x_off_title, 0.75, 1, "#sqrt{{s}} = 13 TeV:#scale[0.55]{{#int}}Ldt = {:.2f} fb^{{-1}}".format(lumi_weight))
        ROOT.myText(x_off_title, 0.75, 1, "13 TeV, {:.1f} fb^{{-1}}".format(lumi_weight))
        ROOT.myText(x_off_title, 0.70, 1, "110 < m_{4l} < 140 GeV")
        if "TVector" in br_info.name_ :
            canvas.SaveAs("dphi.pdf")
        else:
            canvas.SaveAs(br_info.name_.replace('/','_')+"_data.eps")
        c2 = ROOT.TCanvas("c2", "c2", 600, 600)
        c2.SetLogy()
        hist_sys.SetLineColor(2)
        hist_sys.Draw("")
        c2.SaveAs("test_bb.pdf")

        
    def draw_CR(self, treename):
        color_list = color_list_int
        out_postfix = treename.split('_')[1]
        self.tree_name = treename
        global br_info
        canvas = ROOT.TCanvas("canvas", "canvas", 600, 600);
        canvas.SetLogy();
        cr_path = common.minitree_bkg
        f_zee = cr_path+"mc15_13TeV.341103.Sherpa_CT10_Zee_4lMassFilter40GeV8GeV_bkgCR.root"
        f_zmumu = cr_path+"mc15_13TeV.341104.Sherpa_CT10_Zmumu_4lMassFilter40GeV8GeV_bkgCR.root"
        f_ttbar = cr_path+"mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad_bkgCR.root"
        f_qqZZ = "/afs/cern.ch/user/x/xju/work/h4l/workspace/mc15_13TeV_v3/monoH/minitrees/combined_qqZZ_bkgCR.root"
        f_data = "/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v02/mc/Background/data/data15_bkgCR.root"
        samples_dir = {}
        #samples_dir["ggH"] = f_ggH
        samples_dir["Zee"] = f_zee
        samples_dir["Zmumu"] = f_zmumu
        samples_dir["ttbar"] = f_ttbar
        samples_dir["qqZZ"] = f_qqZZ
        
        h_data = self.make_hist(f_data, "h_data", 1)
        print "data", h_data.Integral()
        h_bkgs_list = ROOT.TList()
        h_bkgs_list_no_order = []
        ncount = 1
        for sample, path in samples_dir.iteritems():
            h_bkg = self.make_hist(path, "h_"+sample, color_list[ncount])
            ROOT.add_hist(h_bkgs_list, h_bkg)
            h_bkgs_list_no_order.append(h_bkg)
            print "{}: {:.2f} ({:.2f}%)".format(sample,h_bkg.Integral(),
                                            self.get_fraction_highmass(h_bkg)*100)
            ncount += 1

        hist_stack = ROOT.THStack("hs", "")
        print "samples: ", len(h_bkgs_list)
        for hist in h_bkgs_list:
            hist_stack.Add(hist)
        
        h_data.SetXTitle(br_info.x_title_)
        max_y = h_data.GetMaximum()
        if hist_stack.GetMaximum() > max_y: max_y = hist_stack.GetMaximum()
        h_data.GetYaxis().SetRangeUser(1e-5, max_y*1e6);
        h_data.Draw("EP")
        hist_stack.Draw("HIST SAME")
        h_data.Draw("SAME EP")
        h_data.Draw("AXIS SAME")

        legend = ROOT.myLegend(0.60, 0.55, 0.9, 0.9)
        h_data.SetMarkerSize(1.0)
        legend.AddEntry(h_data, "Data: {}".format(h_data.Integral()), "PL")
        for bkg,name in zip(h_bkgs_list_no_order, samples_dir.keys()):
            legend.AddEntry(bkg, "{}: {:.2f}".format(name,bkg.Integral()), "F")
        legend.Draw()

        x_off_title = 0.185
        lumi = 2.0
        ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Internal")
        ROOT.myText(x_off_title, 0.80, 1, out_postfix+" CR")
        ROOT.myText(x_off_title, 0.75, 1, "#sqrt{{s}} = 13 TeV:#scale[0.55]{{#int}}Ldt = {:.1f} fb^{{-1}}".format(lumi))
        ROOT.myText(x_off_title, 0.70, 1, "110 < m_{4L} < 140 GeV")
        if "TVector" in br_info.name_ :
            canvas.SaveAs("pdf/dphi_CR_"+out_postfix+".pdf")
        else:
            canvas.SaveAs("pdf/"+br_info.name_.replace('/','_')+"_CR_"+out_postfix+".pdf")

    def get_fraction_highmass(self, h1):
        ibin = h1.FindBin(100.0)
        nbins = h1.GetNbinsX()
        return h1.Integral(ibin,nbins)/h1.Integral()

    def overlay_three_regions(self):
        cr_path = common.minitree_bkg
        f_zee_cr = cr_path+"mc15_13TeV.341103.Sherpa_CT10_Zee_4lMassFilter40GeV8GeV_bkgCR.root"
        f_zee_sr = common.minitree_dir+"mc15_13TeV.341103.Sherpa_CT10_Zee_4lMassFilter40GeV8GeV.root"
        f_zmm_cr = cr_path+"mc15_13TeV.341104.Sherpa_CT10_Zmumu_4lMassFilter40GeV8GeV_bkgCR.root"
        f_zmm_sr = common.minitree_dir+"mc15_13TeV.341104.Sherpa_CT10_Zmumu_4lMassFilter40GeV8GeV.root"

        canvas = ROOT.TCanvas("canvas","canvas", 600, 600)
        canvas.SetLogy();
        self.tree_name = "tree_incl_all" 
        ncount = 1
        h_sr = self.make_hist(f_zee_sr, "h_zee_sr", color_list[ncount])
        ncount += 1
        self.tree_name = "tree_invD0" 
        h_cr1 = self.make_hist(f_zee_cr, "h_zee_cr1", color_list[ncount])
        ncount += 1
        self.tree_name = "tree_relaxIsoD0" 
        h_cr2 = self.make_hist(f_zee_cr, "h_zee_cr2", color_list[ncount])
        ncount += 1
        self.tree_name = "tree_ss" 
        h_cr3 = self.make_hist(f_zee_cr, "h_zee_cr3", color_list[ncount])
        h_sr.Scale(1./h_sr.Integral())
        h_cr1.Scale(1./h_cr1.Integral())
        h_cr2.Scale(1./h_cr2.Integral())
        h_cr3.Scale(1./h_cr3.Integral())
        h_sr.GetYaxis().SetRangeUser(1e-4, 1)
        h_sr.SetXTitle("E_{T}^{miss} [GeV]")
        h_sr.Draw()
        h_cr1.Draw("same")
        h_cr2.Draw("same")
        h_cr3.Draw("same")
        legend = ROOT.myLegend(0.60, 0.75, 0.9, 0.9)
        legend.AddEntry(h_sr, "SR", "f")
        legend.AddEntry(h_cr1, "invD0", "f")
        legend.AddEntry(h_cr2, "relaxIsoD0", "f")
        legend.AddEntry(h_cr3, "SS", "f")
        legend.Draw()
        canvas.SaveAs("zee_CR.pdf")

    def compare_met(self):
        global overall_cut
        global br_info
        hist_list = ROOT.TList() 
        model_name = "zphxx"
        for mass in self.config.masses:
            file_dir = self.config.samples_sig[str(mass)][model_name]
            hist_name = model_name+"_"+str(mass)
            h1 = ROOT.draw_hist_from_file(file_dir, self.tree_name, overall_cut,
                                          hist_name, br_info);
            hist_list.Add(h1)
        ROOT.compare_hists(hist_list, "E_{T}^{miss} [GeV]", True, True, True)

def draw_all_CR():
    ploter = Ploter(monoH)
    ploter.with_data = True
    ploter.path = common.minitree_bkg
    ploter.draw_CR("tree_relaxee")
    ploter.draw_CR("tree_invD0")
    ploter.draw_CR("tree_invIso")
    ploter.draw_CR("tree_relaxIsoD0")
    ploter.draw_CR("tree_ss")
    ploter.draw_CR("tree_emu")

if __name__ == "__main__":
    ploter = Ploter(monoH)
    ploter.with_data = True
    #ploter.overlay_three_regions()
    ploter.draw_signal_conf()
    #ploter.draw_signal()
    #draw_all_CR()
    #ploter.compare_met()
