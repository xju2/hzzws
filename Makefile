CXX           = g++ 

ROOTCXXFLAGS  = $(shell root-config --cflags)

CXXFLAGS      = -g -O0 -Wall $(ROOTCXXFLAGS) 

ROOTLIBS   = $(shell root-config --libs) -lRooFitCore -lRooFit -lRooStats -lHistFactory
LIBFLAGS  = -shared -m64 $(ROOTLIBS) -rdynamic -dynamiclib
LIBS          = $(ROOTLIBS)
INCLUDES = -I.
OBJS          = obj/Smoother.o obj/SmoothMan.o Sample.o SystematicsManager.o Category.o Combiner.o

###################################################################################
SILENT=
all: ./lib/libHzzws.so

./lib/libHzzws.so : obj/Smoother.o  obj/SmoothMan.o obj/Sample.o obj/SystematicsManager.o obj/Category.o obj/Combiner.o
	$(SILENT)echo Linking $@
	$(SILENT)$(CXX) obj/Smoother.o obj/SmoothMan.o obj/Sample.o obj/SystematicsManager.o obj/Category.o obj/Combiner.o $(LIBFLAGS) -o $@


./obj/%.o : ./Root/%.cxx
	echo "Compiling $<"
	$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $<  -o $@

clean:
	rm -f ./obj/*.o
	rm -f ./bin/*
