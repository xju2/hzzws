#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <cmath>

#include "RooFitResult.h"
#include "RooMinimizer.h"
#include "RooSimultaneous.h"
#include "RooDataSet.h"
#include "RooCmdArg.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooRandom.h"
#include "RooStats/RooStatsUtils.h"
#include "RooStats/AsymptoticCalculator.h"
#include "RooStats/ModelConfig.h"
#include "TH2.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TStopwatch.h"

#include "Hzzws/RooStatsHelper.h"
#include "Hzzws/Helper.h"

void RooStatsHelper::setDefaultMinimize(){
  ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
  ROOT::Math::MinimizerOptions::SetDefaultStrategy(1);
  ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
}

RooFitResult* RooStatsHelper::minimize(RooNLLVar* nll, 
        RooWorkspace* combWS, 
        bool save, const RooArgSet* minosSet)
{
    nll->enableOffsetting(true);
    int printLevel = ROOT::Math::MinimizerOptions::DefaultPrintLevel();
    // RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();

    if (printLevel < 0) RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

    int strat = ROOT::Math::MinimizerOptions::DefaultStrategy();
  
    RooMinimizer minim(*nll);
    minim.optimizeConst(2); 
    minim.setStrategy(strat);
    minim.setPrintLevel(printLevel);
    minim.setProfile();  // print running time
    minim.setEps(0.001);

    // minim.setErrorLevel(1E-3);


    int status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());


    if (status != 0 && status != 1 && strat < 2)
    {
        strat++;
        cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
        minim.setStrategy(strat);
        status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
    }

    if (status != 0 && status != 1 && strat < 2)
    {
        strat++;
        cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
        minim.setStrategy(strat);
        status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
    }
    // //switch minuit version and try again
    if (status != 0 && status != 1)
    {
        string minType = ROOT::Math::MinimizerOptions::DefaultMinimizerType();
        string newMinType;
        if (minType == "Minuit2") newMinType = "Minuit";
        else newMinType = "Minuit2";

        cout << "Switching minuit type from " << minType << " to " << newMinType << endl;

        ROOT::Math::MinimizerOptions::SetDefaultMinimizer(newMinType.c_str());
        strat = ROOT::Math::MinimizerOptions::DefaultStrategy();
        minim.setStrategy(strat);

        status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());

        if (status != 0 && status != 1 && strat < 2)
        {
            strat++;
            cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
            minim.setStrategy(strat);
            status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
        }

        if (status != 0 && status != 1 && strat < 2)
        {
            strat++;
            cout << "Fit failed with status " << status << ". Retrying with strategy " << strat << endl;
            minim.setStrategy(strat);
            status = minim.minimize(ROOT::Math::MinimizerOptions::DefaultMinimizerType().c_str(), ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo().c_str());
        }

        ROOT::Math::MinimizerOptions::SetDefaultMinimizer(minType.c_str());
    }
    if (minosSet != NULL) {
        minim.minos(*minosSet);
    }
    // minim.minos();
    if (save && status == 0) return minim.save();
    else return NULL;
}

void RooStatsHelper::setVarfixed(RooWorkspace* combined, const char* varName, double value)
{
    RooRealVar* _var = (RooRealVar*) combined->var(varName);
    if (_var) {
        cout<<varName<<" fixed to " << value<<endl;
        _var->setVal(value);
        _var->setConstant(1);
    } 
    else {
        cout << "Error: cannot find " << varName <<endl;
    }
}

void RooStatsHelper::setVarFree(RooWorkspace* combined, const char* varName)
{
    RooRealVar* _var = (RooRealVar*) combined->var(varName);
    if (_var) {
        _var->setConstant(kFALSE);
    }
    else {
        cout << "Error: RooStatsHelper cannot find " << varName << endl;
    }
}

pair<double,double> RooStatsHelper::getVarVal(const RooWorkspace& combined, const char* varName)
{
    if(varName == NULL) return make_pair(-9999., -9999.);
    RooRealVar* mhiggs = dynamic_cast<RooRealVar*>(combined.var(varName));
    if (mhiggs) {
        return make_pair(mhiggs ->getVal(),mhiggs->getError());
    } else {
        return make_pair(-9999.,-9999.); 
    }
}

RooNLLVar* RooStatsHelper::createNLL(RooAbsData* _data, RooStats::ModelConfig* _mc)
{
    const RooArgSet& nuis = *_mc->GetNuisanceParameters();
    RooNLLVar* nll = (RooNLLVar*)_mc->GetPdf()->createNLL(*_data, 
            RooFit::Constrain(nuis), 
            RooFit::GlobalObservables(*_mc->GetGlobalObservables())
            );
    // RooCmdArg agg = condVarSet.getSize() > 0?RooFit::ConditionalObservables(condVarSet):RooCmdArg::none(); // for conditional RooFit
    // RooCmdArg agg = RooCmdArg::none();

    return nll;
}

double RooStatsHelper::getPvalue(RooWorkspace* combined, 
        RooStats::ModelConfig* mc, 
        RooAbsData* data, 
        const char* muName, 
        bool isRatioLogLikelihood)
{
    if(!combined || !mc || !data){
        log_err("check inputs");
        return -9999;
    }
    TString dataname(data->GetName());

    cout<<"Getting pvalue for "<< data->GetName()<<endl;
    if (data->numEntries() <= 0) {
        log_err("total number of events is less than 0: %d", 
                data->numEntries());
        return 1.0;
    }
    else {
        cout<<"total events: "<< data->numEntries() <<" sumEntries = "<< data ->sumEntries()<<endl;
    }

    RooRealVar* mu = (RooRealVar*) combined ->var(muName);
    if (!mu){
        log_err("%s does not exist", mu->GetName());
        return -9999;
    }

    if(!isRatioLogLikelihood){
        mu ->setConstant(0);
        mu->Print();
        cout<<"Fitting with "<<mu->GetName()<<" free"<<endl;
    }else{
        mu ->setVal(1);
        mu ->setConstant(1);
        cout<<"Fitting with "<<mu->GetName()<<"=1"<<endl;
    }

    auto combPdf = mc->GetPdf();
    if(!combPdf) {
        log_err("overall pdf does not exist!");
        return -9999;
    } else {
        combPdf->Print();
    }

    PrintExpEvts(combPdf, mu, mc->GetObservables());
    RooNLLVar* nll = createNLL(data, mc);
    
    minimize(nll, combined);
    double obs_nll_min = nll ->getVal();
    cout << "mu_hat for " << data->GetName() << " " << mu->getVal() << 
        " " << mu->getError() << " " << obs_nll_min << endl;
    delete nll;
    PrintExpEvts(combPdf, mu, mc->GetObservables());
    bool reverse = (mu ->getVal() < 0);

    cout<<"Fitting background only hypothesis "<< mu->GetName()<<endl;
    
    mu ->setVal(1.0e-200);
    mu ->setConstant(1);
    RooNLLVar* nllCond = createNLL(data, mc);
    minimize(nllCond, combined);
    double obs_nll_min_bkg = nllCond ->getVal();
    cout<<"NLL for background only: "<< obs_nll_min_bkg<<endl;
    delete nllCond;

    double obs_q0 = 2*(obs_nll_min_bkg - obs_nll_min);
    if(reverse) obs_q0 = -obs_q0;
    double sign = int(obs_q0 == 0 ? 0 : obs_q0 / fabs(obs_q0));  
    double obs_sig = sign*sqrt(fabs(obs_q0));  
    cout<<"NLL min: "<< obs_nll_min <<" "<< obs_nll_min_bkg <<" "<< obs_sig<<endl;

    return RooStats::SignificanceToPValue(obs_sig);
}

RooDataSet* RooStatsHelper::makeAsimovData(RooWorkspace* combined, 
        double muval, 
        double profileMu, 
        const char* muName, 
        const char* mcname, 
        const char* dataname, 
        bool doprofile)
{
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    cout<<"In making asimov-Data"<<endl;
    stringstream muStr("_");
    muStr << muval;
    if(muval==1){
        if(profileMu == 0) muStr<<"_paz";
        if(profileMu > 1) muStr <<"_pamh";
    }

    RooStats::ModelConfig* mcInWs = (RooStats::ModelConfig*) combined->obj(mcname);
    RooRealVar* mu = (RooRealVar*) combined->var(muName);
    mu->setVal(muval);

    const RooArgSet& mc_globs = *mcInWs->GetGlobalObservables();
    const RooArgSet& mc_nuis  = *mcInWs->GetNuisanceParameters();

    RooDataSet* combData = NULL;
    try { 
        combData = (RooDataSet*) combined->data(dataname);
    } catch (...) {}

    //save the snapshots of nominal parameters, but only if they're not already saved
    if (!combined->loadSnapshot("nominalGlobs"))
    {
        cout << "nominalGlobs doesn't exist. Saving snapshot." << endl;
        combined->saveSnapshot("nominalGlobs",*mcInWs->GetGlobalObservables());
    }
    if (!combined->loadSnapshot("nominalNuis"))
    {
        cout << "nominalNuis doesn't exist. Saving snapshot." << endl;
        combined->saveSnapshot("nominalNuis",*mcInWs->GetNuisanceParameters());
    }

    mu ->setVal(profileMu);
    if (profileMu > 1)
    { // would profile to the \hat_mu, 
        mu ->setConstant(kFALSE);
        mu ->setRange(-40,40);
        RooNLLVar* conditioning_nll = createNLL(combData, mcInWs);
        minimize(conditioning_nll, combined);//find the \hat_mu
        delete conditioning_nll;
    }
    mu ->setConstant(kTRUE); // Fix mu at the profileMu
    mu ->setRange(0,40);
    if (doprofile)
    { // profile nuisance parameters at mu = profileMu
        RooNLLVar* conditioning_nll = createNLL(combData, mcInWs);
        minimize(conditioning_nll, combined);
    }

    // loop over the nui/glob list, grab the corresponding variable from the tmp ws, 
    // and set the glob to the value of the nui
    TIter nuis_iter (mc_globs.createIterator());
    TIter glob_iter (mc_nuis.createIterator());
    RooRealVar* nuis;
    RooRealVar* glob;
    while ((
                glob = (RooRealVar*) glob_iter(),
                nuis = (RooRealVar*) nuis_iter()
           ))
    {
        if (!nuis || !glob) continue;
        // cout<<" nuisance Name: " << nuis->GetName() << endl;
        // cout<<" global Name: " << glob->GetName() << endl;
        glob->setVal(nuis->getVal());
    }
    combined->saveSnapshot(("conditionalGlobs"+muStr.str()).c_str(), mc_globs);
    combined->saveSnapshot(("conditionalNuis" +muStr.str()).c_str(), mc_nuis);


    cout<<"Making asimov data, "<< muval <<endl;
    mu->setVal(muval);
    int iFrame=0;

    const char* weightName="weightVar";
    RooArgSet obsAndWeight;
    obsAndWeight.add(*mcInWs->GetObservables());
    RooRealVar* weightVar = NULL;
    if (!(weightVar = combined->var(weightName)))
    {
        combined->import(*(new RooRealVar(weightName, weightName, 1,0,100000000)));
        weightVar = combined->var(weightName);
    }
    obsAndWeight.add(*combined->var(weightName));

    RooSimultaneous* simPdf = dynamic_cast<RooSimultaneous*>(mcInWs->GetPdf());
    map<string, RooDataSet*> asimovDataMap;

    RooCategory* channelCat = (RooCategory*)&simPdf->indexCat();
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    int nrIndices = 0;
    while((tt=(RooCatType*) iter->Next())) {
        nrIndices++;
    }
    for (int i=0;i<nrIndices;i++){
        channelCat->setIndex(i);
        iFrame++;
        // Get pdf associated with state from simpdf
        RooAbsPdf* pdftmp = simPdf->getPdf(channelCat->getLabel()) ;
        if(!pdftmp){
            cout<<"ERRORs no pdf associated with "<< channelCat ->getLabel()<<endl;
        }
        // Generate observables defined by the pdf associated with this state
        RooArgSet* obstmp = pdftmp->getObservables(*mcInWs->GetObservables()) ;

        RooDataSet* obsDataUnbinned = new RooDataSet(
                Form("combAsimovData%d",iFrame),
                Form("combAsimovData%d",iFrame),
                RooArgSet(obsAndWeight,*channelCat),
                RooFit::WeightVar(*weightVar)
                );
        RooRealVar* thisObs = ((RooRealVar*)obstmp->first());
        double expectedEvents = pdftmp->expectedEvents(*obstmp);
        double thisNorm = 0;
        if (obstmp->getSize() < 2) {
            for(int jj=0; jj<thisObs->numBins(); ++jj){
                thisObs->setBin(jj);

                thisNorm=pdftmp->getVal(obstmp)*thisObs->getBinWidth(jj);
                double expNumber = thisNorm*expectedEvents;
                if ( expNumber <= 0)
                {
                    expNumber = 1e-6;
                }
                if ( expNumber > pow(10.0, -9) && expNumber < pow(10.0, 9)) 
                    obsDataUnbinned->add(*mcInWs->GetObservables(), expNumber);
            }
        }
        else { // higher dimension
            RooArgList obs(*obstmp);
            RooRealVar* x = (RooRealVar*) obs.at(0);
            RooRealVar* y = obs.getSize() > 1? (RooRealVar*) obs.at(1): 0;
            RooRealVar* z = obs.getSize() > 2? (RooRealVar*) obs.at(2): 0;
            RooCmdArg ay = ( y ? RooFit::YVar( *y, RooFit::Binning(y->getBinning().numBins())) : RooCmdArg::none() );
            RooCmdArg az = ( z ? RooFit::YVar( *z, RooFit::Binning(z->getBinning().numBins())) : RooCmdArg::none() );
            RooRealVar* m_KD = (RooRealVar*)combined ->var("KD");
            std::auto_ptr<TH1> hist( pdftmp->createHistogram( "htemp", *x, RooFit::Binning(x->getBinning().numBins()), ay, az, RooFit::ConditionalObservables(RooArgSet(*m_KD))) );
            hist ->Scale(expectedEvents/ hist->Integral());
            if(obs.getSize() == 2){
                TH2& h2 = dynamic_cast<TH2&>(*hist);
                for (int ix = 1, nx = h2.GetNbinsX(); ix <= nx; ++ix){
                    for (int iy = 1, ny = h2.GetNbinsY(); iy <= ny; ++iy){
                        x->setVal( h2.GetXaxis()->GetBinCenter( ix ) );
                        y->setVal( h2.GetYaxis()->GetBinCenter( iy ) );
                        obsDataUnbinned ->add( *mcInWs->GetObservables(), h2.GetBinContent( ix, iy ) );
                    }
                }
            }
        }
        obsDataUnbinned->Print("v");

        if(obsDataUnbinned->sumEntries()!=obsDataUnbinned->sumEntries()){
            cout << "sum entries is nan"<<endl;
            exit(1);
        }

        asimovDataMap[string(channelCat->getLabel())] = obsDataUnbinned;//tempData;
    }

    RooDataSet* asimovData = new RooDataSet(
            ("asimovData_"+muStr.str()).c_str(),
            ("asimovData_"+muStr.str()).c_str(),
            RooArgSet(obsAndWeight,*channelCat),
            RooFit::Index(*channelCat),
            RooFit::Import(asimovDataMap),
            RooFit::WeightVar(*weightVar)
            );
    combined->import(*asimovData);
    cout<<"AsimovData is created"<<endl;
    asimovData ->Print("v");
    cout<< asimovData ->sumEntries()<<endl;

    combined->loadSnapshot("nominalGlobs");
    return asimovData;
}

double RooStatsHelper::getRoughSig(double s, double b)
{
    return TMath::Sqrt(2*((s+b)*TMath::Log(1+s/b)-s));
}

void RooStatsHelper::randomizeSet(RooAbsPdf* pdf, RooArgSet* globs, int seed)
{
    RooRandom::randomGenerator() -> SetSeed(seed) ; // This step is necessary
    RooDataSet *one= pdf->generate(*globs, 1);
    const RooArgSet *values= one->get(0);
    RooArgSet *allVars=pdf->getVariables();
    *allVars=*values;
    delete one;
    delete allVars;
}

void RooStatsHelper::SetRooArgSetConst(RooArgSet& argset, bool flag) {
    TIterator* iter = argset.createIterator();
    RooRealVar* par = NULL;
    while( (par = (RooRealVar*) iter->Next()) ){
        par->setConstant(flag);
    }
    delete iter;
}

void RooStatsHelper::generateToy(RooWorkspace* w , 
        const char* poi_name, double poi_value, int seed, 
        map<string, double>& result)
{
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
    // map<string,double> result ;
    cout<<"in generate toys"<<endl;
    /**
     * before call this function, 
     * better to fit to data first
     * and saved snapshot of NP and GO(global observables)
     * w->saveSnapshot("bestNP", *(mc->GetNuisanceParameters()));
     * w->saveSnapshot("bestGO", *(mc->GetGlobalObservables()));
     ***/
    try {
        w->loadSnapshot("bestNP");
        w->loadSnapshot("bestGO");
    } catch (...){
    }

    RooStats::ModelConfig *mc=(RooStats::ModelConfig*)w->obj("ModelConfig");
    RooSimultaneous* combPdf =  (RooSimultaneous*) mc->GetPdf() ;
    RooArgSet* nuisanceParameters = (RooArgSet*) mc->GetNuisanceParameters();
    RooArgSet* globalObservables = (RooArgSet*) mc->GetGlobalObservables();

    RooRealVar* muVar =(RooRealVar*) w ->var(poi_name);
    double mu_old = muVar->getVal();

    // +++++++++++++++++++++++ Step 1: Generate toy data ++++++++++++++++++++++++

    muVar->setVal(poi_value);
    muVar->setConstant() ;

    /* Fix nuisance parameters, 
     * randomize global observables,
     * make asimov data **/
    SetRooArgSetConst(*nuisanceParameters);
    TIter nuis_iter (nuisanceParameters->createIterator());
    TIter glob_iter (globalObservables->createIterator());
    RooRealVar* nuis;
    RooRealVar* glob;
    while ((
                glob = (RooRealVar*) glob_iter(),
                nuis = (RooRealVar*) nuis_iter()
           ))
    {
        if (!nuis || !glob) continue;
        glob->setVal(nuis->getVal());
    }
    // randomizeSet(combPdf, globalObservables, seed); 
     SetRooArgSetConst(*globalObservables);

    RooArgSet obsAndWeight;
    obsAndWeight.add(*mc->GetObservables());
    map<string, RooDataSet*> toyDataMap;
    RooCategory* channelCat = (RooCategory*)&combPdf->indexCat();
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    int nrIndices = 0;
    while((tt=(RooCatType*) iter->Next())) {
        nrIndices++;
    }
    for (int i=0;i<nrIndices;i++){
        channelCat->setIndex(i);
        // Get pdf associated with state from simpdf
        RooAbsPdf* pdftmp = combPdf->getPdf(channelCat->getLabel()) ;
        if(!pdftmp){
            cout<<"ERRORs no pdf associated with "<< channelCat ->getLabel()<<endl;
        }
        // Generate observables defined by the pdf associated with this state

        RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;
        RooDataSet* obsDataUnbinned = pdftmp->generate(*obstmp, RooFit::Extended());
        obsDataUnbinned->Print("v");

        if(TMath::IsNaN(obsDataUnbinned->sumEntries())){
            cout << "sum entries is nan"<<endl;
            exit(1);
        }
        toyDataMap[string(channelCat->getLabel())] = obsDataUnbinned;//tempData;
    }
    RooDataSet* toyData = new RooDataSet(
            Form("toyData_%d",seed),
            Form("toyData_%d",seed),
            RooArgSet(obsAndWeight,*channelCat),
            RooFit::Index(*channelCat),
            RooFit::Import(toyDataMap)
            );
    toyData->Print();
    // +++++++++++++++++++++++ Step 2: Fit toy data ++++++++++++++++++++++++
    // release mh and mu
    muVar->setConstant(false);

    // release nuisance parameters before fit.
    SetRooArgSetConst(*nuisanceParameters, false);
    SetRooArgSetConst(*globalObservables, false);
    w->loadSnapshot("nominalGlobs");
    SetRooArgSetConst(*globalObservables, true);

    //perform unconditional fit 
    RooNLLVar* nll = createNLL(toyData, mc);

    cout<<"unconditional fit"<<endl;
    int status = (minimize(nll, w) == NULL)?0:1;
    result["nll_hat"] = nll->getVal();
    result["poi_hat"] = muVar->getVal(); 
    result["status_hat"] = status*1.0;
    delete nll;
    // cout<<"finished unconditional fit, running conditional fit(mass)"<<endl;

    cout<<"conditional fit"<<endl;
    //*******perform conditional fit******
    muVar->setVal(poi_value);
    muVar->setConstant(true);
    RooNLLVar* nllcond =createNLL(toyData, mc);
    status = (minimize(nllcond, w) == NULL)?0:1;
    result["nll_cond"] = nllcond->getVal();
    result["poi_cond"] = muVar->getVal(); 
    result["status_cond"] = status*1.0;
    delete nllcond;

    /* reset */
    muVar ->setVal(mu_old);
    delete  toyData;
    cout<<"out of generating toys"<<endl;
}

bool RooStatsHelper::ScanPOI(RooWorkspace* ws, 
        const string& data_name,
        const string& poi_name, 
        int total, double low, double hi,
        TTree* tree)
{
    if(!ws || !tree) {
        log_err("ws or tree is null");
        return false;
    }
    RooStats::ModelConfig*  mc_config = (RooStats::ModelConfig*) ws->obj("ModelConfig");
    RooDataSet* dataset = (RooDataSet*) ws->data(data_name.c_str());
    if(!dataset) {
        log_err("dataset: %s does not exist", data_name.c_str());
        return false;
    }
    RooRealVar* poi = (RooRealVar*) ws->var(poi_name.c_str());
    if(!poi) {
        log_err("POI: %s does not exist", poi_name.c_str());
        return false;
    }
    RooSimultaneous* simPdf = dynamic_cast<RooSimultaneous*>(mc_config->GetPdf());

    RooRealVar* m4l = (RooRealVar*) ws->var("m4l");
    m4l->setRange("signal", 118, 129);
    bool is_fixed_poi = poi->isConstant();
    poi->setConstant(false);

    double val_nll, val_poi, val_status, obs_sig;
    tree->Branch("NLL", &val_nll, "NLL/D");
    tree->Branch("Status", &val_status, "NLL/D");
    tree->Branch(poi_name.c_str(), &val_poi, Form("%s/D",poi_name.c_str()));
    tree->Branch("obs_sig", &obs_sig, "obs_sig/D");

    TStopwatch timer;
    timer.Start();
    //get best fit
    RooNLLVar* nll = createNLL(dataset, mc_config);
    int status = (minimize(nll) == NULL)?0:1 ; 
    timer.Stop();
    cout<<"One fit takes: "<< endl; Helper::printStopwatch(timer);
    timer.Reset(); timer.Start();

    val_nll = nll ->getVal();
    val_poi = poi->getVal();
    val_status = status;
    bool do_subrange = false;
    obs_sig = GetObsNevtsOfSignal(simPdf, poi, mc_config->GetObservables(), do_subrange);
    tree->Fill();
    double step = (hi-low)/total;
    for(int i = 0; i < total; i++){
        double poi_value = low + i*step;
        poi->setVal(poi_value);
        poi->setConstant(true);
        minimize(nll);
        val_nll = nll ->getVal();
        val_poi = poi->getVal();
        val_status = status;
        obs_sig = GetObsNevtsOfSignal(simPdf, poi, mc_config->GetObservables(), do_subrange);
        tree->Fill();
    }
    if(!is_fixed_poi) poi->setConstant(false);
    delete nll;
    return true;
}

void RooStatsHelper::unfoldConstraints(RooArgSet& initial, RooArgSet& final, RooArgSet& obs, RooArgSet& nuis, int& counter)
{
  if (counter > 50)
  {
    cout << "ERROR::Couldn't unfold constraints!" << endl;
    cout << "Initial: " << endl;
    initial.Print("v");
    cout << endl;
    cout << "Final: " << endl;
    final.Print("v");
    exit(1);
  }
  TIterator* itr = initial.createIterator();
  RooAbsPdf* pdf;
  while ((pdf = (RooAbsPdf*)itr->Next()))
  {
    RooArgSet nuis_tmp = nuis;
    RooArgSet constraint_set(*pdf->getAllConstraints(obs, nuis_tmp, false));
    string className(pdf->ClassName());
    if (className != "RooGaussian" && className != "RooLognormal" && className != "RooGamma" && className != "RooPoisson" && className != "RooBifurGauss")
    {
      counter++;
      unfoldConstraints(constraint_set, final, obs, nuis, counter);
    }
    else
    {
      final.add(*pdf);
    }
  }
  delete itr;
}

void RooStatsHelper::PrintExpEvts(RooAbsPdf* inputPdf, 
        RooRealVar* mu, const RooArgSet* obs)
{
    auto simPdf = dynamic_cast<RooSimultaneous*>(inputPdf);
    if (!simPdf) return;

    const RooCategory& category = dynamic_cast<const RooCategory&>(simPdf->indexCat());
    TIter cat_iter(category.typeIterator());
    RooCatType* obj;
    double old_mu = mu->getVal();
    while( (obj= (RooCatType*)cat_iter()) ){
        const char* label_name = obj->GetName();
        RooAbsPdf* pdf = simPdf->getPdf(label_name);
        mu->setVal(0.0);
        double bkg_evts = pdf->expectedEvents(*obs);
        mu->setVal(old_mu);
        double all_evts = pdf->expectedEvents(*obs);
        printf("%s %.3f %.3f %.3f\n", label_name, all_evts, all_evts-bkg_evts, bkg_evts);
    }
    mu->setVal(old_mu);
}

double RooStatsHelper::GetObsNevtsOfSignal(RooSimultaneous* simPdf, RooRealVar* mu, const RooArgSet* obs, bool subrange)
{
    if(!simPdf || !obs || !mu){
        return -999.;
    }
    double result = 0;
    const RooCategory& category = *dynamic_cast<const RooCategory*>(&simPdf->indexCat());
    TIter cat_iter(category.typeIterator());
    RooCatType* obj;
    double old_mu = mu->getVal();
    while( (obj= (RooCatType*)cat_iter()) ){
        const char* label_name = obj->GetName();
        RooAbsPdf* pdf = simPdf->getPdf(label_name);
        mu->setVal(0.0);
        double bkg_evts = pdf->expectedEvents(*obs);
        double fraction_bkg = 1.0;
        if (subrange) fraction_bkg = pdf->createIntegral(*obs, RooFit::Range("signal"))->getVal();
        mu->setVal(old_mu);
        double all_evts = pdf->expectedEvents(*obs);
        double fraction_all = 1.0;
        if(subrange) fraction_all = pdf->createIntegral(*obs, RooFit::Range("signal"))->getVal();
        result += (all_evts*fraction_all - bkg_evts*fraction_bkg);
    }
    mu->setVal(old_mu);
    return result;
}


RooAbsData* RooStatsHelper::generatePseudoData(RooWorkspace* w, const char* poi_name, int seed)
{
    cout<<"in generate pseudo-data with seed: "<< seed << ", poi name: " << poi_name << endl;
    /**
     * before call this function, 
     * fit to data first
     * and saved snapshot of NP and GO(global observables)
     * w->saveSnapshot("bestNP", *(mc->GetNuisanceParameters()));
     * w->saveSnapshot("bestGO", *(mc->GetGlobalObservables()));
     ***/
    auto* poi = (RooRealVar*) w->var(poi_name);
    if(!poi) {
        cout <<"POI is not defined" << endl;
        return NULL;
    }
    double old_poi_val = poi->getVal();
    try {
        w->loadSnapshot("condNP");
        w->loadSnapshot("condGO");
    } catch (...) {
        return NULL;
    }

    RooStats::ModelConfig *mc=(RooStats::ModelConfig*)w->obj("ModelConfig");
    RooSimultaneous* combPdf =  (RooSimultaneous*) mc->GetPdf() ;
    RooArgSet* nuisanceParameters = (RooArgSet*) mc->GetNuisanceParameters();
    RooArgSet* globalObservables = (RooArgSet*) mc->GetGlobalObservables();

    /* Fix nuisance parameters, 
     * set global observables to np values,
     * make asimov data **/
    SetRooArgSetConst(*nuisanceParameters);
    TIter nuis_iter (nuisanceParameters->createIterator());
    TIter glob_iter (globalObservables->createIterator());
    RooRealVar* nuis;
    RooRealVar* glob;
    while ((
                glob = (RooRealVar*) glob_iter(),
                nuis = (RooRealVar*) nuis_iter()
           ))
    {
        if (!nuis || !glob) continue;
        glob->setVal(nuis->getVal());
    }
    randomizeSet(combPdf, globalObservables, seed);
    SetRooArgSetConst(*globalObservables);
    SetRooArgSetConst(*nuisanceParameters, false);

    RooArgSet obsAndWeight;
    obsAndWeight.add(*mc->GetObservables());
    map<string, RooDataSet*> toyDataMap;
    RooCategory* channelCat = (RooCategory*)&combPdf->indexCat();
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    int nrIndices = 0;
    while((tt=(RooCatType*) iter->Next())) {
        nrIndices++;
    }

    poi->setVal(old_poi_val);
    for (int i=0;i<nrIndices;i++){
        channelCat->setIndex(i);
        // Get pdf associated with state from simpdf
        RooAbsPdf* pdftmp = combPdf->getPdf(channelCat->getLabel()) ;
        if(!pdftmp){
            cout<<"ERRORs no pdf associated with "<< channelCat ->getLabel()<<endl;
        }
        // Generate observables defined by the pdf associated with this state

        RooArgSet* obstmp = pdftmp->getObservables(*mc->GetObservables()) ;
        RooDataSet* obsDataUnbinned = pdftmp->generate(*obstmp, RooFit::Extended());
        // obsDataUnbinned->Print("v");

        if(TMath::IsNaN(obsDataUnbinned->sumEntries())){
            cout << "sum entries is nan"<<endl;
            exit(1);
        }
        toyDataMap[string(channelCat->getLabel())] = obsDataUnbinned;//tempData;
    }
    RooDataSet* toyData = new RooDataSet(
            Form("toyData_%d",seed),
            Form("toyData_%d",seed),
            RooArgSet(obsAndWeight,*channelCat),
            RooFit::Index(*channelCat),
            RooFit::Import(toyDataMap)
            );
    try {
        w->loadSnapshot("nominalNP");
        w->loadSnapshot("nominalGO");
    } catch(...) {
        log_err("save a snapshot of nominal values");
    }
    return toyData;
}

bool RooStatsHelper::fixTermsWithPattern(RooStats::ModelConfig* mc, const char* pat) 
{
    RooArgSet nuis(*mc->GetNuisanceParameters());
    TIter iter(nuis.createIterator());
    RooRealVar* par = NULL;
    while( (par = (RooRealVar*) iter()) ) {
        if(string(par->GetName()).find(pat) != string::npos) {
            par->setConstant();
        }
    }
    return true;
}

void RooStatsHelper::fixVariables(RooWorkspace* workspace, const string& options, RooStats::ModelConfig* mc) 
{
    // options can be like: "mG:750,GkM:0.02"
    if (!workspace || options == "") return;
    vector<string> tokens;
    Helper::tokenizeString(options, ',', tokens);
    if (tokens.size() < 1) return ;

    for(auto iter = tokens.begin(); iter != tokens.end(); iter++)
    {
        string token(*iter);
        size_t delim_pos = token.find(':');
        string var_name = token;
        double var_val = nan("NaN");
        if(delim_pos != string::npos){
            var_name = token.substr(0, delim_pos);
            var_val = atof( token.substr(delim_pos+1, token.size()).c_str());
        }

        if(token.find("gamma_stat") != string::npos && mc) {
            const char* pat = "gamma_stat";
            RooArgSet nuis(*mc->GetNuisanceParameters());
            TIter iter(nuis.createIterator());
            RooRealVar* par = NULL;
            while( (par = (RooRealVar*) iter()) ) {
                if(string(par->GetName()).find(pat) != string::npos) {
                    if(!std::isnan(var_val)) { // use std::isnan to avoid ambigulity
                        par->setVal(var_val);
                    }
                    par->setConstant();
                    log_info("%s fixed to %.2f", par->GetName(), par->getVal());
                }
            }
            continue;
        }

        auto par = (RooRealVar*) workspace->var(var_name.c_str());
        if(!par) {
            log_warn("%s does not exist", var_name.c_str());
            continue;
        } else {
            if(!std::isnan(var_val)) {
                double low_val = var_val - 1, hi_val = var_val + 1;
                par->setRange(low_val, hi_val);
                par->setVal(var_val);
            }
            par->setConstant();
            log_info("%s fixed to %.2f", var_name.c_str(), par->getVal());
        }
    }
}

bool RooStatsHelper::CheckNuisPdfConstraint(const RooArgSet* nuis, const RooArgSet* pdfConstraint)
{
    if(!nuis || nuis->getSize() < 1) {
        cout << "no nusiance parameter is found" << endl;
        return true;
    }
    if(nuis->getSize() != pdfConstraint->getSize())
    {
        log_err("number of nuisance %d; pdfConstraint size: %d", nuis->getSize(), pdfConstraint->getSize());

        TIterator* iterNuis = nuis->createIterator();
        TIterator* iterPdfCons = pdfConstraint->createIterator();
        RooRealVar* nuisVal;
        RooAbsPdf* pdfCon;
        while((nuisVal = (RooRealVar*)iterNuis->Next())){
            TString constrainName(Form("%sConstraint",nuisVal->GetName())); 
            if(pdfConstraint->find(constrainName.Data()) == NULL){
                cout <<nuisVal->GetName()<<" not Constraint"<<endl;
            }
        }
        while((pdfCon = (RooAbsPdf*)iterPdfCons->Next())){
            TString nuisName(pdfCon ->GetName());
            nuisName.ReplaceAll("Constraint","");
            if(nuis->find(nuisName.Data()) == NULL){
                cout <<nuisName<<" is omitted"<<endl;
            }
        }
        return false;
    } else {
        cout <<"total number of nuisance parameters: "<<nuis->getSize()<<endl;
    }
    return true;
}
