/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        refactor_test_sc.C
 * @created     Mon Mar 27, 2023 11:19:53 CEST
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
#include "TDatabasePDG.h"

#include "event/SLArMCEvent.hh"
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgAssembly.hh"
#include "config/SLArCfgSuperCellArray.hh"

void refactor_marley_test() 
{
  gStyle->SetPalette(kBlackBody); 
  TColor::InvertPalette(); 
  TFile* mc_file = new TFile("../install/slar_b8_nue_es.root"); 
  TTree* mc_tree = (TTree*)mc_file->Get("EventTree"); 

  //- - - - - - - - - - - - - - - - - - - - - - Access readout configuration
  std::map<int, SLArCfgAnode*>  AnodeSysCfg;
  AnodeSysCfg.insert( std::make_pair(10, (SLArCfgAnode*)mc_file->Get("AnodeCfg50") ) );
  AnodeSysCfg.insert( std::make_pair(11, (SLArCfgAnode*)mc_file->Get("AnodeCfg51") ) );

  //TH1D* hTime = new TH1D("hPhTime", "Photon hit time;Time [ns];Entries", 200, 0, 5e3); 
  TH2D* hEnergyTime = new TH2D("hEnergyTime", "#gamma Energy vs time;Energy [MeV];Time [ns]", 100, 0, 5e3, 20, 0, 5);

  //- - - - - - - - - - - - - - - - - - - - - - Access event
  SLArMCEvent* ev = 0; 
  mc_tree->SetBranchAddress("MCEvent", &ev); 

  for (int i=0; i<mc_tree->GetEntries(); i++) {
    if (i%50 == 0) printf("processing ev %i\n", i); 
    mc_tree->GetEntry(i); 

    auto primaries = ev->GetPrimaries(); 

    for (const auto& p: primaries) {
      if (p->GetParticleName() == "gamma") {
        hEnergyTime->Fill(p->GetTime(), p->GetEnergy()); 
      }
    }
  }

  auto gamma_bin = hEnergyTime->GetYaxis()->FindBin(1.64); 
  TH1D* hTime = hEnergyTime->ProjectionX("hTime", gamma_bin, gamma_bin); 

  TCanvas* cTime = new TCanvas(); 
  TF1* f = new TF1("f", "[0]/[1]*[2]*exp(-x/[1])", 0, 5e3);
  f->SetParameters(400, 330, 50); 
  f->FixParameter(2, 50.); 
  f->SetParName(0, "#it{N}"); 
  f->SetParName(1, "#it{#tau}"); 
  f->SetParName(2, "#Delta"); 
  f->SetLineColor(kOrange+7); 
  f->SetLineWidth(2); 
  hTime->Fit(f, "0", "", 75, 5e3); 
  //hTime->Draw("hist");
  //f->Draw("same"); 
  hEnergyTime->Draw("colz"); 

  new TCanvas(); 
  hTime->Draw("hist"); 
  f->Draw("same"); 
  return;
}

