#include "Hzzws/SmoothMan.h"

int main(int argc, char **argv) {
    SmoothMan *sm = new SmoothMan("testSmooth.ini");
    sm->process();
    delete sm;
    return 0;
}
