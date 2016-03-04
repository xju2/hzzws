CXX           = g++ 
ROOTCXXFLAGS  = $(shell root-config --cflags)

CXXFLAGS      = -std=c++11 -fPIC -g -O0 -Wall $(ROOTCXXFLAGS) 

ROOTLIBS   = $(shell root-config --libs) -lRooFitCore -lRooFit -lRooStats -lHistFactory

LIBFLAGS  =  -shared -m64 $(ROOTLIBS) -rdynamic -dynamiclib -L/usr/local/Cellar/boost/1.55.0_2/lib
BINFLAGS = -L./lib -lHzzws $(ROOTLIBS) -m64 -rdynamic 
INCLUDES = -I. -I/usr/local/Cellar/boost/1.55.0_2/include

### sources and objects ###
SOURCES=$(wildcard Root/*.cxx)
OBJECTSORG=$(patsubst %.cxx,%.o,$(SOURCES))
OBJECTS=$(subst Root,obj,$(OBJECTSORG))

### executes ###
TARGET1=$(wildcard utils/*.cxx)
TARGET2=$(subst utils,bin,$(TARGET1))
TARGET_BIN = $(patsubst %.cxx,%,$(TARGET2))

### programs for test ###
TEST_TARGET1 = $(wildcard test/*.cxx)
TEST_TARGET2 = $(subst test/,test-bin/,$(TEST_TARGET1))
TARGET_TESTBIN = $(patsubst %.cxx,%,$(TEST_TARGET2))

###################################################################################
SILENT=
all: build ./lib/libHzzws.so $(TARGET_BIN) $(TARGET_TESTBIN)

build: 
	@mkdir -p obj
	@mkdir -p lib
	@mkdir -p bin
	@mkdir -p test-bin

./lib/libHzzws.so : $(OBJECTS)
	$(SILENT)echo Linking `basename $@`
	$(SILENT)$(CXX) $(OBJECTS) $(LIBFLAGS) -o $@

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
	@rm -rf ./obj
	@rm -rf ./lib
	@rm -rf ./bin
	@rm -rf ./test-bin

