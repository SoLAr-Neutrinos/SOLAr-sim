/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : build_ligthmap.cc
 * @created     : Monday Jul 07, 2025 11:31:18 CEST
 */

#include <iostream>
#include "TFile.h"
#include "TPRegexp.h"
#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TSystemDirectory.h"
#include "TH3D.h"
#include "TCanvas.h"

#include "event/SLArEventAnode.hh"
#include "event/SLArGenRecords.hh"

#include "TTree.h"

int build_lightmap(
    const TString& input_folder, 
    const TString& input_pattern, 
    const TString& output_file)
{
  TChain EvChain("EventTree");
  TChain GenChain("GenTree");

  TPRegexp input_pattern_re(input_pattern);
  TSystemDirectory dir("input_folder", input_folder);

  for (const auto& file : *dir.GetListOfFiles()) {
    if (file->IsFolder()) continue;
    TString file_name = file->GetName();
    if (input_pattern_re.MatchB(file_name) == false) continue;

    TString full_path = input_folder + "/" + file_name;
    std::cout << "Adding file: " << full_path << std::endl;
    EvChain.AddFile(full_path);
    GenChain.Add(full_path);
  }

  EvChain.AddFriend(&GenChain, "GenTree");
  
  TTreeReader reader(&EvChain);
  TTreeReaderValue<SLArListEventAnode> evAnodeList(reader, "EventAnode");
  TTreeReaderValue<SLArGenRecordsVector> genRecords(reader, "GenTree.GenRecords");

  const double num_photons = 1e7; 

  TH3D* lightmap = new TH3D("LightMap", "Light Map",
      31, -15.5, 15.5,
      36, -18.0, 18.0, // Y range in mm
      36, -18.0, 18.0); // Z range in mm

  TFile* output = TFile::Open(output_file, "RECREATE");
  TTree* tree = new TTree("VisMap", "Light Map Tree");
  double TotalVis = 0.0; 
  std::vector<double> SiPMvis(64, 0.0); 
  double coords[3];
  coords[0] = 0.0;
  coords[1] = 0.0;
  coords[2] = 0.0;

  tree->Branch("coords", coords, "coords[3]/D");
  tree->Branch("TotalVisibility", &TotalVis, "TotalVisibility/D");
  tree->Branch("SiPMVisibility", SiPMvis.data(), "SiPMVisibility[64]/D");


  while (reader.Next()) {
    // Process each event
    const auto& evAnode = evAnodeList->GetEventAnodeByTPCID(10); 
    if (evAnode.GetConstMegaTilesMap().empty()) {
      std::cerr << "No MegaTiles found in the event." << std::endl;
      continue;
    }
    const auto& evMT = evAnode.GetConstMegaTilesMap().at(0); 
    const auto& evT = evMT.GetConstTileMap().at(0);

    const auto& backtrackerColl = evT.GetBacktrackerRecordCollection(); 

    const auto& hits = evT.GetConstHits();
    const auto& genRecord = genRecords->GetRecordsVector().at(0);
    const auto& genStatus = genRecord.GetGenStatus();

    coords[0] = genStatus.at(0)*0.01; // Convert to mm
    coords[1] = genStatus.at(1)*0.01; 
    coords[2] = genStatus.at(2)*0.01; 

    TotalVis = 0.0;
    SiPMvis.assign(64, 0.0); // Reset visibility for each SiPM
    
    for (const auto& hit : hits) {
      TotalVis += hit.second;
      const auto& backtrackers = backtrackerColl.at(hit.first);
      const auto& bktrkSiPM = backtrackers.GetConstRecords().at(0); 
      for (const auto& record : bktrkSiPM.GetConstCounter()) {
        SiPMvis[record.first] += record.second;
      }
    }

    if (TotalVis >= 0.0) TotalVis /= num_photons; // Normalize by number of photons
    for (size_t i = 0; i < SiPMvis.size(); ++i) {
      SiPMvis[i] /= num_photons; // Normalize by number of photons
    }

    output->cd();
    tree->Fill();
    lightmap->Fill(coords[0], coords[1], coords[2], TotalVis);
  }


  // Write the output
  tree->Write();
  output->Close();

  new TCanvas();
  lightmap->SetTitle("Light Map;X (mm);Y (mm);Z (mm);Visibility");
  lightmap->Draw("box2");

  return 0;
}

