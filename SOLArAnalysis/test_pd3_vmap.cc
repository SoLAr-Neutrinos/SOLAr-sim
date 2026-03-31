/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : test_pd3_vmap.cc
 * @created     : Thursday Jan 15, 2026 11:49:15 CET
 */

#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH2Poly.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"
#include "event/SLArMCTruth.hh"

#include "config/SLArCfgAnode.hh"



int test_pd3_vmap(
    const TString input_path = "/home/guff/Dune/SOLAr/test-protodune3-fls/test-lowe-tracing.root",
    const TString vmap_path = "/home/guff/Dune/SOLAr/SOLAr-sim/g4_extra_data/solar-v2-phlibrary.root")
{
  gStyle->SetPalette(kSunset);
  // ====== Photon library access ======
  // open photon library file and retrieve the tree
  TFile* photonlib_file = TFile::Open(vmap_path, "READ");
  TTree* plib = photonlib_file->Get<TTree>("photonLib"); 
  plib->SetCacheSize(10000);

  TTreeReader reader(plib);
  TTreeReaderValue<float> x(reader, "x");
  TTreeReaderValue<float> y(reader, "y");
  TTreeReaderValue<float> z(reader, "z");
  TTreeReaderValue<float> vtot(reader, "vis_tot");
  TTreeReaderValue<float> vdir(reader, "vis_dir");
  TTreeReaderValue<float> vwls(reader, "vis_wls");
  TTreeReaderArray<float> tmain_vtot(reader, "vis_tot_tile_main");
  TTreeReaderArray<float> tmain_vdir(reader, "vis_dir_tile_main");
  TTreeReaderArray<float> tmain_vwls(reader, "vis_wls_tile_main");
  TTreeReaderArray<float> tmain_sipm(reader, "vis_sipm_main");

  // prepare bin edges for the photon library
  auto get_bin_vec = [](const float& x0, const float& x1, const float& step) {
    std::vector<float> v;
    float tmp = x0; 
    while (tmp < x1) {
      v.push_back(tmp);
      tmp+=step;
    }
    return v;
  };

  std::vector<float> binx = get_bin_vec(-1500, 1500, 100);
  std::vector<float> biny = get_bin_vec(-1750, 1750, 100);
  std::vector<float> binz = get_bin_vec(-1600, 1600, 100);

  // ====== Input file access ======
  TFile* input_file = TFile::Open(input_path, "READ");
  if (!input_file || input_file->IsZombie()) {
    std::cerr << "Error: could not open input file " << input_path << std::endl;
    return 1;
  }

  TTree* eve_tree = input_file->Get<TTree>("EventTree");
  if (!eve_tree) {
    std::cerr << "Error: could not find EventTree in file " << input_path << std::endl;
    return 1;
  }

  std::map<int, SLArCfgAnode*> cfgAnode_map;
  cfgAnode_map.insert({11, input_file->Get<SLArCfgAnode>("AnodeCfg51")});
  //cfgAnode_map.insert({12, input_file->Get<SLArCfgAnode>("AnodeCfg61")});
  //cfgAnode_map.insert({13, input_file->Get<SLArCfgAnode>("AnodeCfg71")});

  // build tile map for the bottom anode (TPC 11, 12 and 13)
  std::vector<TH2Poly*> h2TileMaps;

  for (auto& anode_cfg_itr : cfgAnode_map) {
    const int& tpc_id = anode_cfg_itr.first;
    auto& anode_cfg = anode_cfg_itr.second;

    for (const auto& mt_cfg : anode_cfg->GetConstMap()) {
      int mt_idx = mt_cfg.GetIdx();
      h2TileMaps.emplace_back(
          anode_cfg->ConstructPixHistMap(1, std::vector<int>{mt_idx}));
      h2TileMaps.back()->SetDirectory(nullptr);
      h2TileMaps.back()->SetNameTitle(
          Form("h2_tile_tpc%i_mt%i", tpc_id, mt_idx),
          Form("Tile map TPC %i - Megatile %i;X [mm];Y [mm]", tpc_id, mt_idx)
          );
      printf("TPC %i MT %i setup\n", tpc_id, mt_idx);
    }
  }

  // draw tile maps
  TCanvas* cTiles = new TCanvas("cTiles", "Anode Tile Maps", 900, 800);
  TH2D* hframe = new TH2D("hframe", "Anode Tile Maps;Z [mm;X [mm]", 
      100, -2000, 2000, 100, -2000, 2000);
  hframe->SetStats(0);
  cTiles->cd();
  hframe->Draw("axis");
  for (auto& h2tile : h2TileMaps) {
    h2tile->SetLineWidth(2);
    h2tile->Draw("same l");
  }

  const int test_mt_idx = 1;    // change this to test different megatiles
  const int test_tile_idx = 8; // change this to test different tiles
  const int test_sipm_idx = 76;  // change this to test different SiPMs

  const int test_sipm_global_idx = test_sipm_idx + 160*(test_tile_idx + 30*test_mt_idx);

  const auto &mt_cfg = cfgAnode_map[11]->GetBaseElement(test_mt_idx);
  const auto &tile_cfg = mt_cfg.GetBaseElement(test_tile_idx);
  const auto& tile_bin_idx = tile_cfg.GetBinIdx(); 



  printf("Highlighting megatile %i, tile %i - bin %i, sipm %i (glob index %i)\n", 
      test_mt_idx, test_tile_idx, tile_bin_idx, test_sipm_idx, test_sipm_global_idx);

  int ibin = 0;
  for (const auto& bin : *h2TileMaps[0]->GetBins()) {
    TH2PolyBin* bbin = static_cast<TH2PolyBin*>(bin);
    if (auto g = dynamic_cast<TGraph*>(bbin->GetPolygon()) ) {
      g->SetLineColor( gStyle->GetColorPalette( (ibin+1)*255 / 31 ) );
      g->SetLineWidth(2);
    }
    ibin++;
  }

  TH2D* h2_vis_sipm = new TH2D("h2_vis_sipm", "SiPM Visibility Map;Z [mm];X [mm]",
      binz.size()-1, binz.data(), binx.size()-1, binx.data());
  h2_vis_sipm->SetStats(0);

  while (reader.Next()) {
    const int vbin_idx = h2_vis_sipm->FindBin(*z, *x);
    h2_vis_sipm->AddBinContent( vbin_idx, tmain_sipm[test_sipm_global_idx] );
  }
  h2_vis_sipm->SetEntries( plib->GetEntries() );

  h2_vis_sipm->Scale(1.0e2 / biny.size()); 
  new TCanvas();
  h2_vis_sipm->Draw("colz same"); 
  auto tile_bin = (TH2PolyBin*)h2TileMaps[test_mt_idx]->GetBins()->At(tile_bin_idx-1);
  TGraph* gbin = (TGraph*)tile_bin->GetPolygon();
  gbin->SetLineColor(kWhite); 
  gbin->SetLineWidth(3); 
  gbin->Draw("l same");

  SLArCfgMegaTile& cfg_mt = cfgAnode_map[11]->GetBaseElement(test_mt_idx);
  SLArCfgReadoutTile& cfg_tile = cfg_mt.GetBaseElement(test_tile_idx);

  TGraph* g_zx_tile = new TGraph();
  g_zx_tile->AddPoint(cfg_tile.GetPhysZ(), cfg_tile.GetPhysX());
  g_zx_tile->SetMarkerStyle(21);
  g_zx_tile->SetMarkerSize(2);
  g_zx_tile->SetMarkerColor(kViolet);
  g_zx_tile->Draw("p same");


  return 0;
}

