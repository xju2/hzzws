#!/bin/bash
script_name=$BASH_SOURCE
currentDir=$PWD

##setup gcc and python
/bin/grep ' release [2345]\.' /etc/redhat-release >/dev/null 2>&1 && \
  echo "WARNING: This version of the HSG7 ROOT build will not work on an SLC5 machine. Please use a different machine (eg. lxplus.cern.ch)" >&2

# GCC 4.9.3
PATH="/afs/cern.ch/sw/lcg/contrib/gcc/4.9.3/x86_64-slc6/bin:$PATH"
LD_LIBRARY_PATH="/afs/cern.ch/sw/lcg/contrib/gcc/4.9.3/x86_64-slc6/lib64:$LD_LIBRARY_PATH"

# Python 2.7.4
PYTHONDIR="/afs/cern.ch/sw/lcg/external/Python/2.7.4/x86_64-slc6-gcc48-opt"
PATH="$PYTHONDIR/bin:$PATH"
LD_LIBRARY_PATH="$PYTHONDIR/lib:$LD_LIBRARY_PATH"
PYTHONPATH=$PYTHONPATH:"$PYTHONDIR/lib/python2.7"
export PATH LD_LIBRARY_PATH PYTHONDIR PYTHONPATH

#ROOT
#RootDir=/afs/cern.ch/atlas/project/HSG7/root/current/x86_64-slc6-gcc48/
RootDir=/afs/cern.ch/atlas/project/HSG7/root/root_v6-04-02/x86_64-slc6-gcc49/
cd $RootDir
source bin/thisroot.sh

# increase stack size - needed for large workspaces
ulimit -S -s unlimited

cd $currentDir
#set ws code env
export HZZWSCODEDIR=$(cd $(dirname "${script_name}") && pwd -P)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HZZWSCODEDIR}/lib
export PATH=$PATH:${HZZWSCODEDIR}/bin:${HZZWSCODEDIR}/test-bin
export HZZWSDIR="/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/Workspaces/"
