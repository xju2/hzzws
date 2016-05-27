#include "Hzzws/Coefficient.h"
#include "RooStats/HistFactory/FlexibleInterpVar.h"
#include <RooStats/HistFactory/RooBSpline.h>
#include "RooProduct.h"
#include "RooFormulaVar.h"
#include "RooArgList.h"
#include "TSystem.h"

// =============================
// Constructor
// =============================
Coefficient::Coefficient(strmap& input) :
  m_arglist(new RooArgList()),
  m_args(input)
{
  m_fullname=m_nickname=m_channel="";

}

// ============================
// Set Name
// ============================
void Coefficient::setName(const std::string& name)
{
    m_fullname=name;
    m_nickname=m_fullname.substr(m_fullname.find_last_of('_')+1);
}

// =============================
// Set Channel
// ============================
bool Coefficient::setChannel(const char* channelName, bool with_sys){

  m_channel=channelName;
  m_arglist->removeAll(); //empty out the arg list, we're starting into a new channel
  for (auto& s: m_sysHandler) delete s.second;
  m_sysHandler.clear();

  if (m_args.find("sys")!=m_args.end()){

    //no wildcard, just one sys handler
    if (!TString(m_args["sys"].c_str()).MaybeWildcard()){
      //only one sys handler to inform
      m_sysHandler[0]= new SysText();
      m_sysHandler[0]->ReadConfig(Form("%s/%s",Helper::getInputPath().c_str(),m_args["sys"].c_str()));
      m_sysHandler[0]->SetChannel(channelName);
    }

    //wildcard, probably need many sys handlers
    else {

      TString p2 = m_args["sys"].c_str();
      if (!(p2.Contains("(") && p2.Contains(")"))){
        log_err("Received a systematic factor with wildcards but no variable for bases? expected norm_ggF*_low.txt(mH) for example");
        return false;
      }

      TString dep = m_args["sys"].substr(m_args["sys"].find_first_of('(')).c_str();
      p2.ReplaceAll(dep,"");
      dep.ReplaceAll("(",""); dep.ReplaceAll(")","");

      //Populate map of sys handlers and bases
      std::map<float, TString> map;
      Helper::fileList(Form("%s/%s",Helper::getInputPath().c_str(),p2.Data()),&map);

      std::vector<double> basevals;

      //inform all the sys handlers
      for (auto& m: map){
        basevals.push_back(m.first);
        m_sysHandler[m.first] = new SysText();
        m_sysHandler[m.first]->ReadConfig(m.second.Data());
        m_sysHandler[m.first]->SetChannel(channelName);
      }


      //Create the dependent var
      float lo = map.begin()->first;
      float hi = map.rbegin()->first;
      m_base_var = new RooRealVar(dep.Data(),dep.Data(),0.5*(hi+lo),lo,hi);

      //Create the bpsline bases
      const char * bsname = Form("bs_bases_%s_%s",m_nickname.c_str(),channelName);
      m_bspline_bases = new RooStats::HistFactory::RooBSplineBases(bsname, bsname, 3, basevals, *m_base_var);

    }
  }

  return true;
}

// =============================
// Destructor
// =============================
Coefficient::~Coefficient(){
  delete m_arglist;
  for (auto& s : m_sysHandler) delete s.second;
}


// =============================
// getCoefficient : the final function called!
// =============================
RooAbsReal* Coefficient::getCoefficient(){ 

  if (!BuildCoefficient()) return NULL;

  std::cout<<"creating final coefficient for "<<m_fullname<<" using args:"<<std::endl;
  m_arglist->Print("v");

  std::string prodName(Form("nTot%s_%s", m_fullname.c_str(),m_channel.c_str()));
  RooProduct* normProd = new RooProduct(prodName.c_str(), prodName.c_str(), *m_arglist);
  return normProd;

}

bool Coefficient::AddSys(const TString& npName){

  //arguments to global have systematics added later
  if (m_args.find("global")!=m_args.end() && TString(m_args["global"].c_str()).Contains(npName)) return true; 

  //otherwise they must be added through the sys handler
  bool r=true;
  for (auto&s : m_sysHandler)
    r = s.second->AddSys(npName) && r;
  return r;
}

// ===========================
// Build Coefficient (the main task!)
// ==========================

bool Coefficient::BuildCoefficient(){

  // Parameter of interest (keyword 'poi')
  if (m_args.find("poi")!=m_args.end()){
    strvec products;
    Helper::tokenizeString(m_args["poi"],'*',products);
    for (auto& p:products) AddPOI(p);
  }

  // Global factors (keyword 'global')
  if (m_args.find("global")!=m_args.end()){
    strvec products;
    Helper::tokenizeString(m_args["global"],'*',products);
    for (auto& p:products) AddGlobalFactor(p);
  }

  // Generic factors (keyword 'factors')
  if (m_args.find("factors")!=m_args.end()){
    strvec splitbycomma;
    Helper::tokenizeString(m_args["factors"],',',splitbycomma);
    strvec products;
    Helper::tokenizeString(splitbycomma[0],'*',products);

    for (auto& p:products)
      AddGenericFactor(p, splitbycomma[1]);
  }

  // Systematics (keyword 'sys')
  if (m_args.find("sys")!=m_args.end())
    AddSystematicFactor(m_args["sys"]);

  return true;
}      

void Coefficient::AddGlobalFactor(const std::string& p){

  TString p2 = p.c_str();
  if (!(p2.Contains("(")&&p2.Contains(")"))){
    log_err("tried to add global factor for coefficient %s, but it doesn't provide a value! Received: %s",m_fullname.c_str(), p2.Data());
    log_err("Was expected a format like L(24.8/0.97/1.05)");
  }
  TString varname(p2.Data(),p2.First('('));
  p2.ReplaceAll(varname,""); p2.ReplaceAll("(",""); p2.ReplaceAll(")","");

  strvec vals;
  Helper::tokenizeString(p2.Data(),'/',vals);
  RooArgList listA;

  //Add the nominal value for the global factor
  if (vals.size()>=1) {
    float val = atof(vals[0].c_str());
    auto factor = new RooRealVar(varname.Data(),varname.Data(),val);
    factor->setConstant(true);
    m_arglist->add(*factor);
  }

  //Add the systematic value to the fiv
  if (vals.size()==3) {
      for (auto& s: m_sysHandler) {
          s.second->AddGlobalSys(varname.Data(),
                  (float) atof(vals.at(1).c_str()), 
                  (float) atof(vals.at(2).c_str())
                  );
      }
  } else {
      log_err("trying to add global var but received the wrong number of values!");
  }
}



void Coefficient::AddPOI(const std::string& p){

    TString p2 = p.c_str();
    float lo(-10e9),hi(10e9),nom(1.); //some intelligent guesses at poi range (always default to 1.00)
    if (p2.Contains("mu")){lo=-30.;hi=30.;nom=1.;}
    if (p2.Contains("br")||p2.Contains("BR")) {lo=0.;hi=1.;nom=1.;}
    if (p2.Contains("xs")||p2.Contains("XS")) {lo=0.;hi=5e7;nom=1.;}
    if (p2.Contains("N")) {lo=-30.;hi=5000;nom=1.;}

    auto poi = new RooRealVar(p.c_str(),p.c_str(),nom,lo,hi);
  poi->setConstant(true);
  m_arglist->add(*poi);
}


void Coefficient::AddGenericFactor(const std::string& p, const std::string& a){
  std::map< std::string, std::map<std::string, double> > dict;
  Helper::readNormTable((Helper::getInputPath()+a).c_str(), dict);
  RooAbsReal* factor(NULL);
  
  TString p2 =p;
  //Polynomial 
  if (p2.Contains("pol")){ //only special case supported

    if (!(p2.Contains("(") && p2.Contains(")"))){
      log_err("Received an coefficient factor that looks like a polynomial but with no argument? skipping..");
      return;
    }
    TString dep = p.substr(p.find_first_of('(')).c_str();
    p2.ReplaceAll(dep,"");
    dep.ReplaceAll("(",""); dep.ReplaceAll(")","");

    //variables that go into the formula
    RooArgList vars;

    if (dep!=m_base_var->GetName()){
      log_err("Trying to add factor that depends on variable %s, but systematics are configured to depend on %s!",dep.Data(),m_base_var->GetName());
      log_err("This is probably a config file mistake - check your inputs!");
      auto v = new RooRealVar(dep.Data(),dep.Data(),1.,-9e9,9e9);
      vars.add(*v);
    }
    else
      vars.add(*m_base_var);
    
    std::cout<<"Attempting to add polynomial coefficient! Hang on tight."<<std::endl;
    std::vector<float> vals;
    int order=0;
    while (dict.find(std::string(Form("%s_a%d",p2.Data(),order)))!=dict.end())
      vals.push_back(dict[std::string(Form("%s_a%d",p2.Data(),order++))][m_channel]);

    std::cout<<"found polynomial coefficients for "<<m_fullname<<" in channel "<<m_channel<<" to be:"<<std::endl;
    for (auto& f: vals) std::cout<<f<<std::endl;

    p2 = p2+"_"+m_channel.c_str();

    //create the coefficients
    for (int i=0 ;i<(int)vals.size();++i) {
      auto a = new RooRealVar(Form("%s_a%d",p2.Data(),i), Form("%s_a%d",p2.Data(),i), vals[i]);
      a->setConstant(true);
      vars.add(*a);
    }

    //create the polynomial formula
    TString formula=Form("%s_a%d",p2.Data(),--order);
    while (order>0) formula = Form("%s_a%d + %s*(%s)",p2.Data(),--order,dep.Data(),formula.Data());

    std::cout<<"built a formula:"<<formula<<std::endl;
    
    factor = new RooFormulaVar(p2.Data(),formula.Data(),vars);
  }

  //Not a polynomial, just a generic factor
  else {
    float val = dict[p][m_channel];
    TString name = (p + "_" + m_channel).c_str();
    factor = new RooRealVar(name.Data(),name.Data(),val);
    ((RooRealVar*)factor)->setConstant(true);
  }
  m_arglist->add(*factor);
}


void Coefficient::AddSystematicFactor(const std::string& p){
  TString sysarg(p.c_str());
  
  //Form a single set of systematics
  if (!sysarg.MaybeWildcard()){ 
    const char* fivname = Form("fiv_%s_%s",m_nickname.c_str(),m_channel.c_str());
    auto fiv = m_sysHandler[0]->GetSys(fivname); 
    if (!fiv){
      log_err("unable to create FIV for this sample, it will have no systematics");
      return;
    }
    m_arglist->add(*fiv);
  }

  else{ //For a wildcard (multiple) set of systematics.. make a bspline out of them
    RooArgList bs_fiv_list;
    for (auto & s: m_sysHandler){
      const char* fivname = Form("fiv_%f_%s_%s",s.first,m_nickname.c_str(),m_channel.c_str());
      auto fiv = s.second->GetSys(fivname);
      if (!fiv){
        log_err("unable to create FIV for one point of spline, all normalization systematics will be skipped for this sample!");
        return;
      }
      bs_fiv_list.add(*fiv);
    }
    const char* bsname = Form("bs_fiv_%s_%s",m_nickname.c_str(),m_channel.c_str());
    auto bs_fiv = new RooStats::HistFactory::RooBSpline(bsname, bsname, bs_fiv_list, *m_bspline_bases, RooArgSet());
    m_arglist->add(*bs_fiv);
  }
}
