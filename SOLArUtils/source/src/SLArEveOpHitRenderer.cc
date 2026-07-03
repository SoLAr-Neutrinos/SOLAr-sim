/**
 * @file    SLArEveOpHitRenderer.cc
 * @brief   Implementation of SLArEveOpHitRenderer.
 */

#include "SLArEveOpHitRenderer.hh"
#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgBaseSystem.hh"

#include <cmath>
#include <limits>
#include <memory>

#include "Math/EulerAngles.h"
#include "TH2Poly.h"

namespace display {

  // ─────────────────────────────────────────────────────────────────────────────
  // Constructor
  // ─────────────────────────────────────────────────────────────────────────────

  SLArEveOpHitRenderer::SLArEveOpHitRenderer(const SLArEveGeometry& geometry)
    : fGeometry(geometry)
      , fPaletteNHits(std::make_unique<TEveRGBAPalette>())
      , fPaletteTHits(std::make_unique<TEveRGBAPalette>())
  {}

  // ─────────────────────────────────────────────────────────────────────────────
  // Setup
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveOpHitRenderer::Configure(
      const std::map<int, std::unique_ptr<SLArCfgAnode>>& anode_cfgs,
      const CfgPDS_t* pds_cfg,
      const BacktrackerDict_t* backtracker_dict,
      TEveElement& parent)
  {
    fCfgAnodes = &anode_cfgs;
    fCfgPDS    = pds_cfg;
    fBacktrackerDict = backtracker_dict;

    // ── Anode SiPM arrays (one group per TPC) ─────────────────────────────────
    for (const auto& [tpc_id, cfg_anode] : anode_cfgs) {
      const TString name = Form("opHit_anode_%i", tpc_id);
      const TString titl = Form("TPC %i anode optical hits", tpc_id);

      auto bs_nhit = std::make_unique<TEveBoxSet>(name, titl);
      bs_nhit->Reset(TEveBoxSet::kBT_AABox, false, 100);
      bs_nhit->SetRenderMode(TEveBoxSet::kRM_Fill);
      parent.AddElement(bs_nhit.get());

      auto bs_time = std::make_unique<TEveBoxSet>(name, titl);
      bs_time->Reset(TEveBoxSet::kBT_AABox, false, 100);
      bs_time->SetRenderMode(TEveBoxSet::kRM_Fill);
      parent.AddElement(bs_time.get());

      fDetectorNHits.emplace(tpc_id, std::move(bs_nhit));
      fDetectorTHits.emplace(tpc_id, std::move(bs_time));
    }

    // ── X-ARAPUCA PDS walls ───────────────────────────────────────────────────
    if (fCfgPDS) {
      for (const auto& [wall_id, wall_cfg] : fCfgPDS->GetConstMap()) {
        const TString name = Form("opHit_%s", wall_cfg.GetName());
        const TString titl = Form("PDS wall %i optical hits", wall_id);

        auto bs_nhit = std::make_unique<TEveBoxSet>(name, titl);
        bs_nhit->Reset(TEveBoxSet::kBT_AABox, false, 100);
        bs_nhit->SetRenderMode(TEveBoxSet::kRM_Fill);
        parent.AddElement(bs_nhit.get());

        auto bs_time = std::make_unique<TEveBoxSet>(name, titl);
        bs_time->Reset(TEveBoxSet::kBT_AABox, false, 100);
        bs_time->SetRenderMode(TEveBoxSet::kRM_Fill);
        parent.AddElement(bs_time.get());

        fDetectorNHits.emplace(wall_id, std::move(bs_nhit));
        fDetectorTHits.emplace(wall_id, std::move(bs_time));
      }
    }

    SetupHistograms();
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Per-event rendering
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveOpHitRenderer::RenderOpHits(
      const SLArListEventAnode* ev_anode_list,
      const SLArListEventPDS*   ev_pds_list)
  {
    Reset();

    int nhit_max  = 0;
    int time_min  = std::numeric_limits<int>::max();
    int time_max  = 0;

    // ── Anode SiPM hits ───────────────────────────────────────────────────────
    if (ev_anode_list) {
      for (const auto& [tpc_id, ev_anode] : ev_anode_list->GetConstAnodeMap()) {
        // Skip TPCs for which no config was loaded.
        if (!fCfgAnodes || fCfgAnodes->count(tpc_id) == 0) continue;

        const auto limits = RenderFromAnode(tpc_id, ev_anode);
        if (limits.nhit_max > nhit_max)         nhit_max = limits.nhit_max;
        if (limits.time_min < time_min &&
            limits.time_min != std::numeric_limits<int>::max())
          time_min = limits.time_min;
        if (limits.time_max > time_max)          time_max = limits.time_max;
      }
    }

    // ── PDS wall hits ─────────────────────────────────────────────────────────
    if (ev_pds_list && fCfgPDS) {
      for (const auto& [wall_id, ev_array] : ev_pds_list->GetConstOpDetArrayMap()) {
        if (ev_array.GetNhits() == 0) continue;

        const auto limits = RenderFromOpDetArray(wall_id, ev_array);
        if (limits.nhit_max > nhit_max)         nhit_max = limits.nhit_max;
        if (limits.time_min < time_min &&
            limits.time_min != std::numeric_limits<int>::max())
          time_min = limits.time_min;
        if (limits.time_max > time_max)          time_max = limits.time_max;
      }
    }

    // ── Update colour palettes ────────────────────────────────────────────────
    fPaletteNHits->SetLimitsScaleMinMax(0, static_cast<Int_t>(1.2 * nhit_max));
    for (auto& [id, bs] : fDetectorNHits)
      bs->SetPalette(fPaletteNHits.get());

    if (time_min != std::numeric_limits<int>::max()) {
      fPaletteTHits->SetLimitsScaleMinMax(
          static_cast<Int_t>(0.9 * time_min),
          static_cast<Int_t>(2.0 * time_max));
    }
    for (auto& [id, bs] : fDetectorTHits)
      bs->SetPalette(fPaletteTHits.get());
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Reset
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveOpHitRenderer::Reset()
  {
    for (auto& [id, bs] : fDetectorNHits)
      bs->Reset(TEveBoxSet::kBT_AABox, false, 10000);

    for (auto& [id, bs] : fDetectorTHits)
      bs->Reset(TEveBoxSet::kBT_AABox, false, 10000);

    for (auto& [id, hists] : fOpHitsHistograms)
      for (auto& h : hists) h->Reset();
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Private – PDS wall rendering
  // ─────────────────────────────────────────────────────────────────────────────

  OpHitLimits SLArEveOpHitRenderer::RenderFromOpDetArray(
      const int                       idx_array,
      const SLArEventSuperCellArray&  ev_array)
  {
    OpHitLimits limits;

    const auto& cfg_wall = fCfgPDS->GetConstMap().at(idx_array);
    const ROOT::Math::EulerAngles rot(
        cfg_wall.GetPhi(), cfg_wall.GetTheta(), cfg_wall.GetPsi());
    const ROOT::Math::EulerAngles rrot = rot.Inverse();

    const auto& lar_target = fGeometry.GetLArTarget();

    auto& bs_nhit = fDetectorNHits.at(idx_array);
    auto& bs_time = fDetectorTHits.at(idx_array);

    for (auto& hist : fOpHitsHistograms.at(idx_array)) {
      hist->Reset();
    }
    auto& h_first = fOpHitsHistograms.at(idx_array).at(static_cast<int>(EOpHitHistType::kFirstHitTime));
    auto& h_all   = fOpHitsHistograms.at(idx_array).at(static_cast<int>(EOpHitHistType::kAllHitTime  ));
    auto& h_scint = fOpHitsHistograms.at(idx_array).at(static_cast<int>(EOpHitHistType::kScintHitTime));
    auto& h_cher  = fOpHitsHistograms.at(idx_array).at(static_cast<int>(EOpHitHistType::kCherHitTime ));
    auto& h_wls   = fOpHitsHistograms.at(idx_array).at(static_cast<int>(EOpHitHistType::kWLSHitTime  ));
    auto& h_wvl   = fOpHitsHistograms.at(idx_array).at(static_cast<int>(EOpHitHistType::kWavelength  ));
    auto h2_map   = dynamic_cast<TH2Poly*>(
        fOpHitsHistograms.at(idx_array).at(static_cast<int>(EOpHitHistType::kOpHitMap)).get() );

    for (const auto& [idx_xa, ev_xa] : ev_array.GetConstSuperCellMap()) {
      const auto select_result = fOpDetSelector(ev_xa);
      if (select_result.n_hits == 0) continue;

      if (select_result.n_hits > limits.nhit_max) limits.nhit_max = select_result.n_hits;

      const auto& cfg_xa = cfg_wall.GetBaseElement(idx_xa);

      const ROOT::Math::XYZVectorD pos  = {
        cfg_xa.GetPhysX(), cfg_xa.GetPhysY(), cfg_xa.GetPhysZ()};
      const ROOT::Math::XYZVectorD size = {
        0.95 * cfg_xa.GetSizeX(),
        cfg_xa.GetSizeY(),
        0.95 * cfg_xa.GetSizeZ()};

      ROOT::Math::XYZVectorD size_rot = rrot * size;
      size_rot.SetXYZ(std::fabs(size_rot.x()),
          std::fabs(size_rot.y()),
          std::fabs(size_rot.z()));

      double xsize[3] = {};
      double xlar_size[3] = {};
      size_rot.GetCoordinates(xlar_size);
      lar_target.fTransform->LocalToMaster(xlar_size, xsize);

      const ROOT::Math::XYZVectorD pos_corner = pos - 0.5 * size_rot;
      double xlar[3]  = {};
      double xglob[3] = {};
      pos_corner.GetCoordinates(xlar);
      lar_target.fTransform->LocalToMaster(xlar, xglob);

      bs_nhit->AddBox(xglob[0], xglob[1], xglob[2],
          xsize[0],  xsize[1],  xsize[2]);
      bs_nhit->DigitValue(select_result.n_hits);

      const int hit_time = select_result.time_min;
      if (hit_time < limits.time_min) limits.time_min = hit_time;
      if (hit_time > limits.time_max) limits.time_max = hit_time;

      bs_time->AddBox(xglob[0], xglob[1], xglob[2],
          xsize[0],  xsize[1],  xsize[2]);
      bs_time->DigitValue(hit_time);

      h_first->Fill(static_cast<double>(hit_time));
      
      if (h2_map) {
        const int idx_bin = cfg_wall.GetConstMap().at(idx_xa).GetBinIdx(); 
        h2_map->SetBinContent(idx_bin, static_cast<double>(select_result.n_hits));
      }

      const HitsCollection_t& hits_sel = ( OpDetSelectorIsFiltering() ) ? 
        select_result.fHits : ev_xa.GetConstHits();

      for (const auto& [time_bin, count] : hits_sel) {
        h_all->Fill(static_cast<double>(time_bin),
            static_cast<double>(count));

        if (OpDetSelectorIsFiltering() == true) continue;

        const auto& bt_coll = ev_xa.GetBacktrackerRecordCollection();
        if (bt_coll.empty()) continue;
        const auto& records = bt_coll.at(time_bin).GetConstRecords();
            
        // check if wavelength backtracker is active
        if (fOpDetWvlngthBktrkIdx >= 0) {
          if (bt_coll.empty()) continue;
          if (fOpDetWvlngthBktrkIdx >= static_cast<int>(records.size())) continue;
          for (const auto& [wvl_key, count] : records[fOpDetWvlngthBktrkIdx].GetConstCounter()) {
            const float wvl_val = wvl_key;
            h_wvl->Fill(wvl_val, static_cast<double>(count));
          }
        }

        if (fOpDetOpProcBktrkIdx >= 0) {
          const auto& proc_bkt = records[fOpDetOpProcBktrkIdx].GetConstCounter();
          for (const auto& [proc_key, count] : proc_bkt) {
            const int proc_val = proc_key;
            if (count > 0) {
              if (proc_val == static_cast<int>(EPhProcess::kScnt)) {
                h_scint->Fill(static_cast<double>(time_bin), static_cast<double>(count));
              }
              else if (proc_val == static_cast<int>(EPhProcess::kCher)) {
                h_cher->Fill(static_cast<double>(time_bin), static_cast<double>(count));
              }
              else if (proc_val == static_cast<int>(EPhProcess::kWLS)) {
                h_wls->Fill(static_cast<double>(time_bin), static_cast<double>(count));
              }
            }
          }
        }
      }
    }

    h_all->SetEntries( h_all->Integral() );
    h_scint->SetEntries( h_scint->Integral() );
    h_cher->SetEntries( h_cher->Integral() );
    h_wls->SetEntries( h_wls->Integral() );
    h_wvl->SetEntries( h_wvl->Integral() );

    auto finish_boxset = [](TEveBoxSet& bs) {
      bs.RefitPlex();
      bs.SetPickable(kTRUE);
      bs.SetAlwaysSecSelect(kTRUE);
    };
    finish_boxset(*bs_nhit);
    bs_nhit->SetRnrSelf(kTRUE);
    finish_boxset(*bs_time);
    bs_time->SetRnrSelf(kFALSE);  // time view is hidden by default

    return limits;
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Private – anode SiPM rendering
  // ─────────────────────────────────────────────────────────────────────────────

  OpHitLimits SLArEveOpHitRenderer::RenderFromAnode(
      const int              tpc_id,
      const SLArEventAnode&  ev_anode)
  {
    OpHitLimits limits;

    const auto& cfg_anode = fCfgAnodes->at(tpc_id);
    const ROOT::Math::EulerAngles rot(
        cfg_anode->GetPhi(), cfg_anode->GetTheta(), cfg_anode->GetPsi());
    const ROOT::Math::EulerAngles rrot    = rot.Inverse();
    const ROOT::Math::EulerAngles lar_rot =
      fGeometry.GetLArTarget().fRotation.Inverse();

    const ROOT::Math::XYZVectorD sipm_size    = {20., 1., 20.};
    ROOT::Math::XYZVectorD       sipm_size_rot = rrot * sipm_size;
    sipm_size_rot.SetXYZ(std::fabs(sipm_size_rot.x()),
        std::fabs(sipm_size_rot.y()),
        std::fabs(sipm_size_rot.z()));

    auto& bs_nhit = fDetectorNHits.at(tpc_id);
    auto& bs_time = fDetectorTHits.at(tpc_id);

    for (auto& hist : fOpHitsHistograms.at(tpc_id)) {
      hist->Reset();
    }

    auto& h_first = fOpHitsHistograms.at(tpc_id).at(static_cast<int>(EOpHitHistType::kFirstHitTime));
    auto& h_all   = fOpHitsHistograms.at(tpc_id).at(static_cast<int>(EOpHitHistType::kAllHitTime  ));
    auto& h_scint = fOpHitsHistograms.at(tpc_id).at(static_cast<int>(EOpHitHistType::kScintHitTime));
    auto& h_cher  = fOpHitsHistograms.at(tpc_id).at(static_cast<int>(EOpHitHistType::kCherHitTime ));
    auto& h_wls   = fOpHitsHistograms.at(tpc_id).at(static_cast<int>(EOpHitHistType::kWLSHitTime  ));
    auto& h_wvl   = fOpHitsHistograms.at(tpc_id).at(static_cast<int>(EOpHitHistType::kWavelength  ));

    for (const auto& [idx_mt, ev_mt] : ev_anode.GetConstMegaTilesMap()) {
      if (ev_mt.GetNPhotonHits() == 0) continue;

      const auto& cfg_mt = cfg_anode->GetBaseElement(idx_mt);

      for (const auto& [idx_t, ev_t] : ev_mt.GetConstTileMap()) {
        if (ev_t.GetNSiPMHits() == 0) continue;

        const auto& cfg_t = cfg_mt.GetBaseElement(idx_t);

        const auto n_cols = cfg_t.GetNCellCols();

        const ROOT::Math::XYZVectorD t_size = {
          cfg_t.GetSizeX(), cfg_t.GetSizeY(), cfg_t.GetSizeZ()};

        ROOT::Math::XYZVectorD t_size_rot = rrot * t_size;
        t_size_rot.SetXYZ(std::fabs(t_size_rot.x()),
            std::fabs(t_size_rot.y()),
            std::fabs(t_size_rot.z()));
        ROOT::Math::XYZVectorD t_size_rot_lar = lar_rot * t_size_rot;
        t_size_rot_lar.SetXYZ(std::fabs(t_size_rot_lar.x()),
            std::fabs(t_size_rot_lar.y()),
            std::fabs(t_size_rot_lar.z()));

        const ROOT::Math::XYZVectorD t_pos = {
          cfg_t.GetPhysX(), cfg_t.GetPhysY(), cfg_t.GetPhysZ()};
        const ROOT::Math::XYZVectorD t_pos_lar = lar_rot * t_pos;

        for (const auto& [idx_sipm, ev_sipm] : ev_t.GetConstSiPMEvents()) {
          const auto select_result = fSiPMSelector(ev_sipm);
          const int& nhit = select_result.n_hits;
          const int& hit_time = select_result.time_min;
          if (nhit == 0) continue;
          if (nhit > limits.nhit_max) limits.nhit_max = nhit;

          // Unit cell pitch is 30 mm (hard-coded in original code).
          const int row = static_cast<int>(idx_sipm / n_cols);
          const int col = static_cast<int>(idx_sipm % n_cols);
          const ROOT::Math::XYZVectorD sipm_pos = {
            (row + 0.5) * 30. - t_size.x() * 0.5,
            0.,
            (col + 0.5) * 30. - t_size.z() * 0.5};

          const ROOT::Math::XYZVectorD sipm_pos_rot = rrot * sipm_pos;
          const ROOT::Math::XYZVectorD sipm_pos_lar =
            t_pos + sipm_pos_rot - 0.5 * sipm_size_rot;
          const ROOT::Math::XYZVectorD world_pos = lar_rot * sipm_pos_lar;

          bs_nhit->AddBox(world_pos.x(), world_pos.y(), world_pos.z(),
              sipm_size_rot.x(), sipm_size_rot.y(),
              sipm_size_rot.z());
          bs_nhit->DigitValue(nhit);

          if (hit_time < limits.time_min) limits.time_min = hit_time;
          if (hit_time > limits.time_max) limits.time_max = hit_time;

          // Use negative height to visualise time (original convention).
          bs_time->AddBox(world_pos.x(), world_pos.y(), world_pos.z(),
              sipm_size_rot.x(),
              static_cast<double>(-nhit) * 10.,
              sipm_size_rot.z());
          bs_time->DigitValue(hit_time);

          h_first->Fill(static_cast<double>(hit_time));

          const HitsCollection_t& hits_sel = ( SiPMSelectorIsFiltering() ) ? 
            select_result.fHits : ev_sipm.GetConstHits();

          for (const auto& [time_bin, count] : select_result.fHits) {
            h_all->Fill(static_cast<double>(time_bin),
                static_cast<double>(count));

            if (SiPMSelectorIsFiltering() == true) continue;

            const auto& bt_coll = ev_sipm.GetBacktrackerRecordCollection();
            if (bt_coll.empty()) continue;
            const auto& records = bt_coll.at(time_bin).GetConstRecords();
            
            // check if wavelength backtracker is active
            if (fSiPMWvlngthBktrkIdx >= 0) {
              if (bt_coll.empty()) continue;
              if (fSiPMWvlngthBktrkIdx >= static_cast<int>(records.size())) continue;
              for (const auto& [wvl_key, count] : records[fSiPMWvlngthBktrkIdx].GetConstCounter()) {
                const float wvl_val = wvl_key;
                h_wvl->Fill(wvl_val, static_cast<double>(count));
              }
            }

            if (fSiPMOpProcBktrkIdx >= 0) {
              const auto& proc_bkt = records[fSiPMOpProcBktrkIdx].GetConstCounter();
              for (const auto& [proc_key, count] : proc_bkt) {
                const int proc_val = proc_key;
                if (count > 0) {
                  if (proc_val == static_cast<int>(EPhProcess::kScnt)) {
                    h_scint->Fill(static_cast<double>(time_bin), static_cast<double>(count));
                  }
                  else if (proc_val == static_cast<int>(EPhProcess::kCher)) {
                    h_cher->Fill(static_cast<double>(time_bin), static_cast<double>(count));
                  }
                  else if (proc_val == static_cast<int>(EPhProcess::kWLS)) {
                    h_wls->Fill(static_cast<double>(time_bin), static_cast<double>(count));
                  }
                }
              }
            }
          }
        } // SiPM loop
      } // tile loop

      h_all->SetEntries( h_all->Integral() );
      h_scint->SetEntries( h_scint->Integral() );
      h_cher->SetEntries( h_cher->Integral() );
      h_wls->SetEntries( h_wls->Integral() ); 
      h_wvl->SetEntries( h_wvl->Integral() );

      // Refresh once per megatile to avoid RefitPlex spam.
      auto finish = [](TEveBoxSet& bs) {
        bs.RefitPlex();
        bs.SetPickable(kTRUE);
        bs.SetAlwaysSecSelect(kTRUE);
      };
      finish(*bs_nhit);
      bs_nhit->SetRnrSelf(kTRUE);
      finish(*bs_time);
      bs_time->SetRnrSelf(kFALSE);
    } // megatile loop

    return limits;
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Private – histogram initialisation
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveOpHitRenderer::SetupHistograms()
  {
    // Build histogram vectors for every detector group that was registered
    // in Configure() (both anode SiPMs and PDS walls use the same key space).
    for (const auto& [group_id, bs] : fDetectorNHits) {
      bool is_anode = (fCfgAnodes && fCfgAnodes->count(group_id) > 0);
      bool is_opdet = (fCfgPDS && fCfgPDS->GetConstMap().count(group_id) > 0);

      fOpHitsHistograms[group_id] = std::vector<std::unique_ptr<TH1>>{};
      auto& hv = fOpHitsHistograms[group_id];

      hv.emplace_back(std::make_unique<TH1F>(
          Form("hFirstOpHitTime_%i",   group_id),
          Form("Group %i first hit time;Time [bin];Counts", group_id),
          1000, 0., 50000.));
      hv.back()->SetLineWidth(2);

      hv.emplace_back(std::make_unique<TH1F>(
          Form("hOpHitTime_%i",        group_id),
          Form("Group %i all-hit time;Time [ns];Counts", group_id),
          300, 0., 10000.));
      hv.back()->SetLineWidth(2);

      hv.emplace_back(std::make_unique<TH1F>(
          Form("hOpHitTimeScint_%i",        group_id),
          Form("Group %i Scintillation hit time;Time [ns];Counts", group_id),
          300, 0., 10000.));
      hv.back()->SetLineWidth(2);

      hv.emplace_back(std::make_unique<TH1F>(
          Form("hOpHitTimeCher_%i",        group_id),
          Form("Group %i Cherenkov hit time;Time [ns];Counts", group_id),
          300, 0., 10000.));
      hv.back()->SetLineWidth(2);

      hv.emplace_back(std::make_unique<TH1F>(
          Form("hOpHitTimeWLS%i",        group_id),
          Form("Group %i WLS hit time;Time [ns];Counts", group_id),
          300, 0., 10000.));
      hv.back()->SetLineWidth(2);

      hv.emplace_back(std::make_unique<TH1F>(
          Form("hOpHitWavelength_%i",  group_id),
          Form("Group %i wavelength;Wavelength [nm];Counts", group_id),
          100, 100., 900.));
      hv.back()->SetLineWidth(2);

      if (is_opdet) {
        const auto& cfg_plane = fCfgPDS->GetConstMap().at(group_id);
        TH2Poly* h2poly = cfg_plane.BuildPolyBinHist(SLArCfgSuperCellArray::ESubModuleReferenceFrame::kWorld);
        TString x_label = GetAxisLabel(cfg_plane.GetAxis0());
        TString y_label = GetAxisLabel(cfg_plane.GetAxis1());
        if ( cfg_plane.GetAxis0().Dot(TVector3(1, 0, 0)) > 0.5 ) { x_label = "X";}
        h2poly->SetNameTitle(
            Form("hOpHitMap_%i", group_id),
            Form("Group %i hit map;%s [mm];%s [mm]", group_id, x_label.Data(), y_label.Data()));
        hv.emplace_back(std::move(h2poly));
      }
    }
  }

} // namespace display
