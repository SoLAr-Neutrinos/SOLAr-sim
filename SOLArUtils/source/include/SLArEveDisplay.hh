/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEveDisplay.hh
 * @created     : Thursday Apr 11, 2024 16:47:34 CEST
 */

#ifndef SLAREVEDISPLAY_HH

#define SLAREVEDISPLAY_HH

#include <cstddef>
#include <iostream>
#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"

#include "TGLabel.h"
#include "TGNumberEntry.h"
#include "TEveManager.h"
#include "TEveBoxSet.h"
#include "TEveManager.h"
#include "TEveEventManager.h"
#include "TEveViewer.h"
#include "TEveFrameBox.h"
#include "TEveGeoShape.h"
#include "TEveTrack.h"
#include "TRootEmbeddedCanvas.h"
#include "Math/Vector3D.h"
#include "Math/Point3D.h"
#include "Math/EulerAngles.h"
#include "TTimer.h"
#include "TGeoManager.h"

#include "memory"
#include "rapidjson/document.h"

#include "SLArRecoHits.hh"

#include "event/SLArMCTruth.hh"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"

#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgBaseSystem.hh"

namespace display {
  enum class EVolShape { kBox, kTub };

  inline EVolShape string_to_vol_shape(const std::string& shape_str) {
    if (shape_str == "box") {
      return EVolShape::kBox;
    } else if (shape_str == "tub") {
      return EVolShape::kTub;
    } else {
      throw std::invalid_argument("Unknown volume shape: " + shape_str);
    }
  }

  inline TString vol_shape_to_string(const EVolShape shape) {
    switch (shape) {
      case EVolShape::kBox: return "box";
      case EVolShape::kTub: return "tub";
      default: return "unknown";
    }
  }

  /**
   * @class GeoTPC_t
   * @brief Basic geometry attributes of TPC volume
   */
  struct GeoTPC_t {
    std::unique_ptr<TEveGeoShape> fVolume = {};
    ROOT::Math::XYZVectorD fPosition = {};
    ROOT::Math::XYZVectorD fDimension = {};
    EVolShape fShape = EVolShape::kBox;
    Double_t fRadius = 0; // for cylindrical TPCs
    Double_t fHeight = 0; // for cylindrical TPCs
    Int_t fID = {};
    TGeoCombiTrans* fTransform = nullptr; // for hierarchical placement
  };


  struct GeoLArTarget_t {
    std::unique_ptr<TEveGeoShape> fVolume;
    ROOT::Math::XYZVectorD fPosition = {};
    ROOT::Math::XYZVectorD fDimension = {};
    ROOT::Math::EulerAngles fRotation = {};
    EVolShape fShape = EVolShape::kBox;
    double fRadius = 0; // for cylinder
    double fHeight = 0; // for cylinder
    TGeoCombiTrans* fTransform = nullptr; // for hierarchical placement
  };

  struct MCParticleSelector_t {
    TString fName = {}; 
    Bool_t fIsEnabled = {};
    double fLowerEnergyThreshold = 0.0; 
    Color_t fTrackColor = kBlack; 
    Int_t fTrackStyle = 1; 
    TGNumberEntry* fEntryForm = {};

    inline MCParticleSelector_t() : fName(""), fIsEnabled(false), fLowerEnergyThreshold(0.0) {}
    inline MCParticleSelector_t(
        const char* name, const bool is_on, const double thrs, const Color_t col, const int style = 1) : 
      fName(name), fIsEnabled(is_on), fLowerEnergyThreshold( thrs ), 
      fTrackColor(col), fTrackStyle(style) {}

    inline void ToggleEnable() {
      (fIsEnabled) ? fIsEnabled = false : fIsEnabled = true;

      if (fIsEnabled) {
        printf("%s display enabled\n", fName.Data()); 
      }
      else {
        printf("%s display disabled\n", fName.Data()); 
      }
      return;
    }

    inline void SetLowerEnergyDisplayThreshold(const double val) {
      fLowerEnergyThreshold = val; 
      printf("[%s] low energy threshold for display at %g MeV\n", fName.Data(), fLowerEnergyThreshold); 
      return;
    }
    inline void SetLowerEnergyDisplayThreshold() {
      if ( fEntryForm == nullptr ) return;
      else {
        Double_t val = fEntryForm->GetNumberEntry()->GetNumber(); 
        SetLowerEnergyDisplayThreshold(val); 
      }
    }
  }; 

  class IDList {
    private:
      Int_t nID;   // creates unique widget's IDs

    public:
      IDList() : nID(0) {}
      ~IDList() {}
      Int_t GetUnID(void) { return ++nID; }
  };

  class SLArEveDisplay : public TGMainFrame {
    public: 
      SLArEveDisplay();
      ~SLArEveDisplay();

      int LoadHitFile(const TString file_path, const TString tree_key); 
      int LoadMCEventFile(const TString file_path, const TString tree_key);

      void Configure(const rapidjson::Value& config); 
      int  MakeGUI(); 
      int  ReadHits(); 
      int  ReadMCTruth();
      int  ReadTracks();
      int  ReadOpHits();
      std::vector<int>  ReadOpHitsFromOpDetArray(const int idx_array, const SLArEventSuperCellArray& ev_opdet_array); 
      std::vector<int>  ReadOpHitsFromAnode(const int tpc_id, const SLArEventAnode& ev_anode);
      void ResetHits();  
      int  ReDraw(); 
      void NextEvent();
      void PrevEvent();
      void ProcessEvent(); 

      inline void SetEntry() {
        fCurEvent = fEnterEntry->GetNumberEntry()->GetIntNumber();
        ProcessEvent();
      }

      inline void SetEntry(const Long64_t iev) {
        fCurEvent = iev;
        ProcessEvent();
      }

      inline void ToggleModeNHitsTime() {
        for (auto& itr : fPhotonDetectorsNHits) {
          bool showHits = !itr.second->GetRnrSelf();
          itr.second->SetRnrSelf(showHits);
          fPhotonDetectorsTHits.at(itr.first)->SetRnrSelf(!showHits);
        }
        gEve->Redraw3D();
      }

    private: 
      TFile* fHitFile = {};
      TTree* fHitTree = {}; 
      TFile* fMCEventFile = {};
      TTree* fMCEventTree = {};
      reco::hitvarContainerPtr fHitVars = {};
      SLArMCTruth* fEvMCTruth = {};
      SLArListEventAnode* fEvAnodeList = {};
      SLArListEventPDS* fEvPDSList = {};
      bool fIncludeMCTruth = true;
      bool fIncludeTPCHits = true;
      bool fIncludeOpHits = true;
      std::map<int, std::unique_ptr<SLArCfgAnode>> fCfgAnodes = {}; 
      std::unique_ptr<SLArCfgBaseSystem<SLArCfgSuperCellArray>> fCfgPDS = {}; 
      std::unique_ptr<TTimer> fTimer = {};
      std::unique_ptr<TEveManager> fEveManager = {};
      std::vector<std::unique_ptr<TEveBoxSet>> fHitSet = {};
      std::vector<std::unique_ptr<TEveTrackList>> fTrackLists = {}; 
      std::map<int, std::unique_ptr<TEveBoxSet>> fPhotonDetectorsNHits = {}; 
      std::map<int, std::unique_ptr<TEveBoxSet>> fPhotonDetectorsTHits = {}; 
      std::map<int, std::vector<TH1F>> fPhotonDetectorsHitTimeHists = {};
      TEveTrackPropagator* fPropagator = {};
      std::unique_ptr<TEveRGBAPalette> fPaletteQHits = {};
      std::unique_ptr<TEveRGBAPalette> fPaletteOpHits = {};
      std::unique_ptr<TEveRGBAPalette> fPaletteOpHitsTime = {};
      std::vector<GeoTPC_t> fTPCs;
      GeoLArTarget_t fLArTarget;

      Long64_t  fCurEvent = {};
      Long64_t  fLastEvent = {};

      Float_t fXmin = {}; 
      Float_t fXmax = {}; 
      Float_t fYmin = {}; 
      Float_t fYmax = {}; 
      Float_t fZmin = {}; 
      Float_t fZmax = {}; 

      TGNumberEntry* fEnterEntry = {};
      TRootEmbeddedCanvas* fTimeHistCanvas = {};
      TGGroupFrame*  fGgroupframeParticleSelection = {};
      TGVerticalFrame*  fGframeParticleSelection = {};
      TGHorizontalFrame* fGframeParticleSetting[9] = {};
      TGCheckButton* fGParticleSelectionButton[9] = {};
      TGNumberEntry* fGParticleEnergyThreshold[9] = {};
      TGTextButton* fNhitsTimeToggleButton = nullptr;
      IDList fIDs = {}; 


      std::map<TString, MCParticleSelector_t> fParticleSelector; 

      void ConfigureTPC(const rapidjson::Value& tpc_config);

      void MakeTPCBox(const rapidjson::Value& tpc_config, GeoTPC_t& geo_tpc);

      void MakeTPCTub(const rapidjson::Value& tpc_config, GeoTPC_t& geo_tpc);

      void ConfigureLArTarget(const rapidjson::Value& target_config);

      inline Int_t GetTPCindex(const Int_t itpc) {
        Int_t index = 0;
        for (const auto& tpc : fTPCs) {
          if (itpc == tpc.fID)  return index;
          index++;
        }
        return -1;
      }

      void setup_time_hist(); 

      void set_track_style(TEveTrack* track); 

      const MCParticleSelector_t& get_particle_selection(const int pdg);

      void update_time_histograms();

      inline void update_entry_label() {
        fEnterEntry->SetIntNumber( fCurEvent );
        return;
      }

      ClassDef(display::SLArEveDisplay, 0)
  };
}
#endif /* end of include guard SLAREVEDISPLAY_HH */

