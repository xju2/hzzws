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
#include <RooCurve.h>
#include "RooMinimizer.h"

#include <TFile.h>
#include <TCanvas.h>
#include <TIterator.h>
#include <TString.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TGraphAsymmErrors.h>
#include <TColor.h>


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
        cout << argv[0] << " combined.root ws_name mu_name data_name bonly with_data do_visual_error min,max strategy color var:value,var:value np1,np2" << endl;
        return 0;
    }

    string input_name(argv[1]);
    string wsName(argv[2]);
    string muName(argv[3]);
    string dataName(argv[4]);
    string mcName = "ModelConfig";

    int opt_id = 5;

    bool is_bonly = false;
    if (argc > opt_id) {
        is_bonly = (bool) atoi(argv[opt_id]);
    }
    opt_id ++;

    bool with_data = false;
    if (argc > opt_id) {
        with_data = (bool) atoi(argv[opt_id]);
    }
    opt_id ++;

    bool do_visual_error = true;
    if (argc > opt_id) {
        do_visual_error = (bool) atoi(argv[opt_id]);
    }
    opt_id ++;

    double max_obs = 2000, min_obs = 200;
    if (argc > opt_id) {
        string options(argv[opt_id]);
        vector<string> tokens;
        Helper::tokenizeString(options, ',', tokens);
        if (tokens.size() < 2) {
            cout << "option: " << options << " invalid"<<endl;
            cout << "provide two numbers, e.g. 110,140" << endl;
            exit(1);
        }
        min_obs = (double) atof(tokens.at(0).c_str());
        max_obs = (double) atof(tokens.at(1).c_str());
    }
    opt_id ++;

    int strategy = 1;
    if (argc > opt_id) {
        strategy = atoi(argv[opt_id]);
    }
    opt_id ++;
    
    int fill_color = kGreen; // (416)
    if (argc > opt_id){
        fill_color = atoi(argv[opt_id]);
    }
    opt_id ++;
    
    map<string, double> fix_var_map;
    if (argc > opt_id)
    {
        string options(argv[opt_id]);
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
    opt_id ++;

    vector<string> np_names;
    if (argc > opt_id) {
        Helper::tokenizeString(string(argv[opt_id]), ',', np_names);
    }
    opt_id ++;
    if (np_names.size() > 0){
        cout << "total NPs: " << np_names.size() << endl;
    }

    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggTwoSidedCBPdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggScalarLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/HggGravitonLineShapePdf_cc.so");
    gSystem->Load("/afs/cern.ch/user/x/xju/public/src/FlexibleInterpVarMkII_cc.so");
    
   
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
    int nbins = (int) (max_obs-min_obs)/20;
    if(!obs || !mu) {
        file_in->Close();
        return 0;
    } else {
        mu->Print();
        obs->Print();
        // obs->setRange(min_obs, max_obs);
        /*need to pre-define binning!*/
        // obs->setBins(nbins);
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
    // summary of options
    cout<<" Input: " << input_name << endl;
    cout<<" wsName: " << wsName << endl;
    cout<<" muName: " << muName << endl;
    cout<<" dataName: " << dataName << endl;
    cout<<" bonly: " << is_bonly << endl;
    cout<<" withData: " << with_data << endl;
    cout<<" Visual Error: " << do_visual_error << endl;
    cout<<" NP size: " << np_names.size() << endl;
    cout<<" Range of obs: [" << min_obs << "-" << max_obs << "] " << endl;
    cout<<" strategy: " << strategy << endl;


    /* unconditional fit*/
    if(is_bonly) {
        mu->setVal(0);
        mu->setConstant();
        RooStatsHelper::fixTermsWithPattern(mc, "ATLAS_");
        // but free the following two, to include sprious signal uncertainties
        auto hgg_bias = workspace->var("ATLAS_Hgg_BIAS");
        auto mRes = workspace->var("ATLAS_mRes");
        if(hgg_bias) hgg_bias->setConstant(0);
        if (mRes) mRes ->setConstant(0);
    } else {
        // float the masses
        mu->setConstant(0);
        auto mG = workspace->var("mG");
        auto kappa = workspace->var("GkM");
        auto mX = workspace->var("mX");
        auto wX = workspace->var("wX");
        if (mG) mG->setConstant(0);
        if (kappa) kappa->setConstant(0);
        if (mX) mX->setConstant(0);
        if (wX) wX->setConstant(0);
    }

    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(strategy);
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
    // print out the message
    mc->GetParametersOfInterest()->Print("v");
    mc->GetNuisanceParameters()->Print("v");


    auto* nll = RooStatsHelper::createNLL(data, mc);
    RooFitResult* fit_results = RooStatsHelper::minimize(nll, workspace, true);
    if (!fit_results) {
        log_err("no fit results");
    }
    if (!do_visual_error) {
        delete fit_results;
        fit_results = NULL;
    }
    // fit_results = NULL;
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
    obs->setRange(min_obs, max_obs);
    obs->setBins(nbins);
    auto* out_file = TFile::Open("out_hist.root", "recreate");
    while( (obj= (RooCatType*)cat_iter()) )
    {
        const char* label_name = obj->GetName();
        RooAbsPdf* pdf = simPdf->getPdf(label_name);
        pdf->Print();
        auto* obs_frame = obs->frame(RooFit::Binning(nbins));
        obs_frame->SetMarkerSize(0.015);
        int color = 4;
        /* background only plot */
        // mu->setVal(0.0);
        double bkg_evts = pdf->expectedEvents(RooArgSet(*obs));
        auto* hist_bkg = (TH1F*) pdf->createHistogram(Form("hist_bkg_%s", label_name), *obs, RooFit::Binning(nbins));
        hist_bkg->Scale(bkg_evts/hist_bkg->Integral());
        cout << "[INFO] bkg only: " << bkg_evts << endl;
        RooCmdArg add_arg = (fit_results==NULL)?RooCmdArg::none():RooFit::VisualizeError(*fit_results);
        add_arg.Print();
        pdf->plotOn(obs_frame, RooFit::LineStyle(1), 
                RooFit::LineColor(1),
                RooFit::LineWidth(2),
                RooFit::FillColor(fill_color),
                RooFit::Normalization(bkg_evts, RooAbsReal::NumEvent),
                add_arg
                );
        pdf->plotOn(obs_frame, RooFit::LineStyle(1), 
                RooFit::LineColor(1),
                RooFit::LineStyle(2),
                RooFit::LineWidth(1),
                RooFit::Normalization(bkg_evts, RooAbsReal::NumEvent)
                );
        auto bkg_only_pdf = (RooAbsPdf*) workspace->obj("pdf_background_inclusive_13TeV");
        bkg_only_pdf->plotOn(obs_frame, 
                RooFit::LineStyle(1),
                RooFit::LineColor(1),
                RooFit::Normalization(bkg_evts, RooAbsReal::NumEvent)
                );
        /* signal + background plot */
        TH1F* hist_sb = NULL;
        // if(!is_bonly)
        if(false)
        {
            mu->setVal(22.09);
            double splusb_evts = pdf->expectedEvents(RooArgSet(*obs));
            cout << "[INFO] S+B: " << splusb_evts << endl;
            hist_sb = (TH1F*) pdf->createHistogram(Form("hist_SB_%s", label_name), *obs, RooFit::Binning(nbins));
            hist_sb->Scale(splusb_evts/hist_sb->Integral());

            pdf->plotOn(obs_frame, RooFit::LineStyle(7), 
                    RooFit::LineColor(color++),
                    RooFit::LineWidth(2),
                    RooFit::Normalization(splusb_evts, RooAbsReal::NumEvent)
                    );
        }

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

                double splusb_evts = pdf->expectedEvents(RooArgSet(*obs));
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
        if(hist_sb) hist_sb->Write();

        if(data && with_data)
        {
            auto* data_ch = (RooDataSet*) data_lists->At(obj->getVal());
            // double num_data = data_ch->sumEntries();
            data_ch->plotOn(obs_frame, 
                    RooFit::LineStyle(1), 
                    RooFit::LineColor(1),
                    RooFit::LineWidth(2),
                    RooFit::DrawOption("ep")
                    // RooFit::Normalization(num_data, RooAbsReal::NumEvent)
                    );
            cout <<"Data: " << data_ch->sumEntries() << endl;
            auto* hist_data = data_ch->createHistogram(Form("hist_data_%s", label_name), *obs, RooFit::Binning(nbins));
            if(hist_data){
                out_file->cd();
                hist_data->Write();
                delete hist_data;
            }
        }
        obs_frame->GetYaxis()->SetRangeUser(1E-1, 1E4);
        obs_frame->Print();
        obs_frame->Print("v");
        obs_frame->Draw();
        TString out_pdf_name(input_name);
        out_pdf_name.ReplaceAll("root", "pdf");
        canvas->SaveAs(Form("pdf/%s_%s", label_name, out_pdf_name.Data()));
        out_file->cd();

        // get error band
        RooCurve* error_band = (RooCurve*) obs_frame->getObject(0);
        RooCurve* nominal = (RooCurve*) obs_frame->getObject(1);
        RooCurve* bkg_only = (RooCurve*) obs_frame->getObject(2);
        error_band->SetName("error_band");
        nominal ->SetName("nominal");
        bkg_only ->SetName("bkg_only");
        error_band->Write();
        nominal->Write();
        bkg_only->Write();

        obs_frame->SetName("frame_obs");
        obs_frame->Write();
        delete obs_frame;
    }
    out_file->Close();
    delete canvas;

    file_in->Close();
    return 1;
}
