/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        refactor_test_sc.C
 * @created     Mon Mar 27, 2023 11:19:53 CEST
 */

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "TKey.h"
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

UInt_t fill_anode_cfg_map(TFile* file, std::map<int, SLArCfgAnode*>& map) {
  UInt_t n_anode = 0;
  TList* l = file->GetListOfKeys(); 
  for (const auto& kk : *l) {
    TKey* k = static_cast<TKey*>(kk);
    if ( std::strcmp(k->GetClassName(), "SLArCfgAnode") == 0) {
      SLArCfgAnode* cfg = file->Get<SLArCfgAnode>(k->GetName());
      int tpc_id = cfg->GetTPCID(); 
      map.insert( std::make_pair(tpc_id, cfg) ); 
      n_anode++;
    }
  }
  return n_anode;
}

typedef std::map<Int_t, TH2Poly*> AnodeTileMap;

void refactor_test_sc(const TString file_path, const int iev) 
{
  gStyle->SetPalette(kBlackBody); 
  TColor::InvertPalette(); 
  TFile* mc_file = new TFile(file_path); 
  TTree* mc_tree = mc_file->Get<TTree>("EventTree"); 

  //- - - - - - - - - - - - - - - - retrieve detector configuration
  auto PDSSysConfig = (SLArCfgBaseSystem<SLArCfgSuperCellArray>*)mc_file->Get("PDSSysConfig"); 
  std::map<int, SLArCfgAnode*>  AnodeSysCfg;
  fill_anode_cfg_map(mc_file, AnodeSysCfg); 

  //- - - - - - - - - - - - - - - - book histograms
  TH1D* hTime = new TH1D("hPhTime", "Photon hit time;Time [ns];Entries", 1000, 0, 5e3); 
  TH1D* hBacktracker = new TH1D("hBacktracker", "backtracker: trkID", 1000, 0, 1000 );
  // 2D histograms of photodetectors arrays. These include just membrane walls
  // instrumented with X-ARAPUCA-style detectors
  std::map<int, TH2Poly*> h2PDArray;
  // let's put here the anode "tiles" instrumented with VUV-sensitive SiPMs, which 
  // are identified by a pair of indices (megatile, tile)
  std::map<int, AnodeTileMap> h2AnodeTiles;

  // fill h2PDArray with wall modules first
  if (PDSSysConfig) {
    for (auto &cfgSCArray_ : PDSSysConfig->GetMap()) {
      auto& cfgSCArray = cfgSCArray_.second;
      printf("SC cfg config: %i - %lu super-cell\n", cfgSCArray_.first, 
          cfgSCArray.GetConstMap().size());
      printf("\tposition: [%g, %g, %g] mm\n", 
          cfgSCArray.GetPhysX(), cfgSCArray.GetPhysY(), cfgSCArray.GetPhysZ()); 
      printf("\tnormal: [%g, %g, %g]\n", 
          cfgSCArray.GetNormal().x(), cfgSCArray.GetNormal().y(), cfgSCArray.GetNormal().z() );
      printf("\teuler angles: [φ = %g, θ = %g, ψ = %g]\n", 
          cfgSCArray.GetPhi()*TMath::RadToDeg(), 
          cfgSCArray.GetTheta()*TMath::RadToDeg(), 
          cfgSCArray.GetPsi()*TMath::RadToDeg());
      cfgSCArray.BuildGShape(); 
      auto h2 = cfgSCArray.BuildPolyBinHist(SLArCfgSuperCellArray::kWorld);  
      h2PDArray.insert( std::make_pair(cfgSCArray.GetIdx(), std::move(h2)) ); 
    }
  }

  // Now create the map of the anode tiles
  for (const auto& anodeCfg_ : AnodeSysCfg) {
    const auto cfgAnode = anodeCfg_.second;
    printf("Anode config: %i (TPC %i) - %lu mega-tiles\n", 
        cfgAnode->GetIdx(), anodeCfg_.first, cfgAnode->GetMap().size());
    printf("\tposition: [%g, %g, %g] mm\n", 
        cfgAnode->GetPhysX(), cfgAnode->GetPhysY(), cfgAnode->GetPhysZ()); 
    printf("\tnormal: [%g, %g, %g]\n", 
        cfgAnode->GetNormal().x(), cfgAnode->GetNormal().y(), cfgAnode->GetNormal().z() );
    printf("\teuler angles: [φ = %g, θ = %g, ψ = %g]\n", 
        cfgAnode->GetPhi()*TMath::RadToDeg(), 
        cfgAnode->GetTheta()*TMath::RadToDeg(), 
        cfgAnode->GetPsi()*TMath::RadToDeg());

    AnodeTileMap tile_map; 
    for (const auto& megatile_cfg : cfgAnode->GetConstMap()) {
      auto h2 = cfgAnode->ConstructPixHistMap(1, std::vector<int>{megatile_cfg.GetIdx()}); 
      tile_map.insert({megatile_cfg.GetIdx(), std::move(h2)}); 
    }
    h2AnodeTiles.insert({anodeCfg_.first, tile_map}); 
  }


  //- - - - - - - - - - - - - - - - - - - - - - Access event
  SLArMCEvent* ev = 0; 
  mc_tree->SetBranchAddress("MCEvent", &ev); 
  mc_tree->GetEntry(iev); 

  auto& primaries = ev->GetPrimaries(); 

  const std::map<int, SLArEventSuperCellArray> walls = ev->GetEventSuperCellArray();
  const std::map<int, SLArEventAnode>& anodes = ev->GetEventAnode(); 

  // read the anode signals first
  for (const auto &anode_ : anodes) {
    auto& anode_event = anode_.second; 
    const int& tpc_id = anode_.first; 
    auto hAnode = AnodeSysCfg[tpc_id]->GetAnodeMap(0); 
    std::vector<TH2Poly*> h2mt; h2mt.reserve(50); 
    std::vector<TH2Poly*> h2pix; h2pix.reserve(500); 

    double z_max = 0; 
    printf("ANODE %i:\n", anode_event.GetID());
    for (const auto &_mtevent: anode_event.GetConstMegaTilesMap()) {
      const SLArEventMegatile& mtevent = _mtevent.second;
      printf("\tMegatilte %i: %i photon hits - %i charge hits\n", 
          _mtevent.first, mtevent.GetNPhotonHits(), mtevent.GetNChargeHits());
      const SLArCfgMegaTile& mtconfig = AnodeSysCfg[tpc_id]->GetBaseElement(_mtevent.first);
      const int mt_index = mtconfig.GetIdx();
      auto h2mt_ = AnodeSysCfg[tpc_id]->ConstructPixHistMap(1, std::vector<int>{ mt_index }); 
      h2mt.push_back( h2mt_ ); 
  
      if (mtevent.GetNPhotonHits() > 0) {
        // loop over the tiles in the mega-tile
        for (const auto& _tilevent : mtevent.GetConstTileMap()) {
          const SLArEventTile& tilevent = _tilevent.second;
          printf("\t\tTile %i: %i photons hits - %g charge hits\n", 
              _tilevent.first, 
              tilevent.GetNhits(),
              tilevent.GetNPixelHits());

          const SLArCfgReadoutTile& tileconfig = mtconfig.GetBaseElement(_tilevent.first); 

          if (tilevent.GetNhits() > 0) {
            // retrieve the univocal bin index from the tile id
            const Int_t tile_bin_idx = tileconfig.GetBinIdx(); 
            // set bin content in the hit map
            TH2Poly* h2 = h2AnodeTiles[tpc_id].find(mtevent.GetIdx())->second; 
            h2->SetBinContent(tile_bin_idx, tilevent.GetNhits());
            // just for fun, let's get the photons' hit time
            for (const auto& hit : tilevent.GetConstHits()) {
              hTime->Fill(hit.first, hit.second);
            }
          }

          if (tilevent.GetPixelHits() > 0) {
            auto h2t_ = AnodeSysCfg[tpc_id]->ConstructPixHistMap(2, std::vector<int>{_mtevent.first, _tilevent.first}); 
            for (const auto &p : tilevent.GetConstPixelEvents()) {
              //printf("\t\t\tPixel %i has %i hits\n", p.first, p.second.GetNhits());
              h2t_->SetBinContent( p.first, p.second.GetNhits() );
              //p.second.PrintHits();
              if (p.second.GetNhits() > z_max) z_max = p.second.GetNhits(); 
            }
            h2pix.push_back( h2t_ ); 
          }
        }
      }
    }

    // let's read the wall modules now
    for (const auto& wallevent : walls) {
      printf("PDS wall ID %i\n", wallevent.first);
      const SLArCfgSuperCellArray& wallconfig = PDSSysConfig->GetBaseElement(wallevent.first);
      if (wallevent.second.GetNhits() == 0) continue;
      for (const auto& _module : wallevent.second.GetConstSuperCellMap()) {
        const SLArEventSuperCell& modulevent = _module.second; 
        const SLArCfgSuperCell& moduleconfig = wallconfig.GetBaseElement(_module.first); 
        const Int_t module_bin_index = moduleconfig.GetBinIdx(); 
        // Set bin content in the hit map
        h2PDArray[wallevent.first]->SetBinContent(module_bin_index, modulevent.GetNhits());
        // fill time histogram
        for (const auto &hit : modulevent.GetConstHits()) {
          hTime->Fill( hit.first, hit.second ); 
        }
      }
    }

    //- - - - - - - - - - - - - - - - - plot hit maps for this TPC
    TCanvas* cPhotons = new TCanvas(Form("cPhotonHits_TPC%i", tpc_id), 
        Form("Photon hits - TPC %i", tpc_id), 0, 0, 1000, 1000);
    std::vector<std::pair<TVector3, TPad*>> plot_pads = {
      {TVector3(-1,  0,  0), new TPad(Form("p%iSouth" , tpc_id), "south_side", 0.25, 0, 0.75, 0.25) },
      {TVector3( 0,  1,  0), new TPad(Form("p%iBottom", tpc_id), "bottom_side", 0.25, 0.25, 0.75, 0.50) }, 
      {TVector3( 1,  0,  0), new TPad(Form("p%iNorth" , tpc_id), "north_side", 0.25, 0.50, 0.75, 0.75) },
      {TVector3( 0, -1,  0), new TPad(Form("p%iTop"   , tpc_id), "top_side", 0.25, 0.75, 0.75, 1.0) }, 
      {TVector3( 0,  0,  1), new TPad(Form("p%iWest"  , tpc_id), "west_side", 0, 0.25, 0.25, 0.50) },
      {TVector3( 0,  0, -1), new TPad(Form("p%iEast"  , tpc_id), "east_side", 0.75, 0.25, 1.0, 0.50) }
    };

    for (auto& pad : plot_pads) {
      for (auto& hwalls : h2PDArray) {
        if (tpc_id == 10 && hwalls.first >= 40) continue;
        else if (tpc_id == 11 && hwalls.first < 40) continue;
        auto& wallCfg = PDSSysConfig->GetBaseElement(hwalls.first);
        const auto& normal = wallCfg.GetNormal();
        if (normal.Dot(pad.first) == 1) {
          cPhotons->cd(0); 
          pad.second->Draw(); 
          pad.second->cd(); 
          TString frame_name = Form("frame_%s", wallCfg.GetName()); 
          hwalls.second->SetNameTitle( frame_name, frame_name ); 
          hwalls.second->Draw("colz l"); 
          continue;
        }
      }

      if ( pad.first.Dot( AnodeSysCfg[tpc_id]->GetNormal()) == 1 ) {
        cPhotons->cd(0); pad.second->Draw(); pad.second->cd();
        TH2Poly* hframe = AnodeSysCfg[tpc_id]->GetAnodeMap(0); 
        hframe->Draw();
        for (const auto& hmt : h2AnodeTiles[tpc_id]) {
          hmt.second->Draw("col same l");
        }
        continue;
      }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - plot charge hits
/*
 *    TCanvas* c = new TCanvas(Form("cTPC%i", tpc_id), Form("TPC %i", tpc_id), 0, 0, 800, 500); 
 *    c->SetTicks(1, 1); 
 *    hAnode->Draw(); 
 *    for (const auto &hmt : h2mt) hmt->Draw("same"); 
 *    for (auto &ht : h2pix) {
 *      ht->GetZaxis()->SetRangeUser(0, z_max*1.05); 
 *      ht->Draw("col same"); 
 *    }
 *
 *    auto pdg = TDatabasePDG::Instance(); 
 *
 *    for (const auto &p : primaries) {
 *      printf("----------------------------------------\n");
 *      printf("[gen: %s] PRIMARY vertex: %s - K0 = %2f - t = %.2f - vtx [%.1f, %.1f, %.1f]\n", 
 *          p.GetGeneratorLabel().Data(),
 *          p.GetParticleName().Data(), p.GetEnergy(), p.GetTime(), 
 *          p.GetVertex()[0], p.GetVertex()[1], p.GetVertex()[2]);
 *      auto& trajectories = p.GetConstTrajectories(); 
 *      for (auto &t : trajectories) {
 *        auto points = t->GetConstPoints(); 
 *        auto pdg_particle = pdg->GetParticle(t->GetPDGID()); 
 *        printf("%s [%i]: t = %.2f, K = %.2f - n_scint = %g, n_elec = %g\n", 
 *            t->GetParticleName().Data(), t->GetTrackID(), 
 *            t->GetTime(),
 *            t->GetInitKineticEne(), 
 *            t->GetTotalNph(), t->GetTotalNel());
 *        if (t->GetInitKineticEne() < 0.01) continue;
 *        TGraph g; 
 *        Color_t col = kBlack; 
 *        TString name = ""; 
 *
 *        if (!pdg_particle) {
 *          col = kBlack; 
 *          name = Form("g_%i_trk%i", t->GetPDGID(), t->GetTrackID()); 
 *        }
 *        else {
 *          if (pdg_particle == pdg->GetParticle(22)) col = kYellow;
 *          else if (pdg_particle == pdg->GetParticle( 11)) col = kBlue-6;
 *          else if (pdg_particle == pdg->GetParticle(-11)) col = kRed-7;
 *          else if (pdg_particle == pdg->GetParticle(2212)) col = kRed; 
 *          else if (pdg_particle == pdg->GetParticle(2112)) col = kBlue;
 *          else if (pdg_particle == pdg->GetParticle(-211)) col = kOrange+7; 
 *          else if (pdg_particle == pdg->GetParticle( 211)) col = kViolet-2; 
 *          else if (pdg_particle == pdg->GetParticle( 111)) col = kGreen; 
 *          else    col = kGray+2;
 *          name = Form("g_%s_trk_%i", pdg_particle->GetName(), t->GetTrackID()); 
 *        }
 *        
 *        for (const auto &pt : points) {
 *          if (pt.fCopy == tpc_id) g.AddPoint( 
 *              TVector3(pt.fX, pt.fY, pt.fZ).Dot( AnodeSysCfg[tpc_id]->GetAxis0()), 
 *              TVector3(pt.fX, pt.fY, pt.fZ).Dot( AnodeSysCfg[tpc_id]->GetAxis1()) );
 *        }
 *
 *        g.SetName(name); 
 *        g.SetLineColor(col); 
 *        g.SetLineWidth(2);
 *        if (g.GetN() > 2) g.DrawClone("l");
 *      }
 *    }
 */
  }

  TCanvas* cTime = new TCanvas(); 
  hTime->Draw("hist");

  return;
}

