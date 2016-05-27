#include "Hzzws/CoefficientFactory.h"

// ===========================================================
// CreateCoefficient
// ===========================================================

Coefficient* CoefficientFactory::CreateCoefficient(const std::string& coefline){

  if (coefline=="") {
    log_err("CoefficientFactory received empty argument");
    return NULL;
  }

  strmap coefTypeMap;

  strvec coef_splitbysemi;
  Helper::tokenizeString(coefline,';',coef_splitbysemi);

  for (auto& s: coef_splitbysemi){
    TString s2 = s.c_str();
    if (!(s2.Contains("factors:") || s2.Contains("poi:") || s2.Contains("sys:") || s2.Contains("global"))){
      log_err("received invalid argument for Coefficient %s, must be one of: factors, poi, sys, global",s2.Data());
      return NULL;
    }
  }

  for (auto& s: coef_splitbysemi){
    TString s2 = s.c_str();  s2.ReplaceAll(" ",""); s=s2.Data();
    strvec coef_splitbycolon;
    Helper::tokenizeString(s,':',coef_splitbycolon);
    if (coef_splitbycolon.size()!=2){
      log_err("invalid format of coefficient argument %s. Must by type:args",s2.Data());
      return NULL;
    }
    coefTypeMap[coef_splitbycolon[0]]=coef_splitbycolon[1];
  }

  std::cout<<"CoefficientFactory received arguments:"<<std::endl;
  for (auto& m:coefTypeMap)
    std::cout<<"type="<<m.first<<", args="<<m.second<<std::endl;
 
  if (coefTypeMap.size()>4){
    log_err("Coefficient type map is larger than size 4.. something is wrong");
    return NULL;
  }

  for (auto& m:coefTypeMap){
    strvec coef_splitbycomma;
    Helper::tokenizeString(m.second,',',coef_splitbycomma);
    int expS=0;
    if (m.first=="poi" || m.first=="sys" || m.first=="global") expS=1;
    else if (m.first=="factors") expS=2;

    if (coef_splitbycomma.size()!=expS){
      log_err("coefficient argument type %s expect exactly %d argument (comma-seperated), received: %s",m.first.c_str(), expS, m.second.c_str());
      return NULL;
    }
  }

  std::cout<<"Arguments look ok! Going to try to build coefficient..."<<std::endl;

  return new Coefficient(coefTypeMap);
  }
