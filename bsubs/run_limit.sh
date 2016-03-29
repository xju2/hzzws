#!/bin/bash
input_name=$1
ws_name=$2
mu_name=$3
data_name=$4
options=$5
out_dir=$6
#limit inputs/2015_Graviton_histfactory_EKEI_v6.root combWS xs combDatabinned mG:500,GkM:0.01
source /afs/cern.ch/user/x/xju/work/h4l/h4lcode/hzzws/setup.sh
which gcc
which root
which limit

limit $input_name $ws_name $mu_name $data_name "$options" >& fit.log

if [ ! -d ${out_dir} ];then
    mkdir -vp ${out_dir}
fi
echo "save outputs to ${out_dir}"
out_name=`echo $options | sed 's/:/_/g' | sed 's/,/_/g'`
cp fit.log ${out_dir}/${out_name}.txt

