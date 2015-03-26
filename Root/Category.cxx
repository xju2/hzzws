#include "Hzzws/Category.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>

#include <RooRealVar.h>
using namespace std;

//________________________________________________________________________
    Category::Category(const string& label)
    : m_label(label)
{
    signal_samples = new std::vector<Sample*>();
    bkg_samples = new std::vector<Sample*>();
}

//________________________________________________________________________
Category::~Category() 
{
    delete signal_samples;
    delete bkg_samples;
}

void Category::addSample(Sample* sample, bool is_signal, bool with_sys){
    //tell sample, work on this category!
    sample ->setChannel(this->obs, this ->m_label.c_str(), with_sys);
    if(is_signal){
        signal_samples ->push_back(sample);
    }else{
        bkg_samples ->push_back(sample);
    }
}

void Category::setObservables(RooArgSet& _obs)
{
    TIterator* iter = _obs.createIterator();
    TIter next(iter);
    RooRealVar* var;
    while ( (var = (RooRealVar*) next()) ){
        obs.add(*var);
    }
}
