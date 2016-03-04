#!/usr/bin/env python
import math
import glob

def cal_sys(file_name):
    out = file_name
    with open(file_name, 'r') as f:
        sum_val = 0
        icount = 0
        for line in f:
            if line[0] == '[':
                if not icount == 0:
                    out += " & {:.3f} & ".format(math.sqrt(sum_val))
                icount += 1
                sum_val = 0
                continue
            else:
                value = line[:-1].split('=')[1].split()[0]
                vary = abs(1 - float(value))
                sum_val += vary*vary
                
        out += "{:.3f}".format(math.sqrt(sum_val))
        print out

def cal_err(file_name):
    out = ""
    sum_val = [0, 0, 0, 0]
    with open(file_name, 'r') as f:
        for line in f:
            items = line[:-1].split('&')
            err1 = float(items[1])*float(items[3])
            err2 = float(items[2])*float(items[4])
            out += "1 & {} & {:.3E} $\pm$ {:.3E} & {:.3E} $\pm$ {:.3E} \\\\\n".format(items[0],float(items[1]),err1,float(items[2]),err2)
            sum_val[0] += float(items[1])
            sum_val[1] += err1*err1
            sum_val[2] += float(items[2])
            sum_val[3] += err2*err2
    print out
    print "{:.3E} $\pm$ {:.3E} & {:.3E} $\pm ${:.3E}".format(sum_val[0], math.sqrt(sum_val[1]), 
                    sum_val[2], math.sqrt(sum_val[3]))

def run_cal_sys():
    for f in glob.glob("norm*.txt"):
        cal_sys(f)

if __name__ == "__main__":
    #run_cal_sys()
    cal_err("yields.txt")
    #cal_err("shxx_yield.txt")
    #cal_err("zphxx_yield.txt")
