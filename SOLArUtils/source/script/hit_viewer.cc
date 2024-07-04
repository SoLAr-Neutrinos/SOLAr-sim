/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : hit_viewer.cc
 * @created     : Monday Apr 08, 2024 17:44:27 CEST
 */

#include <iostream>
#include <getopt.h>
#include <iterator>
#include <TFile.h>
#include <TTree.h>
#include <TObjString.h>
#include <TEvePointSet.h>
#include <TApplication.h>
#include <TTimer.h>
#include <TCanvas.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <event/SLArMCEvent.hh>
#include <SLArUnit.hpp>

#include <SLArEveDisplay.hh>

void print_usage() {
  printf("SoLAr hit_viewer: Display simulated events into hits collection\n"); 
  printf("Usage: "); 
  printf("hit_viewer\n"); 
  printf("\t[--input     | -i] hit_file path\n"); 
  printf("\t[--control   | -c] simulation_output_path\n"); 
  exit( EXIT_SUCCESS );
}

int process_file(const TString input_file_path, const TString control_file_path = "");

int main (int argc, char *argv[]) {
  const char* short_opts = "i:c:h";
  static struct option long_opts[4] = 
  {
    {"input", required_argument, 0, 'i'}, 
    {"control", required_argument, 0, 'c'}, 
    {"help", no_argument, 0, 'h'}, 
    {nullptr, no_argument, nullptr, 0}
  };

  int c, option_index; 

  TString input_file_path = ""; 
  TString control_file_path = ""; 

  while ( (c = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1) {
    switch(c) {
      case 'i' :
        input_file_path = optarg;
        printf("Monte Carlo input file: %s\n", input_file_path.Data());
        break;
      case 'c' :
        control_file_path = optarg;
        printf("mc truth file: %s\n", control_file_path.Data());
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

  TApplication app("hit_viewer", &argc, argv); 

  process_file( input_file_path, control_file_path) ; 

  app.Run(); 

  return 0;
}

int process_file(const TString input_file_path, const TString control_file_path)
{
  TFile* mc_truth_file = nullptr;
  TTree* mc_truth_tree = nullptr;
  SLArMCEvent* ev = 0; 

  display::SLArEveDisplay* eve_display = new display::SLArEveDisplay();
  eve_display->LoadHitFile( input_file_path, "HitTree" ); 

  if (control_file_path.IsNull() == false) {
    mc_truth_file = new TFile(control_file_path); 
    mc_truth_tree = mc_truth_file->Get<TTree>("EventTree"); 
    mc_truth_tree->SetBranchAddress("MCEvent", &ev); 

    auto geometry_str = mc_truth_file->Get<TObjString>("geometry"); 
    rapidjson::Document d; 
    d.Parse( geometry_str->GetString() ); 
    if (d.HasMember("TPC")) {
      for (const auto& jtpc : d["TPC"].GetArray()) {
        printf("found tpc with id %i\n", jtpc["copyID"].GetInt()); 
      }
      eve_display->Configure( d.GetObj() ); 
    }

    eve_display->LoadTrackFile( control_file_path, "EventTree" ); 
  }

  eve_display->MakeGUI();
  eve_display->SetEntry(0);
  eve_display->ProcessEvent(); 

  if (mc_truth_file) {
    mc_truth_file->Close(); 
  }

  return 0; 
}
