#include "Hzzws/Helper.h"
#include "TString.h"
#include <vector>

int main (){
  const char * path = "/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/Workspaces/SampleFactoryTestArea/20160504/input/norm_ggH*_Low.txt";
  std::map<float, TString> map;
  tstrvec vec = Helper::fileList(path,&map);

  vec = Helper::fileList(path);
  return 0;
}
