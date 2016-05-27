#include "TSystem.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TMultiGraph.h"
#include "TAxis.h"
#include "TTree.h"
#include "TCanvas.h"
#include "Rtypes.h"
#include <map>
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TLine.h"
#include "TLatex.h"

#include "RooWorkspace.h"
#include "RooPlot.h"
#include "RooRealVar.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

void myText(Double_t x,Double_t y,Color_t color,const char *text, float tsize, int font=42);
void ATLASLabel(Double_t x,Double_t y,const char* text,Color_t color);

int main(){

  using namespace RooFit;

  TFile file("combined.root","READ");
  RooWorkspace* wsp = (RooWorkspace*)file.Get("combined");

  std::vector<std::string> prod;
  prod.push_back("ggH");
  prod.push_back("VBFH");
  //prod.push_back("ZH");
  //prod.push_back("WH");

  std::vector<std::string> categories;
  std::vector<std::string> catlabels;
  std::vector<int> color;
  categories.push_back("ggF_4mu"); color.push_back(kViolet); catlabels.push_back("4#mu");
  categories.push_back("ggF_4e"); color.push_back(kOrange); catlabels.push_back("4e");
  categories.push_back("ggF_2mu2e"); color.push_back(kGreen+2); catlabels.push_back("2#mu2e");
  //categories.push_back("ggF_2e2mu"); color.push_back(kCyan); marker.push_back(27);
  //categories.push_back("VBF"); color.push_back(kRed); marker.push_back(32);
  //categories.push_back("VH_Lep"); color.push_back(kBlue); marker.push_back(5);
  //categories.push_back("VH_Had"); color.push_back(kGreen); marker.push_back(34);

  std::vector<std::string> sys;
  std::vector<std::string> sysnames;
  std::vector<int> linestyles;
  sys.push_back("alpha_ATLAS_EL_EFF_ID"); linestyles.push_back(2); sysnames.push_back("EL_EFF_ID");
  sys.push_back("alpha_ATLAS_EL_EFF_RECO"); linestyles.push_back(3); sysnames.push_back("EL_EFF_RECO");
  sys.push_back("alpha_ATLAS_MU_EFF_SYS"); linestyles.push_back(8); sysnames.push_back("MU_EFF_SYS");
  sys.push_back("alpha_ATLAS_MU_EFF_STAT"); linestyles.push_back(4); sysnames.push_back("MU_EFF_STAT");


  TCanvas can("can","",600,600);
  can.SetLeftMargin(0.20);

  //
  // LOOP OVER PRODUCTION MODES
  //

  for (int p(0);p<prod.size();++p){
    std::cout<<prod[p]<<std::endl;

    if (wsp->var(Form("mu_%s",prod[p].c_str())))
         wsp->var(Form("mu_%s",prod[p].c_str()))->setVal(1./((RooRealVar*)wsp->function("nTot_ATLAS_Signal_lumi"))->getVal());

    //
    // LOOP OVER CATEGORIES
    //
    TCanvas can("can","",600,600);

    for (int c(0);c<categories.size();++c){
      std::cout<<categories[c]<<std::endl;

      RooPlot* frame = wsp->var("mH")->frame(Range(200,1000));
      std::string funcname = Form("nTotATLAS_Signal_%s_%s_13TeV", prod[p].c_str(), categories[c].c_str());
      if (!wsp->function(funcname.c_str())) {
        std::cout<<"ERROR! No "<<funcname<<" found! :("<<std::endl;
        continue;
      }

      wsp->function(funcname.c_str())->plotOn(frame, LineStyle(1), LineColor(color[c]),Precision(0.0001));

      for (int s(0);s<sys.size();++s){ //begin loop over systematics
        if (!wsp->var(sys[s].c_str())) {
          std::cout<<"failed to find systematic: "<<sys[s]<<std::endl;
          continue;
        }

        if (categories[c]=="ggF_4mu" && sys[s].find("_EL_")!=std::string::npos) continue;
        if (categories[c]=="ggF_4e" && sys[s].find("_MU_")!=std::string::npos) continue;

        wsp->var(sys[s].c_str())->setVal(1.0);
        wsp->function(funcname.c_str())->plotOn(frame, LineStyle(linestyles[s]), LineColor(kBlack), LineWidth(1), Precision(0.0001));
        wsp->var(sys[s].c_str())->setVal(-1.0);
        wsp->function(funcname.c_str())->plotOn(frame, LineStyle(linestyles[s]), LineColor(kBlack), LineWidth(1), Precision(0.0001));
        wsp->var(sys[s].c_str())->setVal(0.0);

      } //end loop over systematics

      frame->GetXaxis()->SetTitle("m_{H}");
      frame->GetXaxis()->SetLabelSize(0.035);
      frame->GetYaxis()->SetTitle(Form("Acceptance"));
      frame->GetYaxis()->SetTitleOffset(1.4);
      frame->GetYaxis()->SetLabelSize(0.035);
      frame->SetMaximum(0.45);
      frame->SetMinimum(0);
      frame->SetTitle("");
      frame->Draw();

      float x=0.18;
      float y=0.80;
      if (categories[c]=="ggF_2mu2e"){
        x=0.55;
        y=0.40;
      }

      //Decorate the plot
      for (int s(0),o(0);s<sys.size();++s){ //begin loop over systematics
        if (!wsp->var(sys[s].c_str())) {
          std::cout<<"failed to find systematic: "<<sys[s]<<std::endl;
          continue;
        }
        if (categories[c]=="ggF_4mu" && sys[s].find("_EL_")!=std::string::npos) continue;
        if (categories[c]=="ggF_4e" && sys[s].find("_MU_")!=std::string::npos) continue;
        TLine* line = new TLine(x,y-0.04*o,x+0.03,y-0.04*o);
        line->SetNDC(true);
        line->SetLineStyle(linestyles[s]);
        line->SetLineColor(kBlack);
        line->Draw("same");
        myText(x+0.05,y-0.05-0.04*o,kBlack,Form("#pm1#sigma %s",sysnames[s].c_str()),0.03);
        ++o;
      }
      TLine* line = new TLine(x,y,x+0.03,y);
      line->SetNDC(true);
      line->SetLineStyle(0);
      line->SetLineColor(color[c]);
      line->Draw("same");
      myText(x+0.05,y-0.01,color[c],Form("%s #rightarrow %s",prod[p].c_str(),catlabels[c].c_str()),0.03);

      can.Print(Form("plots/normsys_%s_%s.eps",prod[p].c_str(),categories[c].c_str()));
    }//end loop over categories

  } //end loop over prods

}

void myText(Double_t x,Double_t y,Color_t color,const char *text, float tsize, int font) 
{
  TLatex l; //l.SetTextAlign(12); 
  l.SetTextSize(tsize); 
  l.SetNDC();
  l.SetTextColor(color);
  l.SetTextFont(font);
  l.DrawLatex(x,y,text);
}
void ATLASLabel(Double_t x,Double_t y,const char* text,Color_t color) 
{
  TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize); 
  l.SetNDC();
  l.SetTextFont(72);
  l.SetTextSize(0.06);
  l.SetTextColor(color);

  double delx = 0.135*696*gPad->GetWh()/(472*gPad->GetWw());

  l.DrawLatex(x,y,"ATLAS");
  if (text) {
    TLatex p; 
    p.SetNDC();
    p.SetTextFont(42);
    p.SetTextSize(0.05);
    p.SetTextColor(color);
    p.DrawLatex(x+delx,y,text);
  }
}
