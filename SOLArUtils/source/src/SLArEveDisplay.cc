/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEveDisplay
 * @created     : Thursday Apr 11, 2024 16:58:14 CEST
 */

#include "TObject.h"
#include "TKey.h"
#include "TClass.h"
#include "SLArEveDisplay.hh"
#include "geo/SLArUnit.hpp"
#include "event/SLArMCPrimaryInfo.hh"
#include "event/SLArEventTrajectory.hh"
#include "core/SLArDebugUtils.hh"

#include "Math/EulerAngles.h"

#include "TParticle.h"
#include "TParticlePDG.h"
#include "TDatabasePDG.h"
#include "TGeoManager.h"
#include "TEveTrackPropagator.h"
#include "TEvePathMark.h"
#include "TEveVector.h"
#include "TEveTrans.h"
#include "TEveFrameBox.h"
#include "TEveRGBAPalette.h"
#include "TRootBrowser.h"
#include "TEveBrowser.h"
#include "TGeoBBox.h"
#include "TGeoTube.h"

#include "TGTab.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"

#include "TString.h"
#include "TSystem.h"

#include "TStyle.h"
#include <RtypesCore.h>

ClassImp(display::SLArEveDisplay)

namespace display {

  SLArEveDisplay::SLArEveDisplay() 
    : TGMainFrame(nullptr, 800, 800), fHitFile(nullptr), fHitTree(nullptr), fCurEvent(0), fLastEvent(1)
  {
    //gStyle->SetPalette(kSunset);
    fTimer = std::make_unique<TTimer>("gSystem->ProcessEvents();", 50, kFALSE);
    fEveManager = std::unique_ptr<TEveManager>( TEveManager::Create() );
    fPaletteQHits = std::make_unique<TEveRGBAPalette>();
    fPaletteOpHits = std::make_unique<TEveRGBAPalette>();

    fParticleSelector.insert( {"gammas", MCParticleSelector_t("gammas", true, 1.0, kYellow-7, 7)} ); 
    fParticleSelector.insert( {"electrons", MCParticleSelector_t("electrons", true, 1.0, kOrange+7)} ); 
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
    else if ( abs(pdg) == 11) return fParticleSelector["electrons"]; 
    else if ( abs(pdg) == 13 || abs(pdg) == 15) return fParticleSelector["heavy leptons"]; 
    else if ( abs(pdg) == 12 || abs(pdg) == 14 || abs(pdg) == 16) return fParticleSelector["neutrinos"]; 
    else if ( abs(pdg) == 2212 ) return fParticleSelector["protons"]; 
    else if ( abs(pdg) == 2112 ) return fParticleSelector["neutrons"]; 
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
    ResetHits();

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

  int SLArEveDisplay::LoadMCEventFile(const TString file_path, const TString tree_key) {
    if (fMCEventFile) {
      fMCEventFile->Close();
      fMCEventTree = nullptr;
    }

    fMCEventFile = TFile::Open( file_path ); 
    fMCEventTree = fMCEventFile->Get<TTree>( tree_key ); 

    if (fHitTree) {
      if (fMCEventTree->GetBranch("MCTruth") == nullptr) {
        printf("WARNING: branch MCTruth not found in %s\n", file_path.Data());
        fIncludeMCTruth = false;
      }
      else {
        fMCEventTree->SetBranchAddress("MCTruth"  , &fEvMCTruth);
      }

      if (fMCEventTree->GetBranch("EventAnode") == nullptr) {
        printf("WARNING: branch EventAnode not found in %s\n", file_path.Data());
        fIncludeTPCHits = false;
      }
      else {
        fMCEventTree->SetBranchAddress("EventAnode", &fEvAnodeList);
      }

      if (fMCEventTree->GetBranch("EventPDS") == nullptr) {
        printf("WARNING: branch EventPDS not found in %s\n", file_path.Data());
        fIncludeOpHits = false;
      }
      else {
        fMCEventTree->SetBranchAddress("EventPDS"  , &fEvPDSList);
      }
    }

    for (const auto &itr : *fMCEventFile->GetListOfKeys()) {
      TKey* key = static_cast<TKey*>(itr); 
      
      if (strcmp(key->GetClassName(), "SLArCfgAnode") == 0) 
      {
        SLArCfgAnode* cfg_anode = dynamic_cast<SLArCfgAnode*>(key->ReadObj()); 
        const int tpc_id = cfg_anode->GetTPCID(); 
        fCfgAnodes.insert({tpc_id, std::unique_ptr<SLArCfgAnode>(cfg_anode)} );
        const TString name = Form("opHit_%s", cfg_anode->GetName()); 
        const TString titl = Form("TPC %i Anode optical hits", tpc_id);

        fPhotonDetectors.emplace( tpc_id, std::make_unique<TEveBoxSet>(name, titl) );
        fPhotonDetectors.at(tpc_id)->Reset(TEveBoxSet::kBT_AABox, false, 100);
        fLArTarget.fVolume->AddElement( fPhotonDetectors.at(tpc_id).get() );
        printf("addbing box set with key %i\n", tpc_id);
      }
      else if ( strcmp(key->GetClassName(), "SLArCfgBaseSystem<SLArCfgSuperCellArray>") == 0) 
      {
        SLArCfgBaseSystem<SLArCfgSuperCellArray>* cfg_pds = 
          dynamic_cast<SLArCfgBaseSystem<SLArCfgSuperCellArray>*>(key->ReadObj()); 
        fCfgPDS = std::unique_ptr<SLArCfgBaseSystem<SLArCfgSuperCellArray>>( cfg_pds ); 

        for (const auto& wall_cfg_itr : fCfgPDS->GetConstMap()) {
          const TString name = Form("opHit_%s", wall_cfg_itr.second.GetName()); 
          const TString titl = Form("XA wall %i optical hits", wall_cfg_itr.first);
          fPhotonDetectors.emplace(wall_cfg_itr.first, std::make_unique<TEveBoxSet>(name, titl));
          fPhotonDetectors.at(wall_cfg_itr.first)->Reset(TEveBoxSet::kBT_AABox, false, 100);
          fLArTarget.fVolume->AddElement( fPhotonDetectors.at(wall_cfg_itr.first).get() );
          printf("addbing box set with key %i\n", wall_cfg_itr.first);
        }
      }
    }

    return 0;
  }

  void SLArEveDisplay::Configure(const rapidjson::Value& config) {
    if ( config.HasMember("LArTarget") && config["LArTarget"].IsObject() ) {
      try { ConfigureLArTarget( config["LArTarget"] ); }
      catch (const std::exception& e) {
        printf("Error configuring LAr target: %s\n", e.what());
        exit( EXIT_FAILURE );
      }
    }

    debug::require_json_member(config, "TPC");
    if ( config["TPC"].IsObject() ) {
      try {ConfigureTPC( config["TPC"] );} 
      catch (const std::exception& e) {
        printf("Error configuring TPC: %s\n", e.what());
        exit( EXIT_FAILURE );
      }
    }
    else if (config["TPC"].IsArray()) {
      for (const auto& jtpc : config["TPC"].GetArray()) {
        try { ConfigureTPC( jtpc ); }
        catch (const std::exception& e) {
          printf("Error configuring TPC: %s\n", e.what());
          exit( EXIT_FAILURE );
        }
      }
    }

    return;
  }

  void SLArEveDisplay::ConfigureLArTarget(const rapidjson::Value& lar_config) {
    debug::require_json_member(lar_config, "shape");
    debug::require_json_member(lar_config, "dimensions");
    debug::require_json_member(lar_config, "position");
    debug::require_json_member(lar_config, "rot");

    fLArTarget.fShape = string_to_vol_shape(lar_config["shape"].GetString());

    // Position
    const auto& jpos = lar_config["position"].GetObj();
    double pos_unit = (jpos.HasMember("unit")) ? unit::Unit2Val(jpos["unit"]) : 1.0;
    fLArTarget.fPosition.SetX(jpos["xyz"].GetArray()[0].GetDouble() * pos_unit);
    fLArTarget.fPosition.SetY(jpos["xyz"].GetArray()[1].GetDouble() * pos_unit);
    fLArTarget.fPosition.SetZ(jpos["xyz"].GetArray()[2].GetDouble() * pos_unit);

    // Dimensions
    const auto& jdims = lar_config["dimensions"];
    debug::require_json_type(jdims, rapidjson::kArrayType);
    if (fLArTarget.fShape == EVolShape::kBox) {
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "size_x");
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "size_y");
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "size_z");
      for (const auto& jdim : jdims.GetArray()) {
        Double_t d = unit::ParseJsonVal(jdim); 
        TString var_name = jdim["name"].GetString();
        if      ( var_name == "size_x")  fLArTarget.fDimension.SetX( d ); 
        else if ( var_name == "size_y")  fLArTarget.fDimension.SetY( d ); 
        else if ( var_name == "size_z")  fLArTarget.fDimension.SetZ( d ); 
      }
    } else if (fLArTarget.fShape == EVolShape::kTub) {
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "radius");
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "length");
      for (const auto& jdim : jdims.GetArray()) {
        Double_t d = unit::ParseJsonVal(jdim);
        TString var_name = jdim["name"].GetString();
        if      ( var_name == "radius")  fLArTarget.fRadius = d; 
        else if ( var_name == "length")  fLArTarget.fHeight = d;
      }
    }

    // Rotation
    const auto& jrot = lar_config["rot"].GetObj();
    debug::require_json_member(jrot, "val");
    debug::require_json_type(jrot["val"], rapidjson::kArrayType);
    double rot_unit = (jrot.HasMember("unit")) ? unit::Unit2Val(jrot["unit"]) : 1.0;
    auto euler = jrot["val"].GetArray();
    fLArTarget.fRotation = ROOT::Math::EulerAngles(
        euler[0].GetDouble() * rot_unit,
        euler[1].GetDouble() * rot_unit,
        euler[2].GetDouble() * rot_unit
        );

    // Create TGeo shape and transformation
    TGeoShape* shape = nullptr;
    if (fLArTarget.fShape == EVolShape::kBox) {
      shape = new TGeoBBox("lar_box",
          0.5 * fLArTarget.fDimension.x(),
          0.5 * fLArTarget.fDimension.y(),
          0.5 * fLArTarget.fDimension.z());
    } else if (fLArTarget.fShape == EVolShape::kTub) {
      shape = new TGeoTube("lar_cyl", 0, fLArTarget.fRadius, 0.5 * fLArTarget.fHeight);
    }

    // Euler angles to rotation matrix
    TGeoRotation* rot = new TGeoRotation();
    rot->SetAngles(
        fLArTarget.fRotation.Phi() * TMath::RadToDeg(),
        fLArTarget.fRotation.Theta() * TMath::RadToDeg(),
        fLArTarget.fRotation.Psi() * TMath::RadToDeg()
        );
    fLArTarget.fTransform = new TGeoCombiTrans(
        fLArTarget.fPosition.x(), fLArTarget.fPosition.y(), fLArTarget.fPosition.z(), rot);

    // Create TEveGeoShape and apply transformation
    fLArTarget.fVolume = std::make_unique<TEveGeoShape>("LArTarget");
    fLArTarget.fVolume->SetShape(shape);
    fLArTarget.fVolume->SetMainColor(kGray+1);
    fLArTarget.fVolume->SetTransMatrix( *fLArTarget.fTransform );
    fLArTarget.fVolume->SetPickable(kTRUE);
    fLArTarget.fVolume->SetDrawFrame(kTRUE);
    fLArTarget.fVolume->SetMainTransparency( 80 );
    fEveManager->AddElement(fLArTarget.fVolume.get());

    fXmin = fLArTarget.fPosition.x() - 0.7*fLArTarget.fDimension.x();
    fXmax = fLArTarget.fPosition.x() + 0.7*fLArTarget.fDimension.x();
    fYmin = fLArTarget.fPosition.y() - 0.7*fLArTarget.fDimension.y();
    fYmax = fLArTarget.fPosition.y() + 0.7*fLArTarget.fDimension.y();
    fZmin = fLArTarget.fPosition.z() - 0.7*fLArTarget.fDimension.z();
    fZmax = fLArTarget.fPosition.z() + 0.7*fLArTarget.fDimension.z();
  }

  void SLArEveDisplay::MakeTPCBox(const rapidjson::Value& tpc_config, GeoTPC_t& geo_tpc) {
    const auto& jdims = tpc_config["dimensions"].GetArray(); 
    for (const auto& jdim : jdims) {
      TString var_name = jdim["name"].GetString();
      if      ( var_name == "tpc_x") geo_tpc.fDimension.SetX(unit::ParseJsonVal( jdim )); 
      else if ( var_name == "tpc_y") geo_tpc.fDimension.SetY(unit::ParseJsonVal( jdim )); 
      else if ( var_name == "tpc_z") geo_tpc.fDimension.SetZ(unit::ParseJsonVal( jdim )); 
    }

    geo_tpc.fVolume = std::make_unique<TEveGeoShape>( Form("TPC%i", geo_tpc.fID) );
    geo_tpc.fVolume->SetShape( new TGeoBBox(
          0.5*geo_tpc.fDimension.x(), 0.5*geo_tpc.fDimension.y(), 0.5*geo_tpc.fDimension.z()) );

    return;
  }

  void SLArEveDisplay::MakeTPCTub(const rapidjson::Value& tpc_config, GeoTPC_t& geo_tpc) {
    const auto& jdims = tpc_config["dimensions"].GetArray();
    for (const auto& jdim : jdims) {
      TString var_name = jdim["name"].GetString();
      if ( var_name == "tpc_radius" ) geo_tpc.fRadius = unit::ParseJsonVal( jdim ); 
      else if ( var_name == "tpc_height" ) geo_tpc.fHeight = unit::ParseJsonVal( jdim );
    }

    geo_tpc.fVolume = std::make_unique<TEveGeoShape>( Form("TPC%i", geo_tpc.fID) );
    geo_tpc.fVolume->SetShape( new TGeoTube(0, geo_tpc.fRadius, 0.5*geo_tpc.fHeight) );
    return;
  }

  void SLArEveDisplay::ConfigureTPC(const rapidjson::Value& tpc_config) {

    debug::require_json_member(tpc_config, "copyID");
    debug::require_json_member(tpc_config, "position"); 
    debug::require_json_member(tpc_config, "dimensions"); 
    debug::require_json_type( tpc_config["dimensions"], rapidjson::kArrayType );

    GeoTPC_t geo_tpc;

    if ( tpc_config.HasMember("shape") ) {
      geo_tpc.fShape = string_to_vol_shape( tpc_config["shape"].GetString() );
    }
    geo_tpc.fID = tpc_config["copyID"].GetInt();

    if (geo_tpc.fShape == EVolShape::kBox) {
      MakeTPCBox( tpc_config, geo_tpc );
    }
    else if (geo_tpc.fShape == EVolShape::kTub) {
      MakeTPCTub( tpc_config, geo_tpc );
    }
    else {
      throw std::invalid_argument( 
          Form("Unknown TPC shape: %s", tpc_config["shape"].GetString()) );
    }

    const auto& jpos = tpc_config["position"].GetObj(); 
    double pos_unit = unit::Unit2Val( jpos["unit"] );
    geo_tpc.fPosition.SetX( jpos["xyz"].GetArray()[0].GetDouble() * pos_unit ); 
    geo_tpc.fPosition.SetY( jpos["xyz"].GetArray()[1].GetDouble() * pos_unit ); 
    geo_tpc.fPosition.SetZ( jpos["xyz"].GetArray()[2].GetDouble() * pos_unit ); 

    geo_tpc.fVolume->SetMainColor( kGray+2 ); 
    geo_tpc.fVolume->SetMainTransparency( 90 ); 
  
    //printf("Adding TPC at (%.2f %.2f, %.2f) with size (%.2f %.2f, %.2f)\n\n", 
        //geo_tpc.fPosition.x(), geo_tpc.fPosition.y(), geo_tpc.fPosition.z(), 
        //0.5*geo_tpc.fDimension.x(), 0.5*geo_tpc.fDimension.y(), 0.5*geo_tpc.fDimension.z()); 


    // Create a TGeoCombiTrans for the TPC position (relative to detector)
    geo_tpc.fTransform = new TGeoCombiTrans(
        geo_tpc.fPosition.x(), geo_tpc.fPosition.y(), geo_tpc.fPosition.z(), fLArTarget.fTransform->GetRotation() );

    auto hit_set = std::make_unique<TEveBoxSet>();
    hit_set->SetNameTitle(Form("hitsTPC%i", geo_tpc.fID), Form("TPC %i hits", geo_tpc.fID));

    fHitSet.push_back( std::move(hit_set) ); 
    geo_tpc.fVolume->SetTransMatrix( *geo_tpc.fTransform );
    geo_tpc.fVolume->AddElement( fHitSet.back().get() );

    // Add TPC as child of detector volume
    fLArTarget.fVolume->AddElement(geo_tpc.fVolume.get());

    fTPCs.push_back( std::move(geo_tpc) ); 

    return;
  }

  int SLArEveDisplay::ReDraw() {
    auto top = fEveManager->GetCurrentEvent();

    size_t i = 0; 
/*
 *    for (auto& hitset : fHitSet) {
 *      hitset->SetFrame( fTPCs.at(i).fVolume.get() );
 *      i++;
 *    }
 *
 *    for (auto& track_list : fTrackLists) {
 *      fEveManager->AddElement( track_list.get() );
 *    }
 *
 *    for (auto& ophit_set : fPhotonDetectors) {
 *      fEveManager->AddElement( ophit_set.second.get() ); 
 *    }
 *    
 *    fEveManager->GetEditor(); 
 *
 */

    fEveManager->Redraw3D( false, true ); 

    return 0;
  }

  int SLArEveDisplay::ReadHits() {
    fHitTree->GetEntry( fCurEvent );
    float q_max = 0;
    Double_t xtpc[3] = {};
    Double_t xlar[3] = {};
    Double_t xglob[3] = {};

    for (size_t ihit = 0; ihit < fHitVars.hit_tpc->size(); ihit++) {
      int tpc_idx = GetTPCindex( fHitVars.hit_tpc->at(ihit) ); 
      xtpc[0] = fHitVars.hit_x->at(ihit);
      xtpc[1] = fHitVars.hit_y->at(ihit);
      xtpc[2] = fHitVars.hit_z->at(ihit);
      const double* tpc_pos = fTPCs.at(tpc_idx).fTransform->GetTranslation();
      xlar[0] = xtpc[0] - tpc_pos[0];
      xlar[1] = xtpc[1] - tpc_pos[1];
      xlar[2] = xtpc[2] - tpc_pos[2];

      fLArTarget.fTransform->LocalToMaster(xlar, xglob);
      fHitSet.at(tpc_idx)->AddBox( xglob[0], xglob[1], xglob[2] ); 
      fHitSet.at(tpc_idx)->DigitValue( fHitVars.hit_q->at(ihit) ); 
      printf("xtpc: (%.2f, %.2f, %.2f) -> xlar: (%.2f, %.2f, %.2f) -> xglob: (%.2f, %.2f, %.2f) with q = %g\n", 
          xtpc[0], xtpc[1], xtpc[2], 
          xlar[0], xlar[1], xlar[2], 
          xglob[0], xglob[1], xglob[2], 
          fHitVars.hit_q->at(ihit));
      //printf("adding hit at (%.2f, %.2f, %.2f mm) with q = %g\n", 
          //fHitVars.hit_x->at(ihit), fHitVars.hit_y->at(ihit), fHitVars.hit_z->at(ihit), 
          //fHitVars.hit_q->at(ihit));
      if (fHitVars.hit_q->at(ihit) > q_max) q_max = fHitVars.hit_q->at(ihit);
    }

    fPaletteQHits->SetMax(1.1*q_max); 
    fPaletteQHits->SetMin(1500); 
  
    for (auto &hitset : fHitSet) {
      hitset->RefitPlex(); 
      hitset->SetDefDepth(4.0); 
      hitset->SetDefWidth(4.0); 
      hitset->SetDefHeight(4.0); 
      hitset->SetPalette( fPaletteQHits.get() ); 
    }

    return 0;
  }

  int SLArEveDisplay::ReadMCTruth() {
    if (fEvMCTruth) fEvMCTruth->Reset();
    if (fEvAnodeList) fEvAnodeList->Reset();
    if (fEvPDSList) fEvPDSList->Reset();

    fMCEventTree->GetEntry( fCurEvent ); 

    if (fIncludeMCTruth) {
      printf("reading MCTruth\n");
      ReadTracks();
    }

    if (fIncludeOpHits) {
      ReadOpHits();
    }

    return 0;
  }

  int SLArEveDisplay::ReadOpHitsFromOpDetArray(const int idx_array, const SLArEventSuperCellArray& ev_opdet_array)
  {
    const auto& cfg_wall = fCfgPDS->GetBaseElement(idx_array); 
    const ROOT::Math::EulerAngles rot( cfg_wall.GetPhi(), cfg_wall.GetTheta(), cfg_wall.GetPsi() ); 
    const ROOT::Math::EulerAngles rrot = rot.Inverse();
    Double_t xlar[3] = {};
    Double_t xglob[3] = {};
    Double_t xsize[3] = {};

    auto& hitset = fPhotonDetectors.at(idx_array);

    int nhit_max = 0;

    for (const auto& ev_xa_itr : ev_opdet_array.GetConstSuperCellMap()) {
      const auto& idx_xa = ev_xa_itr.first; 
      const auto& ev_xa = ev_xa_itr.second;

      const auto& cfg_xa = cfg_wall.GetBaseElement(idx_xa); 

      int nhit = ev_xa.GetNhits();
      if (nhit > nhit_max) nhit_max = nhit;

      const ROOT::Math::XYZVectorD pos = {cfg_xa.GetPhysX(), cfg_xa.GetPhysY(), cfg_xa.GetPhysZ() }; 
      const ROOT::Math::XYZVectorD size = {cfg_xa.GetSizeX(), cfg_xa.GetSizeY(), cfg_xa.GetSizeZ()}; 
      ROOT::Math::XYZVectorD size_rot = rrot*size;
      size_rot.SetXYZ( fabs(size_rot.x()), fabs(size_rot.y()), fabs(size_rot.z()) ); 
      size_rot.GetCoordinates( xlar ); 
      fLArTarget.fTransform->LocalToMaster(xlar, xsize);
      const ROOT::Math::XYZVectorD pos_center = pos - 0.5*size_rot;
      pos_center.GetCoordinates( xlar );
      fLArTarget.fTransform->LocalToMaster(xlar, xglob);
      printf("[%i] Adding box at (%.0f, %.0f, %.0f) with size [%.0f, %.0f, %.0f]: digi val: %i\n", idx_array,
          xglob[0], xglob[1], xglob[2], xsize[0], xsize[1], xsize[2], nhit);

      hitset->AddBox(xglob[0], xglob[1], xglob[2], xsize[0], xsize[1], xsize[2] );
      hitset->DigitValue( nhit );
    }
    hitset->RefitPlex(); 
    hitset->SetPickable(1);
    hitset->SetAlwaysSecSelect(1);


    return nhit_max;
  }

  int SLArEveDisplay::ReadOpHitsFromAnode(const int tpc_id, const SLArEventAnode& ev_anode) 
  {
    int nhit_max = 0;
    auto& cfg_anode = fCfgAnodes.at(tpc_id);
    const auto tpc_index = GetTPCindex(tpc_id);

    const ROOT::Math::EulerAngles rot(cfg_anode->GetPhi(), cfg_anode->GetTheta(), cfg_anode->GetPsi()); 
    const ROOT::Math::EulerAngles rrot = rot.Inverse();

    const ROOT::Math::EulerAngles lar_rot = fLArTarget.fRotation.Inverse();

    Double_t xlar[3] = {};
    Double_t xglob[3] = {};

    for (const auto& ev_mt_itr : ev_anode.GetConstMegaTilesMap()) {
      const auto& idx_mt = ev_mt_itr.first;
      const auto& ev_mt = ev_mt_itr.second;

      if (ev_mt.GetNPhotonHits() == 0) continue;

      auto& cfg_mt = cfg_anode->GetBaseElement(idx_mt);

      auto& hitset = fPhotonDetectors.at(tpc_id);

      for (const auto& ev_t_itr : ev_mt.GetConstTileMap()) {
        const auto& idx_t = ev_t_itr.first;
        const auto& ev_t = ev_t_itr.second;

        if (ev_t.GetNhits() == 0) continue;

        auto& cfg_t = cfg_mt.GetBaseElement(idx_t); 

        int nhit = ev_t.GetNhits();
        if (nhit > nhit_max) nhit_max = nhit;

        const ROOT::Math::XYZVectorD t_size = {cfg_t.GetSizeX(), cfg_t.GetSizeY(), cfg_t.GetSizeZ()}; 
        ROOT::Math::XYZVectorD size_rot = rrot*t_size;
        size_rot.SetXYZ( fabs(size_rot.x()), fabs(size_rot.y()), fabs(size_rot.z()) );
        ROOT::Math::XYZVectorD size_rot_lar = lar_rot*size_rot;
        size_rot_lar.SetXYZ( fabs(size_rot_lar.x()), fabs(size_rot_lar.y()), fabs(size_rot_lar.z()) );

        const ROOT::Math::XYZVectorD t_pos = {cfg_t.GetPhysX(), cfg_t.GetPhysY(), cfg_t.GetPhysZ() }; 
        const ROOT::Math::XYZVectorD& tpc_pos = fTPCs[tpc_index].fPosition;
        const ROOT::Math::XYZVectorD t_pos_lar = lar_rot*t_pos;

        //printf("t_pos: (%.1f, %.1f, %.1f), tpc_pos: (%.1f, %.1f, %.1f)\n", 
            //t_pos.x(), t_pos.y(), t_pos.z(), tpc_pos.x(), tpc_pos.y(), tpc_pos.z());
        //printf("t_size: (%.1f, %.1f, %.1f), size_rot: (%.1f, %.1f, %.1f), size_lar: (%.2f, %.2f, %.2f)\n", 
            //t_size.x(), t_size.y(), t_size.z(), size_rot.x(), size_rot.y(), size_rot.z(), 
            //size_rot_lar.x(), size_rot_lar.y(), size_rot_lar.z() );

        const ROOT::Math::XYZVectorD lar_pos = t_pos_lar - 0.5*size_rot_lar;
        size_rot *= 0.95;
        //printf("xlar: (%.1f, %.1f, %.1f), xglob: (%.1f, %.1f, %.1f)\n", 
             //xlar[0], xlar[1], xlar[2], xglob[0], xglob[1], xglob[2]);
        //printf("[%i] Adding box at (%.0f, %.0f, %.0f) with size [%.0f, %.0f, %.0f]: digi val: %i\n", tpc_id,
            //xglob[0], xglob[1], xglob[2], size_rot.x(), size_rot.y(), size_rot.z(), nhit);

        hitset->AddBox(lar_pos.x(), lar_pos.y(), lar_pos.z(), size_rot_lar.x(), size_rot_lar.y(), size_rot_lar.z() );
        hitset->DigitValue( nhit); 
      }
      hitset->RefitPlex(); 
      hitset->SetPickable(1);
      hitset->SetAlwaysSecSelect(1);
    }

    return nhit_max;
  }

  int SLArEveDisplay::ReadOpHits() {
    int nhit_max = 0; 

    if (fEvAnodeList != nullptr) 
    {
      const auto& ev_anodes = fEvAnodeList->GetAnodeMap(); 
      for (const auto& ev_anode_itr : ev_anodes) {
        int nhit = ReadOpHitsFromAnode( ev_anode_itr.first, ev_anode_itr.second );
        if (nhit > nhit_max) nhit_max = nhit;
      }
    }

    if (fEvPDSList != nullptr) 
    {
      const auto& ev_pds = fEvPDSList->GetOpDetArrayMap(); 
      for (const auto& ev_wall_itr : ev_pds) {
        if (ev_wall_itr.second.GetNhits() == 0) continue;
        int nhit = ReadOpHitsFromOpDetArray( ev_wall_itr.first, ev_wall_itr.second );
        if (nhit > nhit_max) nhit_max = nhit;
      }
    }

    fPaletteOpHits->SetLimitsScaleMinMax(0, 1.2*nhit_max);
    for (auto& ophitset_itr : fPhotonDetectors) {
      ophitset_itr.second->SetPalette( fPaletteOpHits.get() ); 
    }

    return 1;
  }

  int SLArEveDisplay::ReadTracks() {
    const auto& primaries = fEvMCTruth->GetPrimaries();
    printf("SLArEveDisplay::ReadTracks - found %zu primaries\n", primaries.size());

    TGeoCombiTrans trans( *fLArTarget.fTransform );

    for (const auto& p : primaries) {
      auto track_list = std::unique_ptr<TEveTrackList>( 
          new TEveTrackList(Form("%s_%i", p.GetName(), p.GetTrackID())) ); 
      auto propagator = track_list->GetPropagator();
      propagator->SetMaxZ(1e5); 
      propagator->SetMaxR(1e5); 
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
        double vlar[3] = { vertex.fX, vertex.fY, vertex.fZ };
        double vglob[3] = {}; 
        trans.LocalToMaster(vlar, vglob);
        
        TEveVectorF v(vglob);
        
        auto pdgDB = TDatabasePDG::Instance(); 
        TParticlePDG* pdgP = pdgDB->GetParticle( pdg_code );

        TParticle* particle = new TParticle(); 
        if (pdgP) particle->SetPdgCode( pdg_code ); 
        particle->SetProductionVertex( v.fX, v.fY, v.fZ, t->GetTime() );
        if (t->GetParentID() == t->GetTrackID() ) {
          particle->SetFirstMother(-1);
        }
        else {
          particle->SetFirstMother( t->GetParentID() ); 
        }

        auto track = new TEveTrack(particle, t->GetTrackID(), propagator);
        if (pdgP) track->SetCharge( pdgP->Charge() ); 
        
        Long64_t istep = 0;
        for (auto it = points.begin(); it != points.end(); ++it) {
          const auto& step = *it;
          vlar[0] = step.fX; 
          vlar[1] = step.fY;
          vlar[2] = step.fZ;
          trans.LocalToMaster(vlar, vglob);
          if (istep%10 == 0 || (it == points.end()-1) ) {
            auto pm = new TEvePathMarkF(TEvePathMarkF::kReference, TEveVectorF(vglob[0], vglob[1], vglob[2]), t->GetTime());
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
      fLArTarget.fVolume->AddElement( fTrackLists.back().get() );
    } //-- primaries loop
    return 0;
  }

  void SLArEveDisplay::set_track_style( TEveTrack* track ) {
    if ( abs(track->GetPdg()) == 13 ) { // muons
      track->SetLineColor( kOrange ); 
    }
    else if (abs(track->GetPdg()) == 11) { // electrons
      track->SetLineColor(kOrange+7);
    }
    else if ( track->GetPdg() == 22 ) { // gamms
      track->SetLineColor( kYellow-7 );
    } 
    else if (abs(track->GetPdg()) == 2112) { // protons
      track->SetLineColor(kRed-4);
    } 
    else if (abs(track->GetPdg()) == 2212) { // neutrons
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

    for (auto& hitset_itr : fPhotonDetectors) {
      printf("deleting ophits...\n");
      hitset_itr.second->Reset(TEveBoxSet::kBT_AABox, false, 100);
    }

    fTrackLists.clear();

    return;
  }

  void SLArEveDisplay::ProcessEvent() {
    printf("display event %lld\n", fCurEvent);

    ResetHits(); 

    ReadMCTruth(); 

    if (fIncludeTPCHits) ReadHits(); 

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
