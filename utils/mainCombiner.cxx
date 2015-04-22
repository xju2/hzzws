// =====================================================================================
// 
//       Filename:  test_combiner.cxx
// 
//    Description:  test combiner
// 
//        Version:  1.0
//        Created:  03/17/2015 16:56:20
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Xiangyang Ju (), xiangyang.ju@gmail.com
//        Company:  
// 
// =====================================================================================
#include "Hzzws/Combiner.h"

int main(int argc, char* argv[]){
    string configname = "test.ini";
    if (argc > 1){
        configname = string(argv[1]);
    }
    Combiner* combiner = new Combiner("combined", configname.c_str());
    delete combiner;
    return 0;
}
