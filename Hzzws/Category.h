#ifndef __HZZWS_CATEGORY_H__
#define __HZZWS_CATEGORY_H__

#include <string>
#include <vector>
#include <iostream>
#include <RooArgSet.h>

#include "Hzzws/Sample.h"
class Category {

 public:

  // Constructor using pre-defined labels.  Passing NYEARS for year will add all years to category.
  explicit Category(const std::string& label);
  virtual ~Category();

  std::string label() const { return m_label; }
  void addSample(Sample* sample, bool is_signal);
  void setObservables(RooArgSet& _obs);
 private:

  std::string m_label;
  bool is_2D ;
  std::vector<Sample*>* signal_samples;
  std::vector<Sample*>* bkg_samples;
  RooArgSet obs ;
};

#endif 
