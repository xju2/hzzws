//////////////////////////////////////////////////////////// 
// SampleBase defines general interface for a sample
// It knows how to implement the systematics and acquire PDFs 
//////////////////////////////////////////////////////////// 
#ifndef __HZZWS_SAMPLEBASE_H__
#define __HZZWS_SAMPLEBASE_H__
#include <string>
#include <fstream>
#include <map>

#include <TFile.h>
#include <TH1.h>
#include <TString.h>
#include <RooAbsPdf.h>
#include <RooHistPdf.h>
#include <RooArgSet.h>
#include <RooArgList.h>
#include <RooProduct.h>
#include <RooAbsReal.h>
#include <RooStarMomentMorph.h>
#include <RooMCHistConstraint.h>
#include "RooBinning.h"

#include "Hzzws/SysText.h"

class SysText;

using namespace std;
class SampleBase{

    public:
        typedef map<TString, vector<float> >  NormDic;
        virtual ~SampleBase();

        /* tell sample to provide information in _channelName_ 
         * return false, if no such channel.
         * */ 
        virtual bool setChannel(const RooArgSet& observable, const char* channelName, bool with_sys);

        /* add shape and normalization systematic*/
        virtual bool addShapeSys(const TString& npName) { return false;}
        virtual bool addNormSys(const TString& npName);
        
        /* return final PDF for this Sample in _channelName_ */
        virtual RooAbsPdf* getPDF() = 0;
        /* return co-efficiency for this Sample in _channelName_ */
        virtual RooAbsReal*  getCoeff();
        /* return MC constraint terms */
        virtual RooAbsPdf* get_mc_constraint();

    public:
        bool setNormalizationMap(const map<string, double>& norm_map);
        bool setNormSysDicFromText(const string& file_name);
        double ExpectedEvents() const;
        void SetExpectedEvents(const string& ch_name, double exp) {
            normalization_dic_[ch_name] = exp;
        }
        void useAdaptiveBinning(); // use adaptive binning

        double get_mass() const {return mass_;}
        void set_mass(double mass) { mass_ = mass; }
        bool IsSignal() const {return is_signal_;}
        void SetSignal(bool flag = true){ is_signal_ = flag; }

        const string& get_pdf_name() { return pdf_name_; }
        const string& get_nick_name() { return nickname_; }

    protected:
        /* name: used to construct PDF
         * nickname: used to name the signal strength, if it's signal
         * */
        SampleBase(const char* name, const char* nickname);
        virtual void getExpectedValue(){ return; }
        void addMu(RooArgList& prodSet);
    protected: 
        
        string pdf_name_; // used in pdf (e.g. ATLAS_Signal_ggH)
        string nickname_; // used in mu  (e.g. ggH)
        bool is_signal_ ;
        bool is_bsm_;
        bool do_coupling_;
        bool use_adpt_bin_; // use adaptive binning if true
        double mass_;

        map<string, double> normalization_dic_;

        //////////////////////////////////////// 
        // Following variables depend on category
        //////////////////////////////////////// 
        string category_name_;
        RooArgList obs_list_ ;            
        TString base_name_ ; // name_categoryName
        double expected_events_; 
        RooAbsPdf* norm_pdf;

        //////////////////////////////////////// 
        // Normalization systematics 
        //////////////////////////////////////// 
        SysText* norm_sys_handler_;
        //////////////////////////////////////// 
        // Constraint terms for each binning systematics
        //////////////////////////////////////// 
        RooMCHistConstraint* mc_constraint; 
};

#endif
