#include "Hzzws/Category.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>

#include <RooRealVar.h>
#include <RooAddPdf.h>
#include <RooGaussian.h>
#include <RooProdPdf.h>

#include "Hzzws/Helper.h"
//________________________________________________________________________
    Category::Category(const string& label)
    : m_label(label)
{
}

//________________________________________________________________________
Category::~Category() 
{
}

void Category::addSample(Sample* sample, SystematicsManager* sysMan){
    bool with_sys = sysMan->totalNP() > 0;
    sample ->setChannel(this->obs, this ->m_label.c_str(), with_sys);
    vector<TString>* nps = NULL;
    if(with_sys) {
        nps = sysMan->add_sys(sample);
        if (nps != NULL && nps ->size() > 0)
        {
            for(auto& _np : *nps){
                nps_set.insert( _np );
            }
        }
        delete nps;
    }
    pdfList.add(*(sample->getPDF()));
    coefList.add(*(sample->getCoeff()));
    // add MCConstraint if available
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
    // add gaussian constraint for each systematic 
    for(auto& np : nps_set){
        auto* gauss = Helper::createConstraint(np.Data());
        constraintList.add(*gauss);
    }
    constraintList.add(*sum_pdf);

    TString modelname( Form( "model_%s", m_label.c_str() ) );
    auto* prod_pdf = new RooProdPdf(modelname.Data(), modelname.Data(), constraintList);
    return prod_pdf;
}

