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

    if (fHasProcessSelector)    BuildProcessGroup(parent);
    if (fHasWavelengthSelector) BuildWavelengthGroup(parent);
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

    // ── Collect enabled processes ─────────────────────────────────────────────
    std::vector<EPhProcess> active_procs;
    if (fHasProcessSelector) {
        for (int i = 0; i < kNProc; ++i) {
            if (fProcButtons[i] && fProcButtons[i]->IsDown())
                active_procs.push_back(kDisplayProcs[i]);
        }
    }

    // ── Collect wavelength range ──────────────────────────────────────────────
    float wvl_min = kWvlMin;
    float wvl_max = kWvlMax;
    if (fHasWavelengthSelector && fWvlMinEntry && fWvlMaxEntry) {
        wvl_min = static_cast<float>(fWvlMinEntry->GetNumber());
        wvl_max = static_cast<float>(fWvlMaxEntry->GetNumber());
        if (wvl_min > wvl_max) std::swap(wvl_min, wvl_max);
    }

    const bool all_procs_on  = (static_cast<int>(active_procs.size()) == kNProc)
                                || !fHasProcessSelector;
    const bool full_wvl_range = (!fHasWavelengthSelector)
                                || (wvl_min <= kWvlMin && wvl_max >= kWvlMax);

    // ── Build SiPM selector ───────────────────────────────────────────────────
    if (all_procs_on && full_wvl_range) {
        fRenderer->SetSiPMSelector(MakeSiPMSelectAll());
    } else if (!all_procs_on && full_wvl_range) {
        fRenderer->SetSiPMSelector(
            MakeSiPMSelectByProcessSet(active_procs, fProcRecSiPM));
    } else if (all_procs_on && !full_wvl_range) {
        fRenderer->SetSiPMSelector(
            MakeSiPMSelectByWavelengthRange(wvl_min, wvl_max, fWvlRecSiPM));
    } else {
        // Both process and wavelength filters active — compose them
        fRenderer->SetSiPMSelector(
            MakeSiPMSelectCombined(active_procs, fProcRecSiPM,
                                   wvl_min, wvl_max, fWvlRecSiPM));
    }

    // ── Build SuperCell selector ──────────────────────────────────────────────
    if (all_procs_on && full_wvl_range) {
        fRenderer->SetOpDetSelector(MakeOpDetSelectAll());
    } else if (!all_procs_on && full_wvl_range) {
        fRenderer->SetOpDetSelector(
            MakeOpDetSelectByProcessSet(active_procs, fProcRecOpDet));
    } else if (all_procs_on && !full_wvl_range) {
        fRenderer->SetOpDetSelector(
            MakeOpDetSelectByWavelengthRange(wvl_min, wvl_max, fWvlRecOpDet));
    } else {
        fRenderer->SetOpDetSelector(
            MakeOpDetSelectCombined(active_procs, fProcRecOpDet,
                                        wvl_min, wvl_max, fWvlRecOpDet));
    }

    printf("SLArEveOpHitSelectorPanel::Apply() — "
           "%zu process(es) active, wavelength [%.0f, %.0f] nm\n",
           active_procs.size(), (double)wvl_min, (double)wvl_max);

    if (fOnApply) fOnApply();
}

// ────────────────────────────────────────────────────────────────────────────
// Reset
// ────────────────────────────────────────────────────────────────────────────

void SLArEveOpHitSelectorPanel::Reset()
{
    if (fHasProcessSelector) {
        for (auto* btn : fProcButtons)
            if (btn) btn->SetState(kButtonDown);
    }
    if (fHasWavelengthSelector) {
        if (fWvlMinEntry) fWvlMinEntry->SetNumber(kWvlMin);
        if (fWvlMaxEntry) fWvlMaxEntry->SetNumber(kWvlMax);
    }
    Apply();
}

} // namespace display
