/**
 * @file    SLArEveOpHitSelectorPanel.hh
 * @brief   GUI panel for interactive optical-hit selection in the event display.
 *
 * The panel provides:
 *   • Per-process toggle buttons (PrimaryGen, Scintillation, Cherenkov, WLS).
 *     Only shown when the kOpticalProc backtracker record is available.
 *   • A wavelength range (min / max) in nm, entered via TGNumberEntry widgets.
 *     Only shown when the kWavelength backtracker record is available.
 *   • An "Apply" button that rebuilds the selectors in SLArEveOpHitRenderer
 *     and fires the fOnApply callback so SLArEveDisplay can re-render.
 *
 * Ownership model
 * ────────────────
 *   All TGFrame / TGButton / TGNumberEntry objects are created with new and
 *   adopted by ROOT's TGMainFrame cleanup mechanism (kDeepCleanup).
 *   SLArEveOpHitSelectorPanel holds only non-owning raw pointers to them.
 *   SLArEveOpHitRenderer is referenced as a non-owning pointer; its lifetime
 *   must exceed that of this panel.
 *
 * Usage
 * ──────
 *   panel.Build(parent_frame, renderer,
 *               proc_record_idx_sipm,  proc_record_idx_sc,
 *               wvl_record_idx_sipm,   wvl_record_idx_sc);
 *   panel.SetOnApply([&display](){ display.ProcessEvent(); });
 */

#pragma once

#include <array>
#include <functional>

#include "TGFrame.h"
#include "TGButton.h"
#include "TGLabel.h"
#include "TGNumberEntry.h"

#include "event/SLArEventPhotonHit.hh"   // EPhProcess, EPhProcName
#include "SLArEveOpHitRenderer.hh"

namespace display {

// ────────────────────────────────────────────────────────────────────────────
// SLArEveOpHitSelectorPanel
// ────────────────────────────────────────────────────────────────────────────

class SLArEveOpHitSelectorPanel {
public:
  enum class ESelectorMode { kAll = 0, kProcess = 1, kWavelength = 2 };
    SLArEveOpHitSelectorPanel() = default;
    ~SLArEveOpHitSelectorPanel() = default;

    // Non-copyable
    SLArEveOpHitSelectorPanel(const SLArEveOpHitSelectorPanel&)            = delete;
    SLArEveOpHitSelectorPanel& operator=(const SLArEveOpHitSelectorPanel&) = delete;

    // ── Setup ────────────────────────────────────────────────────────────────

    /**
     * Build all GUI widgets as children of @p parent and wire them up.
     *
     * @param parent                Frame to embed the panel in.
     * @param opdet_renderer        The optical-hit renderer to update on Apply.
     * @param proc_rec_sipm         Record index for kOpticalProc, vuv_sipm  (-1 = absent).
     * @param proc_rec_sc           Record index for kOpticalProc, supercell (-1 = absent).
     * @param wvl_rec_sipm          Record index for kWavelength,  vuv_sipm  (-1 = absent).
     * @param wvl_rec_sc            Record index for kWavelength,  supercell (-1 = absent).
     */
    void Build(TGCompositeFrame&       parent,
               SLArEveOpHitRenderer&  renderer,
               int proc_rec_sipm,
               int proc_rec_sc,
               int wvl_rec_sipm  = -1,
               int wvl_rec_sc    = -1);

    /**
     * Register a callback invoked at the end of Apply() — typically
     * SLArEveDisplay::ProcessEvent() to trigger a full re-render.
     */
    void SetOnApply(std::function<void()> cb) { fOnApply = std::move(cb); }

    // ── Slots (connected via ROOT CINT signals) ───────────────────────────────

    /** Rebuild selectors from current widget state and push to renderer. */
    void Apply();

    /** Update the active selector mode based on which radio button is down. */
    void OnModeChanged();

    /** Reset all widgets to "accept all" and call Apply(). */
    void Reset();

private:
    // ── Helpers ──────────────────────────────────────────────────────────────

    void BuildProcessGroup(TGCompositeFrame& parent);
    void BuildWavelengthGroup(TGCompositeFrame& parent);
    void BuildApplyRow(TGCompositeFrame& parent);

    // ── Non-owning references ─────────────────────────────────────────────────

    SLArEveOpHitRenderer* fRenderer = {};
    
    // ── Selector ──────────────────────────────────────────────────────────────
    ESelectorMode fActiveSelector = ESelectorMode::kAll;
    bool fHasProcessSelector    = false;
    bool fHasWavelengthSelector = false;

    // Resolved backtracker record indices (set during Build, -1 = absent)
    int fProcRecSiPM = -1;
    int fProcRecOpDet = -1;
    int fWvlRecSiPM = -1;
    int fWvlRecOpDet = -1;

    // ── Mode radio buttons (non-owning) ──────────────────────────────────────
    TGRadioButton* fRadioAll        = nullptr;
    TGRadioButton* fRadioOpProcess  = nullptr;
    TGRadioButton* fRadioWavelength = nullptr;

    // Sub-panel frames (shown/hidden based on active mode)
    TGCompositeFrame* fPanelParent = nullptr;
    TGCompositeFrame* fOpProcessSubFrame    = nullptr;
    TGCompositeFrame* fWavelengthSubFrame = nullptr;

    // ── Process toggle buttons ───────────────────────────────────────────────
    // Indices match EPhProcess: kPrimaryGen=4, kScnt=2, kCher=1, kWLS=3
    // We expose 4 controls in display order: PrimaryGen, Scint, Cher, WLS
    static constexpr int kNProc = 4;
    static constexpr std::array<EPhProcess, kNProc> kDisplayProcs = {
        kPrimaryGen, kScnt, kCher, kWLS
    };
    // ROOT check-buttons, one per process (non-owning, ROOT owns via parent)
    std::array<TGCheckButton*, kNProc> fProcButtons = {};

    // ── Wavelength range entries ─────────────────────────────────────────────
    static constexpr float kWvlMin =  100.f;  // nm
    static constexpr float kWvlMax =  800.f;  // nm
    TGNumberEntry* fWvlMinEntry = nullptr;
    TGNumberEntry* fWvlMaxEntry = nullptr;


    // ── Callback ─────────────────────────────────────────────────────────────
    std::function<void()> fOnApply;

public:
    ClassDef(SLArEveOpHitSelectorPanel, 0)
};

} // namespace display
