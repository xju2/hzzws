CXX           = g++ 

ROOTCXXFLAGS  = $(shell root-config --cflags)

CXXFLAGS      = -std=c++11 -fPIC -g -O0 -Wall $(ROOTCXXFLAGS) 

ROOTLIBS   = $(shell root-config --libs) -lRooFitCore -lRooFit -lRooStats -lHistFactory

LIBFLAGS  =  -shared -m64 $(ROOTLIBS) -rdynamic -dynamiclib -L/usr/local/Cellar/boost/1.55.0_2/lib
BINFLAGS = -L./lib -lHzzws $(ROOTLIBS) -m64 -rdynamic 

INCLUDES = -I. -I/usr/local/Cellar/boost/1.55.0_2/include

###################################################################################
SILENT=
all: ./lib/libHzzws.so ./bin/mainCombiner ./test-bin/testSmoother ./bin/pvalue ./test-bin/testReadNormTable ./bin/gen_zz_theory_sys

./lib/libHzzws.so : obj/Smoother.o  obj/SmoothMan.o obj/Sample.o obj/SystematicsManager.o obj/Category.o obj/Combiner.o obj/Helper.o obj/RooStatsHelper.o obj/BinningUtil.o obj/Checker.o
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX) obj/Smoother.o obj/SmoothMan.o obj/Sample.o obj/SystematicsManager.o obj/Category.o obj/Combiner.o obj/Helper.o obj/RooStatsHelper.o obj/BinningUtil.o obj/Checker.o $(LIBFLAGS) -o $@

./test-bin/% :  obj/%.o 
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX)  $< $(BINFLAGS) -o $@

./bin/% : ./obj/%.o 
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX) -o $@ $< $(BINFLAGS)

./obj/%.o : ./Root/%.cxx
	echo "Compiling $<"
	$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $<  -o $@

./obj/%.o : ./test/%.cxx
	echo "Compiling $<"
	$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $<  -o $@

./obj/%.o : ./utils/%.cxx
	echo "Compiling $<"
	$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $<  -o $@

clean:
	\rm -f ./obj/*.o
	\rm -f ./lib/*
	\rm -f ./bin/*
	\rm -f ./test-bin/*

