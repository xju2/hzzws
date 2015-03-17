// =====================================================================================
// 
//       Filename:  NormSys.h
// 
//    Description:  normalize sys
// 
//        Version:  1.0
//        Created:  03/16/2015 23:36:04
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#ifndef __HZZWS_NORMSYS_H__
#define __HZZWS_NORMSYS_H__
#include <TString.h>

class NormSys{
    private:
        TString name;
        float up_value;
        float down_value;
   public:
        explicit NormSys(const char* _name, float up, float down);
        virtual ~NormSys();
        inline float getUp(){ return this->up_value;}
        inline float getDown(){ return this->down_value;}
};

#endif
