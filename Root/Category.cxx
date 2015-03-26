#include "Hzzws/Category.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>

#include <RooRealVar.h>
#include <RooAddPdf.h>

//________________________________________________________________________
    Category::Category(const string& label)
    : m_label(label)
{
}

//________________________________________________________________________
Category::~Category() 
{
}

void Category::addSample(Sample* sample, bool is_signal, SystematicsManager* sysMan){
    bool with_sys = sysMan->totalNP() > 0;
    sample ->setChannel(this->obs, this ->m_label.c_str(), with_sys);

    vector<TString>* nps = NULL;
    if(with_sys) {
        nps = sysMan->add_sys(sample);
        if (nps != NULL && nps ->size() > 0){
            for(auto& _np : *nps){
                nps_set.insert( _np );
            }
        }
        delete nps;
    }
    //TODO to something else to signal
    pdfList.add(*(sample->getPDF()));
    coefList.add(*(sample->getCoeff()));
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

RooAbsPdf* Category::getPDF(){
    TString pdfname( Form( "modelunc_%s", m_label.c_str() ) );
    RooAddPdf* sum_pdf = new RooAddPdf(pdfname.Data(), pdfname.Data(), pdfList, coefList);
    //TODO add constraint!
    return sum_pdf;
}
