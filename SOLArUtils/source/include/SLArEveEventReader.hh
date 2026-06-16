/**
 * @file    SLArEveEventReader.hh
 * @brief   Manages ROOT file / tree I/O for the event display.
 *
 * Responsibilities
 * ─────────────────
 *  • Open and validate the hit reconstruction file (external reco TTree with
 *    hit_x/y/z/q/tpc branches) and the MC simulation file (SLArMCTruth,
 *    SLArListEventAnode, SLArListEventPDS branches + geometry config objects).
 *  • Expose per-event data via const accessors after a GetEntry() call.
 *  • Own the geometry config objects read from the MC file
 *    (SLArCfgAnode, SLArCfgBaseSystem<SLArCfgSuperCellArray>).
 *
 * Ownership model
 * ────────────────
 *  • TFile objects are owned as raw pointers only because ROOT's TTree
 *    lifetime is tied to its parent TFile; the destructor closes and deletes
 *    them explicitly.
 *  • Branch payload objects (fEvMCTruth etc.) are owned here; TTree only
 *    holds non-owning branch addresses.
 *  • Geometry config objects (SLArCfgAnode, SLArCfgBaseSystem<…>) are
 *    heap-allocated by ROOT's key-reading machinery and adopted into
 *    std::unique_ptr here.
 */

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TObjString.h"

#include "SLArRecoHits.hh"
#include "event/SLArMCTruth.hh"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgBaseSystem.hh"
#include "analysis/SLArBacktracker.hh"

namespace display {

// Convenience alias used throughout the display sub-system.
using CfgPDS_t = SLArCfgBaseSystem<SLArCfgSuperCellArray>;

/**
 * @class SLArEveEventReader
 * @brief Single point of contact for ROOT file I/O in the event display.
 */
class SLArEveEventReader {
public:
    SLArEveEventReader()  = default;
    ~SLArEveEventReader();

    // Non-copyable; the class owns raw ROOT file pointers.
    SLArEveEventReader(const SLArEveEventReader&)            = delete;
    SLArEveEventReader& operator=(const SLArEveEventReader&) = delete;
    SLArEveEventReader(SLArEveEventReader&&)                 = default;
    SLArEveEventReader& operator=(SLArEveEventReader&&)      = default;

    // ── File loading ─────────────────────────────────────────────────────────

    /**
     * Load the external reconstruction hit file.
     * @return 0 on success, non-zero on failure.
     */
    int LoadHitFile(const TString& file_path, const TString& tree_key);

    /**
     * Load the SOLAr-sim MC output file.
     * Also reads geometry config objects and registers active backtrackers
     * from the G4 macro stored in the file (if present).
     * @return 0 on success, non-zero on failure.
     */
    int LoadMCEventFile(const TString& file_path, const TString& tree_key);

    // ── Event navigation ─────────────────────────────────────────────────────

    /** Populate all branch payload objects for entry @p ev. */
    void GetEntry(Long64_t ev);

    Long64_t GetLastEvent() const { return fLastEvent; }

    // ── Branch payload accessors (valid after GetEntry) ───────────────────────

    /** Returns nullptr when the MC file was not loaded or branch is absent. */
    const SLArMCTruth*        GetMCTruth()    const { return fEvMCTruth;    }
    const SLArMCTruth*        GetMCTruth()    { return fEvMCTruth;    }
    const SLArListEventAnode* GetAnodeList()  const { return fEvAnodeList;  }
    const SLArListEventPDS*   GetPDSList()    const { return fEvPDSList;    }

    /** Reco hit variables; valid only when the hit file was loaded. */
    const reco::hitvarContainerPtr& GetHitVars() const { return fHitVars; }

    // ── Geometry configs (read once at file load) ─────────────────────────────

    /** Map from TPC copy-ID → SLArCfgAnode; populated from the MC file. */
    const std::map<int, std::unique_ptr<SLArCfgAnode>>& GetCfgAnodes() const
    { return fCfgAnodes; }

    /** PDS geometry config; may be null if not found in file. */
    const CfgPDS_t* GetCfgPDS() const { return fCfgPDS.get(); }

    // ── Feature flags ────────────────────────────────────────────────────────

    bool HasMCTruth()  const { return fIncludeMCTruth; }
    bool HasTPCHits()  const { return fIncludeTPCHits; }
    bool HasOpHits()   const { return fIncludeOpHits;  }
    bool HasHitFile()  const { return fHitFile != nullptr; }

    /** Set of backtrackers whose records are present in the file. */
    const std::set<backtracker::EBacktracker>& GetActiveBacktrackers() const
    { return fActiveBacktrackers; }

private:
    // ── Helpers ───────────────────────────────────────────────────────────────

    /** Parse the G4 macro string and populate fActiveBacktrackers. */
    void ParseBacktrackers(const TObjString* g4_macro);

    // ── ROOT I/O state ────────────────────────────────────────────────────────

    TFile*  fHitFile      = nullptr;
    TTree*  fHitTree      = nullptr;
    TFile*  fMCEventFile  = nullptr;
    TTree*  fMCEventTree  = nullptr;

    Long64_t fLastEvent   = 0;

    // ── Branch payload objects (non-null only when branch is connected) ───────

    reco::hitvarContainerPtr fHitVars    = {};
    SLArMCTruth*             fEvMCTruth  = nullptr;
    SLArListEventAnode*      fEvAnodeList = nullptr;
    SLArListEventPDS*        fEvPDSList   = nullptr;

    // ── Geometry configs loaded from the MC file ──────────────────────────────

    std::map<int, std::unique_ptr<SLArCfgAnode>> fCfgAnodes;
    std::unique_ptr<CfgPDS_t>                    fCfgPDS;

    // ── Feature flags ─────────────────────────────────────────────────────────

    bool fIncludeMCTruth  = true;
    bool fIncludeTPCHits  = true;
    bool fIncludeOpHits   = true;

    std::set<backtracker::EBacktracker> fActiveBacktrackers;
};

} // namespace display
