// =========================================================================
// 
//    Description:  
// 
// ==========================================================================
#include "Hzzws/CBGauss.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include <RooDataHist.h>
#include <TKey.h>
#include <TString.h>
#include <TSystem.h>

#include "RooGlobalFunc.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "RooStats/HistFactory/FlexibleInterpVar.h"
#include <RooStats/HistFactory/RooBSpline.h>
#include "RooExpandedDataHist.h"
#include "RooExpandedHistPdf.h"
#include "RooNDKeysPdf.h"
#include "Roo1DMomentMorphFunction.h"
#include "RooPolyVar.h"
#include "RooCBShape.h"
#include "RooAddPdf.h"
#include "RooWorkspace.h"

#include "Hzzws/Helper.h"
#include "Hzzws/BinningUtil.h"

using namespace RooStats;
using namespace HistFactory;

CBGauss::CBGauss(const char* _name, 
        const char* _input,  
        const char* _shape_sys,
        bool _doSys
        ) : SampleBase(_name)
    , inputParameterFile(_input)
      , inputShapeSysFile(_shape_sys)
      , workspace(new RooWorkspace("CBGauss"))
    , mH(new RooRealVar("mH","mH",200.,1000.))
    , doSys(_doSys)
    , order_(3)
    , masses_(new vector<double>())
      , bases_(NULL)
{
    shape_sys_names_ = new vector<string>();
}

CBGauss::~CBGauss()
{
    if(mH) delete mH;
    //delete workspace;
    delete masses_;
    for (int i(0); i< (int)shape_mean_sys_.size(); ++i)
        delete shape_mean_sys_[i];
    for (int i(0); i< (int)shape_sigma_sys_.size(); ++i)
        delete shape_sigma_sys_[i];

    if(shape_sys_names_) delete shape_sys_names_;
}

bool CBGauss::setChannel(const RooArgSet& _obs, const char* _ch_name, bool with_sys)
{
    SampleBase::setChannel(_obs,_ch_name,with_sys);

    std::cout<<"setting channel for shape_mean_sys_"<<std::endl;
    for (auto& s: shape_mean_sys_)
        s->SetChannel(category_name_.c_str());

    std::cout<<"setting channel for shape_sigma_sys_"<<std::endl;
    for (auto& s: shape_sigma_sys_)
        s->SetChannel(category_name_.c_str());

    shape_sys_names_->clear();
    makeCBGParameterization();
    return true;
}

void CBGauss::makeCBGParameterization()
{


  loadTextInputFile();

  //The mu parameter is a polynomial in all channels    
  RooProduct* a0cb = variable("CB_mu_p0");
  RooProduct* a1cb = variable("CB_mu_p1");

  string outputName = category_name_;

  RooPolyVar meanPoly( ( outputName + "_mean" ).c_str(), "CB #mu", *mH, RooArgList( *a0cb, *a1cb ) );
  workspace->import(meanPoly);

  /* 
    // systematics implemented as shifts in mu
    vector<string> names;
    vector<double> lowValues;
    vector<double> highValues;

    if (channel.find("mu")!=string::npos) {
      names.push_back("ATLAS_MU_MS");
      lowValues.push_back(0.999);
      highValues.push_back(1.001);
    }

    if (channel.find("e_")!=string::npos || channel.find("e2mu_")!=string::npos) {
      names.push_back("ATLAS_EM_ES_Z");
      lowValues.push_back(0.999);
      highValues.push_back(1.001);
    }

    FlexibleInterpVar* fiv = flexibleInterpVar( outputName+"_muCB", names, lowValues, highValues);

    muCBargs.add(*fiv);
  */

  string shortch="";
  if (category_name_.find("4mu")!=string::npos) shortch="4mu";
  else if (category_name_.find("4e")!=string::npos) shortch="4e";
  else if (category_name_.find("2e2mu")!=string::npos) shortch="2e2mu";
  else if (category_name_.find("2mu2e")!=string::npos) shortch="2mu2e";

  //The alpha parameter is a polynomial in all channels    
  RooProduct* a0cba = variable("CB_alpha_p0");
  RooArgList alphaCBargs(*a0cba);
  if (shortch=="4mu" || shortch=="2e2mu" || shortch=="2mu2e") {
    RooProduct* a1cba = variable("CB_alpha_p1");
    alphaCBargs.add(*a1cba);
    RooProduct* a2cba = variable("CB_alpha_p2");      
    alphaCBargs.add(*a2cba);
  }
  RooPolyVar alphaCB( ( outputName + "_alphaCB" ).c_str(), "CB alpha", *mH, alphaCBargs );
  workspace->import(alphaCB);

  //The n parameter is a constant in each channel
  RooRealVar nCB( (outputName + "_nCB_var").c_str(), "nCB", readTextInputFile("CB_nn_p0").first);
  workspace->import(nCB);

  //The s parameter is a polynomial in all channels
  RooProduct* a0ga = variable("GA_s_p0");
  RooProduct* a1ga = variable("GA_s_p1");
  RooArgList sGAArgs(*a0ga, *a1ga);
  if (shortch=="4mu" || shortch=="2mu2e" || shortch=="2e2mu"){
    RooProduct* a2ga = variable("GA_s_p2");
    sGAArgs.add(*a2ga);
  }
  RooPolyVar sGA( ( outputName + "_sGA" ).c_str(), "CB #mu", *mH, sGAArgs);
  workspace->import(sGA);

  //The f parameter 
  RooProduct* f0cb = variable("CB_f_p0");
  RooProduct* f1cb = variable("CB_f_p1");
  RooArgList fCBargs(*f0cb, *f1cb);
  if (shortch=="2e2mu" || shortch=="2mu2e") {
    RooProduct* f2cb = variable("CB_f_p2");
    fCBargs.add(*f2cb);
  }
  RooPolyVar fCB( ( outputName + "_fCB" ).c_str(), "CB f", *mH, fCBargs );
  workspace->import(fCB);

  //The sCB parameter is a polynomial in all channels
  RooProduct* a0cbs = variable("CB_s_p0");
  RooProduct* a1cbs = variable("CB_s_p1");
  RooArgList sCBArgs(*a0cbs, *a1cbs);
  if (shortch=="4mu" || shortch=="2mu2e" || shortch=="2e2mu"){
    RooProduct* a2cbs = variable("CB_s_p2");
    sCBArgs.add(*a2cbs);
  }
  RooPolyVar sCB( ( outputName + "_sCB" ).c_str(), "CB #sigma", *mH, sCBArgs );
  workspace->import(sCB);

  
  
}

bool CBGauss::addShapeSys(const TString& npName)
{
  std::cout<<"called addShapeSys "<<npName<<std::endl;
    /* In this specific channel the NP name will be:
     * pdf_name + category_name + parameter_name
     * ATLAS_Signal_ggH_ggF_4mu_13TeV_GA_s_p0
     */
    bool resultCB = false;

  if(npName.Contains(category_name_.c_str()))         //Cb parameterization systematics
  {
    for(const auto& sys_name : *shape_sys_names_){
      if(npName.Contains(sys_name.c_str())){
        resultCB = true;
        break;
      }
    }
  }

  bool resultMean = true;                                  //Mean shape systematics
  if (!resultCB){
    for(const auto& nSysHandler : shape_mean_sys_){
      if(! nSysHandler->AddSys(npName)) resultMean = false;
    }
  }

  bool resultSigma = true;                                  //Sigma shape systematics
  if (!resultCB){
    for(const auto& nSysHandler : shape_sigma_sys_){
      if(! nSysHandler->AddSys(npName)){ 
          log_err("Cannot add %s for sigma variation in category: %s", npName.Data(),category_name_.c_str() );
          resultSigma = false;
      }
    }
  }

  return resultSigma||resultMean||resultCB;
}

RooAbsPdf* CBGauss::getPDF()
{

  RooRealVar& m=(RooRealVar&)this->obs_list_[0];
  string outputName = category_name_;

  //Get polynomial functions/variables/whatever from workspace
  RooAbsReal* mean= (RooAbsReal*)workspace->obj((category_name_+"_mean").c_str());
  RooAbsReal* sGA = (RooAbsReal*)workspace->obj((category_name_+"_sGA").c_str());
  RooAbsReal* sCB = (RooAbsReal*)workspace->obj((category_name_+"_sCB").c_str());
  RooAbsReal* alphaCB = (RooAbsReal*)workspace->obj((category_name_+"_alphaCB").c_str());
  RooAbsReal* nCB = (RooAbsReal*)workspace->obj((category_name_+"_nCB_var").c_str());
  RooAbsReal* fCB = (RooAbsReal*)workspace->obj((category_name_+"_fCB").c_str());

  if (!mean || !sGA || !sCB || !alphaCB || !nCB || !fCB){
    std::cerr<<"ERROR! in "<<__func__<<" missing CB+GA parameters"<<std::endl;
    return NULL;
  }

  if (doSys){

    // *********************************
    //Systematics on mean
    //
    RooArgList meanArgs(*mean);

    RooAbsReal* meanSys = getShapeSys("mean");
    if (meanSys)
      meanArgs.add(*meanSys);

    mean = new RooProduct(Form("%s_withsys",mean->GetName()), "CB #mu w/ sys",meanArgs);
    
    // *********************************
    //Systematics on sigma (gaus or CB)
    //

    RooAbsReal*& sigma = (category_name_.find("4mu")!=std::string::npos ? sGA : sCB); //4mu has sys on sGA, 2mu2e and 4e have sys on sCB
    RooArgList sigmaArgs(*sigma);

    RooAbsReal* sigmaSys = getShapeSys("sigma");
    if (sigmaSys)
      sigmaArgs.add(*sigmaSys);

    sigma = new RooProduct(Form("%s_withsys",sigma->GetName()), "",sigmaArgs);
  }


  //build pdfs 
  RooCBShape* tmpcb=new RooCBShape( ( outputName + "_cb" ).c_str(), "Crystal Ball", m, *mean, *sCB, *alphaCB, *nCB );
  RooGaussian* tmpga=new RooGaussian( ( outputName + "_ga" ).c_str(), "Gaussian", m, *mean, *sGA );
  RooAddPdf* tmpadd=new RooAddPdf( ( outputName + "_add" ).c_str(), "Crystal Ball + Gaussian", *tmpcb , *tmpga , *fCB );

  workspace->import(*tmpadd,RooFit::RecycleConflictNodes());
  workspace->pdf((outputName+"_add").c_str())->Print();

  return workspace->pdf((outputName+"_add").c_str());

  return 0;      
}


void CBGauss::loadTextInputFile()
{

    string param_file_name = Helper::getInputPath() + "/" + inputParameterFile+"_"+category_name_+".txt";
    string shape_file_name = Helper::getInputPath() + "/" + inputShapeSysFile+"_"+category_name_+".txt";

    //Quickie function to read a text input file

    //Read the whole file
    string fullFileName = param_file_name;

    ifstream inputFile( gSystem->ExpandPathName(fullFileName.c_str()) );
    unsigned int parsedLines = 0;

    textInputParameterValues.clear();

    if (inputFile.is_open()) {

        while ( inputFile.good() ) {
            //Read a line
            string newLine;
            getline( inputFile, newLine );

            std::vector<string> substr;
            Helper::tokenizeString(newLine,' ',substr);
            string blank("");
            substr.erase(std::remove(substr.begin(),substr.end(),blank), substr.end());
            if (substr.empty()) continue;

            //Parse the line
            if (substr.size()==3){
                parsedLines++;
                string parameterName( substr[0]);
                double parameterValue = atof(substr[1].c_str());
                double parameterError = atof(substr[2].c_str());

                //Uniqueness test
                if ( textInputParameterValues.find( parameterName ) == textInputParameterValues.end() ) {
                    //Store name : value pair
                    textInputParameterValues[ parameterName ] = make_pair(parameterValue,parameterError);
                    std::cout<<"storing "<<parameterName<<" = "<<parameterValue<<" +- "<<parameterError<<std::endl;
                }
                else {
                    cerr << "Doubly-defined parameter " << parameterName << " in file " << fullFileName << endl;
                    exit(1);
                }
            }
            else {
                //Ignore irrelevant line
                std::cout<<"failed to read exactly 3 items"<<std::endl;
                std::cout<<"actually read: "<<substr.size()<<" items"<<std::endl;
                for (auto&s : substr) std::cout<<"token: "<<s<<std::endl;
            }
        }
    }
    else {
        cout<<"problem with file "<<fullFileName<<endl;
        exit(3);
    }

    cout << "Parsed " << parsedLines << " lines from input text file " 
        << gSystem->ExpandPathName(fullFileName.c_str()) << endl;
    if (parsedLines==0) {
        exit(3);
    }
}


pair<double, double> CBGauss::readTextInputFile(string ParameterName)
{
    pair<double, double> parameterValue;
    //Throw an error if the parameter name is not found
    if ( textInputParameterValues.find( ParameterName ) == textInputParameterValues.end() ) {
        cerr << "Parameter name " << ParameterName << " not found in cached input" << endl;
        exit(3);
    }
    else {
        //Return the parameter value and report it is used
        parameterValue = textInputParameterValues[ ParameterName ];
        cout << "Using " << ParameterName << " = " << parameterValue.first<<" +/- "<<parameterValue.second << endl;
    }
    return parameterValue;
}

RooProduct* CBGauss::variable(const string& parname)
{
    pair<double,double> pars = readTextInputFile(parname.c_str());
    double ratio = pars.first == 0? 0:fabs(pars.second/pars.first);

    string name(Form("%s_%s", base_name_.Data(),parname.c_str()));
    
    // ignore the systematics that have less than per-mill effect
    if (doSys && fabs(ratio) > 1e-3)
    {
        shape_sys_names_->push_back(parname);
        RooRealVar var( Form("%s_nom", name.c_str()), Form("nominal %s", parname.c_str()), pars.first);

        vector<string> names;
        vector<double> lowValues;
        vector<double> highValues;

        names.push_back(name);
        if(ratio > 1) {
            lowValues.push_back(1E-6);
        } else {
            lowValues.push_back(1. - fabs(pars.second/pars.first));
        }
        if (ratio > 2){
            log_err("The error of %s is over 100%%!: %.4f", name.c_str(), ratio);
        }
        highValues.push_back(1. + fabs(pars.second/pars.first));

        FlexibleInterpVar* fiv = flexibleInterpVar(name, names, lowValues, highValues);

        RooProduct prod(name.c_str(), name.c_str(), RooArgList(var,*fiv));

        workspace->import(prod);
    }
    else {
        RooRealVar var( name.c_str(), name.c_str(), pars.first);
        workspace->import(var);
    }

    return (RooProduct*)workspace->obj(name.c_str());

}

FlexibleInterpVar* CBGauss::flexibleInterpVar(const string& fivName, vector<string>& names, 
        vector<double>& lowValues, vector<double>& highValues)
{
    RooArgList variables;

    for (int inp=0; inp<(int)names.size(); inp++) {
        RooRealVar* np = Helper::createNuisanceVar(names[inp].c_str());
        variables.add(*np);
    }

    FlexibleInterpVar* fiv = new FlexibleInterpVar(("fiv_"+fivName).c_str(), ("fiv_"+fivName).c_str(),
            variables, 1., lowValues, highValues);
    fiv->setAllInterpCodes(4);

    return fiv;
}


void CBGauss::BuildBases()
{
    if(masses_->size() < 1) { throw std::runtime_error("Call AddSample first please!"); }
    bases_ = new RooStats::HistFactory::RooBSplineBases(Form("bases_%s", nickname_.c_str()),
            Form("bases_%s", nickname_.c_str()), order_, *masses_, *mH);
}


void CBGauss::AddMassPoint(float m, std::string shapemeanfile, std::string shapesigmafile){

    std::cout<<"CBGauss adding mass point "<<m<<" using  shape mean file: "<<shapemeanfile<<" and shape sigma file: "<<shapesigmafile<<std::endl;
    masses_->push_back(m);
    auto s2 = new SysText(shapemeanfile.c_str());
    shape_mean_sys_.push_back(s2);
    auto s3 = new SysText(shapesigmafile.c_str());
    shape_sigma_sys_.push_back(s3);
}

RooAbsReal* CBGauss::getShapeSys(std::string name) {

  std::string outputName = category_name_;

  std::vector<SysText*>* sysvec;
  if (name=="mean") sysvec = &shape_mean_sys_;
  if (name=="sigma") {
      sysvec = &shape_sigma_sys_;
  }

    RooArgList bs_fiv_list;

    //loop over masses
    for (unsigned int m(0);m< masses_->size(); ++m){

      auto fiv =  sysvec->at(m)->GetSys(Form("fiv_shape_%s_%s_%d",name.c_str(), base_name_.Data(), (int)masses_->at(m)));
      if (fiv){
        std::cout<<"adding fiv:"<<std::endl;
        fiv->Print();
        bs_fiv_list.add(*fiv);
      } else {
        log_err("no FIV!");
      }
    }
    std::cout<<"bs_fiv_list"<<std::endl;
    bs_fiv_list.Print();

    if (bs_fiv_list.getSize() > 0){
      if (!bases_) BuildBases();
      const char * bs_fiv_name = Form("%s_shape_%s_bs_fiv",outputName.c_str(), name.c_str());
      auto bs_fiv = new RooStats::HistFactory::RooBSpline(bs_fiv_name, bs_fiv_name, bs_fiv_list, *bases_, RooArgSet());
      return bs_fiv;
    }
    return NULL;
}

