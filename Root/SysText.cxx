/*
 */
#include <stdlib.h>
#include <sstream>
#include <stdexcept>

#include <RooRealVar.h>
#include "RooStats/HistFactory/FlexibleInterpVar.h"

#include "Hzzws/SysText.h"
#include "Hzzws/Helper.h"


SysText::SysText(const char* text_path):
    file_name_(text_path)
{
    sys_all_.clear();
    sys_global_.clear();
    Helper::readConfig(text_path, '=', sys_all_);
    Helper::printDic<string>(sys_all_);
    ch_name_ = "";
    Clear();
}

SysText::SysText(){
    sys_all_.clear();
    ch_name_ = "";
    Clear();
}

SysText::~SysText(){
    TIter np_iter(np_.createIterator());
    RooRealVar* np;
    while((np=(RooRealVar*)np_iter())){
        delete np;
    }
}

void SysText::Clear() {
    sys_channel_.clear();
    low_.clear();
    high_.clear();
    np_.removeAll();
}

bool SysText::SetChannel(const char* ch_name){
    Clear();
    ch_name_ = string(ch_name);
    bool has_channel = false;
    for(const auto& sec : sys_all_) {
        if(sec.first.compare(ch_name) == 0){
            for(const auto& npName : sec.second) {
                istringstream iss(npName.second);
                float low_value, high_value;
                iss >> low_value >> high_value ;
                vector<float>  sys;
                sys.push_back(low_value);
                sys.push_back(high_value);
                sys_channel_[TString(npName.first)] = sys;
                has_channel = true;
            }
        }
    }
    if(!has_channel) log_err("%s does not have channel: %s", file_name_.c_str(), ch_name);
    return true;
}

bool SysText::AddSys(const TString& npName)
{
    vector<float>* norm_varies;
    try{
        norm_varies =&( this->sys_channel_.at(npName));
    } catch (const out_of_range& oor) {
        // try global systematics
        try{
            norm_varies = &( this->sys_global_.at(npName));
        } catch (const out_of_range& oor) {
            return false;
        }
    }
    addSys(npName, norm_varies ->at(0), norm_varies->at(1));
    return true;
}

bool SysText::AddSys(const TString& ch_name, const TString& npName)
{
    if(! ch_name.EqualTo(ch_name_.c_str()) ) SetChannel(ch_name.Data());
    return AddSys(npName);
}

void SysText::addSys(const TString& npName, double low, double up)
{
    RooRealVar* npVar = Helper::createNuisanceVar(npName.Data());
    np_.add(*npVar);
    low_.push_back(low);
    high_.push_back(up);
}

RooAbsReal* SysText::GetSys(const char* name) {
    if (low_.size() < 1){ 
        return NULL;
    }
    auto* fiv = new RooStats::HistFactory::FlexibleInterpVar(name, name, np_, 1., low_, high_);
    // 4: piece-wise log outside boundaries, polynomial interpolation inside
    fiv->setAllInterpCodes(4); 
    return fiv;
}

RooAbsReal* SysText::GetSys(){
    return GetSys(Form("fiv_%s", ch_name_.c_str()));
}

bool SysText::ReadConfig(const char* text_file){
    sys_all_.clear();
    Helper::readConfig(text_file, '=', sys_all_);
    return true;
}

void SysText::AddGlobalSys(const char* npName, float low, float up)
{
    vector<float> sys;
    sys.push_back(low);
    sys.push_back(up);
    sys_global_[TString(npName)] = sys;
}
