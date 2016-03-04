#!/bin/bash
script_name=$BASH_SOURCE
currentDir=$PWD
#setup root
RootDir=/afs/cern.ch/atlas/project/HSG7/root/current/x86_64-slc6-gcc48/
cd $RootDir
source bin/thisroot.sh
source $RootDir/setup.sh
cd $currentDir

#set ws code env
export HZZWSCODEDIR=$(cd $(dirname "${script_name}") && pwd -P)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${HZZWSCODEDIR}/lib
export PATH=$PATH:${HZZWSCODEDIR}/bin:${HZZWSCODEDIR}/test-bin
export HZZWSDIR="/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/Workspaces/"
