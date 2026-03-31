/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : build_vmap
 * @created     : Wednesday Mar 05, 2025 17:34:27 CET
 */

#include <iostream>
#include <fstream>
#include <getopt.h>

#include "TFile.h"
#include "TTree.h"
#include "event/SLArMCEvent.hh"
#include "event/SLArMCPrimaryInfo.hh"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventMegatile.hh"
#include "event/SLArEventTile.hh"

void print_usage() {
  std::cout << "Usage: build_vmap"<< std::endl; 
  std::cout << "\t--input  | -i <input_file>" << std::endl; 
  std::cout << "\t--output | -o <output_file>" << std::endl;
  return;
}

void build_vmap(const TString& input_file_name, const TString& output_file_name) {
  TFile* input_file = TFile::Open(input_file_name);
  if (input_file == nullptr || input_file->IsZombie()) {
    std::cerr << "Error opening input file: " << input_file_name << std::endl;
    return;
  }

  TTree* mctree = input_file->Get<TTree>("EventTree");
  SLArMCEvent* event = nullptr;

  mctree->SetBranchAddress("MCEvent", &event);

  for (size_t i = 0; i < mctree->GetEntries(); i++) {
    mctree->GetEntry(i);

    auto& evAnode = event->GetEventAnodeByTPCID(10); 
    const auto& evMegatile = evAnode.GetConstMegaTilesMap().begin()->second;
    const auto& evTile = evMegatile.GetConstTileMap().begin()->second;

    const auto& hits = evTile.GetConstHits();
    const auto& bcoll = evTile.GetBacktrackerRecordCollection();

    for (const auto& h : hits) {
      printf("[%u] : %u\n", h.first, h.second);
    }

    for (const auto& _b : bcoll) {
      printf("backtracker: %u \n", _b.first); 
      const auto& b = _b.second;
      printf("name: %s\n", b.GetName());
      const auto& records = b.GetConstRecords();
      for (const auto& r : records) {
        printf("\t record: %s\n", r.GetName());
        auto& counter = r.GetConstCounter();
        for (const auto& c : counter) {
          printf("\t\t %u - %u\n", c.first, c.second);
        }
      }

    }
  }

  return;
}

int main (int argc, char *argv[]) {
  TString input_file_name;
  TString output_file_name;

  struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0}
  };

  int option_index = 0;

  int c;

  while ((c = getopt_long(argc, argv, "i:o:", long_options, &option_index)) != -1) {
    switch (c) {
      case 'i':
        input_file_name = optarg;
        break;
      case 'o':
        output_file_name = optarg;
        break;
      case '?':
        print_usage();
        return 1;
      default:
        print_usage();
        return 1;
    }
  }

  
  return 0;
}
