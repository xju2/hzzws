#ifndef __HZZWS_CATEGORY_H__
#define __HZZWS_CATEGORY_H__

#include <string>
#include <vector>
#include <iostream>
#include <set>

#include <RooArgSet.h>
#include <RooArgList.h>

#include "Hzzws/Sample.h"
#include "Hzzws/SystematicsManager.h"
using namespace std;

class Category {

 public:

  // Constructor using pre-defined labels.  Passing NYEARS for year will add all years to category.
  explicit Category(const string& label);
  virtual ~Category();

  string label() const { return m_label; }
  /////////////////////////////////////
  //Add sample to this category, tell sample to work in this category
  //Add systematics in SystematicsManager
  /////////////////////////////////////
  void addSample(Sample* sample, bool is_signal, SystematicsManager* sysMan);
  void setObservables(RooArgSet& _obs);
  RooAbsPdf* getPDF();
 private:

  string m_label;
  bool is_2D ;
  RooArgSet obs ;
  set<TString> nps_set;

  RooArgList pdfList;
  RooArgList coefList;
  //RooArgList constraintList; // TODO
};

#endif 
