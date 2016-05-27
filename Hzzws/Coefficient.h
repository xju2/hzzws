#ifndef __HZZWS_COEFFICIENT_H__
#define __HZZWS_COEFFICIENT_H__

#include "Hzzws/SysText.h"
#include "Hzzws/Helper.h"
#include "RooArgList.h"
#include "RooAbsReal.h"
#include <RooStats/HistFactory/RooBSplineBases.h>

class Coefficient {

  public:

    //constructor
    Coefficient(strmap& input);

    //destructor
    ~Coefficient();

    void setName(const std::string& in);
    bool setChannel(const char* channelName, bool with_sys);

    RooAbsReal* getCoefficient();
    RooArgList* getCoefficientList();

    bool AddSys(const TString& npName);

  private:
    RooArgList* m_arglist;
    strmap m_args;
    std::map<float, SysText*> m_sysHandler;

    //for normalization that depends on some poi (like mH)
    RooStats::HistFactory::RooBSplineBases* m_bspline_bases;
    RooRealVar* m_base_var;

    void AddGenericFactor(const std::string& p, const std::string& a);
    void AddSystematicFactor(const std::string& p);
    void AddPOI(const std::string& p);
    void AddGlobalFactor(const std::string& p);

    bool BuildCoefficient();

    std::string m_fullname;
    std::string m_nickname;
    std::string m_channel;
};

#endif
