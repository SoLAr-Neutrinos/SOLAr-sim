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

void refactor_externals_test() 
{
  gStyle->SetPalette(kBlackBody); 
  TColor::InvertPalette(); 
  TFile* ext_file = new TFile("../install/assets/background/externals_energy_spectrum.root"); 
  TFile* mc_file = new TFile("../install/externals.root"); 
  TTree* mc_tree = (TTree*)mc_file->Get("EventTree"); 

  //- - - - - - - - - - - - - - - - - - - - - - Access readout configuration
  std::map<int, SLArCfgAnode*>  AnodeSysCfg;
  AnodeSysCfg.insert( std::make_pair(10, (SLArCfgAnode*)mc_file->Get("AnodeCfg50") ) );
  AnodeSysCfg.insert( std::make_pair(11, (SLArCfgAnode*)mc_file->Get("AnodeCfg51") ) );

  TH1D* hOrigin = (TH1D*)ext_file->Get("EnergySpectrum");
  TH1D* hEnergy = new TH1D("hEnergy", "Externals Energy;Energy [MeV];Entries", 100, 0, 20);

  //- - - - - - - - - - - - - - - - - - - - - - Access event
  SLArMCEvent* ev = 0; 
  mc_tree->SetBranchAddress("MCEvent", &ev); 

  for (int i=0; i<mc_tree->GetEntries(); i++) {
    if (i%50 == 0) printf("processing ev %i\n", i); 
    mc_tree->GetEntry(i); 

    auto primaries = ev->GetPrimaries(); 

    for (const auto& p: primaries) {
      if (p->GetParticleName() == "geantino") {
        hEnergy->Fill(p->GetEnergy()); 
      }
    }
  }

  hOrigin->Scale( (hEnergy->Integral()*hEnergy->GetBinWidth(1)) / (hOrigin->Integral("w")*hOrigin->GetBinWidth(1)) ); 
  hOrigin->Draw("hist"); 
  hEnergy->Draw("hist same"); 
  return;
}

