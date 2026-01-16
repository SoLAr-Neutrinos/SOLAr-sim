#include <TH2F.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

void edit_hist() {
  TFile* f0 = new TFile("./B8_oscillogram_local.root"); 

  TH2F* oscillogram_0 = f0->Get<TH2F>("b8_oscillogram"); 
  auto hh = oscillogram_0->ProjectionY(); 
  hh = (TH1D*)hh->Rebin(4); 

  TH1D* hnadir = new TH1D("nadir_hist_surf", "nadir angle distribution @ SURF;cos(#it{#theta}_{nadir});Prob", 
      500, -1, 1);
  hnadir->SetDirectory(0); 

  for (size_t i = 1; i < 500+1; i++) {
    hnadir->SetBinContent(i, hh->GetBinContent(i));  
  }

  TFile* fo = new TFile("nadir_pdf.root", "recreate"); 
  hnadir->Write(); 
  fo->Close(); 
  f0->Close(); 
}
