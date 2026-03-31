/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : hit_test_viewer.C
 * @created     : Saturday Apr 06, 2024 11:20:15 CEST
 */

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH3F.h>
#include <TGraph2D.h>
#include <TGraphErrors.h>
#include <TPolyLine3D.h>
#include <TPolyMarker3D.h>
#include <TF1.h>
#include <TRandom3.h>
#include <TDatabasePDG.h>

#include <event/SLArMCEvent.hh>
#include <config/SLArCfgAnode.hh>
#include <config/SLArCfgAssembly.hh>
#include <config/SLArCfgSuperCellArray.hh>


int hit_test_viewer(const TString mc_file_path, const TString hit_file_path, const int iev = 0)
{
  TFile* hit_file = new TFile(hit_file_path); 
  TTree* hit_tree = hit_file->Get<TTree>("HitTree"); 
  std::vector<Float_t>* hit_x     = 0;
  std::vector<Float_t>* hit_y     = 0;
  std::vector<Float_t>* hit_z     = 0;
  std::vector<Float_t>* hit_q     = 0;
  std::vector<Float_t>* hit_qtrue = 0;

  hit_tree->SetBranchAddress("hit_x"    , &hit_x); 
  hit_tree->SetBranchAddress("hit_y"    , &hit_y); 
  hit_tree->SetBranchAddress("hit_z"    , &hit_z); 
  hit_tree->SetBranchAddress("hit_q"    , &hit_q); 
  hit_tree->SetBranchAddress("hit_qtrue", &hit_qtrue); 

  TFile* mc_file = new TFile(mc_file_path); 
  TTree* mc_tree = mc_file->Get<TTree>("EventTree"); 
  SLArMCEvent* ev = 0; 
  mc_tree->SetBranchAddress("MCEvent", &ev); 

  TH3F* h3 = new TH3F("h3", "SoLAr TPC;x [mm];z [mm]; y [mm]", 50, 0, +1000, 50, 300, 700, 50, 0, 600); 
  auto pdg = TDatabasePDG::Instance(); 

  hit_tree->GetEntry(0); 
  mc_tree->GetEntry(0); 

  for (size_t i = 0; i < hit_x->size(); i++) {
    printf("[%lu]\t x: %.2f, y: %.2f, z: %.2f\n", i, hit_x->at(i), hit_y->at(i), hit_z->at(i)); 
  }
  TGraph2D* ghit = new TGraph2D(hit_x->size(), &hit_x->at(0), &hit_z->at(0), &hit_y->at(0)); 
  ghit->SetMarkerStyle(20); 
  ghit->SetMarkerSize(0.8); 

  h3->Draw(); 

  const auto& primaries = ev->GetPrimaries();
    for (const auto &p : primaries) {
      printf("----------------------------------------\n");
      printf("[gen: %s] PRIMARY vertex: %s - K0 = %2f - t = %.2f - vtx [%.1f, %.1f, %.1f]\n", 
          p.GetGeneratorLabel().Data(),
          p.GetParticleName().Data(), p.GetEnergy(), p.GetTime(), 
          p.GetVertex()[0], p.GetVertex()[1], p.GetVertex()[2]);
      auto& trajectories = p.GetConstTrajectories(); 
      for (auto &t : trajectories) {
        auto points = t->GetConstPoints(); 
        auto pdg_particle = pdg->GetParticle(t->GetPDGID()); 
        printf("%s [%i]: t = %.2f, K = %.2f - n_scint = %g, n_elec = %g\n", 
            t->GetParticleName().Data(), t->GetTrackID(), 
            t->GetTime(),
            t->GetInitKineticEne(), 
            t->GetTotalNph(), t->GetTotalNel());
        if (t->GetInitKineticEne() < 0.01) continue;
        Color_t col = kBlack; 
        TString name = ""; 

        if (!pdg_particle) {
          col = kBlack; 
          name = Form("g_%i_trk%i", t->GetPDGID(), t->GetTrackID()); 
        }
        else {
          if (pdg_particle == pdg->GetParticle(22)) col = kYellow;
          else if (pdg_particle == pdg->GetParticle( 11)) col = kBlue-6;
          else if (pdg_particle == pdg->GetParticle(-11)) col = kRed-7;
          else if (pdg_particle == pdg->GetParticle(2212)) col = kRed; 
          else if (pdg_particle == pdg->GetParticle(2112)) col = kBlue;
          else if (pdg_particle == pdg->GetParticle(-211)) col = kOrange+7; 
          else if (pdg_particle == pdg->GetParticle( 211)) col = kViolet-2; 
          else if (pdg_particle == pdg->GetParticle( 111)) col = kGreen; 
          else    col = kGray+2;
          name = Form("g_%s_trk_%i", pdg_particle->GetName(), t->GetTrackID()); 
        }
        
        if (points.size() < 2) continue;

        Double_t x_true[points.size()];
        Double_t y_true[points.size()];
        Double_t z_true[points.size()];

        size_t ii = 0;
        for (const auto &pt : points) {
          x_true[ii] = pt.fX;
          y_true[ii] = pt.fY;
          z_true[ii] = pt.fZ;
          ii++;
        }

        TPolyLine3D* gtrue = new TPolyLine3D(points.size(), x_true, z_true, y_true); 
        gtrue->SetLineColor(col); 
        gtrue->SetLineWidth(2);
        gtrue->Draw("same"); 
      }
    }

  ghit->Draw("p same"); 
  return 0;
}

