/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        txt2hist
 * @created     sabato apr 15, 2023 17:23:13 CEST
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
#include "TSystem.h"
#include "TRandom3.h"

void txt2hist(const char* file_txt) 
{
  TGraph* g = new TGraph(file_txt); 
  TF1* f = new TF1("f", [&](double*x, double *p){ return g->Eval(x[0]); }, 0, 20, 0);
  TH1D* h = new TH1D("EnergySpectrum", file_txt, 200, 0, 20);

  double xgmin = g->GetX()[0];
  double xgmax = g->GetX()[g->GetN()-1];

  float x[2] = {0}; 
  int ibin = 1; 
  while ( x[1] <= xgmax ) {
    x[0] = h->GetBinLowEdge(ibin); 
    x[1] = h->GetBinLowEdge(ibin+1); 
    double count = f->Integral(x[0], x[1], 1e-2); 
    h->SetBinContent(ibin, count); 
    ibin++; 
  }

  h->Draw("hist"); 
  g->Draw("l same"); 

  TString basename = gSystem->BaseName(file_txt); 
  basename.Resize( basename.Index(".txt") ); 
  TFile* output = new TFile(basename+".root", "recreate"); 
  h->Write(); 
  output->Close(); 
  
  return;
}

