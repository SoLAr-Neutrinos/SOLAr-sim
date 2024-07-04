/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEveDisplay
 * @created     : Thursday Apr 11, 2024 16:58:14 CEST
 */

#include "TObject.h"
#include <SLArEveDisplay.hh>
#include <SLArUnit.hpp>
#include <cstdio>
#include <event/SLArMCPrimaryInfo.hh>
#include <event/SLArEventTrajectory.hh>
#include <TParticle.h>
#include <TParticlePDG.h>
#include <TDatabasePDG.h>
#include <TGeoManager.h>
#include <TEveTrackPropagator.h>
#include <TEvePathMark.h>
#include <TEveVector.h>
#include <TEveFrameBox.h>
#include <TEveRGBAPalette.h>
#include <TRootBrowser.h>
#include <TEveBrowser.h>

#include <TGTab.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>

#include <TString.h>
#include <TSystem.h>

#include <TStyle.h>
#include <cstddef>

ClassImp(display::SLArEveDisplay)

namespace display {

  SLArEveDisplay::SLArEveDisplay() 
    : TGMainFrame(nullptr, 800, 800), fHitFile(nullptr), fHitTree(nullptr), fLastEvent(1), fCurEvent(0)
  {
    //gStyle->SetPalette(kSunset);
    fTimer = std::make_unique<TTimer>("gSystem->ProcessEvents();", 50, kFALSE);
    fEveManager = std::unique_ptr<TEveManager>( TEveManager::Create() );
    fPalette = std::make_unique<TEveRGBAPalette>();

    fParticleSelector.insert( {"gammas", MCParticleSelector_t("gammas", true, 1.0, kYellow-7, 7)} ); 
    fParticleSelector.insert( {"light leptons", MCParticleSelector_t("light leptons", true, 1.0, kOrange+7)} ); 
    fParticleSelector.insert( {"heavy leptons", MCParticleSelector_t("heavy leptons", true, 1.0, kOrange)} ); 
    fParticleSelector.insert( {"neutrinos", MCParticleSelector_t("neutrinos", false, 1.0, kAzure-9, 2)} ); 
    fParticleSelector.insert( {"mesons", MCParticleSelector_t("mesons", true, 1.0, kMagenta+1)} ); 
    fParticleSelector.insert( {"neutrons", MCParticleSelector_t("neutrons", true, 1.0, kBlue-4)} ); 
    fParticleSelector.insert( {"protons", MCParticleSelector_t("protons", true, 1.0, kRed-4)} ); 
    fParticleSelector.insert( {"baryons", MCParticleSelector_t("baryons", true, 1.0, kRed+2)} ); 
    fParticleSelector.insert( {"ions", MCParticleSelector_t("ions", true, 1.0, kViolet+4)} ); 
    fParticleSelector.insert( {"others", MCParticleSelector_t("others", true, 1.0, kWhite)} ); 
  }

  const MCParticleSelector_t& SLArEveDisplay::get_particle_selection(const int pdg) {
    auto pdgDB = TDatabasePDG::Instance(); 

    if (pdg == 22) return fParticleSelector["gammas"]; 
    else if ( fabs(pdg) == 11) return fParticleSelector["electrons"]; 
    else if ( fabs(pdg) == 13 || fabs(pdg) == 15) return fParticleSelector["heavy leptons"]; 
    else if ( fabs(pdg) == 12 || fabs(pdg) == 14 || fabs(pdg) == 16) return fParticleSelector["neutrinos"]; 
    else if ( fabs(pdg) == 2212 ) return fParticleSelector["protons"]; 
    else if ( fabs(pdg) == 2112 ) return fParticleSelector["neutrons"]; 
    else {
      TParticlePDG* particlePDG = pdgDB->GetParticle( pdg );
      if (particlePDG) {
        const TString particleClass = particlePDG->ParticleClass(); 
        if ( particleClass == "Meson" ) {
          return fParticleSelector["mesons"]; 
        }
        else if ( particleClass == "Baryon" ){
          return fParticleSelector["baryons"]; 
        }
      }
      else {
        return fParticleSelector["ions"]; 
      }
    }

    return fParticleSelector["others"]; 
  }

  SLArEveDisplay::~SLArEveDisplay()
  { 
    if (fHitFile) {
      fHitFile->Close(); 
      delete fHitFile;
    }

  }

  int SLArEveDisplay::LoadHitFile(const TString file_path, const TString tree_key) {
    if (fHitFile) {
      fHitFile->Close();
      fHitTree = nullptr;
    }
    fHitFile = new TFile( file_path ); 
    fHitTree = fHitFile->Get<TTree>( tree_key ); 
    if (fHitTree) {
      fLastEvent = fHitTree->GetEntries() - 1; 
      fHitTree->SetBranchAddress("hit_tpc"  , &fHitVars.hit_tpc); 
      fHitTree->SetBranchAddress("hit_x"    , &fHitVars.hit_x); 
      fHitTree->SetBranchAddress("hit_y"    , &fHitVars.hit_y); 
      fHitTree->SetBranchAddress("hit_z"    , &fHitVars.hit_z); 
      fHitTree->SetBranchAddress("hit_q"    , &fHitVars.hit_q); 
      fHitTree->SetBranchAddress("hit_qtrue", &fHitVars.hit_qtrue); 
    }

    return 0;
  }

  int SLArEveDisplay::LoadTrackFile(const TString file_path, const TString tree_key) {
    if (fMCTruthFile) {
      fMCTruthFile->Close();
      fMCTruthTree = nullptr;
    }
    fMCTruthFile = new TFile( file_path ); 
    fMCTruthTree = fMCTruthFile->Get<TTree>( tree_key ); 
    if (fHitTree) {
      fMCTruthTree->SetBranchAddress("MCEvent"  , &fMCEvent); 
    }

    return 0;
  }

  void SLArEveDisplay::Configure(const rapidjson::Value& config) {

    assert( config.HasMember("TPC") ); 
    if ( config["TPC"].IsObject() ) {
      ConfigureTPC( config["TPC"] );
    }
    else if (config["TPC"].IsArray()) {
      for (const auto& jtpc : config["TPC"].GetArray()) {
        ConfigureTPC( jtpc );
      }
    }

    return;
  }

  void SLArEveDisplay::ConfigureTPC(const rapidjson::Value& tpc_config) {

    assert( tpc_config.HasMember("copyID") );
    assert( tpc_config.HasMember("position") ); 
    assert( tpc_config.HasMember("dimensions") ); 

    GeoTPC_t geo_tpc;

    geo_tpc.fID = tpc_config["copyID"].GetInt();

    const auto& jpos = tpc_config["position"].GetObj(); 
    double pos_unit = unit::Unit2Val( jpos["unit"] );
    geo_tpc.fPosition.SetX( jpos["xyz"].GetArray()[0].GetDouble() * pos_unit ); 
    geo_tpc.fPosition.SetY( jpos["xyz"].GetArray()[1].GetDouble() * pos_unit ); 
    geo_tpc.fPosition.SetZ( jpos["xyz"].GetArray()[2].GetDouble() * pos_unit ); 

    const auto& jdims = tpc_config["dimensions"].GetArray(); 
    for (const auto& jdim : jdims) {
      TString var_name = jdim["name"].GetString();
      if      ( var_name == "tpc_x") geo_tpc.fDimension.SetX(unit::ParseJsonVal( jdim )); 
      else if ( var_name == "tpc_y") geo_tpc.fDimension.SetY(unit::ParseJsonVal( jdim )); 
      else if ( var_name == "tpc_z") geo_tpc.fDimension.SetZ(unit::ParseJsonVal( jdim )); 
    }

    geo_tpc.fVolume = std::make_unique<TEveFrameBox>();
    geo_tpc.fVolume->SetAABoxCenterHalfSize( 
        geo_tpc.fPosition.x(), geo_tpc.fPosition.y(), geo_tpc.fPosition.z(), 
        0.5*geo_tpc.fDimension.x(), 0.5*geo_tpc.fDimension.y(), 0.5*geo_tpc.fDimension.z()); 
    geo_tpc.fVolume->SetFrameColor( kGray+2 ); 
    //printf("Adding TPC at (%.2f %.2f, %.2f) with size (%.2f %.2f, %.2f)\n\n", 
        //geo_tpc.fPosition.x(), geo_tpc.fPosition.y(), geo_tpc.fPosition.z(), 
        //0.5*geo_tpc.fDimension.x(), 0.5*geo_tpc.fDimension.y(), 0.5*geo_tpc.fDimension.z()); 

    auto diff = geo_tpc.fPosition - 0.5*geo_tpc.fDimension;
    auto sum  = geo_tpc.fPosition + 0.5*geo_tpc.fDimension;

    if (diff.x() < fXmin) {
      if (diff.x() < 0 ) fXmin = 1.1*(diff.x());
      else               fXmin = 0.9*(diff.x());
    } 
    if (diff.y() < fYmin) {
      if (diff.y() < 0 ) fYmin = 1.1*(diff.y());
      else               fYmin = 0.9*(diff.y());
    } 
    if (diff.z() < fZmin) {
      if (diff.z() < 0 ) fZmin = 1.1*(diff.z());
      else               fZmin = 0.9*(diff.z());
    } 

    if (sum.x() < fXmax) {
      if (sum.x() < 0 ) fXmax = 1.1*(sum.x());
      else              fXmax = 0.9*(sum.x());
    } 
    if (sum.y() < fYmax) {
      if (sum.y() < 0 ) fYmax = 1.1*(sum.y());
      else              fYmax = 0.9*(sum.y());
    } 
    if (sum.z() < fZmax) {
      if (sum.z() < 0 ) fZmax = 1.1*(sum.z());
      else              fZmax = 0.9*(sum.z());
    } 

    fTPCs.push_back( std::move(geo_tpc) ); 

    auto hit_set = std::make_unique<TEveBoxSet>();
    hit_set->SetNameTitle(Form("hitsTPC%i", geo_tpc.fID), Form("TPC %i hits", geo_tpc.fID));

    fEveManager->AddElement( hit_set.get() ); 
    fHitSet.push_back( std::move(hit_set) ); 

    return;
  }

  int SLArEveDisplay::ReDraw() {
    auto top = fEveManager->GetCurrentEvent();

    size_t i = 0; 
    for (auto& hitset : fHitSet) {
      hitset->SetFrame( fTPCs.at(i).fVolume.get() );
      i++;
    }

    for (auto& track_list : fTrackLists) {
      fEveManager->AddElement( track_list.get() );
    }

    fEveManager->GetEditor(); 

    fEveManager->Redraw3D( false, true ); 

    return 0;
  }

  int SLArEveDisplay::ReadHits() {
    fHitTree->GetEntry( fCurEvent );

    float q_max = 0;
    
    for (size_t ihit = 0; ihit < fHitVars.hit_tpc->size(); ihit++) {
      int tpc_idx = GetTPCindex( fHitVars.hit_tpc->at(ihit) ); 
      fHitSet.at(tpc_idx)->AddBox( 
          fHitVars.hit_x->at(ihit), fHitVars.hit_y->at(ihit), fHitVars.hit_z->at(ihit) ); 
      fHitSet.at(tpc_idx)->DigitValue( fHitVars.hit_q->at(ihit) ); 
      //printf("adding hit at (%.2f, %.2f, %.2f mm) with q = %g\n", 
          //fHitVars.hit_x->at(ihit), fHitVars.hit_y->at(ihit), fHitVars.hit_z->at(ihit), 
          //fHitVars.hit_q->at(ihit));
      if (fHitVars.hit_q->at(ihit) > q_max) q_max = fHitVars.hit_q->at(ihit);
    }

    fPalette->SetMax(1.1*q_max); 
    fPalette->SetMin(1500); 
  
    for (auto &hitset : fHitSet) {
      hitset->RefitPlex(); 
      hitset->SetDefDepth(4.0); 
      hitset->SetDefWidth(4.0); 
      hitset->SetDefHeight(4.0); 
      hitset->SetPalette( fPalette.get() ); 
    }


    return 0;
  }

  int SLArEveDisplay::ReadTracks() {
    fMCEvent->Reset();
    fMCTruthTree->GetEntry( fCurEvent );

    const auto& primaries = fMCEvent->GetPrimaries();

    for (const auto& p : primaries) {
      auto track_list = std::unique_ptr<TEveTrackList>( 
          new TEveTrackList(Form("%s_%i", p.GetName(), p.GetTrackID())) ); 
      auto propagator = track_list->GetPropagator();
      propagator->SetMaxZ(1e4); 
      propagator->SetMaxR(1e4); 
      const auto& trajectories = p.GetConstTrajectories(); 
      double p_tot = 0.0; 
      for (const auto& p_ : p.GetMomentum()) p_tot += TMath::Sq( p_ ); 
      p_tot = sqrt(p_tot); 
      printf("%s - vertex @ [%.2f, %.2f, %.2f] m, t = %g ns, direction = [%.2f, %.2f, %.2f]\n", 
          p.GetName(),
          p.GetVertex().at(0), p.GetVertex().at(1), p.GetVertex().at(2), p.GetTime(),
          p.GetMomentum().at(0) / p_tot,  p.GetMomentum().at(1) / p_tot,  p.GetMomentum().at(2) / p_tot); 

      for (const auto& t : trajectories) {
        const int pdg_code = t->GetPDGID();
        const auto& selector = get_particle_selection( pdg_code ); 

        if ( selector.fIsEnabled == false ) continue;

        if ( t->GetInitKineticEne() < selector.fLowerEnergyThreshold ) continue;

        const auto& points = t->GetConstPoints();
        const auto& vertex = points.front();
        
        auto pdgDB = TDatabasePDG::Instance(); 
        TParticlePDG* pdgP = pdgDB->GetParticle( pdg_code );

        TParticle* particle = new TParticle(); 
        if (pdgP) particle->SetPdgCode( pdg_code ); 
        particle->SetProductionVertex( vertex.fX, vertex.fY, vertex.fZ, t->GetTime() );
        if (t->GetParentID() == t->GetTrackID() ) {
          particle->SetFirstMother(-1);
        }
        else {
          particle->SetFirstMother( t->GetParentID() ); 
        }

        auto track = new TEveTrack(particle, t->GetTrackID(), propagator);
        if (pdgP) track->SetCharge( pdgP->Charge() ); 
        
        Long64_t istep = 0;
        for (const auto& step : points) {
          TEveVectorF v( step.fX, step.fY, step.fZ );
          //printf("adding steppoint at [%.2f, %.2f, %.2f] mm - t %g ns\n", v.fX, v.fY, v.fZ, t->GetTime()); 
          if (istep%10 == 0) {
            auto pm = new TEvePathMarkF(TEvePathMarkF::kReference, v, t->GetTime());
            track->AddPathMark( *pm );
          }
        }
        track->ComputeBBox();
        
        //set_track_style( track ); 
        track->SetLineColor( selector.fTrackColor ); 
        track->SetLineStyle( selector.fTrackStyle ); 
        
        track->SetName( Form("%s_%i", t->GetParticleName().Data(), t->GetTrackID()) );

        track_list->AddElement( track );
      } //-- trajectories loop

      track_list->MakeTracks();
      fTrackLists.push_back( std::move(track_list) ); 
    } //-- primaries loop
    return 0;
  }

  void SLArEveDisplay::set_track_style( TEveTrack* track ) {
    if ( fabs(track->GetPdg()) == 13 ) { // muons
      track->SetLineColor( kOrange ); 
    }
    else if (fabs(track->GetPdg()) == 11) { // electrons
      track->SetLineColor(kOrange+7);
    }
    else if ( track->GetPdg() == 22 ) { // gamms
      track->SetLineColor( kYellow-7 );
    } 
    else if (fabs(track->GetPdg()) == 2112) { // protons
      track->SetLineColor(kRed-4);
    } 
    else if (fabs(track->GetPdg()) == 2212) { // neutrons
      track->SetLineColor(kBlue-7);
    } 
    else {
      track->SetLineColor(kViolet-2);
    }
  }

  void SLArEveDisplay::ResetHits() {
    for (auto& hitset : fHitSet) {
      printf("deleting hits...\n");
      hitset->Reset(TEveBoxSet::kBT_AABoxFixedDim, false, hitset->GetNItems());
      //fEveManager->GetViewers()->DeleteAnnotations();
      //fEveManager->GetCurrentEvent()->DestroyElements();
    }

    fTrackLists.clear(); 
    return;
  }

  void SLArEveDisplay::ProcessEvent() {
    printf("display event %lld\n", fCurEvent);

    ResetHits(); 

    ReadTracks();

    ReadHits(); 

    update_entry_label();

    ReDraw();
  }

  void SLArEveDisplay::NextEvent() { 
    fCurEvent = TMath::Max(static_cast<Long64_t>(0), fCurEvent+1); 
    ProcessEvent(); 
  } 

  void SLArEveDisplay::PrevEvent() {
    fCurEvent = TMath::Min(static_cast<Long64_t>(fLastEvent), fCurEvent-1); 
    ProcessEvent(); 
  }

  int SLArEveDisplay::MakeGUI() {
   // Create minimal GUI for event navigation.

   auto browser = fEveManager->GetBrowser();
   browser->StartEmbedding(TRootBrowser::kLeft);

   auto frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
   frmMain->SetWindowName("XX GUI");
   frmMain->SetCleanup(kDeepCleanup);

   auto hf = new TGHorizontalFrame(frmMain);
   TString icondir(TString::Format("%s/icons/", gSystem->Getenv("ROOTSYS")));
   TGPictureButton* b = 0;

   b = new TGPictureButton(hf, gClient->GetPicture(icondir+"GoBack.gif"), fIDs.GetUnID() );
   hf->AddFrame(b, new TGLayoutHints(kLHintsExpandX));
   b->Connect("Clicked()", "display::SLArEveDisplay", this, "PrevEvent()");

   fEnterEntry = new TGNumberEntry(hf, 0, 5, fIDs.GetUnID(),  
       TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, 
       TGNumberFormat::kNELLimitMin);

   fEnterEntry->Connect("ValueSet(Long_t)", "display::SLArEveDisplay", this, "SetEntry()");
   hf->AddFrame(fEnterEntry, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

   b = new TGPictureButton(hf, gClient->GetPicture(icondir+"GoForward.gif"), fIDs.GetUnID());
   hf->AddFrame(b, new TGLayoutHints(kLHintsExpandX));
   b->Connect("Clicked()", "display::SLArEveDisplay", this, "NextEvent()");
   frmMain->AddFrame(hf);

   fGgroupframeParticleSelection = new TGGroupFrame(frmMain, "MC truth selection"); 
   fGframeParticleSelection = new TGVerticalFrame(fGgroupframeParticleSelection);
   fGframeParticleSelection->SetName("MC truth selection");

   size_t isel = 0;
   for (auto& selector : fParticleSelector) {
     fGframeParticleSetting[isel] = new TGHorizontalFrame( fGframeParticleSelection ); 

     fGParticleSelectionButton[isel] = new TGCheckButton(fGframeParticleSetting[isel], 
         new TGHotString(selector.second.fName), fIDs.GetUnID()); 
     fGParticleSelectionButton[isel]->Connect(
         "Toggled(Bool_t)", 
         "display::MCParticleSelector_t", 
         &(selector.second), "ToggleEnable()");

     fGParticleEnergyThreshold[isel] = new TGNumberEntry(fGframeParticleSetting[isel], 0, 5, fIDs.GetUnID(), 
         TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, 0, 1000 );
     selector.second.fEntryForm = fGParticleEnergyThreshold[isel]; 
     fGParticleEnergyThreshold[isel]->SetNumber( selector.second.fLowerEnergyThreshold ); 
     fGParticleEnergyThreshold[isel]->Connect("ValueSet(Long_t)", 
         "display::MCParticleSelector_t", 
         &(selector.second), "SetLowerEnergyDisplayThreshold()");
     EButtonState button_state; 
     if ( selector.second.fIsEnabled ) {
       button_state = EButtonState::kButtonDown;
     }
     else {
       button_state = EButtonState::kButtonUp;
     }

     fGParticleSelectionButton[isel]->SetState( button_state ); 

     fGframeParticleSetting[isel]->AddFrame( fGParticleSelectionButton[isel], 
         new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 1, 1, 2, 2) ); 
     fGframeParticleSetting[isel]->AddFrame( fGParticleEnergyThreshold[isel], 
         new TGLayoutHints(kLHintsRight|kLHintsCenterY, 0, 0, 2, 2) ); 

     fGframeParticleSetting[isel]->MapSubwindows();
     fGframeParticleSetting[isel]->Resize();  
     fGframeParticleSelection->AddFrame(fGframeParticleSetting[isel],
         new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 1, 1, 1, 1) );
     isel++;
   }
   
   auto button_update = new TGTextButton(fGframeParticleSelection, "&Update", fIDs.GetUnID());
   button_update->Connect("Clicked()", "display::SLArEveDisplay", this, "ProcessEvent()");
   fGframeParticleSelection->AddFrame( button_update, new TGLayoutHints(kLHintsExpandX) ); 
   fGframeParticleSelection->MapSubwindows();
   fGgroupframeParticleSelection->AddFrame( fGframeParticleSelection );  

   frmMain->AddFrame(fGgroupframeParticleSelection);

   frmMain->MapSubwindows();
   frmMain->Resize();
   frmMain->MapWindow();

   browser->StopEmbedding();
   browser->SetTabTitle("Event Control", 0);

   return 1;
  }
}
