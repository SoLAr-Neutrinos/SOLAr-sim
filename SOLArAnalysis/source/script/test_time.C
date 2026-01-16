/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        test_time.C
 * @created     Wed Jan 18, 2023 13:23:19 CET
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
#include "CLHEP/Units/SystemOfUnits.h"

#include "config/SLArCfgSystemPix.hh"
#include "config/SLArCfgBaseSystem.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgReadoutTile.hh"
#include "event/SLArMCEvent.hh"
#include "event/SLArEventMegatile.hh"
#include "event/SLArEventTile.hh"


TH1D* build_time_dist(const char* input_path) 
{
  TFile* file = new TFile(input_path); 
  TTree* tree = (TTree*)file->Get("EventTree"); 

  SLArMCEvent* ev = nullptr; 
  tree->SetBranchAddress("MCEvent", &ev); 

  TH1D* hPhotonHitTime = new TH1D("hPhotonHitTime", 
      "Photon hit time;Time [ms];Entries",
      1000, -100, 3e3); 

  for (int iev = 0; iev < tree->GetEntries(); iev++) {
    tree->GetEntry(iev); 

    auto anodeEv = ev->GetReadoutTileSystem(); 
    auto supClEv = ev->GetSuperCellSystem(); 

    for (const auto& scEv : supClEv->GetSuperCellMap()) {
      for (const auto &lhit : scEv.second->GetHits()) {
        hPhotonHitTime->Fill(lhit->GetTime() ); 
      }
    }

    for (const auto &mt : anodeEv->GetMegaTilesMap()) {
      for (const auto &t : mt.second->GetTileMap()) {
        for (const auto &lhit : t.second->GetHits()) {
          hPhotonHitTime->Fill(lhit->GetTime()); 
        }
      }
    }

  }


  TF1* f = new TF1("fScint", "[0]*([1]*6.0*exp(-(x-5)/6.0) + (1-[1])*1590.0*exp(-(x-5)/1590.0))", 
      5, 3e3); 
  f->SetParameter(0, 0.7); 
  hPhotonHitTime->Scale( 1. / hPhotonHitTime->Integral()); 
  hPhotonHitTime->Fit(f, "", "", 5, 3e3); 
  
  return hPhotonHitTime;
}

void test_time(TString file0, TString file1) {
  TH1D* hTimeAlpha = build_time_dist(file0.Data()); 
  hTimeAlpha->SetNameTitle("hTimeAlpha", "^{222}Rn"); 

  TH1D* hTimeElec = build_time_dist(file1.Data()); 
  hTimeElec->SetNameTitle("hTimeElec", "#it{e}^{-}"); 

  hTimeAlpha->SetLineColor(kRed+2); 
  hTimeElec->SetLineColor(kAzure-4); 
  hTimeAlpha->SetLineWidth(3); 
  hTimeElec->SetLineWidth(3); 

  TCanvas* c = new TCanvas(); 
  c->SetGrid(1, 1); 
  c->SetTicks(1,1); 

  hTimeAlpha->Draw("hist"); 
  hTimeElec->Draw("hist same"); 
}
