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

#include <algorithm>

#include "Hzzws/Helper.h"

Smoother::Smoother(float rho) {
    m_rho_ = rho;
    outfile = NULL;
}

Smoother::Smoother(){
    outfile = NULL;
    m_rho_ = 1.0;
}

Smoother::Smoother(const string& outname, float rho){
    outfile = TFile::Open(outname.c_str(), "RECREATE");
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

    obsAndCut.add(weight);
    obsAndCut.add(event_type);
    obsAndCut.add(prod_type);
    obsAndCut.add(MET);

    RooDataSet *ds = new RooDataSet(Form("%s_RooDataSet", oname.c_str()), "dataset", 
            obsAndCut, RooFit::Import(*tcut),
            RooFit::Cut(cut.c_str()),
            RooFit::WeightVar("weight")
             );

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
       RooKeysPdf *keyspdf = new RooKeysPdf(Form("%s_RooKeysPdf", oname.c_str()), "keyspdf", *x, *ds, mirror, m_rho_);

       h1 =(TH1F*) keyspdf->createHistogram(Form("%s", oname.c_str()), *x, RooFit::Binning(x->getBinning()), arg2);
       h1->Scale(ds->sumEntries()/h1->Integral());
       h1->SetName(oname.c_str());

       if(outfile){
        outfile->cd();
        h1->Write();
        delete keyspdf;   
       }
    }

    // 2D pdf
    TH2F *h2 = NULL;
    if (obsList.getSize() == 2){
        RooRealVar *y = (RooRealVar*) obsList.at(1); 
        arg2 = RooFit::YVar(*y, RooFit::Binning(y->getBinning()));
        RooNDKeysPdf *keyspdf = new RooNDKeysPdf(Form("%s_RooNDKeysPdf", oname.c_str()), "keyspdf", treeobs, *ds, "m", m_rho_);

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




