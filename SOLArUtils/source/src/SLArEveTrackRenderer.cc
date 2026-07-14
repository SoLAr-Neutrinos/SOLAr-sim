/**
 * @file    SLArEveTrackRenderer.cc
 * @brief   Implementation of SLArEveTrackRenderer.
 */

#include "SLArEveTrackRenderer.hh"

#include "TEveTrackPropagator.h"
#include "TEvePathMark.h"
#include "TEveVector.h"
#include "TParticle.h"
#include "TParticlePDG.h"
#include "TDatabasePDG.h"
#include "TMath.h"

#include "event/SLArMCPrimaryInfo.hh"
#include "event/SLArEventTrajectory.hh"

namespace display {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor – populate default selectors
// ─────────────────────────────────────────────────────────────────────────────

SLArEveTrackRenderer::SLArEveTrackRenderer()
{
    // kYellow-7 gives a soft yellow that reads well against dark backgrounds.
    fParticleSelector.insert({"gammas",
        MCParticleSelector_t("gammas",        true,  1.0, kYellow - 7, 7)});
    fParticleSelector.insert({"electrons",
        MCParticleSelector_t("electrons",     true,  1.0, kOrange + 7)});
    fParticleSelector.insert({"heavy leptons",
        MCParticleSelector_t("heavy leptons", true,  1.0, kOrange)});
    fParticleSelector.insert({"neutrinos",
        MCParticleSelector_t("neutrinos",     false, 1.0, kAzure - 9, 2)});
    fParticleSelector.insert({"mesons",
        MCParticleSelector_t("mesons",        true,  1.0, kMagenta + 1)});
    fParticleSelector.insert({"neutrons",
        MCParticleSelector_t("neutrons",      true,  1.0, kBlue - 4)});
    fParticleSelector.insert({"protons",
        MCParticleSelector_t("protons",       true,  1.0, kRed - 4)});
    fParticleSelector.insert({"baryons",
        MCParticleSelector_t("baryons",       true,  1.0, kRed + 2)});
    fParticleSelector.insert({"ions",
        MCParticleSelector_t("ions",          true,  1.0, kViolet + 4)});
    fParticleSelector.insert({"others",
        MCParticleSelector_t("others",        true,  1.0, kWhite)});
}

// ─────────────────────────────────────────────────────────────────────────────
// Public rendering entry point
// ─────────────────────────────────────────────────────────────────────────────

void SLArEveTrackRenderer::RenderTracks(const SLArMCTruth&    truth,
                                        const TGeoCombiTrans& lar_transform,
                                        TEveElement&          parent)
{
    const auto& primaries = truth.GetPrimaries();
    printf("SLArEveTrackRenderer::RenderTracks – %zu primaries\n",
           primaries.size());

    // Copying the transform avoids holding a pointer into potentially
    // temporary geometry state.
    TGeoCombiTrans trans(lar_transform);

    for (const auto& primary : primaries) {
        auto* track_list = new TEveTrackList(
            Form("%s_%i", primary.GetName(), primary.GetTrackID()));

        TEveTrackPropagator* propagator = track_list->GetPropagator();
        propagator->SetMaxZ(1e5);
        propagator->SetMaxR(1e5);

        // Print primary summary.
        double p_tot = 0.;
        for (const double p : primary.GetMomentum()) p_tot += p * p;
        p_tot = std::sqrt(p_tot);
        printf("%s – vertex @ [%.2f, %.2f, %.2f] mm, t = %g ns\n",
               primary.GetName(),
               primary.GetVertex().at(0),
               primary.GetVertex().at(1),
               primary.GetVertex().at(2),
               primary.GetTime());

        for (const SLArEventTrajectory* trj : primary.GetConstTrajectories()) {
            const int pdg_code      = trj->GetPDGID();
            const auto& selector    = GetParticleSelection(pdg_code);

            if (!selector.fIsEnabled) continue;
            if (trj->GetInitKineticEne() < selector.fLowerEnergyThreshold)
                continue;

            const auto& points = trj->GetConstPoints();
            if (points.empty()) continue;

            printf("  [trkID %i] PDG %i – %zu trajectory points\n",
                   trj->GetTrackID(), pdg_code, points.size());

            // Convert vertex from detector frame to world/Eve frame.
            const auto& vtx = points.front();
            double vlar[3]  = {vtx.fX, vtx.fY, vtx.fZ};
            double vglob[3] = {};
            trans.LocalToMaster(vlar, vglob);

            auto* pdgDB  = TDatabasePDG::Instance();
            auto* pdgP   = pdgDB->GetParticle(pdg_code);

            auto* particle = new TParticle();
            if (pdgP) particle->SetPdgCode(pdg_code);
            particle->SetProductionVertex(
                vglob[0], vglob[1], vglob[2], trj->GetTime());

            // Mark root primaries explicitly so Eve's propagator knows.
            if (trj->GetParentID() == trj->GetTrackID())
                particle->SetFirstMother(-1);
            else
                particle->SetFirstMother(trj->GetParentID());

            auto* track = new TEveTrack(particle, trj->GetTrackID(), propagator);
            if (pdgP) track->SetCharge(pdgP->Charge());

            // Add path marks; sub-sample every 10 steps for performance,
            // but always include the last point.
            Long64_t istep = 0;
            for (auto it = points.begin(); it != points.end(); ++it, ++istep) {
                if (istep % 10 != 0 && (it != points.end() - 1)) continue;
                double slar[3]  = {it->fX, it->fY, it->fZ};
                double sglob[3] = {};
                trans.LocalToMaster(slar, sglob);
                auto* pm = new TEvePathMarkF(
                    TEvePathMarkF::kReference,
                    TEveVectorF(sglob[0], sglob[1], sglob[2]),
                    trj->GetTime());
                track->AddPathMark(*pm);
            }

            track->ComputeBBox();
            track->SetLineColor(selector.fTrackColor);
            track->SetLineStyle(selector.fTrackStyle);
            track->SetName(Form("%s_%i",
                                trj->GetParticleName().Data(),
                                trj->GetTrackID()));

            track_list->AddElement(track);
        } // trajectory loop

        track_list->MakeTracks();
        parent.AddElement(track_list);
    } // primary loop
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

const MCParticleSelector_t&
SLArEveTrackRenderer::GetParticleSelection(const int pdg) const
{
    auto* pdgDB = TDatabasePDG::Instance();

    if      (pdg == 22)                             return fParticleSelector.at("gammas");
    else if (std::abs(pdg) == 11)                   return fParticleSelector.at("electrons");
    else if (std::abs(pdg) == 13 ||
             std::abs(pdg) == 15)                   return fParticleSelector.at("heavy leptons");
    else if (std::abs(pdg) == 12 ||
             std::abs(pdg) == 14 ||
             std::abs(pdg) == 16)                   return fParticleSelector.at("neutrinos");
    else if (std::abs(pdg) == 2212)                 return fParticleSelector.at("protons");
    else if (std::abs(pdg) == 2112)                 return fParticleSelector.at("neutrons");
    else {
        TParticlePDG* pPDG = pdgDB->GetParticle(pdg);
        if (pPDG) {
            const TString cls = pPDG->ParticleClass();
            if (cls == "Meson")  return fParticleSelector.at("mesons");
            if (cls == "Baryon") return fParticleSelector.at("baryons");
        } else {
            return fParticleSelector.at("ions");
        }
    }

    return fParticleSelector.at("others");
}

} // namespace display
