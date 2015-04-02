#ifndef __HZZWS_LINKDEF_H__
#define __HZZWS_LINKDEF_H__

#include <Hzzws/Smoother.h>
#include <Hzzws/SmoothMan.h>
#include <Hzzws/Sample.h>
#include <Hzzws/SystematicsManager.h>
#include <Hzzws/Category.h>
#include <Hzzws/Combiner.h>

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass;

#pragma link C++ class Smoother+;
#pragma link C++ class SmoothMan+;
#pragma link C++ class Sample+;
#pragma link C++ class SystematicsManager+;
#pragma link C++ class Category+;
#pragma link C++ class Combiner+;


#endif
#endif
