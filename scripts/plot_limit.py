#!/usr/bin/env python
import ROOT
import AtlasStyle
from array import array
if not hasattr(ROOT, "myText"):
    ROOT.gROOT.LoadMacro("$HZZWSCODEDIR/scripts/loader.c") 

def create_TGraphAsymmErrors(x_, nominal_, up_, down_):
    zero_ = array('f', [0]*len(x_))
    up_var_ = [x-y for x,y in zip(up_, nominal_)]
    down_var_ = [x-y for x,y in zip(nominal_, down_)]
    gr_error = ROOT.TGraphAsymmErrors(
        len(x_), array('f',x_), array('f',nominal_),
        zero_, zero_,
        array('f', down_var_), array('f', up_var_))
    return gr_error

def get_graph_from_list(mass_list, obs_list, exp_list, 
                        up_1sig_list, up_2sig_list,
                        down_1sig_list, down_2sig_list):

    zero_list = [0]*len(mass_list)
    print mass_list
    print obs_list
    gr_obs = ROOT.TGraph(len(mass_list), array('f', mass_list), array('f',obs_list))
    gr_exp = ROOT.TGraph(len(mass_list), array('f', mass_list), array('f', exp_list))
    gr_1sig = create_TGraphAsymmErrors(mass_list, exp_list, up_1sig_list, down_1sig_list)
    gr_2sig = create_TGraphAsymmErrors(mass_list, exp_list, up_2sig_list, down_2sig_list)
    gr_obs.SetLineWidth(2)
    gr_obs.SetLineStyle(1)
    gr_obs.SetMarkerStyle(20)

    gr_exp.SetLineWidth(2)
    gr_exp.SetLineStyle(2)

    gr_1sig.SetFillStyle(1001)
    gr_1sig.SetFillColor(ROOT.kGreen)
    gr_2sig.SetFillStyle(1001)
    gr_2sig.SetFillColor(ROOT.kYellow)
    
    return (gr_obs, gr_exp, gr_1sig, gr_2sig)

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

    gr_obs, gr_exp, gr_1sig, gr_2sig = get_graph_from_list(mass_list, obs_list,
                                                           exp_list,
                                                           up_1sig_list,
                                                           up_2sig_list,
                                                           down_1sig_list,
                                                           down_2sig_list)
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

def make_limit_graph(file1, kappa, is_scalar, w_xs = 1.0):
    ROOT.gROOT.SetBatch()
    mass_list = []
    obs_list = []
    exp_list = []
    up_2sig_list = []
    up_1sig_list = []
    down_1sig_list = []
    down_2sig_list = []
    tag_name = "GkM:"
    if is_scalar: tag_name = "wX:"
    
    with open(file1, 'r') as f:
        for line in f:
            items = line.split()
            
            tag = items[0].replace(".txt", "")
            #print tag
            mass = float(tag.split("_")[2])
            width = float(tag.split("_")[4])
            if not is_scalar and kappa != width: 
                continue
            elif is_scalar and kappa != width*1.0/mass:
                continue
            else: pass

            mass_list.append(mass)
            down_2sig_list.append(float(items[2])*w_xs)
            down_1sig_list.append(float(items[3])*w_xs)
            exp_list.append(float(items[4])*w_xs)
            up_1sig_list.append(float(items[5])*w_xs)
            up_2sig_list.append(float(items[6])*w_xs)
            obs_list.append(float(items[7])*w_xs)
    
    return get_graph_from_list(mass_list, obs_list, exp_list,
                               up_1sig_list, up_2sig_list,
                               down_1sig_list, down_2sig_list)

def make_limit_HT (file1):
    ROOT.gROOT.SetBatch()
    mass_list = []
    obs_list = []
    exp_list = []
    up_2sig_list = []
    up_1sig_list = []
    down_1sig_list = []
    down_2sig_list = []
    
    line_no = 0
    with open(file1, 'r') as f:
        for line in f:
            if line_no == 0: 
                line_no += 1
                continue
            items = line.split()
            mass = float(items[0])
            mass_list.append(mass)
            down_2sig_list.append(float(items[4]))
            down_1sig_list.append(float(items[3]))
            exp_list.append(float(items[2]))
            up_1sig_list.append(float(items[5]))
            up_2sig_list.append(float(items[6]))
            obs_list.append(float(items[1]))

    return get_graph_from_list(mass_list, obs_list, exp_list,
                               up_1sig_list, up_2sig_list,
                               down_1sig_list, down_2sig_list)

def compare_limit(kappa, with_data = False, split = False, is_scalar = False):
    low_y = 0.3
    hi_y = 100
    unit = "95% CL limit on #sigma #times BR(G*#rightarrow#gamma#gamma) [fb]"
    dummy=ROOT.TH2F("dummy",";m_{G*} [GeV];"+unit,
                    200, 200.,3515,3000,low_y,hi_y);
    dummy.GetXaxis().SetNdivisions(509);
    
    #f1_name = "limit_histfactory.txt"
    #f2_name = "limit_func_Mar23.txt"
    f1_name = "scalar_limit.txt"
    f2_name = "limit_0.01_hongtao.txt"
    hist_obs,hist_exp,hist_1s,hist_2s = make_limit_graph(f1_name, kappa, True)
    func_obs,func_exp,func_1s,func_2s = make_limit_HT(f2_name)

    hist_obs.SetLineColor(4)
    hist_exp.SetLineColor(4)
    hist_obs.SetMarkerStyle(20)
    hist_obs.SetMarkerSize(0.1)

    fill_style = 3002
    func_2s.SetFillStyle(fill_style)
    func_1s.SetFillStyle(fill_style)
    func_2s.SetFillColor(34)
    func_1s.SetFillColor(46)
    func_obs.SetLineColor(2)
    func_obs.SetMarkerSize(0.1)
    func_obs.SetMarkerColor(2)
    func_obs.SetMarkerStyle(20)
    func_exp.SetLineColor(2)

    canvas = ROOT.TCanvas("canvas2", " ", 600, 600)
    canvas.SetLogy()
    #canvas.SetGrid()
    ## add ratio 
    #c1 = ROOT.add_ratio_pad(hist_obs.GetHistogram(), func_obs.GetHistogram())
    #c1.SetLogy()
    dummy.Draw()
    hist_2s.Draw("3")
    hist_1s.Draw("3")
    if with_data: hist_obs.Draw("L*")
    hist_exp.Draw("L")

    func_2s.Draw("3")
    func_1s.Draw("3")
    if with_data: func_obs.Draw("L*")
    func_exp.Draw("L")
    dummy.Draw("AXIS SAME")

    tag1_name = "XY"
    tag2_name = "HT"
    legend = ROOT.myLegend(0.62, 0.60, 0.83, 0.90)
    #legend.AddEntry(hist_exp, "Template", "l")
    #legend.AddEntry(func_exp, "Function", "l")
    legend.AddEntry(hist_exp, tag1_name, "l")
    legend.AddEntry(func_exp, tag2_name, "l")
    legend.AddEntry(hist_1s, tag1_name+" #pm 1 #sigma", "f")
    legend.AddEntry(hist_2s, tag1_name+" #pm 2 #sigma", "f")
    legend.AddEntry(func_1s, tag2_name+" #pm 1 #sigma", "f")
    legend.AddEntry(func_2s, tag2_name+" #pm 2 #sigma", "f")
    legend.Draw()

    lumi = 3.2
    x_off_title = 0.285
    ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Preliminary")
    ROOT.myText(x_off_title, 0.80, 1, "13 TeV, {:.2f} fb^{{-1}}".format(lumi))
    ROOT.myText(x_off_title, 0.75, 1, "k/#bar{M_{pl}} = "+str(kappa))
    
    out_name = "compare_limit_"+str(kappa)
    if with_data:
        out_name +="_data"
    canvas.SaveAs(out_name+".pdf")

    if split:
        can_hist = ROOT.TCanvas("can_hist", " ", 600, 600)
        can_hist.SetLogy()
        dummy.Draw()
        hist_2s.Draw("3")
        hist_1s.Draw("3")
        hist_exp.Draw("L")
        if with_data: hist_obs.Draw("L*")
        dummy.Draw("AXIS SAME")
        leg_hist = ROOT.myLegend(0.62, 0.70, 0.83, 0.90)
        leg_hist.AddEntry(hist_exp, "Template", "l")
        leg_hist.AddEntry(hist_1s, "Template #pm 1 #sigma", "f")
        leg_hist.AddEntry(hist_2s, "Template #pm 2 #sigma", "f")
        leg_hist.Draw()
        ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Preliminary")
        ROOT.myText(x_off_title, 0.80, 1, "13 TeV, {:.2f} fb^{{-1}}".format(lumi))
        ROOT.myText(x_off_title, 0.75, 1, "k/#bar{M_{pl}} = "+str(kappa))
        can_hist.SaveAs(out_name+"_hist.pdf")

        #### functional
        can_func = ROOT.TCanvas("can_func", " ", 600, 600)
        can_func.SetLogy()
        dummy.Draw()
        func_2s.Draw("3")
        func_1s.Draw("3")
        func_exp.Draw("L")
        if with_data: func_obs.Draw("L*")
        dummy.Draw("AXIS SAME")
        leg_func = ROOT.myLegend(0.62, 0.70, 0.83, 0.90)
        leg_func.AddEntry(func_exp, "Function", "l")
        leg_func.AddEntry(func_1s, "Function #pm 1 #sigma", "f")
        leg_func.AddEntry(func_2s, "Function #pm 2 #sigma", "f")
        leg_func.Draw()
        ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Preliminary")
        ROOT.myText(x_off_title, 0.80, 1, "13 TeV, {:.2f} fb^{{-1}}".format(lumi))
        ROOT.myText(x_off_title, 0.75, 1, "k/#bar{M_{pl}} = "+str(kappa))
        can_func.SaveAs(out_name+"_func.pdf")


def plot_limit(file_name, kappa, with_data = False, is_scalar = False):
    low_y = 0.3
    hi_y = 100
    unit = "95% CL limits on #sigma #times BR(G*#rightarrow#gamma#gamma) [fb]"
    x_axis_title = "m_{G*} [GeV]"
    ana_name = "G*#rightarrow#gamma#gamma"
    sel_name = "Spin-2 Selection"
    if is_scalar:
        unit = "95% CL limits on #sigma_{fid} #times BR [fb]"
        x_axis_title = "m_{X} [GeV]"
        sel_name = "Spin-0 Selection"
        ana_name = "X#rightarrow#gamma#gamma, #Sigma_{X} = "+str(kappa*100)+"% m_{x}"

    dummy=ROOT.TH2F("dummy",";"+x_axis_title+";"+unit,
                    151, 495.,1998,3000,low_y,hi_y);
    dummy.GetXaxis().SetNdivisions(509);

    hist_obs,hist_exp,hist_1s,hist_2s = make_limit_graph(file_name, kappa,
                                                         is_scalar)

    canvas = ROOT.TCanvas("canvas2", " ", 600, 600)
    canvas.SetLogy()

    dummy.Draw()
    hist_2s.Draw("3")
    hist_1s.Draw("3")
    if with_data: hist_obs.Draw("L*")
    hist_exp.Draw("L")

    hist_obs.SetLineWidth(2)
    hist_obs.SetMarkerStyle(20)
    #hist_obs.SetMarkerSize(0.0)
    hist_exp.SetLineColor(4)
    hist_exp.SetLineWidth(2)

    dummy.Draw("AXIS SAME")

    legend = ROOT.myLegend(0.56, 0.60, 0.83, 0.90)
    legend.AddEntry(hist_obs, "Observed #it{CL_{s}} limit", "l")
    legend.AddEntry(hist_exp, "Expected #it{CL_{s}} limit", "l")
    legend.AddEntry(hist_1s, "Expected #pm 1 #sigma", "f")
    legend.AddEntry(hist_2s, "Expected #pm 2 #sigma", "f")
    legend.Draw()

    lumi = 3.2
    x_off_title = 0.20
    ROOT.myText(x_off_title, 0.85, 1, "#bf{#it{ATLAS}} Preliminary")
    ROOT.myText(x_off_title, 0.80, 1, "13 TeV, {:.2f} fb^{{-1}}".format(lumi))
    ROOT.myText(x_off_title, 0.75, 1, sel_name)
    ROOT.myText(x_off_title, 0.70, 1, ana_name)
    
    out_name = file_name+"_"+str(kappa)
    if with_data:
        out_name +="_data"
    canvas.SaveAs(out_name+".pdf")

def compare():
    is_inclusive = False 
    #get_limit_graph("limit_shxx.txt", "Scalar", is_inclusive)
    #get_limit_graph("limit_zphxx.txt", "Vector", is_inclusive)
    #make_limit_graph("limit_histfactory.txt")
    kappa_list = [0.01, 0.1, 0.2, 0.3]
    with_data = False
    split = True
    for kappa in kappa_list:
        compare_limit(kappa, with_data, split)

def plot():
    file_name = "scalar_limit.txt"
    kappa_list = [0.00, 0.01, 0.06, 0.1]
    with_data = True
    is_scalar = True
    for kappa in kappa_list:
        plot_limit(file_name, kappa, with_data, is_scalar)

if __name__ == "__main__":
    #plot()
    compare_limit(0.01, True, False)
