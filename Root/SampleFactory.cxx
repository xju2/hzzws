#include "Hzzws/SampleFactory.h"

// ===========================================================
// ugly global def
// ===========================================================
strvec& SampleFactory::Categories(strvec* in) { static strvec cats=*in; return cats; }


// ===========================================================
// CreateSample
// ===========================================================

  SampleBase* SampleFactory::CreateSample(std::string& type, strvec& args){
    if (type.find("Count") != string::npos) return FactorySampleCount(args);
    if (type=="SampleHist") return FactorySampleHist(args);
    if (type=="SampleKeys") return FactorySampleKeys(args);
    if (type=="CBGauss") return FactoryCBGauss(args);
    if (type=="ParametrizedSample") return FactoryParametrizedSample(args);
    if (type=="ExpLandau") return FactoryExpLandau(args);                            
    if (type=="AnalyticHMBkg") return FactoryAnalyticHMBkg(args);

    log_err("SampleFactory doesn't understand sampletype %s!",type.c_str());
    return NULL;
  };

// ===========================================================
// SampleCount
// ===========================================================
SampleBase* SampleFactory::FactorySampleCount(strvec& args){
  if (args.size()!=1){
    log_err("SampleCount requires 1 argument! \
        \n\t name (/s used to construct the PDF)");
  }

  std::string name = args[0];
  if (name==""){
    log_err("SampleCount missing mandatory argument 'name'");
    return NULL;
  }
  return new SampleBase(name.c_str());
}

// ===========================================================
// SampleHist
// ===========================================================

SampleBase* SampleFactory::FactorySampleHist(strvec& args)
{
  if (args.size()!=4){
    std::cout<<"SampleHist requires 4 arguments! \
      \n\t name (/s used to construct PDF) \
      \n\t input (/s root filecontains smoothed histograms \
      \n\t shape_sys (/s root file contains shape variations \
      \n\t thesh (/s set the MC thresshold) \
      "<<std::endl;
    return NULL;
  }

  std::string name = args[0];
  std::string input = args[1];
  std::string shape_sys = args[2];
  std::string thresh = args[3];

  if (name==""){
    std::cout<<"SampleHist missing mandatory argument 'name'"<<std::endl;
    return NULL;
  }
  if (input==""){
    std::cout<<"SampleHist missing mandatory argument 'input'"<<std::endl;
    return NULL;
  }
  if (shape_sys==""){
    std::cout<<"SampleHist WARNING: no shape_sys provided!"<<std::endl;
  }
  if (thresh=="" || atof(thresh.c_str())==-999.){
    std::cout<<"SampleHist WARNING: no threshold provided, leaving as default"<<std::endl;
  }

  auto sh = new SampleHist(name.c_str(),input.c_str(),shape_sys.c_str());

  if (!(thresh==""||atof(thresh.c_str())==-999.) ) sh->setMCCThreshold(atof(thresh.c_str()));
  cout << "SampleHist is generated" << endl;
  return sh;
}

// ===========================================================
// SampleKeys
// ===========================================================

SampleBase* SampleFactory::FactorySampleKeys(strvec& args){
  if (args.size()!=6){
    std::cout<<"SampleKeys requires 6 arguments! \
      \n\t name (/s used to construct PDF) \
      \n\t mH (/f) \
      \n\t mH low (/f) \
      \n\t mH high (/f) \
      \n\t minitree dir \
      \n\t shape_sys (/s root file contains shape variations \
      "<<std::endl;
    return NULL;
  }

  std::string name = args[0];
  TString s_mH = args[1];
  TString s_mH_lo = args[2];
  TString s_mH_hi = args[3];
  std::string input = args[4];
  std::string shape_sys = args[5];

  if (name==""){
    std::cout<<"SampleKeys missing mandatory argument 'name'"<<std::endl;
    return NULL;
  }
  if (!s_mH.IsFloat()){
    std::cout<<"SampleKeys mH argument (args[2]) must be a float! received: "<<s_mH<<std::endl;
    return NULL;
  }
  if (!s_mH_lo.IsFloat()){
    std::cout<<"SampleKeys mH_lo argument (args[3]) must be a float! received: "<<s_mH_lo<<std::endl;
    return NULL;
  }
  if (!s_mH_hi.IsFloat()){
    std::cout<<"SampleKeys mH_hi argument (args[4]) must be a float! received: "<<s_mH_hi<<std::endl;
    return NULL;
  }
  if (input==""){
    std::cout<<"SampleKeys missing mandatory argument 'input'"<<std::endl;
    return NULL;
  }
  if (shape_sys==""){
    std::cout<<"SampleKeys WARNING: no shape_sys provided!"<<std::endl;
  }

  float mH = s_mH.Atof();
  float mH_lo = s_mH_lo.Atof();
  float mH_hi = s_mH_hi.Atof();

  return new SampleKeys(name.c_str(),mH,mH_lo,mH_hi,input.c_str(),shape_sys.c_str());

}

// ===========================================================
// ParameterizedSample
// ===========================================================
SampleBase* SampleFactory::FactoryParametrizedSample(strvec& args){
  if (args.size()!=7){
    std::cout<<"SampleKeys requires 7 arguments! \
      \n\t name (/s used to construct PDF) \
      \n\t paraname (/s the name of the parametrized variable (e.g. mH) \
      \n\t low parameter (/f) \
      \n\t high parameter (/f) \
      \n\t config file (/s) \
      \n\t low observable (/f) \
      \n\t high observable (/f) \
      "<<std::endl;
    return NULL;
  }

  std::string name = args[0];
  std::string var = args[1];
  TString s_mH_lo = args[2].c_str();
  TString s_mH_hi = args[3].c_str();
  std::string config = args[4];
  TString s_m4l_lo = args[5].c_str();
  TString s_m4l_hi = args[6].c_str();

  if (name==""){
    std::cout<<"ParametrizedSample missing mandatory argument 'name'"<<std::endl;
    return NULL;
  }
  if (var==""){
    std::cout<<"ParametrizedSample missing mandatory argument 'var'"<<std::endl;
    return NULL;
  }
  if (!s_mH_lo.IsFloat()){
    std::cout<<"ParametrizedSample mH_lo argument (args[3]) must be a float! received: "<<s_mH_lo<<std::endl;
    return NULL;
  }
  if (!s_mH_hi.IsFloat()){
    std::cout<<"ParametrizedSample mH_hi argument (args[4]) must be a float! received: "<<s_mH_hi<<std::endl;
    return NULL;
  }
  if (config==""){
    std::cout<<"ParametrizedSample missing mandatory argument 'config'"<<std::endl;
    return NULL;
  }
  if (!s_m4l_lo.IsFloat()){
    std::cout<<"ParametrizedSample m4l_lo argument (args[7]) must be a float! received: "<<s_m4l_lo<<std::endl;
    return NULL;
  }
  if (!s_m4l_hi.IsFloat()){
    std::cout<<"ParametrizedSample m4l_hi argument (args[8]) must be a float! received: "<<s_m4l_hi<<std::endl;
    return NULL;
  }



  float plo = s_mH_lo.Atof();
  float phi = s_mH_hi.Atof();

  auto ps = new ParametrizedSample(name.c_str(),var.c_str(),plo,phi);

  if (config!=""){
    std::cout<<"adding Keys to ParametrizedSample.."<<std::endl;
    strvec arrrgs;
    arrrgs.push_back(config);
    arrrgs.push_back(s_m4l_lo.Data());
    arrrgs.push_back(s_m4l_hi.Data());

    if (!AddKeysSample(ps,arrrgs)){
      delete ps;
      return NULL;
    }
  }

  return ps;

}



// ===========================================================
// CBGauss
// ===========================================================

SampleBase* SampleFactory::FactoryCBGauss(strvec& args){
  if (args.size()!=4){
    std::cout<<"CBGauss requires 4 arguments! \
        \n\t name (/s used to construct PDF) \
        \n\t input (/s input text file contains parameters \
                \n\t shape_sys (/s test file contains shape variations \
                    \n\t doSys (/b bool to do sys or not) \
                    "<<std::endl;
    return NULL;
  }

  std::string name = args[0];
  std::string input = args[1];
  std::string shape_sys = args[2];
  std::string s_doSys = args[3];

  if (name==""){
      std::cout<<"CBGauss missing mandatory argument 'name'"<<std::endl;
      return NULL;
  }
  if (input==""){
      std::cout<<"CBGauss missing mandatory argument 'input'"<<std::endl;
      return NULL;
  }
  if (shape_sys==""){
      std::cout<<"CBGauss WARNING: no shape_sys provided!"<<std::endl;
  }
  if (!(s_doSys=="true" || s_doSys=="false" || s_doSys=="1" || s_doSys=="0" || s_doSys=="t" || s_doSys=="f")){
      std::cout<<"CB Gauss invalid argument received for doSys: "<<s_doSys<<std::endl;
      return NULL;
  }

  bool doSys = (s_doSys=="true" || s_doSys=="1" || s_doSys=="t");

  return new CBGauss(name.c_str(),input.c_str(),shape_sys.c_str(),doSys);
}



// ===========================================================
// ExpLandau
// ===========================================================

SampleBase* SampleFactory::FactoryExpLandau(strvec& args){
    if (args.size()!=4){
        std::cout<<"ExpLandau requires 4 arguments! \
            \n\t name (/s used to construct PDF) \
            \n\t input (/s input text file contains parameters \
                    \n\t shape_sys (/s root file contains shape variations \
                        \n\t doSys (/b bool to do sys or not) \
                        "<<std::endl;
        return NULL;
    }

    std::string name = args[0];
    std::string input = args[1];
    std::string shape_sys = args[2];
    std::string s_doSys = args[3];

    if (name==""){
        std::cout<<"ExpLandau missing mandatory argument 'name'"<<std::endl;
        return NULL;
    }
    if (input==""){
        std::cout<<"ExpLandau missing mandatory argument 'input'"<<std::endl;
        return NULL;
    }
    if (shape_sys==""){
        std::cout<<"ExpLandau WARNING: no shape_sys provided!"<<std::endl;
    }
    if (!(s_doSys=="true" || s_doSys=="false" || s_doSys=="1" || s_doSys=="0" || s_doSys=="t" || s_doSys=="f")){
        std::cout<<"CB Gauss invalid argument received for doSys: "<<s_doSys<<std::endl;
        return NULL;
    }

    bool doSys = (s_doSys=="true" || s_doSys=="1" || s_doSys=="t");

    return  new ExpLandau(name.c_str(),input.c_str(),shape_sys.c_str(),doSys);
}

// ===========================================================
// AnalyticHMBkg
// ===========================================================

SampleBase* SampleFactory::FactoryAnalyticHMBkg(strvec& args){
  if (args.size()!=3){
    std::cout<<"AnalyticHMBkg requires 3 arguments! \
      \n\t name (/s used to construct PDF) \
      \n\t input (/s input text file contains parameters \
      \n\t shape_sys (/s text file contains shape variations \
            "<<std::endl;
    return NULL;
  }

    std::string name = args[0];
    std::string input = args[1];
    std::string shape_sys = args[2];

    if (name==""){
        std::cout<<"AnalyticHMBkg missing mandatory argument 'name'"<<std::endl;
        return NULL;
    }
    if (input==""){
        std::cout<<"AnalyticHMBkg missing mandatory argument 'input'"<<std::endl;
        return NULL;
    }
    if (shape_sys==""){
        std::cout<<"AnalyticHMBkg WARNING: no shape_sys provided!"<<std::endl;
    }

    return  new AnalyticHMBkg(name.c_str(),input.c_str(),shape_sys.c_str());
}

// ===========================================================
// AddKeysSample
// ===========================================================

bool SampleFactory::AddKeysSample(SampleBase* samplebase, strvec& args){ 
    if (!samplebase) return false;

    auto parametrizedSample = dynamic_cast<ParametrizedSample*>(samplebase);

    if (args.size()<1){
        std::cout<<"SampleFactory::AddKeysSample error! no config file given (args is empty!)"<<std::endl;
        return false;
    }
    std::string config_file = Helper::getInputPath()+args[0];

    map<string, map<string, string> > all_keys_info;
    Helper::readConfig(config_file.c_str(), '=', all_keys_info);
    Helper::printDic<string>(all_keys_info);

    string& all_samples =  all_keys_info.at("Init").at("mcsets");
    string& minitree_dir = all_keys_info.at("Init").at("minitree_path");
    vector<string>* sample_list = new vector<string>();
    Helper::tokenizeString(all_samples, ',', *sample_list);
    if(sample_list->size() < 2) {
        cout << "Only one sample is provided in " << config_file << endl;
        cout << "I don' know parametrization" << endl;
        delete sample_list;
        exit(1);
    }
    for (const auto& sample : *sample_list) {
        const auto& keys_dict = all_keys_info.at(sample);

        //What to do if ParametrizedSample:
        if (parametrizedSample){
            if (args.size()!=4){
                std::cout<<"SampleFactory::AddKeysSample called with ParametrizedSample, but received N!=3 argments! must provide config_file (/s), varlo (/f), varhi (/f)"<<std::endl;
                return false;
            }
            std::cout<<"sample is a ParametrizedSample, adding keys..."<<std::endl;
            float lo = atof(args[1].c_str());
            float hi = atof(args[2].c_str());

            double mH = (double)atof(keys_dict.at("mH").c_str());
            string minitree(minitree_dir + keys_dict.at("minitree"));
            vector<string> yields_str;
            Helper::tokenizeString(keys_dict.at("yield"), ',', yields_str);
            auto* keys_pdf = new SampleKeys(Form("%s_%.f",parametrizedSample->get_pdf_name().c_str(), mH), 
                    mH, lo, hi,
                    minitree.c_str(), 
                    keys_dict.at("shape").c_str());
            parametrizedSample->AddSample(keys_pdf);
        }
    }
    delete sample_list;

    return true;
}



