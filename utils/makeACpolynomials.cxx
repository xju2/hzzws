#include "TSystem.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TAxis.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"
#include "Rtypes.h"
#include <map>
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TLine.h"
#include "TLatex.h"
#include "TMath.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

void myText(Double_t x,Double_t y,Color_t color,const char *text, float tsize, int font=42);
void ATLASLabel(Double_t x,Double_t y,const char* text,Color_t color);

int main(){

  std::string minitreeDir = "/afs/cern.ch/atlas/groups/HSG2/H4l/run2/2015/MiniTrees/Prod_v03/mc/Nominal/";

  std::string outFileName = "polyNorm.txt";
  std::string outFileSIName = "polyNormSI.txt";
  std::string outFileSysName = "polySys.txt";

  float minMH = 200;  //Range to user MC mass points into fit
  float maxMH = 1000;
  float minPlot = 200; //Range to plot fits
  float maxPlot = 1000;
  float minM4l= 140.; //Range to sum weights in
  float maxM4l= 1200.;

  float lumi=1;
  bool storedLumi=false;

  bool doConstrained=true;

  std::vector<std::string> prod;
  prod.push_back("ggH");
  prod.push_back("VBFH");
  //prod.push_back("ZH");
  //prod.push_back("WH");

  std::vector<std::string> categories;
  std::vector<std::string> catlabels;
  std::vector<int> color;
  std::vector<int> marker;
  categories.push_back("ggF_4mu"); color.push_back(kViolet); marker.push_back(24); catlabels.push_back("4#mu");
  categories.push_back("ggF_4e"); color.push_back(kOrange); marker.push_back(25); catlabels.push_back("4e");
  categories.push_back("ggF_2mu2e"); color.push_back(kGreen+2); marker.push_back(26); catlabels.push_back("2#mu2e");
  //categories.push_back("ggF_2e2mu"); color.push_back(kCyan); marker.push_back(27);
  categories.push_back("VBF"); color.push_back(kRed); marker.push_back(32); catlabels.push_back("VBF");
  //categories.push_back("VH_Lep"); color.push_back(kBlue); marker.push_back(5);
  //categories.push_back("VH_Had"); color.push_back(kGreen); marker.push_back(34);

  std::ofstream outfile;
  std::ofstream outfileSI;
  std::ofstream outfileSys;
  outfile.open(outFileName.c_str(), std::ios::out);
  outfileSI.open(outFileSIName.c_str(), std::ios::out);
  outfileSys.open(outFileSysName.c_str(), std::ios::out);


  //
  // LOOP OVER PRODUCTION MODES
  //
  TCanvas can("can","",600,600);
  can.SetLeftMargin(0.15);

  for (int p(0);p<prod.size();++p){
    std::cout<<prod[p]<<std::endl;


    //
    // BUILD MAP OF (MASS POINT VS FILE)
    //
    std::map< float , std::string > fileMap;
    std::string str;
    const char* entry;
    void* dirp = gSystem->OpenDirectory(minitreeDir.c_str());

    std::cout<<"looking for files in "<<minitreeDir<<std::endl;

    while ((entry=(char*)gSystem->GetDirEntry(dirp))) {
      str = entry;
      size_t pos = str.find(prod[p].c_str());
      if (pos!=std::string::npos){
        std::string massString="";
        while (!isdigit(str[++pos])){}
        while (isdigit(str[pos])){ massString+=str[pos++]; }
        if (str.substr(pos,2)=="p5") massString+=".5";
        if (str.substr(pos,3)=="p25") massString+=".25";
        float mass = atof(massString.c_str());
        if (mass<minMH || mass>maxMH) continue;
        //std::cout<<"from file:"<<str<<" extracted mass="<<mass<<std::endl;

        fileMap[mass] = minitreeDir+str;
      }
    }

    std::cout<<"built a map of file names vs mass. size="<<fileMap.size()<<std::endl;
    if (fileMap.size()==0) continue;

    //BUILD A MAP OF (MASS POINT VS ACC)
    std::map<std::string, std::vector<float> > norm, norm_error;
    std::vector<float> zero, masses;

    for (std::map<float,std::string>::iterator it=fileMap.begin(); it!= fileMap.end(); ++it){

      std::cout<<"for mass="<<it->first<<", using file "<<it->second<<std::endl;

      TFile* file = new TFile(it->second.c_str(),"READ");
      TTree* tree = (TTree*)file->Get("tree_incl_all");

      std::map<std::string , float> n;
      std::map<std::string , float> n_e;
      for (int c(0);c<categories.size();++c){
        n[categories[c]]=0.;
        n_e[categories[c]]=0.;
      }

      std::cout<<"mass point "<<(*it).first<<" has "<<tree->GetEntries()<<" entries"<<std::endl;

      float weight;
      float w_xs;
      float w_lumi;
      float w_br;
      float m4l;
      float dijet_deta;
      float dijet_invmass;
      float leadjet_pt;
      float leadjet_eta;
      float subleadjet_pt;
      float subleadjet_eta;
      int event_type, prod_type;

      tree->SetBranchAddress("weight",&weight);
      tree->SetBranchAddress("w_xs",&w_xs);
      tree->SetBranchAddress("w_lumi",&w_lumi);
      tree->SetBranchAddress("w_br",&w_br);
      tree->SetBranchAddress((doConstrained?"m4l_constrained":"m4l_fsr"),&m4l);
      tree->SetBranchAddress("event_type",&event_type);
      tree->SetBranchAddress("prod_type",&prod_type);
      tree->SetBranchAddress("dijet_deltaeta",&dijet_deta);
      tree->SetBranchAddress("dijet_invmass",&dijet_invmass);
      tree->SetBranchAddress("leading_jet_pt",&leadjet_pt);
      tree->SetBranchAddress("leading_jet_eta",&leadjet_eta);
      tree->SetBranchAddress("subleading_jet_pt",&subleadjet_pt);
      tree->SetBranchAddress("subleading_jet_eta",&subleadjet_eta);

      for (int i(0);i<tree->GetEntries();++i){
        tree->GetEntry(i);
        if (m4l<minM4l || m4l>maxM4l) continue;
        float w = weight/(w_xs*w_lumi*w_br);

        w*=(9./4.); //don't want to count taus

        lumi=w_lumi;
        if (!storedLumi){
          outfile<<"[lumi]\n";
          outfile<<"lumi "<<lumi<<std::endl;
          outfileSI<<"[lumi]\n";
          outfileSI<<"lumi "<<lumi<<std::endl;
          storedLumi=true;
        }

        std::string cat;

        if (event_type==0) { cat="ggF_4mu"; } 
        if (event_type==1) { cat="ggF_4e";  }
        if (event_type==2 || event_type==3) { cat="ggF_2mu2e"; }
        if (dijet_invmass>280 && dijet_deta>2.2 &&
            ((leadjet_eta<2.4 && leadjet_pt>25)||(leadjet_eta>2.4 && leadjet_eta<4.5 && leadjet_pt>30)) &&
            ((subleadjet_eta<2.4 && subleadjet_pt>25)||(subleadjet_eta>2.4 && subleadjet_eta<4.5 && subleadjet_pt>30))) { cat="VBF"; }


        n[cat] += w;
        n_e[cat] += (w*w);
      }

      masses.push_back((*it).first);
      zero.push_back(0.);

      for (int c(0);c<categories.size();++c){
        std::string cat = categories[c];
        n_e[cat] = sqrt(n_e[cat]);

        norm[cat].push_back(n[cat]);
        norm_error[cat].push_back(n_e[cat]);
      }
    }

    std::cout<<"built a map of acceptance vs mass. size="<<masses.size()<<std::endl;

    const char* pol="pol0";
    if (masses.size()>1) pol="pol1";
    if (masses.size()>2) pol="pol2";
    //if (masses.size()>3) pol="pol3";
    //if (masses.size()>4) pol="pol4";
    //if (masses.size()>5) pol="pol5";
    //if (masses.size()>5) pol="pol6";

    //BUILD A GRAPH OF MASS POINT VS NORMS
    TMultiGraph* mg = new TMultiGraph();
    can.cd();
    for (int c(0);c<categories.size();++c){
      std::string cat = categories[c];

      TGraphErrors* graph = new TGraphErrors(masses.size(), &(masses[0]), &(norm[cat][0]), &(zero[0]), &(norm_error[cat][0]));
      TFitResultPtr fit = graph->Fit(pol,"qs");
      mg->Add(graph);

      //ESTIMATE SYSTEMATIC
      TF1* polfit = graph->GetFunction(pol);
      std::vector<float> dev;
      for (int m(0);m<masses.size();++m){
        dev.push_back(polfit->Eval(masses[m]) - norm[cat][m]);
        std::cout<<"actual dev "<<masses[m]<<"\t = "<<norm[cat][m] / polfit->Eval(masses[m])<<std::endl;
      }
      float rms = TMath::RMS(dev.size(),&(dev[0]))/sqrt(masses.size()-1);
      dev.clear();
      for (int m(0);m<masses.size();++m){
        float nom = polfit->Eval(masses[m]);
        outfileSys<<prod[p]<<"  "<<cat<<"  "<<masses[m]<<"  "<<Form("%.8f",(nom-rms)/nom)<<"   "<<Form("%.8f",(nom+rms)/nom)<<std::endl;
      }

      can.cd();
      graph->SetMinimum(0);
      graph->GetFunction(pol)->SetLineColor(color[c]);
      //graph->SetTitle(Form("%s %s",prod[p].c_str(), cat.c_str()));
      graph->SetTitle("");
      graph->GetXaxis()->SetTitle("m_{H} [GeV]");
      graph->GetXaxis()->SetRangeUser(masses[0],masses[masses.size()-1]);
      graph->GetXaxis()->CenterTitle();
      graph->GetXaxis()->SetTitleFont(42);
      graph->GetXaxis()->SetTitleSize(0.04);
      graph->GetXaxis()->SetLabelSize(0.035);
      graph->GetXaxis()->SetRangeUser(minPlot,maxPlot);
      graph->GetYaxis()->SetTitle(Form("%s  acceptance",prod[p].c_str()));
      graph->GetYaxis()->SetTitleFont(42);
      graph->GetYaxis()->SetTitleOffset(1.4);
      graph->GetYaxis()->SetTitleSize(0.04);
      graph->GetYaxis()->SetLabelSize(0.035);
      graph->SetMarkerStyle(marker[c]);
      graph->SetMarkerSize(0.75);
      graph->SetMarkerColor(color[c]);
      graph->SetLineColor(color[c]);
      graph->Draw("ap");

      TLine* line = new TLine(0.65,0.75,0.68,0.75);
      line->SetNDC(true);
      line->SetLineColor(color[c]);
      line->SetLineWidth(3);
      line->Draw("same");
      myText(0.70,0.74,kBlack,catlabels[c].c_str(),0.04);
      //ATLASLabel(0.18,0.84,"Internal",kBlack);

      gSystem->Exec(Form("mkdir -p $PWD/plots/%s/",prod[p].c_str()));
      can.Print(Form("$PWD/plots/%s/acceptance_%s_%s_%s.png",prod[p].c_str(),prod[p].c_str(),cat.c_str(),pol)); 
      can.Print(Form("$PWD/plots/%s/acceptance_%s_%s_%s.eps",prod[p].c_str(),prod[p].c_str(),cat.c_str(),pol)); 

      //STORE POLY PARAMS
      std::vector<double> polyParameters = fit->Parameters();
      outfile << Form("[%s %s]\n",prod[p].c_str(),cat.c_str());
      outfileSI << Form("[%s %s]\n",prod[p].c_str(),cat.c_str());
      std::string outline = "Nominal"; //TODO Systematics here
      std::string outlineSI = "& ";
      for (int r(0);r<polyParameters.size();++r) {
        outline=outline+" "+Form("%.20f",polyParameters[r]);
        outlineSI=outlineSI+Form("%.3e",polyParameters[r])+" & ";
      }
      outlineSI+="\n";
      outline+="\n";
      outfile << outline;
      outfileSI<<outlineSI;
    }

    mg->Draw("ap");
    mg->SetMinimum(0);
    mg->SetMaximum(0.4);
    mg->GetXaxis()->SetRangeUser(masses[0],masses[masses.size()-1]);
    mg->GetXaxis()->SetTitle("m_{H} [GeV]");
    mg->GetXaxis()->CenterTitle();
    mg->GetXaxis()->SetTitleFont(42);
    mg->GetXaxis()->SetTitleSize(0.04);
    mg->GetXaxis()->SetLabelSize(0.035);
    mg->GetXaxis()->SetRangeUser(minPlot,maxPlot);
    mg->GetYaxis()->SetTitle(Form("%s  acceptance",prod[p].c_str()));
    mg->GetYaxis()->SetTitleFont(42);
    mg->GetYaxis()->SetTitleOffset(1.4);
    mg->GetYaxis()->SetTitleSize(0.04);
    mg->GetYaxis()->SetLabelSize(0.035);

    float x(0.67),y(0.70);
    for (int c(0);c<categories.size();++c){
      TLine* line = new TLine(x,y-0.04*c,x+0.03,y-0.04*c);
      line->SetNDC(true);
      line->SetLineColor(color[c]);
      line->SetLineWidth(3);
      line->Draw("same");
      myText(x+0.05,y-0.01-0.04*c,kBlack,catlabels[c].c_str(),0.03);
    }
    //ATLASLabel(0.18,0.84,"Internal",kBlack);

    can.Update();
    can.Print(Form("$PWD/plots/%s/acceptance_%s_%s.png",prod[p].c_str(),prod[p].c_str(),pol)); 
    can.Print(Form("$PWD/plots/%s/acceptance_%s_%s.eps",prod[p].c_str(),prod[p].c_str(),pol)); 

  } //end loop over prods

  outfile.close();
  outfileSI.close();
  outfileSys.close();
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
