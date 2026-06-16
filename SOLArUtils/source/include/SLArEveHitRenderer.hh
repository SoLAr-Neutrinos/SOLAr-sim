/**
 * @file    SLArEveHitRenderer.hh
 * @brief   Renders external reconstruction charge-pixel hits into TEveBoxSets.
 *
 * One TEveBoxSet per TPC is created during Configure() and populated each
 * event by RenderHits().  The sets are kept alive here as unique_ptr and
 * exposed via a const reference so SLArEveDisplay can install them in the Eve
 * scene when the geometry is ready.
 *
 * Coordinate convention
 * ──────────────────────
 * Input reco hits carry world-frame coordinates.  The renderer converts them
 * to the local frame of the LAr target volume so they overlay correctly on
 * trajectories when the Eve scene applies the LAr transform.
 */

#pragma once

#include <memory>
#include <vector>

#include "TEveBoxSet.h"
#include "TEveRGBAPalette.h"
#include "TGeoMatrix.h"

#include "SLArRecoHits.hh"
#include "SLArEveEventReader.hh"
#include "SLArEveGeometry.hh"

namespace display {

/**
 * @class SLArEveHitRenderer
 * @brief Maps reco charge-pixel hits onto TEveBoxSets, one set per TPC.
 */
class SLArEveHitRenderer {
public:
    explicit SLArEveHitRenderer(const SLArEveGeometry& geometry);
    ~SLArEveHitRenderer() = default;

    // Non-copyable; TEveBoxSet is a ROOT heap object, ownership is explicit.
    SLArEveHitRenderer(const SLArEveHitRenderer&)            = delete;
    SLArEveHitRenderer& operator=(const SLArEveHitRenderer&) = delete;

    // ── Setup ────────────────────────────────────────────────────────────────

    /**
     * Initialise one TEveBoxSet per TPC and add them to @p parent.
     * Must be called once, after SLArEveGeometry::Configure().
     */
    void Configure(TEveElement& parent);

    // ── Per-event ────────────────────────────────────────────────────────────

    /**
     * Clear previous hit boxes and fill new ones from @p hit_vars.
     * Requires Configure() to have been called first.
     */
    void RenderHits(const reco::hitvarContainerPtr& hit_vars);

    /** Reset all TEveBoxSets to empty (called before each new event). */
    void Reset();

    // ── Accessors ────────────────────────────────────────────────────────────

    const std::vector<std::unique_ptr<TEveBoxSet>>& GetHitSets() const
    { return fHitSets; }

private:
    const SLArEveGeometry&                    fGeometry;   ///< non-owning ref
    std::vector<std::unique_ptr<TEveBoxSet>>  fHitSets;    ///< one per TPC
    std::unique_ptr<TEveRGBAPalette>          fPalette;

    /// Box half-extents [mm] used for all charge-pixel boxes.
    static constexpr float kBoxHalfSize = 2.0f;
};

} // namespace display
