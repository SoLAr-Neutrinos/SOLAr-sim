/**
 * @author  Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file    SLArEveDisplay.cc
 * @brief   Top-level orchestrator and GUI shell — implementation.
 */

#include "SLArEveDisplay.hh"

#include "TEveBrowser.h"
#include "TEveWindow.h"
#include "TGClient.h"
#include "TGLayout.h"
#include "TGPicture.h"
#include "TRootBrowser.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TVirtualX.h"
#include "TMath.h"

ClassImp(display::SLArEveDisplay)

  namespace display {
    // ─────────────────────────────────────────────────────────────────────────────
    // Constructor
    // ─────────────────────────────────────────────────────────────────────────────

    SLArEveDisplay::SLArEveDisplay()
      : TGMainFrame(nullptr, 800, 800)
        , fGeometry()
        , fReader()
        , fTrackRenderer()
        , fHitRenderer(fGeometry)
        , fOpHitRenderer(fGeometry)
        , fEveManager(std::unique_ptr<TEveManager>(TEveManager::Create()))
        , fTimer(std::make_unique<TTimer>("gSystem->ProcessEvents();", 50, kFALSE))
    {}

    // ─────────────────────────────────────────────────────────────────────────────
    // Configuration
    // ─────────────────────────────────────────────────────────────────────────────

    void SLArEveDisplay::Configure(const rapidjson::Value& config)
    {
      fGeometry.Configure(config);
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // File loading
    // ─────────────────────────────────────────────────────────────────────────────

    int SLArEveDisplay::LoadHitFile(const TString& file_path,
        const TString& tree_key)
    {
      const int rc = fReader.LoadHitFile(file_path, tree_key);
      if (rc != 0) return rc;

      // Wire charge-hit boxes into TPC volumes now that we know the geometry.
      SetupHitRenderer();
      return 0;
    }

    int SLArEveDisplay::LoadMCEventFile(const TString& file_path,
        const TString& tree_key)
    {
      const int rc = fReader.LoadMCEventFile(file_path, tree_key);
      if (rc != 0) return rc;

      // Wire optical-hit boxes into the LAr target volume.
      SetupOpHitRenderer();
      return 0;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // Internal setup helpers
    // ─────────────────────────────────────────────────────────────────────────────

    void SLArEveDisplay::SetupHitRenderer()
    {
      // The hit renderer needs TPC volumes as parent Eve elements; they live
      // inside the LAr target volume in the geometry.
      // We add each TPC-level hit box set as a child of the matching TPC volume.
      const auto& tpcs = fGeometry.GetTPCs();

      // Pre-populate the hit sets (one per TPC) and attach to the LAr volume.
      // SLArEveHitRenderer::Configure() handles the TEveBoxSet creation.
      fHitRenderer.Configure(*fGeometry.GetLArTarget().fVolume);
    }

    void SLArEveDisplay::SetupOpHitRenderer()
    {
      fOpHitRenderer.Configure(
          fReader.GetCfgAnodes(),
          fReader.GetCfgPDS(),
          &fReader.GetBacktrackerDictionary(),
          *fGeometry.GetLArTarget().fVolume);

      fProcRecSiPM = fReader.GetBacktrackerRecordIndex(backtracker::EBkTrkReadoutSystem::kVUVSiPM,
          backtracker::EBacktracker::kOpticalProc);
      fProcRecOpDet = fReader.GetBacktrackerRecordIndex(backtracker::EBkTrkReadoutSystem::kOpDet,
          backtracker::EBacktracker::kOpticalProc);
      fWvlRecSiPM = fReader.GetBacktrackerRecordIndex(backtracker::EBkTrkReadoutSystem::kVUVSiPM,
          backtracker::EBacktracker::kWavelength);
      fWvlRecOpDet = fReader.GetBacktrackerRecordIndex(backtracker::EBkTrkReadoutSystem::kOpDet,
          backtracker::EBacktracker::kWavelength);

      // Selectors default to SelectAll; only override when the
      // opticalProc backtracker is actually present for that system.
      if (fProcRecSiPM >= 0) {
        printf("SLArEveDisplay: optical process backtracker available "
            "for vuv_sipm (record %d) — process selector enabled.\n",
            fProcRecSiPM);
        // Default to showing all processes; the GUI can override later.
        fOpHitRenderer.SetSiPMSelector( MakeSiPMSelectAll() );
        fOpHitRenderer.SetSiPMOpProcessBacktrackerIndex(fProcRecSiPM);
      }
      if (fProcRecOpDet >= 0) {
        printf("SLArEveDisplay: optical process backtracker available "
            "for opdets (record %d) — process selector enabled.\n",
            fProcRecOpDet);
        fOpHitRenderer.SetOpDetSelector( MakeOpDetSelectAll() );
        fOpHitRenderer.SetOpDetOpProcessBacktrackerIndex(fProcRecOpDet);
      }
      if (fWvlRecSiPM >= 0) {
        printf("SLArEveDisplay: wavelength backtracker available "
            "for vuv_sipm (record %d) — wavelength selector enabled.\n",
            fWvlRecSiPM);
        fOpHitRenderer.SetSiPMSelector( MakeSiPMSelectAll() );
        fOpHitRenderer.SetSiPMWvlngthBacktrackerIndex(fWvlRecSiPM);
      }
      if (fWvlRecOpDet >= 0) {
        printf("SLArEveDisplay: wavelength backtracker available "
            "for opdets (record %d) — wavelength selector enabled.\n",
            fWvlRecOpDet);
        fOpHitRenderer.SetOpDetSelector( MakeOpDetSelectAll() );
        fOpHitRenderer.SetOpDetWvlngthBacktrackerIndex(fWvlRecOpDet);
      }
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // Event loop
    // ─────────────────────────────────────────────────────────────────────────────

    void SLArEveDisplay::ProcessEvent()
    {
      printf("SLArEveDisplay: processing event %lld\n", fCurEvent);

      // ── Reset renderers ───────────────────────────────────────────────────────
      fHitRenderer.Reset();
      fOpHitRenderer.Reset();
      // Track lists are ephemeral TEve objects added to the LAr volume; remove
      // them by clearing the LAr volume's child list and re-adding static geometry.
      // The simplest approach: destroy existing track children and re-render.
      // We use the Eve element's own child-removal API.
      {
        auto* lar_vol = fGeometry.GetLArTarget().fVolume.get();
        // Remove only TEveTrackList children, leaving TEveBoxSets intact.
        // Iterate backwards so removal doesn't invalidate forward iterators.
        TEveElement::List_t to_remove;
        for (TEveElement::List_i it = lar_vol->BeginChildren(); it != lar_vol->EndChildren(); ++it) 
        {
          TEveElement* el = *it;
          if (dynamic_cast<TEveTrackList*>(el)) to_remove.push_back(el);
        }
        for (auto* el : to_remove) lar_vol->RemoveElement(el);
      }

      // ── Fill data ─────────────────────────────────────────────────────────────
      fReader.GetEntry(fCurEvent);

      // MC truth tracks
      if (fReader.HasMCTruth() && fReader.GetMCTruth()) {
        fTrackRenderer.RenderTracks(
            *fReader.GetMCTruth(),
            *fGeometry.GetLArTarget().fTransform,
            *fGeometry.GetLArTarget().fVolume);
      }

      // Charge pixel hits (from external reco file)
      if (fReader.HasHitFile()) {
        fHitRenderer.RenderHits(fReader.GetHitVars());
      }

      // Optical hits (from MC file)
      if (fReader.HasOpHits()) {
        fOpHitRenderer.RenderOpHits(
            fReader.GetAnodeList(),
            fReader.GetPDSList());
      }

      UpdateEntryLabel();
      ReDraw();
      UpdateTimeHistCanvas();
      UpdateWavelengthCanvas();
    }

    void SLArEveDisplay::NextEvent()
    {
      fCurEvent = TMath::Min(
          fReader.GetLastEvent(),
          fCurEvent + 1);
      ProcessEvent();
    }

    void SLArEveDisplay::PrevEvent()
    {
      fCurEvent = TMath::Max(static_cast<Long64_t>(0), fCurEvent - 1);
      ProcessEvent();
    }

    void SLArEveDisplay::ReDraw()
    {
      fEveManager->Redraw3D(false, true);
    }

    void SLArEveDisplay::UpdateTimeHistCanvas()
    {
      if (!fTimeHistCanvas) return;

      const auto& hists = fOpHitRenderer.GetTimeHistograms();
      if (hists.empty()) return;

      TCanvas* c = fTimeHistCanvas->GetCanvas();
      c->Clear();
      c->DivideSquare(static_cast<Int_t>(hists.size()));

      int ipad = 1;
      for (const auto& [group_id, hvec] : hists) {
        TVirtualPad* pad = c->cd(ipad);
        // Index 1 = all-hits time histogram (same choice as original code).
        printf("[%p] %s pad %s (%p/%p): drawing %s - %g entries\n", c, c->GetName(), gPad->GetName(), gPad, pad, hvec.at(2).GetName(), hvec.at(2).GetEntries());
        auto* h = static_cast<TH1*>(hvec.at(
              static_cast<int>(SLArEveOpHitRenderer::EOpHitHistType::kAllHitTime)
              ).Clone());
        h->SetDirectory(nullptr);
        h->SetLineColor(kBlack);
        h->Draw("hist");
        // check if process backtracker is active for this group and if so, overlay process-specific histograms
        if (fProcRecSiPM >= 0 || fProcRecOpDet >= 0) {
          auto* h_scint = static_cast<TH1*>(hvec.at(
                static_cast<int>(SLArEveOpHitRenderer::EOpHitHistType::kScintHitTime)
                ).Clone());
          h_scint->SetDirectory(nullptr);
          h_scint->SetLineColor(kRed);
          h_scint->Draw("hist same");

          auto* h_cher = static_cast<TH1*>(hvec.at(
                static_cast<int>(SLArEveOpHitRenderer::EOpHitHistType::kCherHitTime)
                ).Clone());
          h_cher->SetDirectory(nullptr);
          h_cher->SetLineColor(kGreen+2);
          h_cher->Draw("hist same");

          auto* h_wls = static_cast<TH1*>(hvec.at(
                static_cast<int>(SLArEveOpHitRenderer::EOpHitHistType::kWLSHitTime)
                ).Clone());
          h_wls->SetDirectory(nullptr);
          h_wls->SetLineColor(kBlue);
          h_wls->Draw("hist same");
        }
        ipad++;
      }

      c->Modified(); 
      c->Update();
    }

    void SLArEveDisplay::UpdateWavelengthCanvas() 
    {
      if (!fWavelenHistCanvas) return;

      const auto& hists = fOpHitRenderer.GetTimeHistograms();
      if (hists.empty()) return;

      TCanvas* c = fWavelenHistCanvas->GetCanvas();
      c->Clear();
      c->DivideSquare(static_cast<Int_t>(hists.size()));

      int ipad = 1;
      for (const auto& [group_id, hvec] : hists) {
        TVirtualPad* pad = c->cd(ipad);
        // Index 2 = wavelength histogram (same choice as original code).
        printf("[%p] %s pad %s (%p/%p) drawing %s - %g entries\n", c, c->GetName(), gPad->GetName(), gPad, pad, hvec.at(2).GetName(), hvec.at(2).GetEntries());
        auto* h = static_cast<TH1*>(hvec.at(
              static_cast<int>(SLArEveOpHitRenderer::EOpHitHistType::kWavelength)
              ).Clone());
        h->SetDirectory(nullptr);
        h->Draw("hist");
        ipad++;
      }

      c->Modified();
      c->Update();
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // GUI toggle
    // ─────────────────────────────────────────────────────────────────────────────

    void SLArEveDisplay::ToggleModeNHitsTime()
    {
      auto& nhit_sets = fOpHitRenderer.GetNHitSets();
      auto& time_sets = fOpHitRenderer.GetTimeSets();

      for (auto& [id, bs_nhit] : nhit_sets) {
        const bool show_nhit = !bs_nhit->GetRnrSelf();
        bs_nhit->SetRnrSelf(show_nhit);
        time_sets.at(id)->SetRnrSelf(!show_nhit);
      }
      gEve->Redraw3D();
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // GUI construction
    // ─────────────────────────────────────────────────────────────────────────────

    int SLArEveDisplay::MakeGUI()
    {
      auto* browser = fEveManager->GetBrowser();
      browser->StartEmbedding(TRootBrowser::kLeft);

      auto* frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
      frmMain->SetWindowName("Event Control");
      frmMain->SetCleanup(kDeepCleanup);

      // ── Navigation row ────────────────────────────────────────────────────────
      auto* hf = new TGHorizontalFrame(frmMain);
      const TString icondir(
          TString::Format("%s/icons/", gSystem->Getenv("ROOTSYS")));

      auto* b_prev = new TGPictureButton(
          hf, gClient->GetPicture(icondir + "GoBack.gif"), fIDs.GetUnID());
      hf->AddFrame(b_prev, new TGLayoutHints(kLHintsExpandX));
      b_prev->Connect("Clicked()", "display::SLArEveDisplay", this, "PrevEvent()");

      fEnterEntry = new TGNumberEntry(hf, 0, 5, fIDs.GetUnID(),
          TGNumberFormat::kNESInteger,
          TGNumberFormat::kNEANonNegative,
          TGNumberFormat::kNELLimitMin);
      fEnterEntry->Connect("ValueSet(Long_t)", "display::SLArEveDisplay",
          this, "SetEntry()");
      hf->AddFrame(fEnterEntry,
          new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

      auto* b_next = new TGPictureButton(
          hf, gClient->GetPicture(icondir + "GoForward.gif"), fIDs.GetUnID());
      hf->AddFrame(b_next, new TGLayoutHints(kLHintsExpandX));
      b_next->Connect("Clicked()", "display::SLArEveDisplay", this, "NextEvent()");

      frmMain->AddFrame(hf);

      // ── Particle selector panel ───────────────────────────────────────────────
      fGgroupframeParticleSelection =
        new TGGroupFrame(frmMain, "MC truth selection");
      fGframeParticleSelection =
        new TGVerticalFrame(fGgroupframeParticleSelection);

      auto& selectors = fTrackRenderer.GetParticleSelectors();
      int isel = 0;
      for (auto& [label, sel] : selectors) {
        if (isel >= kMaxSpecies) break;

        fGframeParticleSetting[isel] =
          new TGHorizontalFrame(fGframeParticleSelection);

        fGParticleSelectionButton[isel] = new TGCheckButton(
            fGframeParticleSetting[isel],
            new TGHotString(sel.fName), fIDs.GetUnID());
        fGParticleSelectionButton[isel]->Connect(
            "Toggled(Bool_t)",
            "display::MCParticleSelector_t",
            &sel, "ToggleEnable()");

        fGParticleEnergyThreshold[isel] = new TGNumberEntry(
            fGframeParticleSetting[isel], 0, 5, fIDs.GetUnID(),
            TGNumberFormat::kNESReal,
            TGNumberFormat::kNEAPositive,
            TGNumberFormat::kNELLimitMinMax, 0., 1000.);
        // Store the GUI pointer so the selector can read back the value.
        sel.fEntryForm = fGParticleEnergyThreshold[isel];
        fGParticleEnergyThreshold[isel]->SetNumber(sel.fLowerEnergyThreshold);
        fGParticleEnergyThreshold[isel]->Connect(
            "ValueSet(Long_t)",
            "display::MCParticleSelector_t",
            &sel, "SetLowerEnergyDisplayThreshold()");

        fGParticleSelectionButton[isel]->SetState(
            sel.fIsEnabled ? kButtonDown : kButtonUp);

        fGframeParticleSetting[isel]->AddFrame(
            fGParticleSelectionButton[isel],
            new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 1, 1, 2, 2));
        fGframeParticleSetting[isel]->AddFrame(
            fGParticleEnergyThreshold[isel],
            new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 0, 2, 2));
        fGframeParticleSetting[isel]->MapSubwindows();
        fGframeParticleSetting[isel]->Resize();
        fGframeParticleSelection->AddFrame(
            fGframeParticleSetting[isel],
            new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 1, 1, 1, 1));

        ++isel;
      }

      auto* btn_update =
        new TGTextButton(fGframeParticleSelection, "&Update", fIDs.GetUnID());
      btn_update->Connect("Clicked()", "display::SLArEveDisplay",
          this, "ProcessEvent()");
      fGframeParticleSelection->AddFrame(btn_update,
          new TGLayoutHints(kLHintsExpandX));
      fGframeParticleSelection->MapSubwindows();
      fGgroupframeParticleSelection->AddFrame(fGframeParticleSelection);

      // Nhits/Time toggle button
      fNhitsTimeToggleButton =
        new TGTextButton(fGgroupframeParticleSelection, "Show: Hit Count");
      fNhitsTimeToggleButton->Connect("Clicked()", "display::SLArEveDisplay",
          this, "ToggleModeNHitsTime()");
      fNhitsTimeToggleButton->SetHeight(30);
      fGgroupframeParticleSelection->AddFrame(
          fNhitsTimeToggleButton,
          new TGLayoutHints(kLHintsExpandX | kLHintsTop, 5, 5, 5, 5));

      frmMain->AddFrame(fGgroupframeParticleSelection);

      auto* grpOpSel = new TGGroupFrame(frmMain, "Optical hit selector");
      fOpHitSelectorPanel.Build(
          *grpOpSel,
          fOpHitRenderer,
          fProcRecSiPM,
          fProcRecOpDet,
          fWvlRecSiPM,
          fWvlRecOpDet);
      fOpHitSelectorPanel.SetOnApply([this]() { ProcessEvent(); });
      grpOpSel->MapSubwindows();
      frmMain->AddFrame(grpOpSel,
          new TGLayoutHints(kLHintsExpandX | kLHintsTop, 4, 4, 4, 4));


      frmMain->MapSubwindows();
      frmMain->Resize();
      frmMain->MapWindow();

      browser->StopEmbedding();
      browser->SetTabTitle("Event Control", 0);

      // ── Time-distribution canvas tab ──────────────────────────────────────────
      {
        TEveWindowSlot*  slot  = TEveWindow::CreateWindowInTab(
            gEve->GetBrowser()->GetTabRight());
        TEveWindowFrame* frame = slot->MakeFrame();
        frame->SetElementName("Time Distributions");

        fTimeHistCanvas = new TRootEmbeddedCanvas(
            "TimeHistCanvas",
            frame->GetGUICompositeFrame(), 800, 600);
        frame->GetGUICompositeFrame()->AddFrame(
            fTimeHistCanvas,
            new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        frame->GetGUICompositeFrame()->MapSubwindows();

        const std::size_t ngroups = fOpHitRenderer.GetTimeHistograms().size();
        if (ngroups > 0)
          fTimeHistCanvas->GetCanvas()->DivideSquare(
              static_cast<Int_t>(ngroups));
      }

      // ── Wavelength-spectrum canvas tab ────────────────────────────────────────
      {
        TEveWindowSlot*  slot  = TEveWindow::CreateWindowInTab(
            gEve->GetBrowser()->GetTabRight());
        TEveWindowFrame* frame = slot->MakeFrame();
        frame->SetElementName("Wavelength Spectrum");

        fWavelenHistCanvas = new TRootEmbeddedCanvas(
            "WavelenHistCanvas",
            frame->GetGUICompositeFrame(), 800, 600);
        frame->GetGUICompositeFrame()->AddFrame(
            fWavelenHistCanvas,
            new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        frame->GetGUICompositeFrame()->MapSubwindows();

        const std::size_t ngroups = fOpHitRenderer.GetTimeHistograms().size();
        if (ngroups > 0)
          fWavelenHistCanvas->GetCanvas()->DivideSquare(
              static_cast<Int_t>(ngroups));

      }

      // ── Connect browser close to application exit ─────────────────────────────
      fEveManager->GetBrowser()->Connect(
          "CloseWindow()", "TApplication", gApplication, "Terminate(=0)");

      return 1;
    }

  } // namespace display
