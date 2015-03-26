// =====================================================================================
// 
//       Filename:  SmoothMan.h
// 
//    Description:  do the smooth job
// 
//        Version:  1.0
//        Created:  03/13/2015 18:17:18
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#ifndef __SMOOTHMAN_H__
#define __SMOOTHMAN_H__
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "RooRealVar.h"
#include "RooArgSet.h"

using namespace std;

class SmoothMan{
    public:
        SmoothMan(const char *configFile);
        virtual ~SmoothMan();

        void readConfig(const char *configFile);
        void printDic(map<string, map<string, string> > &dic);

        void process();

    private:
        map<string, map<string, string> > m_dic;

        vector<string> parser(string s, char d);
         
};
#endif
