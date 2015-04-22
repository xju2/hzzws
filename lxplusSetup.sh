#!/bin/bash

export WSDIR=/afs/cern.ch/user/x/xju/work/h4l/hzzws

### Make necessary folders
if [ ! -d $WSDIR/obj ]; then mkdir $WSDIR/obj; fi
if [ ! -d $WSDIR/lib ]; then mkdir $WSDIR/lib; fi
if [ ! -d "$WSDIR/test-bin" ]; then mkdir "$WSDIR/test-bin"; fi
if [ ! -d "$WSDIR/bin" ]; then mkdir "$WSDIR/test-bin"; fi
if [ ! -d $WSDIR/share ]; then mkdir $WSDIR/share; fi
if [ ! -d $WSDIR/intermediates ]; then mkdir $WSDIR/intermediates; fi

. /afs/cern.ch/atlas/project/HSG7/root/current/x86_64-slc6-gcc48/setup.sh 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
