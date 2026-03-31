/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : check_neutron_final_state.cc
 * @created     : Monday Nov 04, 2024 11:28:07 CET
 */

#include "RtypesCore.h"
#include "TString.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <vector>
#include <map>
#include <regex>

#include <event/SLArMCEvent.hh>
#include <event/SLArMCPrimaryInfo.hh>
#include <event/SLArEventTrajectory.hh>
#include <event/SLArGenRecords.hpp>

const TString marley_direcotry_path = "/home/daniele/Software/marley-1.2.0/examples/neutron_decay/";

struct EventInfo_t {
  Long64_t event_nr;
  Long64_t file_id;

  EventInfo_t(Long64_t event_nr, Long64_t file_id) : event_nr(event_nr), file_id(file_id) {}
};

std::vector<EventInfo_t> read_csv_list(const char* list_path) {
  std::vector<EventInfo_t> events;
  std::ifstream file_list; 
  file_list.open(list_path);

  std::string line;
  while (std::getline(file_list, line)) {
    std::istringstream ss(line);
    std::string token;
    std::getline(ss, token, ',');
    Long64_t event_nr = std::stoll(token);
    std::getline(ss, token, ',');
    Long64_t file_id = std::stoll(token);
    events.push_back(EventInfo_t(event_nr, file_id));
  }
  return events;
}

int check_neutron_final_state()
{
  const auto event_list = read_csv_list("neutron_decay_events.csv");

  TFile* file = nullptr; 
  TTree* tree_eve = nullptr; 
  TTree* tree_gen = nullptr;

  SLArMCEvent* ev = nullptr;
  SLArGenRecordsVector* gen_records = nullptr;

  for (const auto& event : event_list) {
    const TString file_path = Form("%ssolarfd_marleyflat_%010lld.root", marley_direcotry_path.Data(), event.file_id);
    if (file == nullptr || file->GetName() != file_path) {
      if (file != nullptr) {
        file->Close();
        tree_eve = nullptr;
        tree_gen = nullptr;
        delete file;
      }
      file = TFile::Open(file_path);
      if (file == nullptr) {
        std::cerr << "Error: file " << file_path << " not found" << std::endl;
        return 1;
      }
      tree_eve = file->Get<TTree>("EventTree");
      tree_gen = file->Get<TTree>("GenTree");

      tree_eve->SetBranchAddress("MCEvent", &ev);
      tree_gen->SetBranchAddress("SLArGenRecords", &gen_records);
    }


    tree_eve->GetEntry(event.event_nr);
    tree_gen->GetEntry(event.event_nr);

    const auto& primaries = ev->GetPrimaries();
    const auto& gen_info = gen_records->GetRecordsVector().at(0);

    std::cout << "Event " << event.event_nr << " in file " << event.file_id << std::endl;
    std::cout << "Primary: " << primary->GetPDGCode() << " " << primary->GetEnergy() << " MeV" << std::endl;
    std::cout << "Trajectory: " << trajectory->GetPDGCode() << " " << trajectory->GetEnergy() << " MeV" << std::endl;
    std::cout << "Gen: " << gen_info->GetPDGCode() << " " << gen_info->GetEnergy() << " MeV" << std::endl;
    std::cout << std::endl;
  }
    
  return 0;
}

