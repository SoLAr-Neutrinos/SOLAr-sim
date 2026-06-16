/**
 * @file    SLArEveHitRenderer.cc
 * @brief   Implementation of SLArEveHitRenderer.
 */

#include "SLArEveHitRenderer.hh"

#include <limits>

namespace display {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

SLArEveHitRenderer::SLArEveHitRenderer(const SLArEveGeometry& geometry)
    : fGeometry(geometry)
    , fPalette(std::make_unique<TEveRGBAPalette>())
{}

// ─────────────────────────────────────────────────────────────────────────────
// Setup
// ─────────────────────────────────────────────────────────────────────────────

void SLArEveHitRenderer::Configure(TEveElement& parent)
{
    const auto& tpcs = fGeometry.GetTPCs();
    fHitSets.reserve(tpcs.size());

    for (const auto& tpc : tpcs) {
        auto hit_set = std::make_unique<TEveBoxSet>();
        hit_set->SetNameTitle(
            Form("hitsTPC%i", tpc.fID),
            Form("TPC %i charge hits", tpc.fID));
        hit_set->Reset(TEveBoxSet::kBT_AABoxFixedDim, false, 1024);
        hit_set->SetDefDepth (2.f * kBoxHalfSize);
        hit_set->SetDefWidth (2.f * kBoxHalfSize);
        hit_set->SetDefHeight(2.f * kBoxHalfSize);
        hit_set->SetPalette(fPalette.get());
        hit_set->SetPickable(kTRUE);

        parent.AddElement(hit_set.get());
        fHitSets.push_back(std::move(hit_set));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Per-event rendering
// ─────────────────────────────────────────────────────────────────────────────

void SLArEveHitRenderer::RenderHits(const reco::hitvarContainerPtr& hit_vars)
{
    if (!hit_vars.hit_tpc || hit_vars.hit_tpc->empty()) return;

    const auto& tpcs        = fGeometry.GetTPCs();
    const auto& lar_target  = fGeometry.GetLArTarget();

    float q_max = 0.f;

    for (std::size_t ihit = 0; ihit < hit_vars.hit_tpc->size(); ++ihit) {
        const int tpc_copy = hit_vars.hit_tpc->at(ihit);
        const int tpc_idx  = fGeometry.GetTPCIndex(tpc_copy);
        if (tpc_idx < 0 || tpc_idx >= static_cast<int>(fHitSets.size())) {
            printf("SLArEveHitRenderer: unknown TPC copy-ID %i (index %i - hitset size %ld); skipping hit\n",
                   tpc_copy, tpc_idx, fHitSets.size());
            continue;
        }

        // Reco hits are stored in the world/detector frame.
        // Convert to the LAr-target local frame so that LocalToMaster
        // in the Eve scene produces consistent world coordinates.
        const double* tpc_pos =
            tpcs[static_cast<std::size_t>(tpc_idx)].fTransform->GetTranslation();

        const double xtpc[3] = {
            hit_vars.hit_x->at(ihit),
            hit_vars.hit_y->at(ihit),
            hit_vars.hit_z->at(ihit)};
        const double xlar[3] = {
            xtpc[0] - tpc_pos[0],
            xtpc[1] - tpc_pos[1],
            xtpc[2] - tpc_pos[2]};
        double xglob[3] = {};
        lar_target.fTransform->LocalToMaster(xlar, xglob);

        fHitSets[static_cast<std::size_t>(tpc_idx)]->AddBox(
            xglob[0], xglob[1], xglob[2]);
        fHitSets[static_cast<std::size_t>(tpc_idx)]->DigitValue(
            static_cast<Int_t>(hit_vars.hit_q->at(ihit)));

        const float q = hit_vars.hit_q->at(ihit);
        if (q > q_max) q_max = q;
    }

    // Update palette range and refresh each hit set.
    fPalette->SetMax(static_cast<Int_t>(1.1f * q_max));
    fPalette->SetMin(1500);

    for (auto& hs : fHitSets) {
        hs->RefitPlex();
        hs->SetPalette(fPalette.get());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Reset
// ─────────────────────────────────────────────────────────────────────────────

void SLArEveHitRenderer::Reset()
{
    for (auto& hs : fHitSets) {
        hs->Reset(TEveBoxSet::kBT_AABoxFixedDim, false,
                  static_cast<Int_t>(hs->GetNItems()));
    }
}

} // namespace display
