/**
 * @file    SLArEveOpHitSelectorPanel.cc
 * @brief   Implementation of SLArEveOpHitSelectorPanel.
 */

#include "SLArEveOpHitSelectorPanel.hh"
#include "SLArEveOpHitSelector.hh"

#include "TGLayout.h"
#include "TGLabel.h"

ClassImp(display::SLArEveOpHitSelectorPanel)

namespace display {

// ── constexpr out-of-line definition (required pre-C++17 in some setups) ─────
constexpr std::array<EPhProcess, SLArEveOpHitSelectorPanel::kNProc>
    SLArEveOpHitSelectorPanel::kDisplayProcs;

// ────────────────────────────────────────────────────────────────────────────
// Build
// ────────────────────────────────────────────────────────────────────────────

void SLArEveOpHitSelectorPanel::Build(
    TGCompositeFrame&      parent,
    SLArEveOpHitRenderer&  renderer,
    int proc_rec_sipm,
    int proc_rec_sc,
    int wvl_rec_sipm,
    int wvl_rec_sc)
{
    fPanelParent = &parent;
    fRenderer    = &renderer;
    fProcRecSiPM = proc_rec_sipm;
    fProcRecOpDet = proc_rec_sc;
    fWvlRecSiPM  = wvl_rec_sipm;
    fWvlRecOpDet = wvl_rec_sc;

    fHasProcessSelector    = (proc_rec_sipm >= 0 || proc_rec_sc >= 0);
    fHasWavelengthSelector = (wvl_rec_sipm  >= 0 || wvl_rec_sc  >= 0);

    if (!fHasProcessSelector && !fHasWavelengthSelector) {
        // Nothing to show — backtracker data absent. Add a label.
        parent.AddFrame(
            new TGLabel(&parent, "No backtracker data available"),
            new TGLayoutHints(kLHintsCenterX | kLHintsTop, 4, 4, 6, 2));
        return;
    }

    // ── Mode selection row ────────────────────────────────────────────────
    // Radio buttons: "All" | "By process" | "By wavelength"
    // Only show options for which backtracker data is actually available.
    {
        auto* grpMode = new TGGroupFrame(&parent, "Selector mode");
        fRadioAll = new TGRadioButton(grpMode, "All hits");
        fRadioAll->SetState(kButtonDown);  // default
        fRadioAll->Connect("Clicked()",
            "display::SLArEveOpHitSelectorPanel", this, "OnModeChanged()");
        grpMode->AddFrame(fRadioAll,
            new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));

        if (fHasProcessSelector) {
            fRadioOpProcess = new TGRadioButton(grpMode, "By process");
            fRadioOpProcess->Connect("Clicked()",
                "display::SLArEveOpHitSelectorPanel", this, "OnModeChanged()");
            grpMode->AddFrame(fRadioOpProcess,
                new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
        }

        if (fHasWavelengthSelector) {
            fRadioWavelength = new TGRadioButton(grpMode, "By wavelength");
            fRadioWavelength->Connect("Clicked()",
                "display::SLArEveOpHitSelectorPanel", this, "OnModeChanged()");
            grpMode->AddFrame(fRadioWavelength,
                new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
        }

        grpMode->MapSubwindows();
        parent.AddFrame(grpMode,
            new TGLayoutHints(kLHintsExpandX | kLHintsTop, 4, 4, 4, 2));
    }

    // ── Process sub-panel (initially hidden) ──────────────────────────────
    if (fHasProcessSelector) {
        fOpProcessSubFrame = new TGVerticalFrame(&parent);
        BuildProcessGroup(*fOpProcessSubFrame);
        fOpProcessSubFrame->MapSubwindows();
        parent.AddFrame(fOpProcessSubFrame,
            new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 0, 0));
        fOpProcessSubFrame->UnmapWindow();   // hidden until "By process" selected
    }

    // ── Wavelength sub-panel (initially hidden) ───────────────────────────
    if (fHasWavelengthSelector) {
        fWavelengthSubFrame = new TGVerticalFrame(&parent);
        BuildWavelengthGroup(*fWavelengthSubFrame);
        fWavelengthSubFrame->MapSubwindows();
        parent.AddFrame(fWavelengthSubFrame,
            new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 0, 0));
        fWavelengthSubFrame->UnmapWindow(); // hidden until "By wavelength" selected
    }

    //if (fHasProcessSelector)    BuildProcessGroup(parent);
    //if (fHasWavelengthSelector) BuildWavelengthGroup(parent);
    BuildApplyRow(parent);
}

// ────────────────────────────────────────────────────────────────────────────
// BuildProcessGroup
// ────────────────────────────────────────────────────────────────────────────

void SLArEveOpHitSelectorPanel::BuildProcessGroup(TGCompositeFrame& parent)
{
    auto* grp = new TGGroupFrame(&parent, "Optical process");

    for (int i = 0; i < kNProc; ++i) {
        const EPhProcess proc = kDisplayProcs[i];
        // Use the human-readable title from the existing EPhProcTitle array
        fProcButtons[i] = new TGCheckButton(grp, EPhProcTitle[proc]);
        fProcButtons[i]->SetState(kButtonDown);  // all selected by default
        grp->AddFrame(fProcButtons[i],
            new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
    }

    grp->MapSubwindows();
    parent.AddFrame(grp,
        new TGLayoutHints(kLHintsExpandX | kLHintsTop, 4, 4, 4, 2));
}

// ────────────────────────────────────────────────────────────────────────────
// BuildWavelengthGroup
// ────────────────────────────────────────────────────────────────────────────

void SLArEveOpHitSelectorPanel::BuildWavelengthGroup(TGCompositeFrame& parent)
{
    auto* grp = new TGGroupFrame(&parent, "Wavelength range [nm]");

    // Min row
    {
        auto* row = new TGHorizontalFrame(grp);
        row->AddFrame(new TGLabel(row, "Min:"),
            new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 4, 2, 2));
        fWvlMinEntry = new TGNumberEntry(
            row, kWvlMin, 6, -1,
            TGNumberFormat::kNESReal,
            TGNumberFormat::kNEANonNegative,
            TGNumberFormat::kNELLimitMinMax,
            kWvlMin, kWvlMax);
        row->AddFrame(fWvlMinEntry,
            new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2, 2, 2, 2));
        row->MapSubwindows();
        grp->AddFrame(row, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
    }

    // Max row
    {
        auto* row = new TGHorizontalFrame(grp);
        row->AddFrame(new TGLabel(row, "Max:"),
            new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 4, 2, 2));
        fWvlMaxEntry = new TGNumberEntry(
            row, kWvlMax, 6, -1,
            TGNumberFormat::kNESReal,
            TGNumberFormat::kNEANonNegative,
            TGNumberFormat::kNELLimitMinMax,
            kWvlMin, kWvlMax);
        row->AddFrame(fWvlMaxEntry,
            new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2, 2, 2, 2));
        row->MapSubwindows();
        grp->AddFrame(row, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
    }

    grp->MapSubwindows();
    parent.AddFrame(grp,
        new TGLayoutHints(kLHintsExpandX | kLHintsTop, 4, 4, 2, 2));
}

// ────────────────────────────────────────────────────────────────────────────
// BuildApplyRow
// ────────────────────────────────────────────────────────────────────────────

void SLArEveOpHitSelectorPanel::BuildApplyRow(TGCompositeFrame& parent)
{
    auto* row = new TGHorizontalFrame(&parent);

    auto* btn_apply = new TGTextButton(row, "&Apply");
    btn_apply->Connect("Clicked()",
        "display::SLArEveOpHitSelectorPanel", this, "Apply()");
    row->AddFrame(btn_apply,
        new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 2, 2, 4, 4));

    auto* btn_reset = new TGTextButton(row, "&Reset");
    btn_reset->Connect("Clicked()",
        "display::SLArEveOpHitSelectorPanel", this, "Reset()");
    row->AddFrame(btn_reset,
        new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 2, 2, 4, 4));

    row->MapSubwindows();
    parent.AddFrame(row,
        new TGLayoutHints(kLHintsExpandX | kLHintsTop, 4, 4, 2, 4));
}

// ────────────────────────────────────────────────────────────────────────────
// Apply
// ────────────────────────────────────────────────────────────────────────────

void SLArEveOpHitSelectorPanel::Apply()
{
    if (!fRenderer) return;

    switch (fActiveSelector) {
      case ESelectorMode::kAll:
        fRenderer->SetSiPMSelector(MakeSiPMSelectAll(), EOpHitSelectorMode::kAll);
        fRenderer->SetOpDetSelector(MakeOpDetSelectAll(), EOpHitSelectorMode::kAll);
        printf("SLArEveOpHitSelectorPanel::Apply() — mode: All\n");
        break;

      case ESelectorMode::kProcess: {
        std::vector<EPhProcess> active_procs;
        for (int i = 0; i < kNProc; ++i)
            if (fProcButtons[i] && fProcButtons[i]->IsDown())
                active_procs.push_back(kDisplayProcs[i]);

        if (active_procs.empty() ||
            static_cast<int>(active_procs.size()) == kNProc) {
            // Nothing selected or everything selected → same as All
            fRenderer->SetSiPMSelector(MakeSiPMSelectAll(), EOpHitSelectorMode::kAll);
            fRenderer->SetOpDetSelector(MakeOpDetSelectAll(), EOpHitSelectorMode::kAll);
        } else {
            fRenderer->SetSiPMSelector(
                MakeSiPMSelectByProcessSet(active_procs, fProcRecSiPM), 
                EOpHitSelectorMode::kProcess
                );
            fRenderer->SetOpDetSelector(
                MakeOpDetSelectByProcessSet(active_procs, fProcRecOpDet),
                EOpHitSelectorMode::kProcess
                );
        }
        printf("SLArEveOpHitSelectorPanel::Apply() — mode: Process "
               "(%zu active)\n", active_procs.size());
        break;
      }
    
      case ESelectorMode::kWavelength: {
                float wvl_min = static_cast<float>(fWvlMinEntry->GetNumber());
        float wvl_max = static_cast<float>(fWvlMaxEntry->GetNumber());
        if (wvl_min > wvl_max) std::swap(wvl_min, wvl_max);

        const bool full_range = (wvl_min <= kWvlMin && wvl_max >= kWvlMax);
        if (full_range) {
          fRenderer->SetSiPMSelector(MakeSiPMSelectAll(), EOpHitSelectorMode::kAll);
          fRenderer->SetOpDetSelector(MakeOpDetSelectAll(), EOpHitSelectorMode::kAll);
        } 
        else {
          fRenderer->SetSiPMSelector(
              MakeSiPMSelectByWavelengthRange(wvl_min, wvl_max, fWvlRecSiPM),
              EOpHitSelectorMode::kWavelength
              );
          fRenderer->SetOpDetSelector(
              MakeOpDetSelectByWavelengthRange(wvl_min, wvl_max, fWvlRecOpDet), 
              EOpHitSelectorMode::kWavelength
              );
        }
        printf("SLArEveOpHitSelectorPanel::Apply() — mode: Wavelength "
               "[%.0f, %.0f] nm\n", (double)wvl_min, (double)wvl_max);
        break;
      }
    }

    if (fOnApply) fOnApply();
}

void SLArEveOpHitSelectorPanel::OnModeChanged()
{
    // Determine which radio button is now down
    if (fRadioOpProcess    && fRadioOpProcess->IsDown()) {
        fActiveSelector = ESelectorMode::kProcess;
        if (fRadioAll)        fRadioAll->SetState(kButtonUp, kFALSE);
        if (fRadioWavelength) fRadioWavelength->SetState(kButtonUp, kFALSE);
    } else if (fRadioWavelength && fRadioWavelength->IsDown()) {
        fActiveSelector = ESelectorMode::kWavelength;
        if (fRadioAll)     fRadioAll->SetState(kButtonUp, kFALSE);
        if (fRadioOpProcess) fRadioOpProcess->SetState(kButtonUp, kFALSE);
    } else {
        fActiveSelector = ESelectorMode::kAll;
        if (fRadioOpProcess)    fRadioOpProcess->SetState(kButtonUp, kFALSE);
        if (fRadioWavelength) fRadioWavelength->SetState(kButtonUp, kFALSE);
        if (fRadioAll)        fRadioAll->SetState(kButtonDown, kFALSE);
    }

    // Show/hide sub-panels
    if (fOpProcessSubFrame) {
        if (fActiveSelector == ESelectorMode::kProcess)
            fOpProcessSubFrame->MapWindow();
        else
            fOpProcessSubFrame->UnmapWindow();
    }
    if (fWavelengthSubFrame) {
        if (fActiveSelector == ESelectorMode::kWavelength)
            fWavelengthSubFrame->MapWindow();
        else
            fWavelengthSubFrame->UnmapWindow();
    }

    // Force layout update so the parent frame reflows
    if (fPanelParent) {
        fPanelParent->Layout();
        fPanelParent->MapSubwindows();
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Reset
// ────────────────────────────────────────────────────────────────────────────

void SLArEveOpHitSelectorPanel::Reset()
{
    // Return to "All hits" mode
    fActiveSelector = ESelectorMode::kAll;
    if (fRadioAll)        fRadioAll->SetState(kButtonDown, kFALSE);
    if (fRadioOpProcess)  fRadioOpProcess->SetState(kButtonUp, kFALSE);
    if (fRadioWavelength) fRadioWavelength->SetState(kButtonUp, kFALSE);

    // Reset process buttons to all-on
    for (auto* btn : fProcButtons)
        if (btn) btn->SetState(kButtonDown);

    // Reset wavelength entries to full range
    if (fWvlMinEntry) fWvlMinEntry->SetNumber(kWvlMin);
    if (fWvlMaxEntry) fWvlMaxEntry->SetNumber(kWvlMax);

    // Hide sub-panels
    if (fOpProcessSubFrame)  fOpProcessSubFrame->UnmapWindow();
    if (fWavelengthSubFrame) fWavelengthSubFrame->UnmapWindow();
    Apply();
}

} // namespace display
