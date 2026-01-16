/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : test_photonbomb_backtracker.cc
 * @created     : Saturday Aug 23, 2025 17:42:51 CEST
 */

#include <iostream>
#include "TPRegexp.h"
#include "TSystemDirectory.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "event/SLArGenRecords.hh"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"

int test_photonbomb_backtracter(
    const TString& input_pattern, 
    const TString input_folder, 
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
  TTreeReaderValue<SLArListEventPDS> evPDSList(reader, "EventPDS");
  TTreeReaderValue<SLArGenRecordsVector> genRecords(reader, "GenTree.GenRecords");

  const double num_photons = 1e5; 
  double coords[3] = {0.0, 0.0, 0.0};

  TH1D* hHitsPerSiPM[6] = {0}; // [0]=Scintillation, [1]=Cerenkov, [2]=WLS absorption, [3]=WLS re-emission, [4]=PrimaryGen, [5]=Other
  for (int i=0; i<6; i++) {
    EPhProcess kProc = static_cast<EPhProcess>(i);
    TString hname = Form("hHitsPerSiPM_proc%s", EPhProcName[kProc].Data() );
    TString htitl = Form("%s hits per SiPM;%s hits ID;Counts", EPhProcTitle[kProc].Data(), EPhProcTitle[kProc].Data());
    hHitsPerSiPM[i] = new TH1D(hname, htitl, 10, -0.5, 9.5);
  }

  while (reader.Next()) {
    // 1. Access generator information
    const auto& genRecord = genRecords->GetRecordsVector().at(0);
    const auto& genStatus = genRecord.GetGenStatus();
    coords[0] = genStatus.at(0)*0.01; // Convert to mm
    coords[1] = genStatus.at(1)*0.01; 
    coords[2] = genStatus.at(2)*0.01; 


    // 2. Process anode events (skip the top TPC)
    for (const auto& evAnode_itr : evAnodeList->GetConstAnodeMap()) {
      if (evAnode_itr.first == 10) continue; // Skip top TPC

      printf("---------------------------------------------------------------------------\n");
      printf("Processing Anode %i\n", evAnode_itr.first);

      for (const auto& evMT_itr : evAnode_itr.second.GetConstMegaTilesMap()) {
        printf("  MegaTile %i\n", evMT_itr.first);

        for (const auto& evT_itr : evMT_itr.second.GetConstTileMap()) {
          printf("    Tile %i\n", evT_itr.first);
        
          for (const auto& evSiPM_itr : evT_itr.second.GetConstSiPMEvents()) {
            printf("      SiPM %i: %i hits\n", evSiPM_itr.first, evSiPM_itr.second.GetNhits());
          
            const auto& evSiPM = evSiPM_itr.second;
            const auto& backtrackerColl = evSiPM.GetBacktrackerRecordCollection(); 
            int nHitsPerProc[6] = {0, 0, 0, 0, 0, 0};
            for (const auto& hit : evSiPM.GetConstHits()) {
              const auto& backtrackers = backtrackerColl.at(hit.first);
              const auto& bktrkProc = backtrackers.GetConstRecords().at(0); 
              const auto& bktrkVol = backtrackers.GetConstRecords().at(1);
              for (const auto& proc : bktrkProc.GetConstCounter()) {
                nHitsPerProc[proc.first] += proc.second;
              }
            }
            nHitsPerProc[0] = evSiPM.GetNhits();
            for (int i=0; i<6; i++) {
              if (nHitsPerProc[i] > 0) hHitsPerSiPM[i]->Fill(nHitsPerProc[i]);
            }
          }
        }
      }
    }
  }

  TCanvas* cHits = new TCanvas("cHits", "Hits per SiPM", 1200, 800);
  cHits->Divide(3,2);
  for (int i=0; i<6; i++) {
    cHits->cd(i+1);
    hHitsPerSiPM[i]->Draw();
  }

  return 0;
}


