/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : read_mc_truth.cc
 * @created     : Friday May 03, 2024 12:40:41 CEST
 */

#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>

#include <event/SLArMCEvent.hh>
#include <event/SLArMCPrimaryInfo.hh>

int read_mc_truth(const TString mc_file_path) 
{
  TFile* mc_file = new TFile(mc_file_path); 
  TTree* mc_tree = mc_file->Get<TTree>("EventTree");

  TH1D* hPrimaryGammaEnergy = new TH1D("hPrimaryGammaEnergy", 
      "Primary #gamma energy;Energy [MeV];Counts", 
      3500, 0, 3.5); 

  TH2D* hEdepProfile = new TH2D("hEdepProfile", "#it{E}_{dep} profile;x [mm];y [mm]", 
      60, -1200, 1200, 60, -1200, 1200); 

  SLArMCEvent* ev = nullptr; 
  mc_tree->SetBranchAddress("MCEvent", &ev); 

  const Long64_t n_entries = mc_tree->GetEntries();

  for (Long64_t i = 0; i < n_entries; i++) {
    mc_tree->GetEntry(i); 

    // get the vector contaning the primary particles (here is just 1)
    auto& primaries = ev->GetPrimaries();

    for (const auto& p : primaries) {
      hPrimaryGammaEnergy->Fill(p.GetEnergy());

      // access the trajectoris of the primary itself and of all its daughters
      const auto& trajectories = p.GetConstTrajectories();

      for (const auto& t : trajectories) {
        // access the step poinst
        const auto& steps = t->GetConstPoints();
        if (t->GetPDGID() == 11) {
          for (const auto& p : steps) {
            // here you can access the step-level information
            // p.fX, p.fY, p.fZ; step-point coordinates [mm]  
            // p.fLAr [bool] if the step was in LAr or not
            // p.fEdep; Energy loss in step [MeV]
            // p.fCopy; Copy Nr of the volume
            // p.fKEnergy; Current Kinetic Energy of the particle
            // p.Nel; Nr of ionization electrons produced
            // p.fNph; Nr of scintillation produces
            if ( TMath::Abs(p.fZ) < 500 && p.fLAr && p.fEdep > 0) {
              hEdepProfile->Fill(p.fX, p.fY, p.fEdep); 
            }
          }
        }
      }
    }
  }

  new TCanvas(); 
  hPrimaryGammaEnergy->Draw("hist"); 

  new TCanvas(); 
  hEdepProfile->Draw("colz"); 
  return 0;
}

