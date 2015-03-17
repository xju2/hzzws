CXX           = g++ 

ROOTCXXFLAGS  = $(shell root-config --cflags)

CXXFLAGS      = -g -O0 -Wall $(ROOTCXXFLAGS) 

ROOTLIBS   = $(shell root-config --libs) -lRooFitCore -lRooFit -lRooStats -lHistFactory

LIBFLAGS  = -shared -m64 $(ROOTLIBS) -rdynamic -dynamiclib
BINFLAGS = -L./lib $(ROOTLIBS) -m64 -rdynamic

INCLUDES = -I.

###################################################################################
SILENT=
all: ./lib/libHzzws.so ./test-bin/test_combiner

./lib/libHzzws.so : obj/Smoother.o  obj/SmoothMan.o obj/Sample.o obj/SystematicsManager.o obj/Category.o obj/Combiner.o
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX) obj/Smoother.o obj/SmoothMan.o obj/Sample.o obj/SystematicsManager.o obj/Category.o obj/Combiner.o $(LIBFLAGS) -o $@

./test-bin/test_combiner : obj/Combiner.o obj/test_combiner.o | ./lib/libHzzws.so
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX) -o $@ $< $(BINFLAGS)

./test-bin/% : ./obj/%.o | ./lib/libHzzws.so
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX) -o $@ $< $(BINFLAGS)

./bin/% : ./obj/%.o | ./lib/libHzzws.so
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX) -o $@ $< $(BINFLAGS)

./obj/%.o : ./Root/%.cxx
	echo "Compiling $<"
	$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $<  -o $@

./obj/%.o : ./test/%.cxx
	echo "Compiling $<"
	$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $<  -o $@

clean:
	rm -f ./obj/*.o
	rm -f ./lib/*
	rm -f ./bin/*
