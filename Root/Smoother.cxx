#include "Hzzws/Smoother.h"

#include <RooDataSet.h>
#include <RooNDKeysPdf.h>
#include <RooKeysPdf.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <RooCmdArg.h>
#include <RooArgList.h>
#include <RooRealVar.h>
#include "RooPlot.h"

#include <algorithm>

#include "Hzzws/Helper.h"

const bool makeValidPlots(false);

Smoother::Smoother(TVectorD rho) {
    m_rho_.ResizeTo(rho.GetNoElements());
    m_rho_= rho;
    outfile = NULL;
}

Smoother::Smoother(){
    outfile = NULL;
    m_rho_.ResizeTo(1);
    m_rho_(0) = 1.0;
}

Smoother::Smoother(const string& outname, TVectorD rho){
    outfile = TFile::Open(outname.c_str(), "RECREATE");
    m_rho_.ResizeTo(rho.GetNoElements());
    m_rho_ = rho;
}

Smoother::~Smoother() {
    if(outfile) outfile->Close();
}


void Smoother::smooth(const string& input_name, 
        const string& oname, const string& treename, 
        const RooArgSet &treeobs, const string& cut) const 
{

    TChain* tcut = Helper::loader(input_name, treename);

    // hard-code to add Cut
    RooArgSet obsAndCut(treeobs);
    RooRealVar event_type("event_type", "event_type", -10, 10);
    RooRealVar prod_type("prod_type", "prod_type", -10, 10);
    RooRealVar weight("weight", "weight", -10, 10000);
    RooRealVar MET("met_et", "met_et", 0, 5000);
    RooRealVar n_jets("n_jets", "n_jets", 0, 50);
    RooRealVar dijet_invmass("dijet_invmass", "dijet_invmass", -1200, 10000);
    RooRealVar dijet_deltaeta("dijet_deltaeta","dijet_deltaeta",0,100);
    RooRealVar leading_jet_eta("leading_jet_eta","leading_jet_eta",-5,5);
    RooRealVar leading_jet_pt("leading_jet_pt","leading_jet_pt",0,13000);
    RooRealVar subleading_jet_eta("subleading_jet_eta","subleading_jet_eta",-5,5);
    RooRealVar subleading_jet_pt("subleading_jet_pt","subleading_jet_pt",0,13000);

    obsAndCut.add(weight);
    obsAndCut.add(event_type);
    obsAndCut.add(prod_type);
    obsAndCut.add(MET);
    obsAndCut.add(n_jets);
    obsAndCut.add(dijet_invmass);
    obsAndCut.add(dijet_deltaeta);
    obsAndCut.add(leading_jet_eta);
    obsAndCut.add(leading_jet_pt);
    obsAndCut.add(subleading_jet_eta);
    obsAndCut.add(subleading_jet_pt);


    RooDataSet *ds = new RooDataSet(Form("%s_RooDataSet", oname.c_str()), "dataset", 
            obsAndCut, RooFit::Import(*tcut),
            RooFit::Cut(cut.c_str()),
            RooFit::WeightVar("weight")
             );

    ds->Print();
    /****
    if(ds->sumEntries() < 10) {
        cout <<"[WARNING]: dataset has less than 10 events: " << ds->sumEntries() << endl;
        delete ds;
        delete tcut;
        return NULL;
    }
    ***/
    RooArgList obsList(treeobs);
    RooCmdArg arg2 = RooCmdArg::none();
    RooRealVar *x = (RooRealVar*) obsList.at(0); 
    
    // 1D pdf
    TH1F *h1 = NULL;
    if (obsList.getSize() == 1){
       const RooKeysPdf::Mirror& mirror = RooKeysPdf::NoMirror;  // NoMirror, MirrorBoth
       RooKeysPdf *keyspdf = new RooKeysPdf(Form("%s_RooKeysPdf", oname.c_str()), "keyspdf", *x, *ds, mirror, m_rho_(0));

       h1 =(TH1F*) keyspdf->createHistogram(Form("%s", oname.c_str()), *x, RooFit::Binning(x->getBinning()), arg2);
       h1->Scale(ds->sumEntries()/h1->Integral());
       h1->SetName(oname.c_str());

       RooPlot* frame=NULL;
       if (makeValidPlots){ //TODO make this true/false to make/don'tmake validation plots
         frame= x->frame(RooFit::Bins(30));
         ds->plotOn(frame);
         keyspdf->plotOn(frame);
       }

       if(outfile){
        outfile->cd();
        if (frame) frame->Write(Form("validation_%s",oname.c_str()));
        h1->Write();
        delete keyspdf;   
       }
    }

    // 2D pdf
    TH2F *h2 = NULL;
    if (obsList.getSize() == 2){
        RooRealVar *y = (RooRealVar*) obsList.at(1); 
        arg2 = RooFit::YVar(*y, RooFit::Binning(y->getBinning()));
        RooNDKeysPdf *keyspdf = new RooNDKeysPdf(Form("%s_RooNDKeysPdf", oname.c_str()), "keyspdf", treeobs, *ds, m_rho_, "m");

        h2 =(TH2F*) keyspdf->createHistogram(Form("%s", oname.c_str()), *x, RooFit::Binning(x->getBinning()), arg2);
        h2->Scale(ds->sumEntries()/h2->Integral());
        h2->SetName(oname.c_str());

       if(outfile){
        outfile->cd();
        h2->Write();
        delete keyspdf;   
       }
    }

    delete ds;
    delete tcut;
 
}




