//    Description:  normalize sys
// 
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
