
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TH1F.h"
#include "TSystem.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TGaxis.h"

#include "RooRealVar.h"
#include "RooArgSet.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "RooNDKeysPdf.h"

using namespace std;
using namespace RooFit;
using namespace HiggsWorkspace;

int main() {

  int m4lLow=140.;
  int m4lHigh=800.;

  float binWidth=5.;
  int m4lBins = (int)((m4lHigh-m4lLow)/binWidth);

  float splitVal=225.;
  float overlap=30.;
  int m4lBinsLow=(int)((splitVal+overlap-m4lLow)/binWidth);
  int m4lBinsHigh=(int)((m4lHigh-splitVal+overlap)/binWidth);


  RooRealVar m4l("m4l","m4l",m4lBins,m4lLow,m4lHigh);
  RooRealVar m4lLowV("m4lLow","m4l",m4lBinsLow,m4lLow,splitVal+overlap);
  RooRealVar m4lHighV("m4lHigh","m4l",m4lBinsHigh,splitVal-overlap,m4lHigh);

  // input tree
  TChain* tree=new TChain("tree_incl_all");
  tree->Add("/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v02/mc/Nominal/combined/mc15_qq2ZZ.root");

  
  float tree_m4l, tree_weight;
  int tree_event_type;
  tree->SetBranchAddress("m4l_constrained", &tree_m4l);
  tree->SetBranchAddress("weight", &tree_weight);
  tree->SetBranchAddress("event_type", &tree_event_type); //  _4mu, _4e, _2mu2e, _2e2mu


  TH1F** hists=new TH1F*[3];
  hists[0]=new TH1F("hist0","4mu",m4lBins,m4lLow,m4lHigh);
  hists[1]=new TH1F("hist1","2e2mu",m4lBins,m4lLow,m4lHigh);
  hists[2]=new TH1F("hist2","4e",m4lBins,m4lLow,m4lHigh);

  TH1F** histsLow=new TH1F*[3];
  histsLow[0]=new TH1F("histLow0","hist0",m4lBinsLow,m4lLow,splitVal+overlap);
  histsLow[1]=new TH1F("histLow1","hist1",m4lBinsLow,m4lLow,splitVal+overlap);
  histsLow[2]=new TH1F("histLow2","hist2",m4lBinsLow,m4lLow,splitVal+overlap);

  TH1F** histsHigh=new TH1F*[3];
  histsHigh[0]=new TH1F("histHigh0","hist0",m4lBinsHigh,splitVal-overlap,m4lHigh);
  histsHigh[1]=new TH1F("histHigh1","hist1",m4lBinsHigh,splitVal-overlap,m4lHigh);
  histsHigh[2]=new TH1F("histHigh2","hist2",m4lBinsHigh,splitVal-overlap,m4lHigh);

  for (int i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);
    if (tree_m4l > m4lHigh || tree_m4l < m4lLow) continue;
      

    float nnlo_corr = 1.0;
    float weight_nnlo = tree_weight * nnlo_corr;

    int ich=-1;
    if (tree_event_type==0) ich=0;
    else if (tree_event_type==1) ich=2;
    else ich=1;

    hists[ich]->Fill(tree_m4l, weight_nnlo);
    if (tree_m4l<(splitVal+overlap)) 
      histsLow[ich]->Fill(tree_m4l, weight_nnlo);
    if (tree_m4l>(splitVal-overlap))
      histsHigh[ich]->Fill(tree_m4l, weight_nnlo);
  }

  TH1F** rats=new TH1F*[3];

  TCanvas canv;
  canv.Divide(1,3);

  for (int ich=0; ich<3; ich++) {

    stringstream outputNameStr;
    outputNameStr<<"hist_"<<ich;
    string outputName = outputNameStr.str();

    // Smoothing for high mass
    TH1F* hist=hists[ich];
    TH1F* histLo=histsLow[ich];
    TH1F* histHi=histsHigh[ich];

    // low mass region
    m4lLowV.setRange(m4lLow, splitVal+overlap);
    m4lLowV.setBins(m4lBinsLow);

    RooArgSet obstempLo(m4lLowV);
    float rho_lo=.25;
    RooNDKeysPdf *keyspdflo = new RooNDKeysPdf((outputName + "_keyspdf_lo").c_str(), "keyspdf", obstempLo, *histLo, "m", rho_lo);

    TH1F* smoothedhistLo = (TH1F*) keyspdflo->createHistogram((outputName + "_TH1_smoothedLo").c_str(), m4lLowV, Binning(m4lBinsLow));
    smoothedhistLo->Scale(histLo->Integral() / smoothedhistLo->Integral());

    // high mass region
    m4lHighV.setRange(splitVal-overlap, m4lHigh);
    m4lHighV.setBins(m4lBinsHigh);

    RooArgSet obstempHi(m4lHighV);
    float rho_hi=.35;
    RooNDKeysPdf *keyspdfhi = new RooNDKeysPdf((outputName + "_keyspdf_hi").c_str(), "keyspdf", obstempHi, *histHi, "m", rho_hi);

    TH1F *smoothedhistHi = (TH1F*) keyspdfhi->createHistogram((outputName + "_TH1_smoothedHi").c_str(), m4lHighV, Binning(m4lBinsHigh));
    smoothedhistHi->Scale(histHi->Integral() / smoothedhistHi->Integral());

 
    TH1F *smoothedhist = new TH1F("smoothedhist", "smoothedhist", m4lBins, m4lLow, m4lHigh);
    int overlapbins=(int)(overlap/binWidth+.001);
    for (int ibin=0; ibin<m4lBinsLow-overlapbins; ibin++) {
      smoothedhist->SetBinContent(ibin+1,smoothedhistLo->GetBinContent(ibin+1));
    }
    for (int ibin=0; ibin<=m4lBinsHigh-overlapbins; ibin++) {
      smoothedhist->SetBinContent(m4lBinsLow-overlapbins+ibin+1,smoothedhistHi->GetBinContent(overlapbins+ibin+1));
    }

    
    smoothedhist->Scale(hist->Integral() / smoothedhist->Integral());

    m4l.setRange(m4lLow, m4lHigh);
    m4l.setBins(m4lBins);

    rats[ich]=(TH1F*)smoothedhist->Clone("rat");

    canv.cd(ich+1);
    TPad* pad1=new TPad("pad1","pad1",0,0.3,1,1.0);
    pad1->SetBottomMargin(0);
    pad1->SetGridx();
    pad1->Draw();
    pad1->cd()->SetLogy(1);

    hist->SetStats(0);
    hist->SetLineColor(kBlack);
    hist->Draw();
    smoothedhist->SetLineColor(kRed);
    smoothedhist->Draw("same");

    hist->GetYaxis()->SetLabelSize(0.);
    TGaxis* axis = new TGaxis(-5,20,-5,220,20,220,510,"");
    axis->SetLabelFont(43);
    axis->SetLabelSize(15);
    axis->Draw();
   
    canv.cd(ich+1);
    TPad* pad2 = new TPad("pad2","pad2",0,0.05,1,0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.2);
    pad2->SetGridx();
    pad2->Draw();
    pad2->cd();
 
    TH1F* rat=rats[ich];
    rat->SetMinimum(0.);
    rat->SetMaximum(2.);
    rat->Sumw2();
    rat->SetStats(0);

    rat->Divide(hist);
    rat->SetMarkerStyle(7);
    rat->Draw("ep");

    hist->GetYaxis()->SetTitleSize(20);
    hist->GetYaxis()->SetTitleFont(43);
    hist->GetYaxis()->SetTitleOffset(1.55);
   
    rat->SetTitle("");

    rat->GetYaxis()->SetTitle("ratio smoothed hist/raw hist");
    rat->GetYaxis()->SetNdivisions(5);
    rat->GetYaxis()->SetTitleSize(8);
    rat->GetYaxis()->SetTitleFont(43);
    rat->GetYaxis()->SetTitleOffset(1.55);
    rat->GetYaxis()->SetLabelFont(43);
    rat->GetYaxis()->SetLabelSize(8);

    rat->GetXaxis()->SetTitleSize(20);
    rat->GetXaxis()->SetTitleFont(43); 
    rat->GetXaxis()->SetTitleOffset(4.);
    rat->GetXaxis()->SetLabelFont(43);
    rat->GetXaxis()->SetLabelSize(15);
  }
  canv.Print("hists.eps");
}
