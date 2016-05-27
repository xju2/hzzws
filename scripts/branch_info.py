#!/usr/bin/env python
import ROOT
if not hasattr(ROOT, "loader"):
    ROOT.gROOT.LoadMacro("/afs/cern.ch/user/x/xju/tool/loader.c") 
"""
dphi_br = ROOT.BranchInfo()
dphi_br.name_ = "TVector2::Phi_mpi_pi(met_phi-phi4l_unconstrained)"
dphi_br.n_ = 50
dphi_br.low_ = 0
dphi_br.high_ = 3.2
dphi_br.x_title_ = "#Delta#phi(E_{T}^{miss},h)"

rpt_br = ROOT.BranchInfo()
rpt_br.name_ = "met_et/pt4l_unconstrained"
rpt_br.n_ = 50
rpt_br.low_ = 0
rpt_br.high_ = 5
rpt_br.x_title_ = "E_{T}^{miss}/p_{T}^{h}"
"""
met_br = ROOT.BranchInfo()
met_br.name_ = "met_et"
met_br.n_ = 30
met_br.low_ = 0
met_br.high_ = 300
met_br.x_title_ = "E_{T}^{miss} [GeV]"

"""
hpt_br = ROOT.BranchInfo()
hpt_br.name_ = "pt4l_unconstrained"
hpt_br.n_ = 50
hpt_br.low_ = 0
hpt_br.high_ = 500
hpt_br.x_title_ = "p_{T}^{h} [GeV]"

m4l_br = ROOT.BranchInfo()
m4l_br.name_ = "m4l_unconstrained"
m4l_br.n_ = 60
m4l_br.low_ = 110
m4l_br.high_ = 140 
m4l_br.x_title_ = "m_{4L} [GeV]"
"""
