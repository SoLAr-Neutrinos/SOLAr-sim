/**
 * @author  Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file    SLArEveDisplay.hh
 * @brief   Top-level orchestrator and GUI shell for the SOLAr-sim event display.
 *
 * Responsibilities kept here
 * ──────────────────────────
 *  • Own the five sub-components by value and wire them together.
 *  • Build and manage the TGMainFrame / TEve GUI (navigation buttons, particle
 *    selector panel, embedded canvases).
 *  • Drive the per-event render loop: GetEntry → reset → render → redraw.
 *  • Handle CloseWindow / application termination.
 *
 * What is NOT here
 * ─────────────────
 *  • Geometry construction        → SLArEveGeometry
 *  • File / tree I/O              → SLArEveEventReader
 *  • MC-truth track rendering     → SLArEveTrackRenderer
 *  • Charge-pixel hit rendering   → SLArEveHitRenderer
 *  • Optical hit rendering        → SLArEveOpHitRenderer
 */

#ifndef SLAR_EVE_DISPLAY_HH
#define SLAR_EVE_DISPLAY_HH

#include <memory>
#include <set>

#include "TApplication.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGNumberEntry.h"
#include "TGTab.h"
#include "TRootEmbeddedCanvas.h"
#include "TTimer.h"
#include "TEveManager.h"
#include "rapidjson/document.h"

#include "SLArEveGeometry.hh"
#include "SLArEveEventReader.hh"
#include "SLArEveTrackRenderer.hh"
#include "SLArEveHitRenderer.hh"
#include "SLArEveOpHitRenderer.hh"
#include "SLArEveOpHitSelectorPanel.hh"

namespace display {

  // ── Simple unique-widget-ID generator ────────────────────────────────────────

  class IDList {
    public:
      IDList() : fNextID(0) {}
      Int_t GetUnID() { return ++fNextID; }
    private:
      Int_t fNextID;
  };

  // ─────────────────────────────────────────────────────────────────────────────
  // SLArEveDisplay
  // ─────────────────────────────────────────────────────────────────────────────

  class SLArEveDisplay : public TGMainFrame {
    public:
      SLArEveDisplay();
      ~SLArEveDisplay() override = default;

      // ── Configuration (call before MakeGUI) ──────────────────────────────────

      /**
       * Parse geometry JSON and build the Eve scene graph.
       * Must be called before LoadMCEventFile / MakeGUI.
       */
      void Configure(const rapidjson::Value& config);

      // ── File loading ─────────────────────────────────────────────────────────

      /**
       * Load the external reconstruction hit file.
       * Optional; charge-pixel boxes are omitted if this is never called.
       */
      int LoadHitFile(const TString& file_path, const TString& tree_key);

      /**
       * Load the SOLAr-sim MC output file.
       * Also reads geometry configs and wires the optical-hit renderer.
       * Must be called after Configure().
       */
      int LoadMCEventFile(const TString& file_path, const TString& tree_key);

      // ── GUI ──────────────────────────────────────────────────────────────────

      /** Build and map the full GUI (navigation + particle-selector panel +
       *  embedded histogram canvases).  Call after LoadMCEventFile(). */
      int MakeGUI();

      // ── Event navigation (connected to GUI buttons via ROOT signals) ──────────

      void NextEvent();
      void PrevEvent();
      void ProcessEvent();

      void SetEntry()
      {
        fCurEvent = fEnterEntry->GetNumberEntry()->GetIntNumber();
        ProcessEvent();
      }
      void SetEntry(Long64_t iev)
      {
        fCurEvent = iev;
        ProcessEvent();
      }

      /** Toggle the N-hits / first-hit-time display mode for optical hits. */
      void ToggleModeNHitsTime();

      // ── TGMainFrame override ──────────────────────────────────────────────────

      void CloseWindow() override
      {
        gApplication->Terminate(0);
      }

    private:
      // ── Internal helpers ─────────────────────────────────────────────────────

      /** Wire optical-hit renderer Eve elements into the LAr target volume and
       *  initialise hit-set boxes after the MC file has been opened. */
      void SetupOpHitRenderer();

      /** Wire charge-hit renderer Eve elements into TPC volumes. */
      void SetupHitRenderer();

      /** Re-draw the 3D scene. */
      void ReDraw();

      /** Push current event number into the navigation entry widget. */
      void UpdateEntryLabel() { fEnterEntry->SetIntNumber(fCurEvent); }

      /** Repaint the time-histogram canvas with current event data. */
      void UpdateTimeHistCanvas();

      /** Repaint the wavelength-histogram canvas with current event data. */
      void UpdateWavelengthCanvas();

      // ── Sub-components (owned by value; initialised in constructor) ───────────

      SLArEveGeometry      fGeometry;
      SLArEveEventReader   fReader;
      SLArEveTrackRenderer fTrackRenderer;
      SLArEveHitRenderer   fHitRenderer;
      SLArEveOpHitRenderer fOpHitRenderer;

      // ── Eve infrastructure ────────────────────────────────────────────────────

      std::unique_ptr<TEveManager> fEveManager;
      std::unique_ptr<TTimer>      fTimer;

      // ── Event state ───────────────────────────────────────────────────────────

      Long64_t fCurEvent  = 0;

      // ── GUI widgets (raw non-owning pointers; ROOT owns them via TGMainFrame) ─

      IDList             fIDs;
      TGNumberEntry*     fEnterEntry                    = nullptr;
      TGGroupFrame*      fGgroupframeParticleSelection  = nullptr;
      TGVerticalFrame*   fGframeParticleSelection       = nullptr;
      // Up to 10 particle species; indices mirror fParticleSelector iteration order.
      static constexpr int kMaxSpecies = 10;
      TGHorizontalFrame* fGframeParticleSetting[kMaxSpecies]   = {};
      TGCheckButton*     fGParticleSelectionButton[kMaxSpecies] = {};
      TGNumberEntry*     fGParticleEnergyThreshold[kMaxSpecies] = {};
      TGTextButton*      fNhitsTimeToggleButton = nullptr;

      SLArEveOpHitSelectorPanel fOpHitSelectorPanel;
      int fProcRecSiPM = -1;
      int fProcRecOpDet = -1;
      int fWvlRecSiPM = -1;
      int fWvlRecOpDet = -1;

      TRootEmbeddedCanvas* fTimeHistCanvas   = nullptr;
      TRootEmbeddedCanvas* fWavelenHistCanvas = nullptr;

    public:
      ClassDefOverride(display::SLArEveDisplay, 1)
  };

} // namespace display

#endif // SLAR_EVE_DISPLAY_HH
