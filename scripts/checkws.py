#!/usr/bin/env python

import ROOT
import sys
import monoH

class wschecker:
    def __init__(self, file_name, poi_name):
        print "I will check: ", file_name
        file_in = ROOT.TFile.Open(file_name, "read")
        self.ws =  file_in.Get("combined")
        self.poi_name = poi_name
        self.simPdf = self.ws.obj("simPdf")
        self.mc = self.ws.obj("ModelConfig")
        self.nuisances = self.mc.GetNuisanceParameters()
        #self.mc = ROOT.RooStats.ModelConfig("mc", self.ws)
        self.mu = self.ws.var(self.poi_name)
        print self.ws.GetName()

    def get_yields(self):
        data = self.ws.data("obsData")
        simPdf = self.ws.obj("simPdf")
        m4l = self.ws.obj("m4l")
        mH = self.ws.obj("mH")
        if mH:
            mH.setRange(110., 140)
            mH.setVal(125.09)
            mH.Print()
        self.mu.Print()
        old_mu = self.mu.getVal()
        categories = self.simPdf.indexCat()
        datasets = data.split(categories, True)
        for data_ch in datasets:
            self.mu.setVal(1.0)
            ch_name = data_ch.GetName()
            pdf_ch = self.simPdf.getPdf(ch_name)
            #print pdf_ch
            ndata = data_ch.numEntries()
            #nsplusb = pdf_ch.expectedEvents(self.mc.GetObservables())
            nsplusb = pdf_ch.expectedEvents(ROOT.RooArgSet(m4l))
            self.mu.setVal(0.0)
            #nbkg = pdf_ch.expectedEvents(self.mc.GetObservables())
            nbkg = pdf_ch.expectedEvents(ROOT.RooArgSet(m4l))
            nsig = nsplusb-nbkg;
            print "{} {} {:.2f} {:.2f} {:.2f}".format(
                ch_name, ndata, nsplusb, nsig, nbkg)
        self.mu.setVal(old_mu)
    
    @staticmethod
    def get_add_formular(arg_list):
        res = "@1"
        for i in range(arg_list.getSize()-1):
            res += "+@"+str(i+2)
        print res
        return res

    def get_yields_monoH(self):
        categories = self.simPdf.indexCat()
        cat_iter = ROOT.TIter(categories.typeIterator())
        obj = cat_iter()
        res = ""
        mu_bsm = self.ws.var("mu_BSM")
        mu_bsm.setVal(0.)
        while obj:
            cat_name = obj.GetName()
            new_name = cat_name.replace("Cat","")
            print "in channel: ", new_name
            pdf_ch = self.simPdf.getPdf(cat_name)
            self.set_nusiance(0.0)
            nominal_list = self.get_bkg(pdf_ch)
            up_list = self.set_nusiance2(1.0, pdf_ch)
            #up_list = self.get_bkg(pdf_ch)
            down_list = self.set_nusiance2(-1.0, pdf_ch)
            #down_list = self.get_bkg(pdf_ch)
            self.print_vary2(nominal_list, up_list, down_list) 
            obj = cat_iter()

    def get_bkg(self, pdf_ch):
        m4l = self.ws.var('m4l')
        n_all = pdf_ch.expectedEvents(ROOT.RooArgSet(m4l))

        mu_Zjets = self.ws.var("mu_Zjets")
        mu_tt = self.ws.var("mu_tt")
        mu = self.ws.var("mu")
        mu_zhllvv = self.ws.var("mu_ZHllvv")
        mu_zhlvlv = self.ws.var("mu_ZHlvlv")
        mu_ggZZ = self.ws.var("mu_ggZZ")
        mu_qqZZ = self.ws.var("mu_qqZZ")

        mu_Zjets.setVal(0.)
        mu_tt.setVal(0.)
        mu_zhllvv.setVal(0.)
        mu_zhlvlv.setVal(0.)
        mu_ggZZ.setVal(0.)
        mu_qqZZ.setVal(0.)
        n_4l = pdf_ch.expectedEvents(ROOT.RooArgSet(m4l))

        mu.setVal(0.)
        mu_ggZZ.setVal(1.0)
        mu_qqZZ.setVal(1.)
        n_zz = pdf_ch.expectedEvents(ROOT.RooArgSet(m4l))
          
        mu_ggZZ.setVal(0)
        mu_qqZZ.setVal(0)
        mu_Zjets.setVal(1)
        mu_tt.setVal(1.)
        n_reducible = pdf_ch.expectedEvents(ROOT.RooArgSet(m4l))

        on = 1.
        mu_Zjets.setVal(on)
        mu_tt.setVal(on)
        mu_zhllvv.setVal(on)
        mu_zhlvlv.setVal(on)
        mu_ggZZ.setVal(on)
        mu_qqZZ.setVal(on)
        mu.setVal(on)
        return [n_all, n_4l, n_zz, n_reducible]

    def set_nusiance2(self, value, pdf_ch):
        iter_nuis = ROOT.TIter(self.nuisances.createIterator())
        obj = iter_nuis()
        res_list = []
        while obj:
            obj.setVal(value)
            res_list.append(self.get_bkg(pdf_ch))
            obj.setVal(0.)
            obj = iter_nuis()
        res = [0]*len(res_list[0])
        for nuis in res_list:
            for i in nuis::

        return res_list
        
        #print self.ws.obj("fiv_ATLAS_Signal_ggH_low_met").Print()

    def set_nusiance(self, value):
        iter_nuis = ROOT.TIter(self.nuisances.createIterator())
        obj = iter_nuis()
        while obj:
            obj.setVal(value)
            obj = iter_nuis()

    @staticmethod
    def print_vary(nominal, up, down):
        for x,y,z in zip(nominal, up, down):
            print "{:.3E} {:.3E} {:.3E},".format(x, y-x,x-z),
        print

    def print_vary2(nominal, up, down):
        for x,y,z in zip(nominal, up, down):
            print "{:.3E} {:.3E} {:.3E},".format(x, y-x,x-z),
        print

#TMath::ChisquareQuantile(TMath::Prob(0, 1), 1)
if __name__ == "__main__":
    if len(sys.argv) > 2:
        file_name = sys.argv[1]
        poi_name = sys.argv[2]
    else: 
        file_name = "combined.root"
        poi_name = "mu"

    check = wschecker(file_name, poi_name)
    #check.get_yields()
    check.get_yields_monoH()
    #check.plot_categories()
