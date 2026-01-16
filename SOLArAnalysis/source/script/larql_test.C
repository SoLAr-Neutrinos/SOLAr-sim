/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : larql_test.C
 * @created     : Tuesday Jun 25, 2024 17:09:25 CEST
 */

#include <iostream>
#include <TH1D.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMath.h>
#include <TF1.h>

#include <physics/SLArLArProperties.hh>
#include <physics/SLArIonAndScintModel.h>
#include <physics/SLArIonAndScintLArQL.h>

TGraph* print_to_graph(TF1* f) {
  TGraph* g = new TGraph(f->GetNpx()); 
  for (size_t i = 0; i < f->GetNpx(); i++) {
    auto h = f->GetHistogram();
    double x = h->GetBinCenter(i+1); 
    g->SetPoint(i, x, f->Eval(x)); 
  }
  g->SetTitle(f->GetTitle()); 
  g->SetName(Form("g%s",f->GetName())); 
  return g; 
}

int larql_test()
{
  SLArLArProperties lar;
  lar.SetElectricField(0.5); 

  SLArIonAndScintLArQL larql; 

  TF1* fChi0 = new TF1("fChi0", 
      [larql](double* x, double*p){return larql.QChi(x[0], p[0]);}, 1e-1, 1e3, 1); 
  fChi0->SetParameter(0, 0.5); 

  new TCanvas(); 
  fChi0->DrawClone(); 

  TF1* fQvsE = new TF1("fQvsE", 
      [larql](double* x, double* p) {return 1e-3*larql.ComputeIonYield(p[0],x[0]);}, 
      0, 0.5, 1); 

  TH1D* hframedQdx = new TH1D("hframedQdx", "dQ/dx;dE/dx [MeV/cm];dQ/dx [10^{3} e^{-}/cm]", 35,0, 35);

  new TCanvas(); 
  fQvsE->SetParameter(0, 2.1); fQvsE->SetLineColor(kBlack);
  fQvsE->DrawClone(); 
  fQvsE->SetParameter(0, 5.0); fQvsE->SetLineColor(kGray+1); 
  fQvsE->DrawClone("same"); 
  fQvsE->SetParameter(0, 20.0); fQvsE->SetLineColor(kGray); 
  fQvsE->DrawClone("same"); 
  return 0;
}

