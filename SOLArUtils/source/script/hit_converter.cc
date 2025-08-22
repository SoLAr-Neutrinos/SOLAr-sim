/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : hit_converter.cc
 * @created     : Thursday Apr 04, 2024 19:30:16 CEST
 */

// std
#include <cstdio>
#include <getopt.h>
#include <vector>
#include <iterator>

// rapidjson
#include "rapidjson/document.h"

// SOLAr-sim unit utility
#include "geo/SLArUnit.hpp"

// config
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgReadoutTile.hh"
#include "physics/SLArElectronDrift.hh"

// event
#include "event/SLArEventAnode.hh"
#include "event/SLArEventMegatile.hh"
#include "event/SLArEventTile.hh"
#include "event/SLArEventSuperCell.hh"
#include "event/SLArEventSuperCellArray.hh"

// hits
#include "TChannelAnalyzer.hh"
#include "SLArRecoHits.hh"

// root
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TObjString.h"


void print_usage() {
  printf("SoLAr hit_converter: Converting simulated events into hits collection\n"); 
  printf("Usage: "); 
  printf("hit_converter\n"); 
  printf("\t[--input     | -i] input_simulatin_file\n"); 
  printf("\t[--output    | -o] output_hit_file\n"); 
  printf("\t[--noise     | -n] optional - noise rms in Vee (default = 900)\n"); 
  printf("\t[--threshold | -t] optional - hit threshold (default 1500 Vee)\n"); 
  printf("\t[--window    | -w] optional - charge integration window in μs (default 1)\n\n");

  exit( EXIT_SUCCESS );
}

/**
 * Convert solar simulation event into a collection of hits
 */
int main (int argc, char *argv[]) {
   const char* short_opts = "i:o:n:t:w:h";
   static struct option long_opts[7] = 
   {
     {"input", required_argument, 0, 'i'}, 
     {"output", required_argument, 0, 'o'}, 
     {"noise", required_argument, 0, 'n'}, 
     {"threshold", required_argument, 0, 't'}, 
     {"window", required_argument, 0, 'w'}, 
     {"help", no_argument, 0, 'h'}, 
     {nullptr, no_argument, nullptr, 0}
   };

  int c, option_index; 

  TString input_file_path = ""; 
  TString output_file_path = ""; 
  Float_t noise_rms_eeV =  900.0; 
  Float_t threshold_eeV = 3800.0; 
  Float_t window_int_us =    1.8; 

  while ( (c = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1) {
    switch(c) {
      case 'i' :
        input_file_path = optarg;
        break;
      case 'o' :
        output_file_path = optarg;
        break;
      case 'n' : 
        noise_rms_eeV = std::atof(optarg);
        break;
      case 't' :
        threshold_eeV = std::atof(optarg);
        break;
      case 'w' : 
        window_int_us = std::atof(optarg);
        break;
      case 'h' : 
        print_usage(); 
        exit( EXIT_SUCCESS ); 
        break;
      case '?' : 
        printf("solar_sim error: unknown flag %c\n", optopt);
        print_usage(); 
        exit( EXIT_FAILURE ); 
        break;
    }
  }
  printf("Monte Carlo input file: %s\n", input_file_path.Data());
  printf("hit output file: %s\n", output_file_path.Data());
  printf("pixel noise rms [eeV]: %.2f\n", noise_rms_eeV);  
  printf("hit threshold [eeV]: %.2f\n", threshold_eeV);  
  printf("integration window: %.2f us\n", window_int_us); 

  // Setup input file and MC event
  TFile* input_file = new TFile(input_file_path); 
  if (input_file->IsZombie()) {
    fprintf(stderr, "ERROR: Unable to open input Monte Carlo file %s\n", 
        input_file_path.Data());
    exit( EXIT_FAILURE ); 
  }
  TTree* mc_tree = input_file->Get<TTree>("EventTree"); 
  if (mc_tree == nullptr) {
    fprintf(stderr, "ERROR: Cannot open EventTree from Monte Carlo file %s.\n",
        input_file_path.Data()); 
    exit( EXIT_FAILURE ); 
  }

  TTreeReader mc_tree_reader(mc_tree);
  TTreeReaderValue<SLArListEventAnode> anode_list_ev(mc_tree_reader, "EventAnode");
  
  // Setup anode configuration
  std::map<Int_t, SLArCfgAnode*> anodeConfig; 
  std::map<Int_t, TVector3> tpcCenterPos;
  for (const auto& key : *input_file->GetListOfKeys()) {
    TString obj_name = key->GetName();
    if (obj_name.Contains("AnodeCfg")) {
      SLArCfgAnode* anode_cfg = input_file->Get<SLArCfgAnode>(key->GetName()); 
      anodeConfig.insert( {anode_cfg->GetTPCID(), anode_cfg} ); 
      printf("Found anode configuration for TPC %i\n", anode_cfg->GetTPCID()); 
    }
  }
  auto geometry_str = input_file->Get<TObjString>("geometry"); 
  rapidjson::Document d; 
  d.Parse<rapidjson::kParseCommentsFlag>( geometry_str->GetString() ); 
  if (d.HasMember("TPC")) {
    for (const auto& jtpc : d["TPC"].GetArray()) {
      printf("found tpc with id %i\n", jtpc["copyID"].GetInt()); 
      if (jtpc.HasMember("position") == false) {
        throw std::invalid_argument("Error reading TPC configuration: mandatory \"position\" field is not defined");
      }

      const auto& jposition = jtpc["position"]; 
      const auto& jxyz = jposition["xyz"]; 
      const double unit = unit::Unit2Val( jposition["unit"] ); 
      TVector3 position( jxyz.GetArray()[0].GetDouble()*unit, 
                         jxyz.GetArray()[1].GetDouble()*unit,
                         jxyz.GetArray()[2].GetDouble()*unit ); 
    
      tpcCenterPos.insert( {jtpc["copyID"].GetInt(), position} ); 
    }
  }

  Float_t drift_velocity = 1.582e-3; // mm/ns

  // Setup output file
  TFile* output_file = new TFile(output_file_path, "recreate"); 
  TTree* hit_tree = new TTree("HitTree", "hit collection tree"); 
  UInt_t iev = 0; 
  Int_t itpc = 0;
  reco::hitvarContainer hitvars; 
  hit_tree->Branch("iev", &iev); 
  hit_tree->Branch("hit_x", &hitvars.hit_x);
  hit_tree->Branch("hit_y", &hitvars.hit_y);
  hit_tree->Branch("hit_z", &hitvars.hit_z);  
  hit_tree->Branch("hit_q", &hitvars.hit_q);  
  hit_tree->Branch("hit_qtrue", &hitvars.hit_qtrue);
  hit_tree->Branch("hit_tpc", &hitvars.hit_tpc);

  TChannelAnalyzer ch_analyzer; 
  ch_analyzer.set_hit_threshold( threshold_eeV );
  ch_analyzer.set_channel_rms( noise_rms_eeV ); 
  ch_analyzer.set_integration_window_us( window_int_us );
  
  while( mc_tree_reader.Next() ) {
    hitvars.reset(); 
    iev = anode_list_ev->GetEventNumber();

    const auto& anodes_map = anode_list_ev->GetAnodeMap(); 

    for (const auto& anode_itr : anodes_map) {
      itpc = anode_itr.first;
      const SLArEventAnode& anode = anode_itr.second;
      SLArCfgAnode* anode_cfg = anodeConfig[anode_itr.first]; 
      const TVector3 drift_direction = anode_cfg->GetNormal(); 

      ch_analyzer.set_anode_config( anode_cfg ); 
      ch_analyzer.set_drift_direction( drift_direction ); 
      ch_analyzer.set_drift_velocity( drift_velocity );
      ch_analyzer.set_tpc_id( itpc ); 
      ch_analyzer.set_tpc_center_position( tpcCenterPos[itpc] ); 

      const auto& mt_map = anode.GetConstMegaTilesMap();
      for (const auto& mt_itr : mt_map) {
        const auto& mt = mt_itr.second;
        //std::printf("[%i] mt hits: %i\n", mt_itr.first, mt.GetNChargeHits()); 
        if (mt.GetNChargeHits() > 0) {
          const auto& mt_cfg = anode_cfg->GetBaseElement(mt_itr.first);
          ch_analyzer.set_megatile_config( &mt_cfg ); 
          const auto& t_map = mt.GetConstTileMap(); 
          for (const auto &t_itr : t_map) {
            const auto& t = t_itr.second;
            if (t.GetPixelHits() == 0) continue;
            //printf("\t[%i]: tile hits: %g\n", t_itr.first, t.GetNPixelHits()); 
            const auto& t_cfg = mt_cfg.GetBaseElement(t_itr.first); 
            ch_analyzer.set_tile_config( &t_cfg ); 
            const auto& pixels = t.GetConstPixelEvents();
            for (const auto& pixel_itr : pixels) {
              ch_analyzer.process_channel(pixel_itr.first, pixel_itr.second, hitvars ); 
            } // end of loop over pixels
          }
        } // end of loop over tiles
      } // end of loop over megatiles
    } // end of loop over anodes

    hit_tree->Fill(); 

  } // end of loop over tree entries

  hit_tree->Write(); 
  output_file->Close(); 

  input_file->Close(); 

  return 0;
}



