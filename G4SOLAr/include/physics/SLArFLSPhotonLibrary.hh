/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFLSPhotonLibrary.hh
 * @created     : Sunday Dec 14, 2025 15:10:57 CET
 */

#ifndef SLARFLSPHOTONLIBRARY_HH

#define SLARFLSPHOTONLIBRARY_HH

#include "SLArFastLightSim.hh"
#include "SLArEventAnode.hh"
#include "SLArEventSuperCellArray.hh"

#include "TFile.h"
#include "TTree.h"

/**
 * @class SLArFLSPhotonLibrary
 * @brief Class implementing a Photon Library-based Fast Light Simulation
 *
 */
class SLArFLSPhotonLibrary : public SLArFastLightSim {
  typedef std::vector<int> NComponentBase_t;
  public:
    /**
     * @class PhotonLibraryEntry
     * @brief Data structure holding the data in the photon library file
     */
    struct PhotonLibraryEntry {
      float x_ = {};        //!< voxel x coordinate
      float y_ = {};        //!< voxel y coordinate
      float z_ = {};        //!< voxel z coordinate
      float vis_tot_ = {};  //!< total visibility
      float vis_dir_ = {};  //!< direct (scintillation) light total visibility 
      float vis_wls_ = {};  //!< WLS light total visibility

      //! Map containing the visibilities for a tile hashed by name of the sub-system
      std::map<std::string, std::vector<float>> tilearrayBuffers_;
      //! Map containing the SiPM visibilities hashed by name of the sub-system
      std::map<std::string, std::vector<float>> sipmBuffers_;


      /**
       * @brief Reset to zero all variables
       */
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


    /**
     * @brief Default constructor
     */
    inline SLArFLSPhotonLibrary() : SLArFastLightSim() {
      fType = (SLArFastLightSim::kLUT);
    }
    /**
     * @brief Constructor setting the path to the config file
     *
     * @param config_path Photon-Library configuration file path
     */
    inline SLArFLSPhotonLibrary(const G4String config_path)
        : fPhotonLibConfigPath(config_path) {}
    inline ~SLArFLSPhotonLibrary() override {
      if (fPhotonLibraryFile) {
        fEntry.Clear();
        fPhotonLibraryFile->Close();
      }
    }

    /**
     * @brief Source configuration file and initialize the Photon-Library engine
     *
     * @param config config file path
     */
    void Initialize(const rapidjson::Value& config) override;


    /**
     * @brief Propagate optical photons from the emission points
     *
     * @param particleDef particle definition (for particle-specific optical properties)
     * @param volumeName volume name
     * @param emissionPoint emission point
     * @param numPhotons number of photons to be simulated
     * @param emissionTime vector of emission times of the photons to be propagated
     * @param emissionEnergy vector of energies (wavelength) of the photons to be propagated
     */
    void PropagatePhotons(
        const G4ParticleDefinition* particleDef,
        const G4String& volumeName,
        const G4ThreeVector& emissionPoint,
        const int numPhotons,
        const std::vector<double>& emissionTime, 
        const std::vector<double>& emissionEnergy) override;

    /**
     * @brief Get the PDE scale factor for a given sub-system
     *
     * @param key sub-system name
     * @return PDE scale factor
     */
    inline float GetPDEScaleFactor(const std::string& key) const 
    {
      if (fPDEScaleFactor.find(key) == fPDEScaleFactor.end()) {
        G4Exception(
            "SLArFLSPhotonLibrary::GetPDEScaleFactor",
            "ConfigError",
            JustWarning,
            Form("PDE scale factor for key '%s' not found. Returning default value of 1.0.", key.c_str())
            );
        return 1.0;
      }
      else return fPDEScaleFactor.at(key);
    }

    /**
     * @brief Set a scale factor for the PDE of a given sub-system
     *
     * @param key sub-system name
     * @param scale_factor PDE scale factor
     */
    inline void SetPDEScaleFactor(const std::string key, const float scale_factor) {fPDEScaleFactor[key] = scale_factor;}

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

    //! Map EventAnode instance to the corresponding Photon Library SiPM branch name
    std::map<std::string, SLArEventAnode*> fBranchTargetAnodeSiPMMap = {}; 
    //! Map EventAnode instance to the corresponding Photon Library tile branch name
    std::map<std::string, SLArEventAnode*> fBranchTargetAnodeTileMap = {};
    //! Map EventSCArray instance to the corresponding Photon Library branch name
    std::map<std::string, SLArEventSuperCellArray*> fBranchTargetSCArrayMap = {};
    //! Dictionary of component structure for anode plane
    std::map<std::string, NComponentBase_t> fAnodeNComponentMap = {};
    //! Dictionary of component structure for optical detector arrays 
    std::map<std::string, NComponentBase_t> fSCArrayNComponentMap = {};
    //! Dictionary of PDE scale factors
    std::map<std::string, float> fPDEScaleFactor = {}; 

};



#endif /* end of include guard SLARFLSPHOTONLIBRARY_HH */

