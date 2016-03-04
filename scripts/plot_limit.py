#!/usr/bin/env python
import ROOT
import AtlasStyle
from array import array
ROOT.gROOT.LoadMacro("/afs/cern.ch/user/x/xju/tool/loader.c") 

def create_TGraphAsymmErrors(x_, nominal_, up_, down_):
    zero_ = array('f', [0]*len(x_))
    up_var_ = [x-y for x,y in zip(up_, nominal_)]
    down_var_ = [x-y for x,y in zip(nominal_, down_)]
    gr_error = ROOT.TGraphAsymmErrors(
        len(x_), array('f',x_), array('f',nominal_),
        zero_, zero_,
        array('f', down_var_), array('f', up_var_))
    return gr_error

def get_limit_graph(file_name, model_name, is_inclusive):
    ROOT.gROOT.SetBatch()
    mass_list = []
    obs_list = []
    exp_list = []
    up_2sig_list = []
    up_1sig_list = []
    down_1sig_list = []
    down_2sig_list = []
    unit = "95% CL limit on #sigma [pb]"
    with open(file_name, 'r') as f:
        for line in f:
            items = line[7:].split()
            print items
            if len(items) > 7:
                w_xs = float(eval(items[7]))
                if not is_inclusive:
                    w_xs *= 1.25E-1
                    unit = "95% CL limit on #sigma #times BR(H#rightarrow ZZ*#rightarrow 4l) [fb]"
            else:
                w_xs = 1.
            mass_list.append(float(items[0]))
            down_2sig_list.append(float(items[1])*w_xs)
            down_1sig_list.append(float(items[2])*w_xs)
            exp_list.append(float(items[3])*w_xs)
            up_1sig_list.append(float(items[4])*w_xs)
            up_2sig_list.append(float(items[5])*w_xs)
            obs_list.append(float(items[6])*w_xs)
    zero_list = [0]*len(mass_list)
    print mass_list
    print obs_list
    gr_obs = ROOT.TGraph(len(mass_list), array('f', mass_list), array('f',obs_list))
    gr_obs.SetLineWidth(2)
    gr_obs.SetLineStyle(1)
    gr_obs.SetMarkerStyle(20)

    gr_exp = ROOT.TGraph(len(mass_list), array('f', mass_list),
                         array('f', exp_list))
    gr_exp.SetLineWidth(2)
    gr_exp.SetLineStyle(2)

    gr_1sig = create_TGraphAsymmErrors(mass_list, exp_list, up_1sig_list, down_1sig_list)
    gr_2sig = create_TGraphAsymmErrors(mass_list, exp_list, up_2sig_list, down_2sig_list)

    gr_1sig.SetFillStyle(1001)
    gr_1sig.SetFillColor(ROOT.kGreen)
    gr_2sig.SetFillStyle(1001)
    gr_2sig.SetFillColor(ROOT.kYellow)
    
    #dummy=ROOT.TH2F("dummy",";m_{med} [GeV];95% CL limit on #sigma/#sigma_{expected}",
    low_y = 0.3
    hi_y = 100
    if is_inclusive:
        low_y = 3 
        hi_y = 1E4
    dummy=ROOT.TH2F("dummy",";m_{med} [GeV];"+unit,
                    500, 0.,2000,3000,low_y,hi_y);
    dummy.GetXaxis().SetNdivisions(9);
    canvas = ROOT.TCanvas("canvas", " ", 600, 600)
    canvas.SetLogy()
    legend = ROOT.myLegend(0.65, 0.70, 0.85, 0.90)
    legend.AddEntry(gr_obs, "Observed #it{CL}_{s}", "l")
    legend.AddEntry(gr_exp, "Expected #it{CL}_{s}", "l")
    legend.AddEntry(gr_1sig, "#pm 1 #sigma", "f")
    legend.AddEntry(gr_2sig, "#pm 2 #sigma", "f")
    dummy.Draw()
    gr_2sig.Draw("3")
    gr_1sig.Draw("3")
    gr_obs.Draw("L*")
    gr_exp.Draw("L")
    gr_obs.SetLineColor(1)
    gr_obs.SetMarkerStyle(20)
    legend.Draw()
    dummy.Draw("AXIS SAME")

    lumi = 3.21
    x_off_title = 0.185
    #ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Internal")
    ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Preliminary")
    ROOT.myText(x_off_title, 0.80, 1, model_name+": m_{#chi} = 1 GeV")
    #ROOT.myText(x_off_title, 0.75, 1, "#sqrt{{s}} = 13 TeV:#scale[0.55]{{#int}}Ldt = {:.2f} fb^{{-1}}".format(lumi))
    ROOT.myText(x_off_title, 0.75, 1, "13 TeV, {:.2f} fb^{{-1}}".format(lumi))
    ROOT.myText(x_off_title, 0.70, 1, "110 < m_{4l} < 140 GeV")

    if is_inclusive:
        canvas.SaveAs(model_name+"_limit_inc.pdf")
        canvas.SaveAs(model_name+"_limit_inc.eps")
    else:
        canvas.SaveAs(model_name+"_limit_br.pdf")
        canvas.SaveAs(model_name+"_limit_br.eps")

if __name__ == "__main__":
    is_inclusive = False 
    get_limit_graph("limit_shxx.txt", "Scalar", is_inclusive)
    get_limit_graph("limit_zphxx.txt", "Vector", is_inclusive)
