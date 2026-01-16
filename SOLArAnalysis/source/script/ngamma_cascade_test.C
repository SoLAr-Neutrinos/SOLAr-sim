/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : ngamma_cascade_test
 * @created     : Wednesday Jul 03, 2024 11:09:39 CEST
 */

#include <iostream>
#include <TCanvas.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TH1D.h>

#include <event/SLArMCEvent.hh>
#include <event/SLArMCPrimaryInfo.hh>
#include <event/SLArEventTrajectory.hh>

int ngamma_cascade_test()
{
  std::vector<TString> file_list[2]; 
  file_list[0].push_back( "/home/guff/Dune/SOLAr/SOLAr-sim/install/cavern_neutrons_rock_std.root"); 
  file_list[0].push_back( "/home/guff/Dune/SOLAr/SOLAr-sim/install/cavern_neutrons_rock_std_1.root"); 
  file_list[1].push_back( "/home/guff/Dune/SOLAr/SOLAr-sim/install/cavern_neutrons_rock_cascade.root"); 
  file_list[1].push_back( "/home/guff/Dune/SOLAr/SOLAr-sim/install/cavern_neutrons_rock_cascade_1.root"); 

  Color_t cols[2] = {kBlue+1, kRed+1}; 
  TH1D* hGamma[2]; 
  hGamma[0] = new TH1D("hgamma_std", "Seconday gamma - HP;Energy [MeV];Counts", 300, 0, 15); 
  hGamma[1] = new TH1D("hgamma_csc", "Seconday gamma - CASCADE nCapture;Energy [MeV];Counts", 300, 0, 15); 
  TH1D* hGamma_ncap[2]; 
  hGamma_ncap[0] = new TH1D("hgamma_ncap_std", "Seconday n capture gamma - HP;Energy [MeV];Counts", 300, 0, 15); 
  hGamma_ncap[1] = new TH1D("hgamma_ncap_csc", "Seconday n capture gamma - CASCADE nCapture;Energy [MeV];Counts", 300, 0, 15); 

  SLArMCEvent* ev = nullptr; 

  for (int it =0; it < 2; it++) {
    TChain chain("EventTree"); 
    for (const auto& ff : file_list[it]) {
      chain.AddFile(ff, TTree::kMaxEntries, "EventTree"); 
    }
    chain.SetBranchAddress("MCEvent", &ev); 

    for (Long64_t iev = 0; iev < chain.GetEntries(); iev++) {
      chain.GetEntry(iev); 

      const auto& primaries = ev->GetPrimaries();
      for (const auto& p : primaries) {
        const auto& trajectories = p.GetConstTrajectories(); 
        for (const auto& t : trajectories) {
          if (t->GetPDGID() == 22) {
            hGamma[it]->Fill( t->GetInitKineticEne() ); 
            TString creator = t->GetCreatorProcess(); 
            if (creator.Contains("nCapture")) {
              hGamma_ncap[it]->Fill( t->GetInitKineticEne() ); 
            }
          }
        }
      }
    }

    hGamma[it]->SetLineWidth(2); hGamma[it]->SetLineColor( cols[it] );  
    hGamma_ncap[it]->SetLineWidth(2); hGamma_ncap[it]->SetLineColor( cols[it] );  
  }

  TCanvas* cGamma = new TCanvas(); 
  cGamma->SetTicks(1, 1); 
  cGamma->SetGrid(1, 1); 
  hGamma[0]->Draw("hist"); 
  hGamma[1]->Draw("hist same"); 
  
  TCanvas* cGamma_ncap = new TCanvas(); 
  cGamma_ncap->SetTicks(1, 1); 
  cGamma_ncap->SetGrid(1, 1); 
  hGamma_ncap[0]->Draw("hist"); 
  hGamma_ncap[1]->Draw("hist same"); 


  return 0;
}

