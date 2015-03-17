#include "Hzzws/Category.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>

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

void Category::addSample(Sample* sample, bool is_signal){
    //tell sample, work on this category!
    sample ->setChannel(this ->m_label.c_str());
    sample ->setObs(obs);
    if(is_signal){
        signal_samples ->push_back(sample);
    }else{
        bkg_samples ->push_back(sample);
    }
}
