/**
 * @file    SLArEveOpHitRenderer.hh
 * @brief   Renders optical hits from both X-ARAPUCA PDS walls and anode-side
 *          SiPM arrays into TEveBoxSets and fills associated time histograms.
 *
 * Two TEveBoxSets are maintained per detector group (PDS wall or TPC anode):
 *   • fPhotonDetectorsNHits — box colour encodes total hit count.
 *   • fPhotonDetectorsTHits — box colour encodes the earliest hit time bin.
 *
 * Time histograms (first-hit time, all-hit time) are stored as TH1F vectors
 * keyed by the same group index.  SLArEveDisplay can retrieve them for display
 * in an embedded canvas.
 *
 * Ownership model
 * ────────────────
 *  • TEveBoxSet objects are owned here via unique_ptr and parented into the
 *    provided Eve element during Configure().  TEveManager manages the scene
 *    graph from that point, but our unique_ptrs remain valid; they are
 *    explicitly reset on destruction.
 *  • TH1F objects live in std::vector<TH1F> (value semantics); histograms are
 *    Reset() between events, not re-created.
 *  • Geometry config objects (SLArCfgAnode, CfgPDS_t) are NOT owned here;
 *    the reader keeps them alive for the display session.
 */

#pragma once

#include <limits>
#include <map>
#include <memory>
#include <vector>

#include "TH1F.h"
#include "TEveBoxSet.h"
#include "TEveRGBAPalette.h"

#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgBaseSystem.hh"

#include "SLArEveEventReader.hh"
#include "SLArEveGeometry.hh"
#include "SLArEveOpHitSelector.hh"

namespace display {

  using CfgPDS_t = SLArCfgBaseSystem<SLArCfgSuperCellArray>;

  // ─────────────────────────────────────────────────────────────────────────────
  // Result type returned from per-group rendering helpers
  // ─────────────────────────────────────────────────────────────────────────────

  struct OpHitLimits {
    int nhit_max = 0;
    int time_min = std::numeric_limits<int>::max();
    int time_max = 0;
  };

  // ─────────────────────────────────────────────────────────────────────────────
  // SLArEveOpHitRenderer
  // ─────────────────────────────────────────────────────────────────────────────

  /**
   * @class SLArEveOpHitRenderer
   * @brief Handles all photon-detection-system rendering.
   *
   * Call order:
   *   1. Configure(anode_cfgs, pds_cfg, parent) — once, after reader is loaded.
   *   2. RenderOpHits(ev_anode_list, ev_pds_list) — once per event.
   *   3. Reset() — call before RenderOpHits() on each new event; or let
   *      RenderOpHits() call it automatically (it does).
   */
  class SLArEveOpHitRenderer {
    public:
      //! Enum for indexing the time histogram vector;
      enum class EOpHitHistType { 
        kFirstHitTime = 0,
        kAllHitTime   = 1,
        kScintHitTime = 2,
        kCherHitTime  = 3, 
        kWLSHitTime   = 4, 
        kWavelength   = 5
      };


      explicit SLArEveOpHitRenderer(const SLArEveGeometry& geometry);
      ~SLArEveOpHitRenderer() = default;

      SLArEveOpHitRenderer(const SLArEveOpHitRenderer&)            = delete;
      SLArEveOpHitRenderer& operator=(const SLArEveOpHitRenderer&) = delete;

      // ── Setup ────────────────────────────────────────────────────────────────

      /**
       * Create TEveBoxSets for every detector group discovered in @p anode_cfgs
       * and @p pds_cfg, and attach them to @p parent.  Also initialises the
       * time histograms.
       *
       * @param anode_cfgs  Map TPC-ID → SLArCfgAnode (non-owning refs).
       * @param pds_cfg     PDS system config (may be nullptr if unavailable).
       * @param parent      Eve element to attach box sets to.
       */
      void Configure(
          const std::map<int, std::unique_ptr<SLArCfgAnode>>& anode_cfgs,
          const CfgPDS_t* pds_cfg,
          const BacktrackerDict_t* backtracker_dict,
          TEveElement& parent);

      inline void SetSiPMWvlngthBacktrackerIndex(int idx) { fSiPMWvlngthBktrkIdx = idx; }
      inline void SetOpDetWvlngthBacktrackerIndex(int idx) { fOpDetWvlngthBktrkIdx = idx; }
      inline void SetSiPMOpProcessBacktrackerIndex(int idx) { fSiPMOpProcBktrkIdx = idx; }
      inline void SetOpDetOpProcessBacktrackerIndex(int idx) { fOpDetOpProcBktrkIdx = idx; }

      // ── Per-event ────────────────────────────────────────────────────────────

      /**
       * Clear old hit boxes, repopulate from the current event, and update
       * colour palettes.
       *
       * @param ev_anode_list  Charge-anode event data (may be nullptr).
       * @param ev_pds_list    PDS event data (may be nullptr).
       */
      void RenderOpHits(const SLArListEventAnode* ev_anode_list,
          const SLArListEventPDS*   ev_pds_list);

      /** Clear all TEveBoxSets and reset all histograms. */
      void Reset();

      // -- Selectors 
      void SetSiPMSelector(SiPMSelectorFn sel, EOpHitSelectorMode kMode = EOpHitSelectorMode::kAll) 
      { 
        fSiPMSelector = std::move(sel);
        fSiPMSelectorMode = kMode;
      }
      void SetOpDetSelector(OpDetSelectorFn sel, EOpHitSelectorMode kMode = EOpHitSelectorMode::kAll)  
      { 
        fOpDetSelector = std::move(sel); 
        fOpDetSelectorMode = kMode;
      }
      //! Restore both selectors to SelectAll
      inline void ResetSelectors() {
        fSiPMSelector = MakeSiPMSelectAll(); 
        fSiPMSelectorMode = EOpHitSelectorMode::kAll;
        fOpDetSelector = MakeOpDetSelectAll();
        fOpDetSelectorMode = EOpHitSelectorMode::kAll;
      }

      // ── Accessors ────────────────────────────────────────────────────────────

      /** Time histograms keyed by detector-group index; for canvas drawing. */
      const std::map<int, std::vector<TH1F>>& GetTimeHistograms() const
      { return fOpHitsHistograms; }

      std::map<int, std::vector<TH1F>>& GetTimeHistograms()
      { return fOpHitsHistograms; }

      /** Map from group index → nhit TEveBoxSet (for palette toggle in GUI). */
      std::map<int, std::unique_ptr<TEveBoxSet>>& GetNHitSets()
      { return fDetectorNHits; }

      /** Map from group index → time TEveBoxSet. */
      std::map<int, std::unique_ptr<TEveBoxSet>>& GetTimeSets()
      { return fDetectorTHits; }

    private:
      // ── Per-group rendering helpers ───────────────────────────────────────────

      OpHitLimits RenderFromOpDetArray(
          int idx_array,
          const SLArEventSuperCellArray& ev_array);

      OpHitLimits RenderFromAnode(
          int tpc_id,
          const SLArEventAnode&  ev_anode);

      //! Returns true when the SiPM selector applies a real filter (not accept-all).
      bool SiPMSelectorIsFiltering()  const
      { return fSiPMSelectorMode  != EOpHitSelectorMode::kAll; }


      //! Returns true when the OpDet selector applies a real filter (not accept-all).
      bool OpDetSelectorIsFiltering() const
      { return fOpDetSelectorMode != EOpHitSelectorMode::kAll; }

      // ── Histogram initialisation ──────────────────────────────────────────────

      void SetupTimeHistograms();

      // ── Data members ─────────────────────────────────────────────────────────

      const SLArEveGeometry& fGeometry;   ///< non-owning ref

      // Non-owning config pointers; lifetimes guaranteed by SLArEveEventReader.
      const std::map<int, std::unique_ptr<SLArCfgAnode>>* fCfgAnodes = nullptr;
      const CfgPDS_t*                                     fCfgPDS   = nullptr;
      const BacktrackerDict_t*                            fBacktrackerDict;  

      std::map<int, std::unique_ptr<TEveBoxSet>> fDetectorNHits;
      std::map<int, std::unique_ptr<TEveBoxSet>> fDetectorTHits;

      //! Key: group index (TPC ID for anodes, wall ID for PDS); value: vector of time histograms for that group.
      std::map<int, std::vector<TH1F>> fOpHitsHistograms;

      std::unique_ptr<TEveRGBAPalette> fPaletteNHits;
      std::unique_ptr<TEveRGBAPalette> fPaletteTHits;

      int fSiPMWvlngthBktrkIdx = -1;  ///< backtracker record index for SiPM wavelength (if present)
      int fOpDetWvlngthBktrkIdx = -1; ///< backtracker record index for OpDet wavelength (if present)
      int fSiPMOpProcBktrkIdx = -1; ///< backtracker record index for SiPM optical process (if present)
      int fOpDetOpProcBktrkIdx = -1; ///< backtracker record index for OpDet optical process (if present)

      SiPMSelectorFn  fSiPMSelector = MakeSiPMSelectAll();
      OpDetSelectorFn fOpDetSelector = MakeOpDetSelectAll();
      EOpHitSelectorMode fSiPMSelectorMode = EOpHitSelectorMode::kAll;
      EOpHitSelectorMode fOpDetSelectorMode = EOpHitSelectorMode::kAll;
  };

} // namespace display
