
#ifndef __HZZWS_CBGAUSS_H__
#define __HZZWS_CBGAUSS_H__

#include <string>
#include <fstream>
#include <map>
using namespace std;

#include <TFile.h>
#include <TH1.h>
#include <TString.h>
#include <RooAbsPdf.h>
#include <RooHistPdf.h>
#include <RooArgSet.h>
#include <RooArgList.h>
#include <RooProduct.h>
#include <RooAbsReal.h>
#include <RooStats/HistFactory/RooBSplineBases.h>

#include "RooRealVar.h"
#include "RooBinning.h"

#include "Hzzws/SampleBase.h"


class RooWorkspace;
namespace RooStats {
  namespace HistFactory {
    class FlexibleInterpVar;
  }
}

class CBGauss : public SampleBase {

  public:

    CBGauss(const char* name, // used to construct PDF
        const char* nickname,     // used to name the signal strength, if it's signal
        const char* input,        // input text file contains parameters
        const char* shape_sys,    // input text file contains shape variations
        const char* norm_sys,     // text files contains normalization uncertainties
        const char* _path,        // path of previous files
        bool _doSys
        );
    virtual ~CBGauss();

    virtual RooAbsPdf* getPDF();
    virtual RooAbsReal* getCoeff();

    virtual bool setChannel(const RooArgSet& _obs, const char* _ch_name, bool with_sys);
    virtual bool addNormSys(const TString& npName);

    virtual bool addShapeSys(const TString& npName);
    void AddMassPoint(float m, std::string normfile, std::string shapemeanfile, std::string shapesigmafile);

  private:

    void makeCBGParameterization();
    void loadTextInputFile();
    pair<double,double> readTextInputFile(string ParameterName);
    RooProduct* variable(const string& parname);

    RooStats::HistFactory::FlexibleInterpVar* flexibleInterpVar(const string& fivName, 
        vector<string>& names, vector<double>& lowValues, vector<double>& highValues);
    void BuildBases();

    RooAbsReal* getShapeSys(std::string name);

  protected:

    string inputFilePath;
    string inputParameterFile;
    string inputShapeSysFile;

    map< string, pair<double, double> > textInputParameterValues;
    // map<string, map<string, string> > all_dic_;
    // map<string, string>* para_ch_dict_;


  private:
    RooWorkspace* workspace;
    RooRealVar* mH;
    bool doSys;

    int order_;
    vector<double>* masses_;
    vector<SysText*> norm_sys_;
    vector<SysText*> shape_mean_sys_;
    vector<SysText*> shape_sigma_sys_;
    RooStats::HistFactory::RooBSplineBases* bases_;
    
    vector<string>* shape_sys_names_;
};
#endif
