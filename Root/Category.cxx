#include "Hzzws/Category.h"

#include <iostream>
#include <cstdlib>
#include <algorithm>

#include <RooRealVar.h>
#include <RooAddPdf.h>
#include <RooGaussian.h>
#include <RooProdPdf.h>
#include <RooMCHistConstraint.h>

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

void Category::addSample(SampleBase* sample, SystematicsManager* sysMan){
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
    RooAbsPdf* sample_pdf = sample->getPDF();
    if(!sample_pdf){
        log_err("Cannot find pdf for %s in %s", sample->get_nick_name().c_str(), m_label.c_str());
        return;
    }
    sample_pdf->Print();
    pdfList.add(*sample_pdf);
    coefList.add(*(sample->getCoeff()));
    RooMCHistConstraint* mc_constrt = 
        dynamic_cast<RooMCHistConstraint*>(sample->get_mc_constraint());
    // add MC constraint terms
    if (mc_constrt != nullptr){
        constraintList.add(*mc_constrt);
        TIter next_global(mc_constrt->getGlobalObservables().createIterator());
        TIter next_nuis(mc_constrt->getNuisanceParameters().createIterator());
        RooAbsReal* global;
        RooAbsReal* nuisance;
        while ((
                    global = (RooAbsReal*) next_global(),
                    nuisance = (RooAbsReal*) next_nuis()
                    )) 
        {
            if (nuisance->isConstant()) continue; // not include the ones below threshold.
            global_obs_list_.add(*global);
            nuisance_obs_list_.add(*nuisance);
        }
    }
}

void Category::setObservables(const RooArgSet& _obs)
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
    cout << "Total pdf: " << pdfList.getSize() << endl;
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

