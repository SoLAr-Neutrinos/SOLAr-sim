/**
 * @file    SLArEveEventReader.cc
 * @brief   Implementation of SLArEveEventReader.
 */

#include "SLArEveEventReader.hh"

#include "TKey.h"
#include "TClass.h"

#include <sstream>

namespace display {

  // ─────────────────────────────────────────────────────────────────────────────
  // Destructor
  // ─────────────────────────────────────────────────────────────────────────────

  SLArEveEventReader::~SLArEveEventReader()
  {
    // Branch payload objects are NOT owned by TTree in this pattern;
    // we clear them before closing files so ROOT doesn't double-free.
    fEvMCTruth   = nullptr;
    fEvAnodeList = nullptr;
    fEvPDSList   = nullptr;

    if (fHitFile) {
      fHitFile->Close();
      delete fHitFile;
      fHitFile = nullptr;
    }
    if (fMCEventFile) {
      fMCEventFile->Close();
      delete fMCEventFile;
      fMCEventFile = nullptr;
    }
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // File loading
  // ─────────────────────────────────────────────────────────────────────────────

  int SLArEveEventReader::LoadHitFile(const TString& file_path,
      const TString& tree_key)
  {
    if (fHitFile) {
      fHitFile->Close();
      delete fHitFile;
      fHitFile  = nullptr;
      fHitTree  = nullptr;
    }

    fHitFile = TFile::Open(file_path);
    if (!fHitFile || fHitFile->IsZombie()) {
      printf("SLArEveEventReader: cannot open hit file %s\n",
          file_path.Data());
      return 1;
    }

    fHitTree = fHitFile->Get<TTree>(tree_key);
    if (!fHitTree) {
      printf("SLArEveEventReader: tree '%s' not found in %s\n",
          tree_key.Data(), file_path.Data());
      return 2;
    }

    fLastEvent = fHitTree->GetEntries() - 1;

    fHitTree->SetBranchAddress("hit_tpc",   &fHitVars.hit_tpc);
    fHitTree->SetBranchAddress("hit_x",     &fHitVars.hit_x);
    fHitTree->SetBranchAddress("hit_y",     &fHitVars.hit_y);
    fHitTree->SetBranchAddress("hit_z",     &fHitVars.hit_z);
    fHitTree->SetBranchAddress("hit_q",     &fHitVars.hit_q);
    fHitTree->SetBranchAddress("hit_qtrue", &fHitVars.hit_qtrue);

    return 0;
  }

  int SLArEveEventReader::LoadMCEventFile(const TString& file_path,
      const TString& tree_key)
  {
    if (fMCEventFile) {
      fMCEventFile->Close();
      delete fMCEventFile;
      fMCEventFile = nullptr;
      fMCEventTree = nullptr;
    }

    fMCEventFile = TFile::Open(file_path);
    if (!fMCEventFile || fMCEventFile->IsZombie()) {
      printf("SLArEveEventReader: cannot open MC file %s\n",
          file_path.Data());
      return 1;
    }

    fMCEventTree = fMCEventFile->Get<TTree>(tree_key);
    if (!fMCEventTree) {
      printf("SLArEveEventReader: tree '%s' not found in %s\n",
          tree_key.Data(), file_path.Data());
      return 2;
    }

    // Update last-event count; the hit tree may have set it already —
    // use the MC tree count as the authoritative reference.
    fLastEvent = fMCEventTree->GetEntries() - 1;

    // ── Branch connections ────────────────────────────────────────────────────

    if (fMCEventTree->GetBranch("MCTruth") == nullptr) {
      printf("WARNING: branch MCTruth not found in %s\n", file_path.Data());
      fIncludeMCTruth = false;
    } else {
      fMCEventTree->SetBranchAddress("MCTruth", &fEvMCTruth);
    }

    if (fMCEventTree->GetBranch("EventAnode") == nullptr) {
      printf("WARNING: branch EventAnode not found in %s\n", file_path.Data());
      fIncludeTPCHits = false;
    } else {
      fMCEventTree->SetBranchAddress("EventAnode", &fEvAnodeList);
    }

    if (fMCEventTree->GetBranch("EventPDS") == nullptr) {
      printf("WARNING: branch EventPDS not found in %s\n", file_path.Data());
      fIncludeOpHits = false;
    } else {
      fMCEventTree->SetBranchAddress("EventPDS", &fEvPDSList);
    }

    // ── Geometry configs and optional G4 macro ────────────────────────────────

    for (const auto& itr : *fMCEventFile->GetListOfKeys()) {
      TKey* key = static_cast<TKey*>(itr);

      if (strcmp(key->GetClassName(), "SLArCfgAnode") == 0) {
        SLArCfgAnode* cfg =
          dynamic_cast<SLArCfgAnode*>(key->ReadObj());
        if (!cfg) continue;
        const int tpc_id = cfg->GetTPCID();
        fCfgAnodes.insert({tpc_id,
            std::unique_ptr<SLArCfgAnode>(cfg)});

      } else if (strcmp(key->GetClassName(),
            "SLArCfgBaseSystem<SLArCfgSuperCellArray>") == 0) {
        CfgPDS_t* cfg =
          dynamic_cast<CfgPDS_t*>(key->ReadObj());
        if (!cfg) continue;
        fCfgPDS = std::unique_ptr<CfgPDS_t>(cfg);

      } else if (strcmp(key->GetClassName(), "TObjString") == 0) {
        // The G4 macro may be stored as a TObjString named "G4Macro"
        // (or similar); parse it to discover active backtrackers.
        TObjString* macro = dynamic_cast<TObjString*>(key->ReadObj());
        if (macro) ParseBacktrackers(macro);
      }
    }

    return 0;
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Event navigation
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveEventReader::GetEntry(const Long64_t ev)
  {
    // Reset owned payload objects before ROOT fills them from the tree.
    if (fEvMCTruth)   fEvMCTruth->Reset();
    if (fEvAnodeList) fEvAnodeList->Reset();
    if (fEvPDSList)   fEvPDSList->Reset();

    if (fMCEventTree) fMCEventTree->GetEntry(ev);
    if (fHitTree)     fHitTree->GetEntry(ev);
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Private helpers
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveEventReader::ParseBacktrackers(const TObjString* g4_macro)
  {
    if (!g4_macro) return;


    std::istringstream stream(g4_macro->GetString().Data());
    std::string line;
    while (std::getline(stream, line)) {
      if (line.find("registerBacktracker") == std::string::npos) continue;

      std::istringstream lss(line);
      std::string cmd, bt_token;
      lss >> cmd >> bt_token;

      // Token format: "<system>:<name>"
      const auto colon = bt_token.find(':');
      if (colon == std::string::npos) continue;
      const std::string bk_system_str  = bt_token.substr(0, colon);
      const std::string bk_name_str = bt_token.substr(colon + 1);

      const auto bk = string_to_backtracker( bk_name_str ); 
      const auto bk_sys = string_to_bktrk_system( bk_system_str ); 
      
      fBacktrackerDict[bk_sys].push_back( bk );
      printf("SLArEveEventReader: registered backtracker '%s' for system %s\n",
          bk_name_str.c_str(), bk_system_str.c_str());
    }
  }

} // namespace display
