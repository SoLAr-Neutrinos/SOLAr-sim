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

// function headers
TGraph* rotate_hist(TH1* h); 
TGraph* rotate_hist(TH1* h, double x_ref, TVectorD x, TMatrixD r); 


void test_ql(const char* file_path) {

  // style 
  gStyle->SetLabelFont(43, "xyz");
  gStyle->SetLabelSize(23, "xyz");
  gStyle->SetTitleFont(43, "xyz");
  gStyle->SetTitleSize(23, "xyz");
  gStyle->SetTitleOffset(1.5, "xy");
  gStyle->SetPadTickX(1); gStyle->SetPadTickY(1); 
  gStyle->SetPadGridX(1); gStyle->SetPadGridY(1); 
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
      75, 180e3, 220e3, 75, 160e3, 200e3); 
  hql->SetTitleFont(43, "xyzt");
  hql->SetLabelFont(43, "xyzt"); 
  hql->SetTitleSize(24, "xyzt"); 
  hql->SetLabelSize(24, "xyzt"); 

  TPrincipal pca(2); 
  double data[2]; 

  for (int k=0; k<tree->GetEntries(); k++) {
    tree->GetEntry(k); 

    data[0] = q; data[1] = l;
    pca.AddRow(data); 

    hql->Fill(q, l); 
  }
  


  pca.MakePrincipals(); 

  const TVectorD* mean = pca.GetMeanValues(); 
  const TMatrixD* Tconst = pca.GetEigenVectors(); 
  TMatrixD mFlip(2, 2); mFlip(0,0) = 1; mFlip(1, 1) = -1; 
  TMatrixD T(*Tconst); 

  pca.GetEigenValues()->Print();
  Tconst->Print(); 
  T.Print(); 

  TH2D* hql2 = new TH2D(*hql); 
  hql2->Reset(); 

  TH1D* h1ql = new TH1D("h1ql", "Q+L", 100, 160e3, 200e3); 
  for (int k=0; k<tree->GetEntries(); k++) {
    tree->GetEntry(k); 

    TVectorD xdata(2); xdata[0] = q; xdata[1] = l;  
    xdata -= (*mean);

    xdata = (T)*(xdata);

    xdata += (*mean);

    hql2->Fill(xdata[0], xdata[1]); 
    h1ql->Fill(xdata[1]);
  }


  // ---------------------------------------------------- 
  // DRAW PLOTS
  // ---------------------------------------------------- 
  gStyle->SetOptStat(0); 
  gStyle->SetOptTitle(0); 
  TCanvas* cplot = new TCanvas("cQL", "cQL", 0, 0, 900, 800);
  TPad* pMain = new TPad("pMain", "QL correlation", 0, 0, 0.65, 0.6); 
  TPad* pQ    = new TPad("pQ", "Q distribution", 0, 0.6, 0.65, 1.0); 
  TPad* pL    = new TPad("pL", "L distribution", 0.65, 0., 1.0, 0.6); 

  pMain->Draw(); 
  pMain->cd(); 
  pMain->SetTopMargin(0.07); 
  pMain->SetRightMargin(0.10); 
  pMain->SetLeftMargin(0.20); 
  pMain->SetBottomMargin(0.20); 
  hql->Draw("col"); 

  cplot->cd(); 
  pQ->Draw(); 
  pQ->cd(); 
  pQ->SetBottomMargin(0.02); 
  pQ->SetRightMargin(0.10); 
  pQ->SetTopMargin(0.25); 
  pQ->SetLeftMargin(0.20); 
  hql->ProjectionX()->Draw("hist x+"); 

  cplot->cd(); 
  pL->Draw(); 
  pL->cd(); 
  pL->SetTopMargin(0.07); 
  pL->SetLeftMargin(0.02); 
  pL->SetBottomMargin(0.20); 
  pL->SetRightMargin(0.25); 
  TGraph* gl = rotate_hist(hql->ProjectionY()); 
  TH1D* hframe = new TH1D("L frame", "#it{L} signal;Entries;Nr of photons", 25, 0, 
      1.05*(*std::max_element(gl->GetX(), gl->GetX()+gl->GetN()))); 
  hframe->GetYaxis()->SetRangeUser(hql->GetYaxis()->GetXmin(), hql->GetYaxis()->GetXmax());
  hframe->Draw("axisy+");hframe->Draw("axig same"); 
  gl->Draw("l");


  
  pMain->cd(); 
  double xmax = h1ql->GetBinCenter(h1ql->GetMaximumBin()); 
  TGraph* gql = rotate_hist(h1ql, xmax, *mean, mFlip*T); 
  gql->SetLineWidth(2); 
  gql->Draw("l"); 


  return;
}

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


