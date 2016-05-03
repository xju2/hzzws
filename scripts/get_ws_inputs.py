#!/usr/bin/env python
import subprocess
import glob
import os
from  sets import Set
import shutil
import re
import sys
from optparse import OptionParser

import ROOT

import common
import analysis
import dump_yields


class PrepareWS:
    def __init__(self):
        print "Hello "

    @staticmethod
    def get_smooth_config(config_name, analysis_, out_dir, out_prefix):
        """ generate a config file for smoothing """
        out = "[main]\n"
        out += 'categories = '+','.join([str(x) for x in analysis_.categories]) + '\n'
        out += 'filedir = ./\n'
        out += 'outdir = '+out_dir+'\n'
        out += 'outputname = '+out_prefix+'\n'

        signals = []
        bkgs = []
        for sample_ in analysis_.samples:
            name = sample_.name
            out += name +' = '+name+".list, "+str(sample_.rho) + '\n'
            # write file location to name.list
            out_files = ""
            if sample_ in analysis_.signal:
                signals.append(name)
                for key,value in sample_.samples.iteritems():
                    out_files += value+" "+str(key)+"\n"
            else:
                bkgs.append(name)
                out_files += sample_.samples[-1]+"\n"

            with open(name+".list", 'w') as f:
                f.write(out_files)

        out += "signals = " + ','.join(signals) + '\n'
        out += "backgrounds = " + ','.join(bkgs) + '\n'
        for category in analysis_.categories:
            out += "["+category.name+"]\n"
            out += "cut = " + category.cut + '\n'
            out += "observables = " + category.branch.obs_name+ '\n'
            out += "branch = " + category.branch.get_str() + '\n'
            out += "treename = tree_incl_all\n"

        with open(config_name, 'w') as f:
            f.write(out)

        print config_name," has been created"

    @staticmethod
    def get_ws_config(analysis_, mass, input_dir, np_list_name, out_prefix):
        out = "[main]\n"
        out += "data = "+common.data + '\n'
        out += "fileDir = "+input_dir + '\n'
        out += 'NPlist = '+np_list_name+'\n'
        out += "observable = "+analysis_.categories[0].branch.get_str()+"\n"
        out += 'categories = '+','.join([str(x) for x in analysis_.categories]) + '\n'
        out += 'mcsets = '+','.join([str(x) for x in analysis_.samples])+'\n'

        ### normalization info ###
        yield_table = "yields_"+analysis_.name+"_"+str(mass)+".txt"
        if not os.path.isfile(yield_table):
            dump = dump_yields.DumpYield()
            with open(yield_table, 'w') as f:
                yield_out = dump.get_table(analysis_, mass)
                print yield_out
                f.write(yield_out)
        out += 'normalization = '+yield_table+'\n'

        ### sample information ####
        out += '[samples]\n'
        for sample_ in analysis_.samples:
            key = sample_.name
            out += key+' = '
            hist_input_name = out_prefix+'_'+key
            shape_input_name = key
            norm_input_name = "norm_"+key
            if sample_ in analysis_.signal:
                hist_input_name += '_'+str(mass)
                shape_input_name = key+str(mass)
                norm_input_name += str(mass)
            hist_input_name += '.root'
            shape_input_name += '_Shape.root'
            norm_input_name += '.txt'
            PrepareWS.check_file(input_dir+hist_input_name)
            PrepareWS.check_file(input_dir+shape_input_name)
            PrepareWS.check_file(input_dir+norm_input_name)

            out += hist_input_name+', '+shape_input_name+', '+\
                norm_input_name+', '+sample_.nick
            out += '\n'
        out_name = input_dir+"config/ws_"+str(mass)+".ini"
        if not os.path.isdir(os.path.dirname(out_name)):
            os.mkdir(os.path.dirname(out_name))
        with open(out_name, 'w') as f :
            f.write(out)
        print out_name," written"
        return out_name

    def rename(self, name):
        """ rename shape sys, changing mc_channel_number to a string"""
        if "qqZZ_Powheg" in name:
            return name.replace("qqZZ_Powheg", "qqZZ")
        suffix = name.split('.')[1]
        real_name = name.split('.')[0]
        newname = ""
        if len(self.channel_dic) < 1:
            if os.path.isfile(self.ws_input_dir+self.channel_dic_name):
                self.read_chan()
            else:
                self.get_list()
        for index, token in enumerate(real_name.split('_')):

            if index > 0:
                newname += '_'
            try:
                app = self.channel_dic[token]
            except KeyError:
                app = token
            newname += app
        newname += '.'+suffix
        return newname
        #print name, newname

    def read_chan(self):
        with open(self.ws_input_dir+self.channel_dic_name, 'r') as f:
            for line in f:
                key, value = line[:-1].split(' ')
                self.channel_dic[key] = value

    def get_zz_shape_config(self):
        out = "[main]\n"
        out += 'categories = '+','.join(iter(self.config.categories)) + '\n'
        out += self.obs_name+' = '+self.config.obs_binning+'\n'
        out += '[ggZZ]\n'
        out += 'pdfName = '+self.ggZZ_pdfname+'\n'
        out += 'qcdName = '+self.ggZZ_qcdname+'\n'
        out += '[qqZZ]\n'
        out += 'pdfName = '+self.qqZZ_pdfname+'\n'
        out += 'qcdName = '+self.qqZZ_qcdname+'\n'
        out_name = self.ws_input_dir+self.zz_config_name
        with open(out_name, 'w') as f:
            f.write(out)
        print out_name," written"

    @staticmethod
    def check_file(name):
        if not os.path.isfile(name):
            print "[ERROR]: ", name, " does not exist"

    def get_Analytic(self):
        out = "[main]\n"
        out += "fileDir = "+self.ws_input_dir + '\n'
        out += 'NPlist = '+self.np_list_name+'\n'
        out += "observable = "+self.obs_name+", "+self.config.obs_binning+"\n"
        out += 'categories = '+','.join(sorted(iter(self.config.categories))) + '\n'
        sample_list = self.config.samples_para
        out += 'mcsets = '+','.join(sample_list)+'\n'

        ### normalization info ###
        yield_table = "yields_13TeV.txt"
        dump = dump_yields.DumpYield(self.config)
        with open(yield_table, 'w') as f:
            f.write(dump.get_table(str(125)))
        out += 'normalization = '+yield_table+'\n'

        ### sample information ####
        out += '[samples]\n'
        #for key, value in sorted(self.samples.iteritems()):
        for key in sample_list:
            value = self.sample_dict[key]
            out += key+' = '
            if value[1]: # keys based
                config_file = key+"_config.ini"
                out +=  config_file+', '+value[2] + '\n'
                self.write_para_config(key, config_file)
            else:  #hist based
                hist_input_name = self.out_prefix+'_'+key
                hist_input_name += '.root'
                shape_input_name = key
                norm_input_name = "norm_"+key
                if self.ws_type != "":
                    shape_input_name += "_"+self.ws_type+'_Shape.root'
                    norm_input_name += "_"+self.ws_type+'.txt'
                else:
                    shape_input_name += '_Shape.root'
                    norm_input_name += '.txt'

                self.check_file(self.ws_input_dir+hist_input_name)
                self.check_file(self.ws_input_dir+shape_input_name)
                self.check_file(self.ws_input_dir+norm_input_name)
                out += hist_input_name+', '+shape_input_name+', '+\
                    norm_input_name+', '+value[2]
                if self.statistic_thr > 0:
                    out += ', '+str(self.statistic_thr) 
                out += '\n'

        out_name = self.ws_input_dir+"wsconfig.ini"
        with open(out_name, 'w') as f :
            f.write(out)
        print out_name," written"
        return out_name

    def write_para_config(self, key, config_file):
        if not key in self.config.sig_samples:
            return 
        out = "[Init]\n"
        good_masses = []
        mass_info = ""
        dump2 = dump_yields.DumpYield(self.config)
        for im in range(self.config.mass_points):
            mass = str(self.config.get_mass(im))
            try: 
                file_pa = self.config.samples_sig[mass][key]
            except:
                print "cannot find",mass,key
                continue
            good_masses.append(mass)
            mass_info += "["+mass+"]\n"
            mass_info += 'minitree = ' + os.path.basename(file_pa) + '\n'
            mass_info += 'mH = ' + mass+ '\n'
            mass_info += 'shape = mean_'+key+mass+"_"+self.config.name+".txt\n"
            mass_info += 'norm = norm_'+key+mass+"_" + self.config.name + '.txt\n'
            text_info, list_info = dump2.get_yield(file_pa)
            mass_info += 'yield = '+','.join(list_info) + '\n'

        out += 'mcsets = ' + ','.join(good_masses)
        out += '\n'
        out += 'minitree_path = ' +common.minitree_dir+"\n"
        out += mass_info
        with open(config_file, 'w') as f:
            f.write(out)

    @staticmethod
    def make_ws(self, wsconfig_name, mh):
        cmds = ["mainCombiner", wsconfig_name, "combined_"+mh+".root"]
        p = subprocess.Popen(cmds, stdout=subprocess.PIPE).communicate()[0]
        print p

    def get_np_list(self):
        all_norm_files = glob.glob(self.ws_input_dir+"norm_*.txt")
        for norm_file in all_norm_files:
            if not os.path.isfile(norm_file): continue
            with open(norm_file, 'r') as f:
                for line in f:
                    if line[0] == '[':
                        continue
                    self.np_norm.add(line.split('=')[0].strip())
        print len(self.np_norm)," norm systematics found"

        all_shape_files = glob.glob(self.ws_input_dir+"*.root")
        for shape_file in all_shape_files:
            handle = ROOT.TFile(shape_file, "read")
            for key in handle.GetListOfKeys():
                tokens = key.GetName().split('-')
                if len(tokens) < 2 or "Nominal" in key.GetName():
                    continue
                np_name = key.GetName().split('-')[1]
                self.np_shape.add(np_name)
        print len(self.np_shape), " shape systematics found"

        self.np_shape.add(self.qqZZ_pdfname)
        self.np_shape.add(self.ggZZ_pdfname)
        self.np_shape.add(self.qqZZ_qcdname)
        self.np_shape.add(self.ggZZ_qcdname)
        combined_np = self.np_shape.union(self.np_norm)
        print "total: ", len(combined_np)
        print sorted(combined_np)
        out_list = self.ws_input_dir+self.np_list_name
        with open(out_list, 'w') as f:
            f.write('\n'.join(sorted(combined_np)))
        print "write to: ",out_list

def run_pvalue(config):
    out =""
    for im in range(config.mass_points):
        mh = config.get_mass(im)
        out += str(mh) + "\n"
        ws_name = "combined_"+str(mh)+".root"
        cmds = [common.pvalue_cmd, ws_name, "mu_BSM"]
        p = subprocess.Popen(cmds, stdout=subprocess.PIPE).communicate()[0]
        out += p
    pattern = re.compile("expected")
    print re.findall("expected p0: [0-9]*.[0-9]*", out)
    with open("pvalue.txt", 'w') as f:
        f.write(out)

def smooth():
    PrepareWS.get_smooth_config(
        "smooth.ini",
        analysis.LOWMASS,
        "./",
        "test"
    )

def workspace():
    PrepareWS.get_ws_config(
        analysis.LOWMASS,
        125,
        "./", "nuisance.txt", "test"
    )

if __name__ == '__main__':
    workspace()
