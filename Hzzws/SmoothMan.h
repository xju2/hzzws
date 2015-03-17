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

class SmoothMan{
    private:
        std::string tree_path;
        std::string tree_name;
        std::string outfilename;
        std::string branch_name;

    public:
        SmoothMan(const char* configFile);
        virtual ~SmoothMan();
         
};
#endif
