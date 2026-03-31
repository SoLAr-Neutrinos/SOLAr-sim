/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        strip_nu_data.C
 * @created     2023-11-09 16:40
 */

#include <iostream>
#include <memory>
#include <vector>
#include <map>
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
#include "config/SLArCfgAssembly.hh"
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgSuperCellArray.hh"

void strip_data(const TString file_path) 
{
  gStyle->SetPalette(kBlackBody); 
  TColor::InvertPalette(); 
  TFile* mc_file = new TFile(file_path); 
  TTree* mc_tree = (TTree*)mc_file->Get("EventTree"); 

  auto pdg = TDatabasePDG::Instance(); 

  SLArMCEvent* ev = 0; 
  mc_tree->SetBranchAddress("MCEvent", &ev); 

  TH1D* hEdep = new TH1D("hEdep", "Edep;Edep [MeV];Entries", 200, 0, 20); 

  for (int iev = 0 ; iev < mc_tree->GetEntries(); iev++ ){
    mc_tree->GetEntry(iev); 

    float edep = 0; 
    auto& primaries = ev->GetPrimaries(); 

    for (const auto &p : primaries) {
      auto& trajectories = p.GetConstTrajectories(); 
      for (const auto& t : trajectories) {
        edep += t->GetTotalEdep(); 
      }
    }

    hEdep->Fill(edep * gRandom->Gaus(1, 0.07)); 
  }

  return;
}


