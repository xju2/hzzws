/*
 * Plot the shape
 */

#include <RooWorkspace.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <RooPlot.h>
#include <RooSimultaneous.h>
#include <RooCategory.h>
#include <RooStats/ModelConfig.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TIterator.h>
#include <TString.h>
#include <TSystem.h>
#include <TStyle.h>


#include <stdlib.h>
#include <string>

#include "Hzzws/Helper.h"
#include "Hzzws/RooStatsHelper.h"

using namespace std;
int main(int argc, char** argv)
{
    if ((argc > 1 && string(argv[1]) == "help") ||
            argc < 5)
    {
        cout << argv[0] << " combined.root ws_name mu_name data_name var:value,var:value np1,np2" << endl;
        return 0;
    }

    string input_name(argv[1]);
    string wsName(argv[2]);
    string muName(argv[3]);
    string dataName(argv[4]);
    string mcName = "ModelConfig";

    map<string, double> fix_var_map;
    if (argc > 5){
        string options(argv[5]);
        cout << options << endl;
        vector<string> tokens;
        Helper::tokenizeString(options, ',', tokens);
        for(auto iter = tokens.begin(); iter != tokens.end(); iter++){
            string token(*iter);
            cout << token << endl;
            size_t delim_pos = token.find(':');
            if(delim_pos != string::npos){
                string var_name = token.substr(0, delim_pos);
                double var_val = atof( token.substr(delim_pos+1, token.size()).c_str());
                fix_var_map[var_name] = var_val;
            }
        }
    }
    vector<string> np_names;
    if (argc > 6) {
        Helper::tokenizeString(string(argv[6]), ',', np_names);
    }
    if (np_names.size() > 0){
        cout << "total NPs: " << np_names.size() << endl;
    }

    gSystem->Load("/afs/cern.ch/user/x/xju/work/lee2d/src/HggTwoSidedCBPdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/work/lee2d/src/HggScalarLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/work/lee2d/src/HggGravitonLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/work/lee2d/src/FlexibleInterpVarMkII_cc.so");
    
   
    auto* file_in = TFile::Open(input_name.c_str(), "read");
    auto* workspace = (RooWorkspace*) file_in->Get(wsName.c_str());
    auto* mc = (RooStats::ModelConfig*) workspace->obj(mcName.c_str());
    RooSimultaneous* simPdf = NULL;
    try {
        simPdf = (RooSimultaneous*) mc->GetPdf();
    } catch (...){
        cout << "ERROR: no combined pdf in ModelConfig" << endl;
        exit(1);
    }

    // const RooArgSet* pois = mc->GetParametersOfInterest();
    RooRealVar* obs = (RooRealVar*) mc->GetObservables()->first();
    auto* mu = (RooRealVar*) workspace->var(muName.c_str()); 
    auto* data =(RooDataSet*) workspace->data(dataName.c_str());
    if(!data) {
        log_err("data(%s) does not exist", dataName.c_str());
        exit(2);
    }
    int nbins = 66;
    if(!obs || !mu) {
        file_in->Close();
        return 0;
    } else {
        mu->Print();
        obs->Print();
        obs->setRange(200., 2000.);
        /*need to pre-define binning!*/
        obs->setBins(nbins);
    }

    /* fix parameters given in option*/
    if (fix_var_map.size() > 0) {
        for (auto it = fix_var_map.begin(); it != fix_var_map.end(); it++){
            string var_name (it->first);
            auto* par = (RooRealVar*) workspace->var(var_name.c_str());
            if(!par) {
                log_warn("%s does not exist!", var_name.c_str());
            } else {
                log_info("%s fixed to %.2f", var_name.c_str(), it->second);
                par->setVal(it->second);
                par->setConstant();
            }
        }
    }
    // workspace->loadSnapshot("raw");
    RooStatsHelper::fixGammaTerms(mc);

    /* unconditional fit*/
    auto* nll = RooStatsHelper::createNLL(data, mc);
    RooFitResult* fit_results = RooStatsHelper::minimize(nll, workspace, true);
    if (!fit_results) {
        log_err("not fit results");
    }
    auto* canvas = new TCanvas("c1", "c1", 600, 600);
    gStyle->SetMarkerSize(0.5);
    canvas->SetLogy();

    const RooCategory& category = *dynamic_cast<const RooCategory*>(&simPdf->indexCat());
    TIter cat_iter(category.typeIterator());
    RooCatType* obj;
    TList* data_lists = NULL;
    if(data) {
        data_lists = data->split(category, true);
    }
    
    auto* out_file = TFile::Open("out_hist.root", "recreate");
    while( (obj= (RooCatType*)cat_iter()) )
    {
        const char* label_name = obj->GetName();
        RooAbsPdf* pdf = simPdf->getPdf(label_name);
        pdf->Print();
        auto* obs_frame = obs->frame();
        obs_frame->SetMarkerSize(0.015);
        int color = 2;
        /* background only plot */
        mu->setVal(0.0);
        double bkg_evts = pdf->expectedEvents(RooArgSet(*obs));
        auto* hist_bkg = (TH1F*) pdf->createHistogram(Form("hist_bkg_%s", label_name), *obs, RooFit::Binning(nbins));
        hist_bkg->Scale(bkg_evts/hist_bkg->Integral());
        cout << "[INFO] bkg only: " << bkg_evts << endl;
        RooCmdArg add_arg = (fit_results==NULL)?RooCmdArg::none():RooFit::VisualizeError(*fit_results);
        pdf->plotOn(obs_frame, RooFit::LineStyle(1), 
                RooFit::LineColor(1),
                RooFit::LineWidth(2),
                RooFit::Normalization(bkg_evts, RooAbsReal::NumEvent),
                add_arg
                );
        pdf->plotOn(obs_frame, RooFit::LineStyle(1), 
                RooFit::LineColor(1),
                RooFit::LineStyle(2),
                RooFit::LineWidth(1),
                RooFit::Normalization(bkg_evts, RooAbsReal::NumEvent)
                );
        /* signal + background plot */
        mu->setVal(22.09);
        double splusb_evts = pdf->expectedEvents(RooArgSet(*obs));
        cout << "[INFO] S+B: " << splusb_evts << endl;
        auto* hist_sb = (TH1F*) pdf->createHistogram(Form("hist_SB_%s", label_name), *obs, RooFit::Binning(nbins));
        hist_sb->Scale(splusb_evts/hist_sb->Integral());

        pdf->plotOn(obs_frame, RooFit::LineStyle(7), 
                RooFit::LineColor(color++),
                RooFit::LineWidth(2),
                RooFit::Normalization(splusb_evts, RooAbsReal::NumEvent)
                );

        /* deal with nuisance parameters */
        if(np_names.size() > 0)
        {
            double sigma_level = 1.0;
            for(auto itr = np_names.begin(); itr != np_names.end(); itr++)
            {
                string np_name(*itr);
                auto* np_var = (RooRealVar*) workspace->var(np_name.c_str());
                if(!np_var) continue;
                np_var->setVal(1.0);

                splusb_evts = pdf->expectedEvents(RooArgSet(*obs));
                cout << "[INFO] S+B ("<< sigma_level << " sigma up): " << splusb_evts << endl;
                auto* hist_sb_2s_up = (TH1F*) pdf->createHistogram(Form("hist_sb_2up_%s", label_name), *obs, RooFit::Binning(nbins));
                if(hist_sb_2s_up){
                    hist_sb_2s_up->Scale(splusb_evts/hist_sb_2s_up->Integral());
                    out_file->cd();
                    hist_sb_2s_up->Write();
                    delete hist_sb_2s_up;
                }
                pdf->plotOn(obs_frame, RooFit::LineStyle(7), 
                        RooFit::LineColor(color++),
                        RooFit::LineWidth(2),
                        RooFit::Normalization(splusb_evts, RooAbsReal::NumEvent)
                        );
                // bkg only
                mu->setVal(0);
                bkg_evts = pdf->expectedEvents(RooArgSet(*obs));
                auto* hist_bkg_2sup = (TH1F*) pdf->createHistogram(Form("hist_bonly_2up_%s", label_name), *obs, RooFit::Binning(nbins));
                if(hist_bkg_2sup){
                    hist_bkg_2sup->Scale(bkg_evts/hist_bkg_2sup->Integral());
                    out_file->cd();
                    hist_bkg_2sup->Write();
                    delete hist_bkg_2sup;
                }
                np_var->setVal(0);
            }
        }
        out_file->cd();
        hist_bkg->Write();
        hist_sb->Write();
        
        if(data)
        {
            auto* data_ch = (RooDataSet*) data_lists->At(obj->getVal());
            double num_data = data_ch->sumEntries();
            auto data_frame = data_ch->plotOn(obs_frame, 
                    RooFit::LineStyle(1), 
                    RooFit::LineColor(1),
                    RooFit::LineWidth(2),
                    RooFit::DrawOption("ep")
                    // RooFit::Normalization(num_data, RooAbsReal::NumEvent),

                    );
            cout <<"Data: " << data_ch->sumEntries() << endl;
            auto* hist_data = data_ch->createHistogram(Form("hist_data_%s", label_name), *obs, RooFit::Binning(nbins));
            if(hist_data){
                out_file->cd();
                hist_data->Write();
                delete hist_data;
            }
        }
        obs_frame->GetYaxis()->SetRangeUser(1E-4, 1E4);
        obs_frame->Draw();
        canvas->SaveAs(Form("pdf/pdf_vary_%s.pdf", label_name));
        delete obs_frame;
    }
    out_file->Close();
    delete canvas;

    file_in->Close();
    return 1;
}
