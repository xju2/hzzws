#!/usr/bin/env python

import os
import sys
import commands

workdir = os.getcwd()

submit = True
do_hist = False
do_scalar = True

exe = "/afs/cern.ch/user/x/xju/work/h4l/h4lcode/hzzws/bsubs/run_limit.sh"
cal_opt = "pvalue" # limit,pvalue
data_opt = "obs" #obs, exp
ws_name = "combWS"
mu_name = "xs"

#mG_low = 500
#mG_hi = 3500
mG_low = 200
mG_hi = 2000
mG_step = 10
#kappa_list = [0.00, 0.01, 0.06, 0.1]
kappa_list = [0.01]

n_mG = int((mG_hi - mG_low)/mG_step)

out_name = workdir
if do_hist:
    input_ws = "/afs/cern.ch/user/x/xju/work/diphoton/limits_hist_floating/inputs/2015_Graviton_histfactory_EKHI_v6.root"
    data_name = "combDatabinned"
    out_name += "/histofactory/"
else:
    #input_ws = "/afs/cern.ch/user/x/xju/work/diphoton/limits_hist_floating/inputs/2015_Graviton_2D_EKHI_200.root"
    #input_ws = "/afs/cern.ch/user/x/xju/work/diphoton/limits_hist_floating/inputs/2015_Graviton_2D_EKHI_200_Mar23.root"
    #out_name += "/functional_Mar23/"
    input_ws = "/afs/cern.ch/user/x/xju/work/HWWStatisticsCode/workspaces/2015_Scalar_2D_v4.root"
    data_name = "combData"
    out_name += "/scalar_2d/"



goodjobs = []
badjobs = []
print out_name
for kappa in kappa_list:
    for mG in range(mG_low, mG_hi+mG_step, mG_step):
        if not do_scalar: 
            fix_vars = "mG:"+str(mG)+",GkM:"+str(kappa)
        else: 
            width = mG*kappa
            fix_vars = "mX:"+str(mG)+",wX:"+str(width)
        run_cmd = exe+" "+input_ws+" "+ws_name+" "+mu_name+" "+\
                data_name+" "+fix_vars+" "+cal_opt+" "+data_opt+" "+out_name

        if not submit: print run_cmd
        #-G u_zp -q 8nh for atlas sources
        #-G ATLASWISC_GEN -q wisc for wisconsin sources
        bsubs_cmd = "bsub -q wisc  -R 'pool>4000' -C 0 -o" + \
                workdir+ "/output "+ run_cmd
        if submit: 
            status,output=commands.getstatusoutput(bsubs_cmd)
        else: 
            continue
        if status != 0:
            print output
            badjobs.append(0)
        else:
            goodjobs.append(1)

print "Good jobs: "+ str(len(goodjobs))+", "+str(len(badjobs))+" failed!"
