/*
 */
#ifndef __HZZWS_SYSTEXT_H__
#define __HZZWS_SYSTEXT_H__

#include <TString.h>
#include <RooArgList.h>

#include <string>
#include <map>

using namespace std;
class RooAbsReal;

class SysText {
public:
    SysText(const char* text_file);
    SysText();
    virtual ~SysText();
    bool SetChannel(const char* ch_name);
    bool AddSys(const TString& npName);
    bool AddSys(const TString& ch_name, const TString& npName);
    bool ReadConfig(const char* text_file);
    RooAbsReal* GetSys(const char* name);
    RooAbsReal* GetSys();
    void AddGlobalSys(const char* npName, float low, float up);

private:
    void Clear();
    void addSys(const TString& npName, double low, double up);
private:
   string file_name_;
   map<string, map<string, string> > sys_all_; // systematics: all categories (different value)
   map<TString, vector<float> > sys_channel_; // systematics: this category
   map<TString, vector<float> > sys_global_; // systematic: all categories (same value)
   string ch_name_;
   vector<double> low_;
   vector<double> high_;
   RooArgList np_;
};

#endif
