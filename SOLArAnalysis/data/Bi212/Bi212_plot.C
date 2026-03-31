/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        Bi212_plot.C
 * @created     Wed May 03, 2023 09:21:25 CEST
 */

#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom3.h"

void Bi212_plot(const char* file_path) 
{
  TFile* file = new TFile(file_path); 
  TTree* t = (TTree*)file->Get("externals"); 

  int n_events = 0; 
  double time_beta = 0; 
  double time_gamma = 0; 
  double time_alpha = 0; 
  double time = 0; 
  int pdgID = 0; 
  int iev = -1; 
  int current_iev = iev;

  t->SetBranchAddress("pdgID", &pdgID); 
  t->SetBranchAddress("time", &time); 
  t->SetBranchAddress("iEv", &iev); 

  TH1D* hTime = new TH1D("hTime", "#Delta#it{t};Time [ns];Entries", 100, 0, 2e3);
  TH1D* hEnergy_beta = new TH1D("hEnergy_beta", "Primary energy;Energy [MeV];Counts (scaled)", 200, 0, 10); 
  TH1D* hEnergy_alpha = new TH1D("hEnergy_alpha", "Primary energy;Energy [MeV];Counts (scaled)", 200, 0, 10); 
  TH1D* hEnergy_gamma = new TH1D("hEnergy_gamma", "Primary energy;Energy [MeV];Counts (scaled)", 200, 0, 10); 

  hEnergy_gamma->SetLineWidth(3); 
  hEnergy_gamma->SetLineColor(kBlack); 
  hEnergy_gamma->SetFillColor(kOrange+1); 

  hEnergy_beta->SetLineColor(kBlue+2); 
  hEnergy_beta->SetFillColor(kAzure-4); 
  hEnergy_beta->SetLineWidth(3); 

  hEnergy_alpha->SetLineWidth(3); 
  hEnergy_alpha->SetFillColor(kRed-4); 
  hEnergy_alpha->SetLineColor(kRed+2); 

  auto _reset = [&]() {
    time_alpha = -1; 
    time_beta  = -1; 
    time_gamma = -1; 
  };

  auto _fill = [&]() {
    if (time_beta >=0) {
      hTime->Fill(time_alpha - time_beta);
    }
  };

  for (int i=0; i<t->GetEntries(); i++) {
    t->GetEntry(i); 
    if (iev != current_iev) {
      n_events++; 
      current_iev = iev;
      if (n_events == 0) continue;
      _fill(); 
      _reset(); 
    }

    if (pdgID == 11) time_beta = time;
    else if (pdgID == 22) time_gamma = time; 
    else if (pdgID == 1000020040) time_alpha = time; 

  }

  gStyle->SetOptTitle(0); 
  gStyle->SetOptStat(0); 
  gStyle->SetOptFit(1); 

  TF1* fExp = new TF1("fexp", "[0]*exp(-x/[1])", 
      hTime->GetXaxis()->GetXmin(), hTime->GetXaxis()->GetXmax()); 
  fExp->SetParName(0, "#it{N}"); 
  fExp->SetParName(1, "#it{#tau}"); 
  fExp->SetLineColor(kOrange+7); 
  fExp->SetParameters(2e3, 3e2); 
  
  hTime->Fit(fExp, "0"); 

  TCanvas* cTime = new TCanvas("cTime", "time", 0, 0, 500, 600); 
  gPad->SetTicks(1, 1); gPad->SetGrid(1, 1); 
  gPad->SetRightMargin(0.05); gPad->SetTopMargin(0.05); 
  hTime->SetLineWidth(2); 
  hTime->Draw(); 
  fExp->Draw("same");

  TCanvas* cEnergy = new TCanvas("cEnergy", "cEnergy", 200, 200, 500, 600); 
  gPad->SetTicks(1, 1); gPad->SetGrid(1, 1); 
  gPad->SetRightMargin(0.05); gPad->SetTopMargin(0.05); 
  t->Draw("origin_k>>hEnergy_beta", "pdgID == 11 && (origin_vol == 10 || origin_vol == 11)", "goff"); 
  t->Draw("origin_k>>hEnergy_gamma", "pdgID == 22 && (origin_vol == 10 || origin_vol == 11)", "goff"); 
  t->Draw("origin_k>>hEnergy_alpha", "pdgID ==  1000020040 && (origin_vol == 10 || origin_vol == 11)", "goff"); 
  hEnergy_alpha->Draw("hist");
  hEnergy_gamma->Draw("hist same"); 
  hEnergy_beta->Draw("hist same"); 

  
  
  return;
}

