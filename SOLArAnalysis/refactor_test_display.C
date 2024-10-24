/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        refactor_test_sc.C
 * @created     Mon Mar 27, 2023 11:19:53 CEST
 */

#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "TKey.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TPaletteAxis.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include <TTimer.h>
#include "TRandom3.h"
#include "TDatabasePDG.h"

#include "event/SLArMCEvent.hh"
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgAssembly.hh"
#include "config/SLArCfgSuperCellArray.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

inline void write_csv(std::string filename, std::vector<std::pair<std::string, std::vector<double>>> dataset){
  
    // Create an output filestream object
    std::ofstream myFile(filename);
    
    // Send column names to the stream
    for(int j = 0; j < dataset.size(); ++j)
    {
        myFile << dataset.at(j).first;
        if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
    }
    myFile << "\n";
    
    // Send data to the stream
    for(int i = 0; i < dataset.at(0).second.size(); ++i)
    {
        for(int j = 0; j < dataset.size(); ++j)
        {
            myFile << dataset.at(j).second.at(i);
            if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
        }
        myFile << "\n";
    }
    
    // Close the file
    myFile.close();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

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

const bool verbose = 0; 

Color_t get_color(const int& pdg_code) {
  if      ( pdg_code ==   11) return (kBlue-6);
  else if ( pdg_code ==  -11) return (kRed-7); 
  else if ( pdg_code ==   22) return (kYellow);
  else if ( pdg_code == 2212) return (kRed+1); 
  else if ( pdg_code == 2112) return (kBlue+1); 
  else if ( pdg_code == -211) return (kOrange+7);
  else if ( pdg_code ==  211) return (kViolet-2);
  else if ( pdg_code ==  111) return (kGreen);
  else return (kGray+2);
}

int process_anode_light( 
    const SLArEventAnode& ev_anode, 
    SLArCfgAnode* cfg_anode, 
    AnodeTileMap& h2_anode,
    TH1D* hTime
    )
{
  int nhits_anode = 0;
  const int& anode_id = ev_anode.GetID();
  std::vector<TH2Poly*> h2mt; h2mt.reserve(50); 
  std::vector<TH2Poly*> h2pix; h2pix.reserve(500); 

  for (const auto &_mtevent: ev_anode.GetConstMegaTilesMap()) {
    const SLArEventMegatile& mtevent = _mtevent.second;
    if (verbose) {
      printf("\tMegatilte %i: %i photon hits - %i charge hits\n", 
          _mtevent.first, mtevent.GetNPhotonHits(), mtevent.GetNChargeHits());
    }
    const SLArCfgMegaTile& mtconfig = cfg_anode->GetBaseElement(_mtevent.first);
    const int mt_index = mtconfig.GetIdx();
    auto h2mt_ = cfg_anode->ConstructPixHistMap(1, std::vector<int>{ mt_index }); 
    h2mt.push_back( h2mt_ ); 

    if (mtevent.GetNPhotonHits() > 0) {
      // loop over the tiles in the mega-tile
      for (const auto& _tilevent : mtevent.GetConstTileMap()) {
        const SLArEventTile& tilevent = _tilevent.second;
        if (verbose) {
          printf("\t\tTile %i: %i photons hits - %g charge hits\n", 
              _tilevent.first, 
              tilevent.GetNhits(),
              tilevent.GetNPixelHits());
        }

        if (tilevent.GetNhits() > 0) {
          const SLArCfgReadoutTile& tileconfig = mtconfig.GetBaseElement(_tilevent.first); 
          // Get number of hits on all SiPMs in the tile
          const int& nhits = tilevent.GetNhits();
          nhits_anode += nhits;
          // retrieve the univocal bin index from the tile id
          const Int_t tile_bin_idx = tileconfig.GetBinIdx(); 
          // set bin content in the hit map
          TH2Poly* h2 = h2_anode.find(mtevent.GetIdx())->second; 
          h2->SetBinContent(tile_bin_idx, nhits);
          // just for fun, let's get the photons' hit time
          for (const auto& hit : tilevent.GetConstHits()) {
            hTime->Fill(hit.first, hit.second*tilevent.GetClockUnit());
          }
        }
      }
    }
  }

  return nhits_anode;
}

void get_mctruth_tracks(
    std::vector<TGraph>& gtracks,
    const std::vector<SLArMCPrimaryInfo>& primaries, 
    SLArCfgAnode* cfg_anode) {
  int tpc_copy = cfg_anode->GetTPCID(); 

  const TVector3& axis0 = cfg_anode->GetAxis0();
  const TVector3& axis1 = cfg_anode->GetAxis1();

  for (const auto& primary : primaries) {
    const auto& trajectories = primary.GetConstTrajectories(); 
    for (const auto& t : trajectories) {
      const auto& points = t->GetConstPoints();
      
      TGraph g;
      g.SetLineWidth(3); 
      for (const auto& p : points) {
        if (p.fCopy == tpc_copy) {
          TVector3 ppos(p.fX, p.fY, p.fZ);
          g.AddPoint(axis0.Dot(ppos), axis1.Dot(ppos));
        }
      }

      if (g.GetN() > 10) {
        g.SetName( 
            Form("g_%s_trkid%i_%gMeV", 
              t->GetParticleName().Data(), t->GetTrackID(), t->GetInitKineticEne()));
        g.SetLineColor( get_color(t->GetPDGID()) ); 
        gtracks.emplace_back(g);
      }
    }
  }
}

int process_anode_charge(const SLArEventAnode& ev_anode, 
    SLArCfgAnode* cfg_anode, 
    std::vector<TH2Poly*>& h2AnodeCharge) 
{
  int qmax = 0; 
  int qhits_total = 0; 
  
  for ( const auto& itr_megatile : ev_anode.GetConstMegaTilesMap() ) {
    int qhits_mt = 0;
    const int& idx_megatile = itr_megatile.first;
    const SLArEventMegatile& ev_megatile = itr_megatile.second;

    if (ev_megatile.GetNChargeHits() == 0) continue;

    const SLArCfgMegaTile& cfg_megatile = cfg_anode->GetBaseElement(idx_megatile);

    for ( const auto& itr_tile : ev_megatile.GetConstTileMap() ) {
      int qhits_t = 0;
      const int& idx_tile = itr_tile.first; 
      const SLArEventTile& ev_tile = itr_tile.second;

      if (ev_tile.GetNPixelHits() == 0) continue;

      const SLArCfgReadoutTile& cfg_tile = cfg_megatile.GetBaseElement(idx_tile); 
      TH2Poly* h2 = cfg_anode->ConstructPixHistMap(2, std::vector<int>{idx_megatile, idx_tile});

      for (const auto& itr_pixel : ev_tile.GetConstPixelEvents()) {
        const int& idx_pixel = itr_pixel.first; 
        const SLArEventChargePixel& ev_pixel = itr_pixel.second; 
        const int& qpix = ev_pixel.GetNhits(); 
        if (qpix > qmax) qmax = qpix;
        h2->SetBinContent( idx_pixel, qpix ); 
        for (const auto& hit : ev_pixel.GetConstHits()) {
          qhits_t += hit.second;
        }
      }
      h2AnodeCharge.emplace_back( std::move(h2) ); 
      printf("\t\tTile %i: %i charge hits\n", idx_tile, qhits_t);
      qhits_mt += qhits_t;
    }

    printf("\tMegatile %i: %i charge hits\n", idx_megatile, qhits_mt);
    qhits_total += qhits_mt;
  }

  for (auto& h2 : h2AnodeCharge) {
    h2->GetZaxis()->SetRangeUser(0, qmax);
  }

  return qhits_total;
}

int process_pdarray(
    const SLArEventSuperCellArray& ev_pdwall, 
    SLArCfgSuperCellArray* cfg_pdwall,
    TH2Poly* h2_pdwall, 
    TH1D* hTime
    )
{
  int nhits_pdarray = 0;

  for (const auto& _module : ev_pdwall.GetConstSuperCellMap()) {
    const SLArEventSuperCell& modulevent = _module.second; 
    const SLArCfgSuperCell& moduleconfig = cfg_pdwall->GetBaseElement(_module.first); 
    const int& nhits = modulevent.GetNhits();
    nhits_pdarray += nhits;
    const Int_t module_bin_index = moduleconfig.GetBinIdx(); 
    // Set bin content in the hit map
    h2_pdwall->SetBinContent(module_bin_index, nhits);
    // fill time histogram
    for (const auto &hit : modulevent.GetConstHits()) {
      hTime->Fill( hit.first, hit.second*modulevent.GetClockUnit() ); 
    }
  }

  return nhits_pdarray;
}

int process_event(SLArMCEvent* ev, 
    const std::map<int, SLArCfgAnode*>& AnodeSysCfg, 
    SLArCfgBaseSystem<SLArCfgSuperCellArray>* PDSSysConfig, 
    std::map<int, std::vector<TH2Poly*>>& h2AnodeTilesCharge,
    std::map<int, AnodeTileMap>& h2AnodeTiles, 
    std::map<int, TH2Poly*>& h2PDArray, 
    TH1D* hTime)
{
  int nhits_total = 0; 

  const std::map<int, SLArEventSuperCellArray> walls = ev->GetEventSuperCellArray();
  const std::map<int, SLArEventAnode>& anodes = ev->GetEventAnode(); 

  // read the anode signals first
  for (const auto &anode_ : anodes) {
    const SLArEventAnode& ev_anode = anode_.second;
    int nhits = process_anode_light( 
        ev_anode, AnodeSysCfg.at(anode_.first), h2AnodeTiles.at(anode_.first), hTime);
    int qhits = process_anode_charge(
        ev_anode, AnodeSysCfg.at(anode_.first), h2AnodeTilesCharge.at(anode_.first)
        );
    nhits_total += nhits;
  }
  // let's read the wall modules now
  for (const auto& wallevent : walls) {
    const SLArEventSuperCellArray& ev_pdarray = wallevent.second;
    SLArCfgSuperCellArray& cfg_pdarray = PDSSysConfig->GetBaseElement(wallevent.first);
    int nhits = process_pdarray(
        ev_pdarray, &cfg_pdarray, h2PDArray.at(wallevent.first), hTime);
    nhits_total += nhits;
  }

  return nhits_total;
}

//void draw_opdet() {}
int get_pad_number(const TVector3& norm) {
  if      ( norm == TVector3( 1,  0,  0) ) return 1; 
  else if ( norm == TVector3( 0,  1,  0) ) return 2; 
  else if ( norm == TVector3(-1,  0,  0) ) return 3; 
  else if ( norm == TVector3( 0, -1,  0) ) return 4; 
  else if ( norm == TVector3( 0,  0,  1) ) return 5; 
  else if ( norm == TVector3( 0,  0, -1) ) return 6; 
  else return 0; 
}

void draw_colorscale(TH2* h2) {
  h2->SetZTitle("Number of hits");
  h2->SetTitleSize(0.06, "z"); 
  h2->SetLabelSize(0.06, "z"); 
  h2->Draw("colz"); 
  gPad->Update();
  auto palette = (TPaletteAxis*)h2->GetListOfFunctions()->FindObject("palette"); 
  gPad->Clear(); 
  palette->SetX1NDC(0.2); 
  palette->SetX2NDC(0.4); 
  palette->Draw();
  gPad->Update();
}

TCanvas* setup_canvas(const int& tpc_id) {
  TCanvas* cPhotons = new TCanvas(Form("cPhotonHits_TPC%i", tpc_id), 
      Form("Photon hits - TPC %i", tpc_id), 0, 0, 1000, 1000);
  // create pads and draw them
  std::vector<TPad*> plot_pads = {
    new TPad(Form("p%iSouth" , tpc_id), "south side", 0.25, 0, 0.75, 0.25) ,
    new TPad(Form("p%iBottom", tpc_id), "bottom side", 0.25, 0.25, 0.75, 0.50) , 
    new TPad(Form("p%iNorth" , tpc_id), "north side", 0.25, 0.50, 0.75, 0.75) ,
    new TPad(Form("p%iTop"   , tpc_id), "top side", 0.25, 0.75, 0.75, 1.0) , 
    new TPad(Form("p%iWest"  , tpc_id), "west side", 0, 0.25, 0.25, 0.50) ,
    new TPad(Form("p%iEast"  , tpc_id), "east side", 0.75, 0.25, 1.0, 0.50) ,
    new TPad("pScale", "hits_scale", 0.75, 0.50, 1.0, 1.0)
  };
  
  for (auto& pad : plot_pads) {
    cPhotons->cd(0); 
    pad->Draw(); 
  }
  return cPhotons;
};

void refactor_test_full_display(const TString file_path) 
{
  TFile* mc_file = new TFile(file_path); 
  TTree* mc_tree = mc_file->Get<TTree>("EventTree"); 

  //- - - - - - - - - - - - - - - - retrieve detector configuration
  auto PDSSysConfig = (SLArCfgBaseSystem<SLArCfgSuperCellArray>*)mc_file->Get("PDSSysConfig"); 
  std::map<int, SLArCfgAnode*>  AnodeSysCfg;
  fill_anode_cfg_map(mc_file, AnodeSysCfg); 

  //- - - - - - - - - - - - - - - - book histograms
  TH1D* hTime = new TH1D("hPhTime", "Photon hit time;Time [ns];Entries", 1000, 0, 5e3); 
  hTime->SetDirectory(nullptr);
  TH1D* hBacktracker = new TH1D("hBacktracker", "backtracker: trkID", 1000, 0, 1000 );
  hBacktracker->SetDirectory(nullptr);
  // 2D histograms of photodetectors arrays. These include just membrane walls
  // instrumented with X-ARAPUCA-style detectors
  std::map<int, TH2Poly*> h2PDArray;
  // let's put here the anode "tiles" instrumented with VUV-sensitive SiPMs, which 
  // are identified by a pair of indices (megatile, tile)
  std::map<int, AnodeTileMap> h2AnodeTiles;
  std::map<int, std::vector<TH2Poly*>> h2AnodeTilesCharge; 

  // - - - - - - - - - - - - - - - setup canvas for event display 
  gStyle->SetOptStat(0);
  std::map<int, TCanvas*> cPhotons; 
  std::map<int, TCanvas*> cCharge;
  for (const auto& cfg_anode : AnodeSysCfg) {
    const auto& tpc_id = cfg_anode.first;
    TCanvas* c = setup_canvas(tpc_id); 
    cPhotons.insert( {tpc_id, c} ); 

    TCanvas* cq = new TCanvas(Form("cChargeTPC%i", tpc_id), Form("Anode charge - TPC %i", tpc_id), 
        0, 0, 800, 600);
    cCharge.insert( {tpc_id, cq} ); 
  }

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
      h2->SetDirectory(nullptr);
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
      h2->SetDirectory(nullptr);
      tile_map.insert({megatile_cfg.GetIdx(), std::move(h2)}); 
    }
    h2AnodeTiles.insert({anodeCfg_.first, tile_map}); 

    h2AnodeTilesCharge.insert( {anodeCfg_.first, std::vector<TH2Poly*>()} );
  }


  //- - - - - - - - - - - - - - - - - - - - - - Setup event access
  SLArMCEvent* ev = 0; 
  mc_tree->SetBranchAddress("MCEvent", &ev); 

  auto& primaries = ev->GetPrimaries();


  TTimer* timer = new TTimer("gSystem->ProcessEvents();", 50, false); 

  std::vector<double> event_vec={};
  std::vector<double> hit_vec={};
  std::vector<double> edep_vec={};
  std::vector<double> x_vec={};
  std::vector<double> y_vec={};
  std::vector<double> z_vec={};

  for (Long64_t iev = 0; iev < mc_tree->GetEntries(); iev++) {
    // reset maps and clear canvas
    for (auto& h2 : h2PDArray) h2.second->Reset("M"); 
    for (auto & anode_tile : h2AnodeTiles) {
      for (auto& h2 : anode_tile.second) h2.second->Reset("M"); 
    }
    for (auto & anode_tiles_vector : h2AnodeTilesCharge) {
      for (auto* tiles : anode_tiles_vector.second) {
        delete tiles;
      }
      anode_tiles_vector.second.clear();
    }
    for (auto & canvas : cPhotons) canvas.second->Clear("D"); 
    for (auto & canvas : cCharge ) canvas.second->Clear("D"); 


    // read new event
    mc_tree->GetEntry(iev); 

    // fill hit maps
    int n_photon_hits = 
      process_event(ev, AnodeSysCfg, PDSSysConfig, h2AnodeTilesCharge, h2AnodeTiles, h2PDArray, hTime); 

    // Fill vectors with the event number and total photons detected per event.
    hit_vec.push_back(n_photon_hits);
    event_vec.push_back(iev);
    
    auto pdg = TDatabasePDG::Instance(); 

    for (const auto &p : primaries) {
      std::cout << "Total Edep is: " <<  p.GetTotalEdep() << std::endl;
      std::cout << "Total Edep in LAr is: " <<  p.GetTotalLArEdep() << std::endl;
      edep_vec.push_back(p.GetEnergy());
      x_vec.push_back(p.GetVertex()[0]);
      y_vec.push_back(p.GetVertex()[1]);
      z_vec.push_back(p.GetVertex()[2]);

      /*
      printf(
            "[gen: %s] PRIMARY vertex: %s - K0 = %2f - t = %.2f - vtx [%.1f, %.1f, %.1f]\n", 
            p.GetGeneratorLabel().Data(),
            p.GetParticleName().Data(), p.GetEnergy(), p.GetTime(), 
            p.GetVertex()[0], p.GetVertex()[1], p.GetVertex()[2]
            );

       auto& trajectories = p.GetConstTrajectories(); 

       for (auto &t : trajectories) {
        auto points = t->GetConstPoints(); 
        auto pdg_particle = pdg->GetParticle(t->GetPDGID()); 
         if (t -> GetInitKineticEne() < 2){
          printf(
             "%s [%i]: t = %.2f, K = %.2f - n_scint = %g, n_elec = %g\n", 
             t->GetParticleName().Data(), t->GetTrackID(), 
             t->GetTime(),
             t->GetInitKineticEne(), 
             t->GetTotalNph(), t->GetTotalNel()
             );

      }
    }
    */
  }

    // get the maximum number of hits in all the maps for proper colorscale handling
    double hit_max = 0; 
    for (const auto& h2 : h2PDArray) {
      if (h2.second->GetMaximum() > hit_max) hit_max = h2.second->GetMaximum(); 
    }
    for (const auto& anode_tile: h2AnodeTiles) {
      for (const auto& h2 : anode_tile.second) {
        if (h2.second->GetMaximum() > hit_max) hit_max = h2.second->GetMaximum();         
      }
    }

    // draw hit maps
    std::vector<TGraph> gtracks; 

    for (const auto& anode_cfg : AnodeSysCfg) {
      const int& tpc_id = anode_cfg.first; 
      TCanvas* c = cPhotons.at(tpc_id); c->cd();

      // get anode map
      AnodeTileMap& anode_map = h2AnodeTiles.at(tpc_id); 
      // get "frame" covering the full anode size
      TH2Poly* hframe = anode_cfg.second->GetAnodeMap(0); 
      hframe->SetNameTitle(Form("anode_tpc%i", tpc_id), Form("anode_tpc%i", tpc_id)); 
      const int pad_nr = get_pad_number( anode_cfg.second->GetNormal() ); 
      TPad* pad = (TPad*)c->GetListOfPrimitives()->At(pad_nr-1);
      pad->cd();
      hframe->Draw("l"); 
      for(auto& h2 : anode_map) {
        if (h2.second->GetEntries() > 0) {
          h2.second->GetZaxis()->SetRangeUser(0, 1.05*hit_max); 
          h2.second->SetTitle( pad->GetTitle() ); 
          h2.second->Draw("col0 l same"); 
        }
      }
      // display MC truth
      gtracks.clear(); 
      get_mctruth_tracks(gtracks, ev->GetPrimaries(), AnodeSysCfg.at(tpc_id)); 
      printf("Drawing %ld tracks for TPC %i on canvas %s.%s\n", 
          gtracks.size(), tpc_id, c->GetName(), pad->GetName());
      for (auto& g : gtracks) {
        g.Draw("l same"); 
      }
      pad->Modified();
      pad->Update();


      // loop over the photodetector arrays and select those beloging to 
      // this TPC 
      bool check = true;
      for (auto &itr : h2PDArray) {
        if (tpc_id == 10 && itr.first >= 40) continue;
        if (tpc_id == 11 && itr.first <  40) continue; 

        TH2Poly* h2 = itr.second;
        h2->GetZaxis()->SetRangeUser(0, 1.05*hit_max); 

        if (check) {
          // draw colorscale
          TPad* pad = (TPad*)c->GetListOfPrimitives()->At(6); 
          pad->cd(); 
          TH2Poly* h2_clone = (TH2Poly*)h2->Clone("h2_clone"); 
          draw_colorscale( h2_clone ); 
          check = false;
        }

        SLArCfgSuperCellArray& cfg_pdarray = PDSSysConfig->GetBaseElement( itr.first ); 
        const int pad_nr = get_pad_number( cfg_pdarray.GetNormal() ); 
        TPad* pad = (TPad*)c->GetListOfPrimitives()->At(pad_nr-1);
        pad->cd();
        h2->SetTitle( pad->GetTitle() ); 
        h2->Draw("col0");
        h2->Draw("l same"); 
      }

      c->Modified(); 
      c->Update(); 

      TCanvas* cq = cCharge.at(tpc_id); cq->cd(); 
      hframe->Draw("l"); 
      auto& h2tiles = h2AnodeTilesCharge.at(tpc_id); 
      for (auto& h2 : h2tiles) {
        printf("Drawing tile charge %s\n", h2->GetName());
        h2->Draw("col0 same"); 
      }
      for (auto& g : gtracks) {
        g.Draw("l same"); 
      }
      cq->Modified(); 
      cq->Update(); 
    }
    

    printf("Event %lld: %i photons detected\n", iev, n_photon_hits);
    timer->TurnOn(); 
    timer->Reset(); 
    printf("Press enter to display the next event, \'q\' to exit.\n"); 
    std::string input; 
    std::getline(std::cin, input);
    timer->TurnOff();

    if (input == "q") break;
  }

  mc_file->Close();

  std::vector<std::pair<std::string, std::vector<double>>> DetectionVec = {{"Event", event_vec}, {"Photons Detected", hit_vec}};
  std::vector<std::pair<std::string, std::vector<double>>> DepositionVec = {{"Energy Deposition (MeV)", edep_vec}};
  std::vector<std::pair<std::string, std::vector<double>>> VertexVec = {{"x (mm)", x_vec}, {"y (mm)", y_vec}, {"z (mm)", z_vec}};

  write_csv("Deposition_vector.csv", DepositionVec);
  write_csv("Vertex_vector.csv", VertexVec);
  write_csv("Detection_vector.csv", DetectionVec);
  return;
}

