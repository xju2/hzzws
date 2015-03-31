//    Description:  A set of helper functions

#ifndef _HZZWS_HELPER_H
#define _HZZWS_HELPER_H

#include <map>
#include <string>
#include <vector>

using namespace std;
class Helper{
  public:
    Helper(){}
    virtual ~Helper(){}
    static void readConfig(const char* filename, // input file name
            char delim, 
            map<string, map<string, string> >& all_dic // reference to a dictionary
            );
    static void tokenizeString(const string& str, char delim, vector<string>& tokens);
    static void printDic( const map<string, map<string, string> >& all_dic );
};
#endif
