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
int main(int argc, char** argv){
    Combiner* combiner = new Combiner("combined","test.ini");
    delete combiner;
    return 0;
}
