/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFLSPhotonLibrary
 * @created     : Sunday Dec 14, 2025 15:10:57 CET
 */

#ifndef SLARFLSPHOTONLIBRARY_HH

#define SLARFLSPHOTONLIBRARY_HH

#include "SLArFastLightSim.hh"
#include "SLArEventAnode.hh"
#include "SLArEventSuperCellArray.hh"

#include "TFile.h"
#include "TTree.h"

class SLArFLSPhotonLibrary : public SLArFastLightSim {
  typedef std::vector<int> NComponentBase_t;
  public:
    struct PhotonLibraryEntry {
      float x_ = {};
      float y_ = {};
      float z_ = {};
      float vis_tot_ = {}; 
      float vis_dir_ = {};
      float vis_wls_ = {};

      std::map<std::string, std::vector<float>> tilearrayBuffers_;
      std::map<std::string, std::vector<float>> sipmBuffers_;

      inline void Clear() {
        x_ = {};
        y_ = {};
        z_ = {};
        vis_tot_ = {}; 
        vis_dir_ = {};
        vis_wls_ = {};

        for (auto& [key, vec] : tilearrayBuffers_) {
          vec.resize( vec.size(), 0.0f ); 
        }

        for (auto& [key, vec] : sipmBuffers_) {
          vec.resize( vec.size(), 0.0f ); 
        }
      }
    };

    inline SLArFLSPhotonLibrary() : SLArFastLightSim() {
      fType = (SLArFastLightSim::kLUT);
    }
    inline SLArFLSPhotonLibrary(const G4String config_path)
        : fPhotonLibConfigPath(config_path) {}
    inline ~SLArFLSPhotonLibrary() override {
      if (fPhotonLibraryFile) {
        fEntry.Clear();
        fPhotonLibraryFile->Close();
      }
    }

    void Initialize(const rapidjson::Value& config) override;

    void PropagatePhotons(
        const G4ParticleDefinition* particleDef,
        const G4String& volumeName,
        const G4ThreeVector& emissionPoint,
        const int numPhotons,
        const std::vector<double>& emissionTime, 
        const std::vector<double>& emissionEnergy) override;

    inline float GetPDEScaleFactor() const {return fPDEScaleFactor;}
    inline void SetPDEScaleFactor(const float scale_factor) {fPDEScaleFactor = scale_factor;}

    void Print() const override;

  private:
    G4String fPhotonLibConfigPath = {};
    TFile* fPhotonLibraryFile = nullptr;
    TTree* fPhotonLibraryTree = nullptr;
    PhotonLibraryEntry fEntry = {};
    double fVoxelSize[3] = {100.0, 100.0, 100.0}; // in mm
    G4ThreeVector fShift = {};
    Long64_t fNumVoxelsX = 0;
    Long64_t fNumVoxelsY = 0;
    Long64_t fNumVoxelsZ = 0;
    float    fPDEScaleFactor = 1.0; 

    std::map<std::string, SLArEventAnode*> fBranchTargetAnodeSiPMMap = {};
    std::map<std::string, SLArEventAnode*> fBranchTargetAnodeTileMap = {};
    std::map<std::string, SLArEventSuperCellArray*> fBranchTargetSCArrayMap = {};
    std::map<std::string, NComponentBase_t> fAnodeNComponentMap = {};
    std::map<std::string, NComponentBase_t> fSCArrayNComponentMap = {};

};



#endif /* end of include guard SLARFLSPHOTONLIBRARY_HH */

