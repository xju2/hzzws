/*
 */

#include <RooWorkspace.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooPlot.h>
#include <RooSimultaneous.h>
#include <RooCategory.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TIterator.h>
#include <TString.h>

#include <stdlib.h>
#include <string>

using namespace std;
int main(int argc, char** argv)
{
    if (argc > 1 && string(argv[1]) == "help"){
        cout << argv[0] << " combined.root mu_name" << endl;
        return 0;
    }

    string input_name("combined.root");
    string wsName = "combined";
    string mcName = "ModelConfig";
    string dataName = "obsData";
    string muName = "mu";
    string mhName = "mH";
   
    auto* file_in = TFile::Open(input_name.c_str(), "read");
    auto* workspace = (RooWorkspace*) file_in->Get(wsName.c_str());
    RooSimultaneous* simPdf = (RooSimultaneous*) workspace->obj("simPdf");
    RooRealVar* mH = (RooRealVar*) workspace->var(mhName.c_str());
    RooRealVar* m4l = (RooRealVar*) workspace->var("m4l");
    RooRealVar* mu = (RooRealVar*) workspace->var("mu");
    auto* data =(RooDataSet*) workspace->data("obsData");
    if(!m4l || !mu) {
        file_in->Close();
        return 0;
    } else {
        mu->Print();
        m4l->Print();
    }
    auto* el_res = (RooRealVar*) workspace->var("alpha_ATLAS_EL_RES");

    auto* canvas = new TCanvas("c1", "c1", 600, 600);
    double m4l_min = 200, m4l_max = 1000;
    double m4l_step = 50;
    m4l->setBins(212);
    if(m4l->getMax() < 150){
        m4l_min = 124, m4l_max = 126, m4l_step = 0.5;
        m4l->setBins(60);
    }
    if (mH) {
        mH->Print();
        mH->setRange(m4l_min, m4l_max);
    }
    const RooCategory& category = *dynamic_cast<const RooCategory*>(&simPdf->indexCat());
    TIter cat_iter(category.typeIterator());
    RooCatType* obj;
    TList* data_lists = NULL;
    if(data){
        data_lists = data->split(category, true);
    }
    while( (obj= (RooCatType*)cat_iter()) ){
        const char* label_name = obj->GetName();
        RooAbsPdf* pdf = simPdf->getPdf(label_name);
        pdf->Print();
        auto* m4l_frame = m4l->frame();
        int color = 2;
        mu->setVal(0.0);
        double bkg_evts = pdf->expectedEvents(RooArgSet(*m4l));
        pdf->plotOn(m4l_frame, RooFit::LineStyle(1), 
                RooFit::LineColor(1),
                RooFit::LineWidth(2),
                RooFit::Normalization(bkg_evts, RooAbsReal::NumEvent)
                );
        mu->setVal(5.0);
        cout << "[INFO] bkg only: " << bkg_evts << " s+B: " << endl;
        if(mH){
            for(double ini_mass=m4l_min; ini_mass <= m4l_max; ini_mass += m4l_step)
            {
                mH->setVal(ini_mass);
                double splusb_evts = pdf->expectedEvents(RooArgSet(*m4l));
                cout << "[INFO] mH("<< ini_mass <<"): " << splusb_evts ;
                pdf->plotOn(m4l_frame, RooFit::LineStyle(7), 
                        RooFit::LineColor(color++),
                        RooFit::LineWidth(2),
                        RooFit::Normalization(splusb_evts, RooAbsReal::NumEvent)
                        );
            }
        } else {
            double splusb_evts = pdf->expectedEvents(RooArgSet(*m4l));
            cout << "[INFO] S+B: " << splusb_evts << endl;
            pdf->plotOn(m4l_frame, RooFit::LineStyle(7), 
                    RooFit::LineColor(color++),
                    RooFit::LineWidth(2),
                    RooFit::Normalization(splusb_evts, RooAbsReal::NumEvent)
                    );
        }
        if(el_res){
            el_res->setVal(3.0);
            double splusb_evts = pdf->expectedEvents(RooArgSet(*m4l));
            cout << "[INFO] S+B (3sigma up): " << splusb_evts << endl;
            pdf->plotOn(m4l_frame, RooFit::LineStyle(7), 
                    RooFit::LineColor(color++),
                    RooFit::LineWidth(2),
                    RooFit::Normalization(splusb_evts, RooAbsReal::NumEvent)
                    );
            el_res->setVal(0);
        }
        auto* el_eff = (RooRealVar*) workspace->var("alpha_ATLAS_EL_EFF_RECO");
        if(el_eff){
            el_eff->setVal(3.0);
            double splusb_evts = pdf->expectedEvents(RooArgSet(*m4l));
            cout << "[INFO] S+B (3sigma: ATLAS_EL_EFF_RECO): " << splusb_evts << endl;
            el_eff->setVal(0.0);
        }
        //if(data_lists)
        if(false)
        {
            auto* data_ch = (RooDataSet*) data_lists->At(obj->getVal());
            data_ch->plotOn(m4l_frame, RooFit::LineStyle(1), 
                    RooFit::LineColor(1),
                    RooFit::LineWidth(2)
                    );
        }
        m4l_frame->Draw();
        canvas ->SaveAs(Form("pdf/pdf_vary_%s.pdf", label_name));
        delete m4l_frame;
    }
    delete canvas;

    file_in->Close();
    return 1;
}
