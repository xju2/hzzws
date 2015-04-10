#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "RooFitResult.h"
#include "RooMinimizer.h"
#include "RooSimultaneous.h"
#include "RooDataSet.h"
#include "TSystem.h"
#include "TROOT.h"
#include "RooCmdArg.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooStats/RooStatsUtils.h"
#include "TH2.h"

#include "Hzzws/RooStatsHelper.h"

namespace RooStatsHelper { 

void setDefaultMinimize(){
  ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
  ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
  ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(-1);
}

int minimize(RooNLLVar* nll, RooWorkspace* combWS)
{
  int printLevel = ROOT::Math::MinimizerOptions::DefaultPrintLevel();
  // RooFit::MsgLevel msglevel = RooMsgService::instance().globalKillBelow();

  if (printLevel < 0) RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

  int strat = 1;
  RooMinimizer minim(*nll);
  minim.optimizeConst(1);
  minim.setStrategy(strat);
  minim.setPrintLevel(printLevel);
  
  
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
  if(status != 0 && status != 1) cout<<"Error: the fit does not converge"<<endl;
  return status;
}

void setVarfixed(RooWorkspace* combined, const char* varName, double imass)
{
  RooRealVar* _var = (RooRealVar*) combined->var(varName);
  if (_var) {
    cout<<varName<<" fixed to " << imass<<endl;
    _var->setVal(imass);
    _var->setConstant(1);
  } 
  else {
    cout << "Error: cannot find " << varName <<endl;
  }
}

void setVarFree(RooWorkspace* combined, const char* varName){
  RooRealVar* _var = (RooRealVar*) combined->var(varName);
  if (_var) {
    _var->setConstant(kFALSE);
  }
  else {
    cout << "Error: RooStatsHelper cannot find " << varName << endl;
  }
}

pair<double,double> getVarVal(RooWorkspace* combined, const char* varName){
  RooRealVar* mhiggs = (RooRealVar*) combined->var(varName);
  if(mhiggs){
    return make_pair(mhiggs ->getVal(),mhiggs->getError());
  }else{
    return make_pair(-99999.,-99999.); 
  }
}

RooNLLVar* createNLL(RooAbsData* _data, RooStats::ModelConfig* _mc, const char* channelName)
{
  const RooArgSet& nuis = *_mc->GetNuisanceParameters();
  TString chname(channelName);
  RooNLLVar* nll(NULL);

  // RooCmdArg agg = condVarSet.getSize() > 0?RooFit::ConditionalObservables(condVarSet):RooCmdArg::none(); // for conditional RooFit
  RooCmdArg agg = RooCmdArg::none();
     
  if(!chname.EqualTo("combined")){
    RooAbsPdf* pdf =((RooSimultaneous*) _mc->GetPdf())->getPdf(channelName);
    //can I put all nusiance parameter for subchannel?
    nll = (RooNLLVar*) pdf->createNLL(*_data, RooFit::Constrain(nuis), agg);
  }else{
    nll = (RooNLLVar*)_mc->GetPdf()->createNLL(*_data, RooFit::Constrain(nuis), agg);
  }
  return nll;
}

double getPvalue(RooWorkspace* combined, RooAbsPdf* combPdf, RooStats::ModelConfig* mc, 
        RooAbsData* data, RooArgSet* nuis_tmp, const char* conditionalName, 
        const char* muName, bool isRatioLogLikelihood)
{
    TString dataname(data->GetName());

    cout<<"Getting pvalue for "<< data->GetName()<<endl;
    if (data->numEntries() <= 0) {
        cout<<"total number of events is less than 0: "<< data->numEntries()<<", return 1"<<endl;
        return 1.0;
    }
    else {
        cout<<"total events: "<< data->numEntries() <<" sumEntries = "<< data ->sumEntries()<<endl;
    }

    RooRealVar* mu = (RooRealVar*) combined ->var(muName);
    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);

    if(!isRatioLogLikelihood){
        mu ->setConstant(0);
        cout<<"Fitting with "<<mu->GetName()<<" free"<<endl;
    }else{
        mu ->setVal(1);
        mu ->setConstant(1);
    }
    combined ->loadSnapshot("nominalGlobs");
    RooNLLVar* nll = createNLL(data, mc, "combined");
    minimize(nll, combined);
    double obs_nll_min = nll ->getVal();
    cout << "mu_hat for " << data->GetName() << " " << mu->getVal() << 
        " " << mu->getError() << " " << obs_nll_min << endl;
    delete nll;
    bool reverse = (mu ->getVal() < 0);

    cout<<"Fitting background only hypothesis "<< mu->GetName()<<endl;
    combined ->loadSnapshot("nominalGlobs");
    mu ->setVal(1.0e-200);
    mu ->setConstant(1);
    RooNLLVar* nllCond = createNLL(data, mc, "combined");
    minimize(nllCond, combined);
    double obs_nll_min_bkg = nllCond ->getVal();
    cout<<"NLL for background only: "<< obs_nll_min_bkg<<endl;
    delete nllCond;

    double obs_q0 = 2*(obs_nll_min_bkg - obs_nll_min);
    if(reverse) obs_q0 = -obs_q0;
    double sign = int(obs_q0 == 0 ? 0 : obs_q0 / fabs(obs_q0));  
    double obs_sig = sign*sqrt(fabs(obs_q0));  
    cout<<"NLL min: "<< obs_nll_min <<" "<< obs_nll_min_bkg <<" "<< obs_sig<<endl;

    //return 0.5*(1 - TMath::Erf(obs_sig/sqrt(2.0))); 
    return RooStats::SignificanceToPValue(obs_sig);
}

RooDataSet* makeAsimovData(RooWorkspace* combined, 
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

    // RooAbsPdf*  combPdf = (RooAbsPdf*) mcInWs ->GetPdf();
    RooDataSet* combData = (RooDataSet*) combined->data(dataname);

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
    //combPdf ->fitTo(*combData,Hesse(false),Minos(false),PrintLevel(0),Extended(), Constrain(nuiSet_tmp));
    if (profileMu > 1)
    { // would profile to the \hat_mu, 
        mu ->setConstant(kFALSE);
        mu ->setRange(-40,40);
        RooNLLVar* conditioning_nll = createNLL(combData, mcInWs, "combined");
        minimize(conditioning_nll, combined);//find the \hat_mu
        delete conditioning_nll;
    }
    mu ->setConstant(kTRUE); // Fix mu at the profileMu
    mu ->setRange(0,40);
    if (doprofile)
    { // profile nuisance parameters at mu = profileMu
        RooNLLVar* conditioning_nll = createNLL(combData, mcInWs, "combined");
        minimize(conditioning_nll, combined);
    }

    //loop over the nui/glob list, grab the corresponding variable from the tmp ws, and set the glob to the value of the nui
    TIterator* nuis_iter = mc_globs.createIterator();
    TIterator* glob_iter = mc_nuis.createIterator();
    RooRealVar* nuis;
    RooRealVar* glob;
    while ((glob = (RooRealVar*) glob_iter->Next(),
                nuis = (RooRealVar*) nuis_iter->Next()))
    {
        glob->setVal(nuis->getVal());
    }
    delete nuis_iter;
    delete glob_iter;
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
            ("asimovData"+muStr.str()).c_str(),
            ("asimovData"+muStr.str()).c_str(),
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
 
}
