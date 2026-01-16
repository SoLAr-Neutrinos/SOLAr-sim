/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : test_ql.cc
 * @created     : Monday Nov 07, 2022 16:52:13 CET
 */

#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TROOT.h"
#include "TRandom3.h"
#include "TPrincipal.h"

#include "config/SLArCfgBaseSystem.hh"
#include "event/SLArMCEvent.hh"

TGraph* rotate_hist(TH1* h) {
  TGraph* g = new TGraph(); 
  for (int i=1; i<=h->GetNbinsX(); i++) {
    g->AddPoint(h->GetBinContent(i), h->GetBinLowEdge(i)); 
    g->AddPoint(h->GetBinContent(i), h->GetBinLowEdge(i) + h->GetBinWidth(i)); 
  }

  g->SetLineColor(h->GetLineColor()); 
  g->SetLineStyle(h->GetLineStyle()); 
  g->SetLineWidth(h->GetLineWidth()); 
  g->SetNameTitle(h->GetName(), h->GetTitle()); 

  return g; 
}

TGraph* rotate_hist(TH1* h, double x_ref, TVectorD x, TMatrixD r) {
  TGraph* g = new TGraph(); 

  double h_scale = 30; 

  for (int i=1; i<=h->GetNbinsX(); i++) {
    g->AddPoint(h->GetBinLowEdge(i), h_scale* h->GetBinContent(i)); 
    g->AddPoint(h->GetBinLowEdge(i) + h->GetBinWidth(i), h_scale*h->GetBinContent(i)); 
  }

  TVectorD x_g(2); x_g[0] = x_ref; x_g[1] = 0.; 
  TVectorD xg(2); 

  for (int i=0; i<g->GetN(); i++) {
    xg[0] = g->GetX()[i]; xg[1] = g->GetY()[i]; 
    xg -= x_g; 
    xg = r*xg; 
    xg += x;
    g->GetX()[i] = xg[0]; g->GetY()[i] = xg[1]; 
  }

  return g; 
}

#include "../include/solar_root_style.hpp"

TFile* strip_data(const char* mc_file_path) {
  TFile* file = new TFile(mc_file_path); 
  TTree* tree = (TTree*)file->Get("EventTree"); 
  SLArMCEvent* ev = 0; 
  tree->SetBranchAddress("MCEvent", &ev); 

  TFile* output_file = new TFile(Form("strip_%s", gSystem->BaseName(mc_file_path)), "recreate"); 
  TTree* tmp_tree = new TTree("tmp", "temporary_tree"); 
  double q, l; 
  tmp_tree->Branch("q", &q); tmp_tree->Branch("l", &l); 

  for (int j=0; j<tree->GetEntries(); j++) {
    if (j%50==0) printf("Processing ev %i / %llu\n", j, tree->GetEntries()); 
    tree->GetEntry(j); 
    auto primaries = ev->GetPrimaries(); 

    q = 0; 
    l = 0;

    for (const auto &p : primaries) {
      double pTotScint = p->GetTotalScintPhotons(); 
      auto trajectories = p->GetTrajectories(); 
      for (const auto &t : trajectories) {
        l += t->GetTotalNph();  
        q += t->GetTotalNel(); 
      }
    }

    tmp_tree->Fill(); 
  }

  tmp_tree->Write(); 

  output_file->Close(); 

  return output_file;
}

void test_ql(const char* file_path) {

  slide_default(); 
  gROOT->SetStyle("slide_default"); 
  gStyle->SetPalette(kBlackBody);
  TColor::InvertPalette(); 

  TFile* file = new TFile(file_path); 
  TTree* tree = (TTree*)file->Get("tmp"); 
  double q, l; 
  tree->SetBranchAddress("q", &q); 
  tree->SetBranchAddress("l", &l); 

  TH2D* hql = new TH2D("hql", 
      Form("%s;%s;%s", 
        "Charge vs Light signal", 
        "Nr of ionization electrons", 
        "Nr of scintillation photons"), 
      40, 180e3, 225e3, 40, 165e3, 200e3); 
  hql->SetTitleFont(43, "xyzt");
  hql->SetLabelFont(43, "xyzt"); 
  hql->SetTitleSize(24, "xyzt"); 
  hql->SetLabelSize(24, "xyzt"); 

  TPrincipal* pca = new TPrincipal(2); 

  double f_res_l = 0.07; 
  double f_res_q = 0.10; 

  double data[tree->GetEntries()][2]; 

  for (int k=0; k<tree->GetEntries(); k++) {
    tree->GetEntry(k); 

    data[k][0] = q; //gRandom->Gaus(q, q*f_res_q); 
    data[k][1] = l; //gRandom->Gaus(l, l*f_res_l);
    double* data_tmp = new double(2); 
    data_tmp[0] = data[k][0]; data_tmp[1] = data[k][1]; 
    pca->AddRow(data_tmp); 

    hql->Fill(data[k][0], data[k][1]); 
    delete data_tmp; 
  }
  
  



  gStyle->SetOptStat(0); 
  gStyle->SetOptTitle(0); 

  TCanvas* cplot = new TCanvas("cQL", "cQL", 0, 0, 900, 800);
  TPad* pMain = new TPad("pMain", "QL correlation", 0, 0, 0.65, 0.6); 
  TPad* pQ    = new TPad("pQ", "Q distribution", 0, 0.6, 0.65, 1.0); 
  TPad* pL    = new TPad("pL", "L distribution", 0.65, 0., 1.0, 0.6); 

  pMain->Draw(); 
  pMain->cd(); 
  gPad->SetTicks(1, 1); gPad->SetGrid(1, 1); 
  pMain->SetTopMargin(0.01); 
  pMain->SetRightMargin(0.02); 
  pMain->SetLeftMargin(0.20); 
  pMain->SetBottomMargin(0.20); 
  hql->Draw("col"); 

  cplot->cd(); 
  pQ->Draw(); 
  pQ->cd(); 
  gPad->SetTicks(1, 1); gPad->SetGrid(1, 1); 
  pQ->SetBottomMargin(0.01); 
  pQ->SetRightMargin(0.02); 
  pQ->SetTopMargin(0.20); 
  pQ->SetLeftMargin(0.20); 
  hql->ProjectionX()->Draw("hist x+"); 

  cplot->cd(); 
  pL->Draw(); 
  pL->cd(); 
  gPad->SetTicks(1, 1); gPad->SetGrid(1, 1); 
  pL->SetTopMargin(0.01); 
  pL->SetLeftMargin(0.02); 
  pL->SetBottomMargin(0.20); 
  pL->SetRightMargin(0.20); 
  TGraph* gl = rotate_hist(hql->ProjectionY()); 
  TH1D* hframe = new TH1D("L frame", "#it{L} signal;Entries;Nr of photons", 25, 0, 
      1.05*(*std::max_element(gl->GetX(), gl->GetX()+gl->GetN()))); 
  hframe->GetYaxis()->SetRangeUser(hql->GetYaxis()->GetXmin(), hql->GetYaxis()->GetXmax());
  hframe->Draw("axisy+ ");
  hframe->Draw("axig same");

  gl->Draw("l");


  pca->MakePrincipals(); 
  pca->MakeHistograms(); 

  const TVectorD* mean = pca->GetMeanValues(); 
  const TMatrixD* Tconst = pca->GetEigenVectors(); 
  TMatrixD mFlip(2, 2); mFlip(0,0) = 1; mFlip(1, 1) = -1; 
  TMatrixD T(*Tconst); 

  pca->GetEigenValues()->Print();
  T.Print(); 

  TH2D* hql2 = new TH2D(*hql); 
  hql2->Reset(); 

  TH1D* h1ql = new TH1D("h1ql", "Q+L", 100, 100e3, 300e3); 
  for (int k=0; k<tree->GetEntries(); k++) {

    TVectorD xdata(2); xdata[0] = data[k][0]; xdata[1] = data[k][1];  
    xdata -= (*mean);

    xdata = (T)*(xdata);

    xdata += (*mean);

    hql2->Fill(xdata[0], xdata[1]); 
    h1ql->Fill(xdata[1]);
  }
  
  pMain->cd(); 
  double xmax = h1ql->GetBinCenter(h1ql->GetMaximumBin()); 
  TGraph* gql = rotate_hist(h1ql, xmax, *mean, mFlip*T); 
  gql->SetLineWidth(2); 
  //gql->Draw("l"); 

  //TCanvas* cPCA = new TCanvas(); 
  //cPCA->DivideSquare(pca->GetHistograms()->GetSize()); 
  //int ih = 0; 
  //for (const auto&h : *(pca->GetHistograms())) {
    //cPCA->cd(ih+1); 
    //h->DrawClone(); 
    //ih++; 
  //}
  //cPCA->cd(1); hql2->Draw("col"); 
  //cPCA->cd(2); h1ql->Draw("hist"); 

  return;
}

