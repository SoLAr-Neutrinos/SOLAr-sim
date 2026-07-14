/**
 * @file    SLArEveTrackRenderer.hh
 * @brief   Converts SLArMCTruth trajectories into TEveTrackList objects and
 *          attaches them to a parent Eve element.
 *
 * Ownership model
 * ────────────────
 *  • TEveTrackList objects are created with new and handed to the Eve scene
 *    graph via the parent element passed to RenderTracks().  TEveManager then
 *    owns them.  The renderer does NOT keep owning references to them.
 *  • The MCParticleSelector map is owned here; SLArEveDisplay reads it only
 *    to build the GUI controls and to apply changes requested by the user.
 */

#pragma once

#include <map>
#include <set>

#include "TGNumberEntry.h"
#include "TEveGeoShape.h"
#include "TEveTrack.h"
#include "TGeoMatrix.h"

#include "event/SLArMCTruth.hh"
#include "analysis/SLArBacktracker.hh"

namespace display {

// ─────────────────────────────────────────────────────────────────────────────
// Per-species display settings
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @struct MCParticleSelector_t
 * @brief  Bundles per-species visibility toggle, energy threshold, and
 *         ROOT colour / line-style for track rendering.
 *
 * GUI integration: SLArEveDisplay connects TGCheckButton and TGNumberEntry
 * widgets to ToggleEnable() and SetLowerEnergyDisplayThreshold() via CINT
 * signals.  The fEntryForm pointer is set from outside (by SLArEveDisplay)
 * after the GUI is built.
 */
struct MCParticleSelector_t {
    TString          fName                  = {};
    Bool_t           fIsEnabled             = kFALSE;
    double           fLowerEnergyThreshold  = 0.;
    Color_t          fTrackColor            = kBlack;
    Int_t            fTrackStyle            = 1;
    TGNumberEntry*   fEntryForm             = nullptr;  ///< non-owning GUI ptr

    MCParticleSelector_t() = default;
    MCParticleSelector_t(const char* name,
                         bool        is_on,
                         double      threshold,
                         Color_t     colour,
                         int         style = 1)
        : fName(name)
        , fIsEnabled(is_on)
        , fLowerEnergyThreshold(threshold)
        , fTrackColor(colour)
        , fTrackStyle(style)
    {}

    void ToggleEnable()
    {
        fIsEnabled = !fIsEnabled;
        printf("%s display %s\n", fName.Data(),
               fIsEnabled ? "enabled" : "disabled");
    }

    void SetLowerEnergyDisplayThreshold(double val)
    {
        fLowerEnergyThreshold = val;
        printf("[%s] lower energy threshold set to %g MeV\n",
               fName.Data(), fLowerEnergyThreshold);
    }

    /** Variant called via ROOT signal (reads value from fEntryForm). */
    void SetLowerEnergyDisplayThreshold()
    {
        if (!fEntryForm) return;
        SetLowerEnergyDisplayThreshold(
            fEntryForm->GetNumberEntry()->GetNumber());
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// SLArEveTrackRenderer
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @class SLArEveTrackRenderer
 * @brief Translates SLArMCTruth into Eve track objects.
 *
 * Usage:
 * @code
 *   SLArEveTrackRenderer renderer;
 *   // optionally adjust selectors via GetParticleSelectors()
 *   renderer.RenderTracks(truth, *lar_transform, *parent_eve_element);
 * @endcode
 */
class SLArEveTrackRenderer {
public:
    SLArEveTrackRenderer();
    ~SLArEveTrackRenderer() = default;

    // ── Rendering ────────────────────────────────────────────────────────────

    /**
     * Convert all primaries and their trajectories in @p truth into
     * TEveTrackList objects.  Each list is added as a child of @p parent.
     *
     * @param truth        MC truth for the current event.
     * @param lar_transform Local→master transform for the LAr volume
     *                      (used to convert detector-frame trajectory points
     *                      to world/Eve coordinates).
     * @param parent        Eve element to attach the new track lists to.
     */
    void RenderTracks(const SLArMCTruth&    truth,
                      const TGeoCombiTrans& lar_transform,
                      TEveElement&          parent);

    // ── Selector access (for GUI wiring in SLArEveDisplay) ───────────────────

    std::map<TString, MCParticleSelector_t>& GetParticleSelectors()
    { return fParticleSelector; }

    const std::map<TString, MCParticleSelector_t>& GetParticleSelectors() const
    { return fParticleSelector; }

private:
    // ── Helpers ───────────────────────────────────────────────────────────────

    /** Return the selector entry appropriate for the given PDG code. */
    const MCParticleSelector_t& GetParticleSelection(int pdg) const;

    // ── Data members ─────────────────────────────────────────────────────────

    std::map<TString, MCParticleSelector_t> fParticleSelector;
};

} // namespace display
