/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        test_larql
 * @created     martedì feb 28, 2023 16:52:58 CET
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
#include "TLatex.h"

#include "physics/SLArIonAndScintLArQL.h"
#include "physics/SLArIonAndScintSeparate.h"
#include "physics/SLArScintillation.h"

void test_larql() 
{
  gStyle->SetPalette(kSunset); 
  gStyle->SetOptStat(0); 
  gStyle->SetOptTitle(0); 
  SLArIonAndScintSeparate* ionAndScint = new SLArIonAndScintSeparate(); 
  
  std::vector<double> dEdX = {1.5, 2, 3, 4, 5, 7, 10}; 
  double dElec = 0.01;
  double Elec_tmp = 0.; 
  std::vector<double> Elec; 
  while (Elec_tmp <= 0.5) {
    Elec.push_back( Elec_tmp );
    Elec_tmp += dElec; 
  }

  std::vector<TGraph*> gLArQ; 
  std::vector<TGraph*> gLArL; 

  for (const auto &dEdx : dEdX) {
    std::vector<double> QQ(Elec.size()); 
    std::vector<double> LL(Elec.size()); 
    int i=0; 
    for (const auto &E : Elec) {
      ionAndScint->SetLightYield(16800); 
      QQ[i] = ionAndScint->ComputeIonYield(dEdx, 1, E); 
      LL[i] = ionAndScint->ComputeScintYield(dEdx, 1, E); 
      i++; 
    }

    TGraph* gQ = new TGraph(Elec.size(), &Elec[0], &QQ[0]); 
    TGraph* gL = new TGraph(Elec.size(), &Elec[0], &LL[0]); 
    gQ->SetNameTitle(Form("gQ_dEdX%g", dEdx), 
        Form("d#it{E}/d#it{x} = %g MeV/cm;Electric Field [kV/cm];#it{Q} yield [MeV^{-1}]", dEdx)); 
    gL->SetNameTitle(Form("gL_dEdX%g", dEdx), 
        Form("d#it{E}/d#it{x} = %g MeV/cm;Electric Field [kV/cm];#it{L} yield [MeV^{-1}]", dEdx)); 
    
    gLArQ.push_back( gQ ); 
    gLArL.push_back( gL ); 
  }

  TCanvas* cQQ = new TCanvas("cQQ", "cQQ", 0, 0, 950, 400); 
  cQQ->SetTicks(1, 1); cQQ->SetGrid(1, 1); cQQ->SetRightMargin(0.05); cQQ->SetBottomMargin(0.05); cQQ->SetLeftMargin(0.15); 
  TH1D* hQQ = new TH1D("hQQ", "Ionization Yield;Electric Field [kV/cm];Ionization Yield [MeV^{-1}]", 200, 0, 0.62); 
  hQQ->SetTitleSize(0.05, "xy"); hQQ->SetLabelSize(0.05, "xy"); 
  hQQ->GetYaxis()->SetRangeUser(0e3, 34e3); 
  hQQ->Draw("axis x+"); 
  hQQ->Draw("axig same"); 


  int i = 0; 
  TLatex label; 
  label.SetTextFont(42); label.SetTextAlign(11); 
  for (auto &gq : gLArQ) {
    gq->SetLineWidth(2); 
    Color_t col = gStyle->GetColorPalette(i*255 / dEdX.size()); 
    gq->SetLineColor( col ); 
    gq->DrawClone("l same"); 
    label.SetTextColor(col); 
    label.DrawLatex(0.5, gq->GetY()[gq->GetN()-1], Form("%g MeV/cm", dEdX.at(i))); 
    ++i; 
  }
  
  TCanvas* cLL = new TCanvas("cLL", "cLL", 800, 0, 950, 400); 
  cLL->SetTicks(1, 1); cLL->SetGrid(1, 1); cLL->SetRightMargin(0.05);  cLL->SetTopMargin(0.05); cLL->SetLeftMargin(0.15); 
  TH1D* hLL = new TH1D("hLL", "Scintillation Yield;Electric Field [kV/cm];Light Yield [MeV^{-1}]", 200, 0, 0.62); 
  hLL->SetTitleSize(0.05, "xy"); hLL->SetLabelSize(0.05, "xy"); 
  hLL->GetYaxis()->SetRangeUser(21e3, 48e3); 
  hLL->Draw("axis"); 
  hLL->Draw("axig same"); 


  i = 0; 
  for (auto &gl : gLArL) {
    gl->SetLineWidth(2); 
    Color_t col = gStyle->GetColorPalette(i*255 / dEdX.size());
    gl->SetLineColor( col ); 
    gl->Draw("l same"); 
    label.SetTextColor(col); 
    label.DrawLatex(0.5, gl->GetY()[gl->GetN()-1], Form("%g MeV/cm", dEdX.at(i))); 
    ++i; 
  }

  return;
}

