// =====================================================================================
// 
//       Filename:  SystematicsManager.h
// 
//    Description:  Have all the systematics and know how to add systematics
// 
//        Version:  1.0
//        Created:  03/13/2015 17:17:43
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#ifndef __HZZWS_SYSTEMATICSMANAGER_H__
#define __HZZWS_SYSTEMATICSMANAGER_H__

#include <vector>
#include "Sample.h"
#include <RooWorkspace.h>
#include <TString.h>


class SystematicsManager{
    private:
        std::vector<TString>* all_nps;
        void add_shape_sys(Sample*, std::vector<TH1*>& shape_varies);
        void add_norm_sys(Sample*, std::vector<float>& norm_varies);

    public:
        SystematicsManager();
        SystematicsManager(const char* fileName);
        virtual ~SystematicsManager();
        void readNPs(const char* fileName);
        void add_sys(Sample*);
        void add_np(RooWorkspace*);
};
#endif
