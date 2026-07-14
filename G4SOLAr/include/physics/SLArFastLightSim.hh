/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFastLightSim.hh
 * @created     : Sunday Dec 14, 2025 14:55:24 CET
 */

#ifndef SLARFASTLIGHTSIM_HH

#define SLARFASTLIGHTSIM_HH

#include "G4TransportationManager.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"

#include "rapidjson/document.h"
#include "G4Material.hh"
#include "TMath.h"
#include "G4PhysicsLinearVector.hh"

/**
 * @class SLArFastLightSimTime
 * @brief Class responsible for producing optical photon emission and propagation times in the `SLArFastLightSim` framework.
 *
 * This class provides methods to sample the emission time of optical photons
 * based on the scintillating material properties and on the particle type
 * that produced the scintillation. 
 * It also provides methods to sample the propagation time of optical 
 * photons in LAr based on [1].
 *
 *
 * [1] D. Garcia-Gamez, P. Green, A.M. Szelc, 
 * "Predicting transport effects of scintillation light signals in large-scale liquid argon detectors", 
 * Eur. Phys. J. C (2022) 81:349
 *
 */
class SLArFastLightSimTime {
  public:
    SLArFastLightSimTime() {}

    ~SLArFastLightSimTime() {
      for (int i = 0; i < 2; i++) {
        if (fParsLandauNormEntries[i]) delete fParsLandauNormEntries[i];
        if (fParsLandauMPV[i]) delete fParsLandauMPV[i];
        if (fParsLandauWidth[i]) delete fParsLandauWidth[i];
        if (fParsExpSlope[i]) delete fParsExpSlope[i];
        if (fParsExpNormOverLandau[i]) delete fParsExpNormOverLandau[i];
      }
    }

    void Initialize(const rapidjson::Value& config);

    inline void SetMaterial(const G4String& materialName) {
      G4Material* mat = G4Material::GetMaterial(materialName);
      if (!mat) {
        char msg[256];
        sprintf(msg, "SLArFastLightSimTime::SetMaterial: Material '%s' not found. Time simulation will not be functional.\n",
            materialName.data());
        G4Exception("SLArFastLightSimTime::SetMaterial", "UnknownMaterial", FatalException, msg);
        fMaterial = nullptr;
      } else {
        fMaterial = mat;
      }
    }

    virtual G4double SamplePhotonEmissionTime(const G4ParticleDefinition* pDef) const; 

    virtual G4double SamplePropagationTime(const G4ThreeVector& emissionPoint, 
        const G4ThreeVector& opDetPoint, 
        const G4double ph_ene) const;

  protected:
    G4Material* fMaterial = nullptr;
    G4PhysicsLinearVector* fParsLandauNormEntries[2] = {nullptr};
    G4PhysicsLinearVector* fParsLandauMPV[2] = {nullptr};
    G4PhysicsLinearVector* fParsLandauWidth[2]  = {nullptr};
    G4PhysicsLinearVector* fParsExpSlope[2]    = {nullptr};
    G4PhysicsLinearVector* fParsExpNormOverLandau[2] = {nullptr};

    G4double fInflextionPointDist = -1.0; 
    G4double fAngleBin = -1.0; 
    G4double fMaxDist = -1.0;
    G4double fMinDist = -1.0;

    void GetScintillationYieldByParticleType(
        const G4ParticleDefinition* pDef, G4double& yield1,
        G4double& yield2, G4double& yield3) const;

    inline G4double model_far(double* x, double* par) const {
      // par[1] = Landau MPV
      // par[2] = Landau width
      // par[3] = normalization
      // par[0] = t_min
      if (x[0] <= par[0]) return 0.;
      return par[3] * TMath::Landau(x[0], par[1], par[2]);
    }

    inline G4double model_close(double* x, double* par) const {
      // par0 = joining point
      // par1 = Landau MPV
      // par2 = Landau width
      // par3 = normalization
      // par4 = Expo cte
      // par5 = Expo tau
      // par6 = t_min
      double y1 = par[3] * TMath::Landau(x[0], par[1], par[2]);
      double y2 = TMath::Exp(par[4] + x[0] * par[5]);
      if (x[0] <= par[6] || x[0] > par[0]) y1 = 0.;
      if (x[0] < par[0]) y2 = 0.;
      return (y1 + y2);
    }

    inline G4double finter_d(double* x, double* par) const {
      double y1 = par[2] * TMath::Landau(x[0], par[0], par[1]);
      double y2 = TMath::Exp(par[3] + x[0] * par[4]);
      return TMath::Abs(y1 - y2);
    }

    G4double sample_time(G4double tau1, G4double tau2) const;

    inline G4double single_exp(G4double t, G4double tau2) const
    {
      return std::exp(-1.0 * t / tau2) / tau2;
    }

    inline G4double bi_exp(G4double t, G4double tau1, G4double tau2) const
    {
      return std::exp(-1.0 * t / tau2) * (1 - std::exp(-1.0 * t / tau1)) / tau2 /
        tau2 * (tau1 + tau2);
    }

};

/**
 * @class SLArFastLightSim
 * @brief Abstract base class for fast light simulation in the `SLArFastLightSim` framework.
 *
 * This class defines the interface for the fast light simulation modules that can 
 * be registed in the `SLArFastLightSimDispatcher`. Each module should inherit from this class
 * and implement the `PropagatePhotons` method, which is responsible for simulating the propagation
 * of optical photons from the emission point to the optical detectors. 
 *
 */
class SLArFastLightSim {
  public:
    /**
     * @brief Enumeration of the available fast light simulation types.
     */
    enum EFLSType {kUndefined = 0, kLUT = 1};
    
    /**
     * @brief Mapping of the `EFLSType` enumeration values to their string representations
     */
    const std::map<EFLSType, G4String> EFLSTypeNames = {
      {kUndefined, "Undefined"},
      {kLUT, "LUT"}
    };

    inline const EFLSType GetType() const {return fType;}
    inline const EFLSType GetType(const G4String& type_name) const {
      for (const auto& type_itr : EFLSTypeNames) {
        if (type_itr.second == type_name) {
          return type_itr.first;
        }
      }
      char msg[256];
      sprintf(msg, "SLArFastLightSim::GetType: Unknown fast light simulation type name '%s'. Returning 'Undefined'.\n",
          type_name.data());
      G4Exception("SLArFastLightSim::GetType", "UnknownFLSimType", JustWarning, msg);
      return kUndefined;
    };
    inline const G4String& GetTypeName() const {
      if (EFLSTypeNames.find(fType) != EFLSTypeNames.end()) {
        return EFLSTypeNames.at(fType);
      }
      static G4String undefined_str = "Undefined";
      return undefined_str;
    }

    virtual ~SLArFastLightSim() = default;

    void SetName(const G4String& name) {fName = name;}
    const G4String& GetName() const {return fName;}

    /**
     * @brief Pure virtual method to propagate optical photons from the emission point to the optical detectors.
     *
     * @param particleDef Particle definition (for particle specific scintillation properties)
     * @param volumeName Name of the volume where the photons are emitted
     * @param emissionPoint 3D position of the photon emission point
     * @param numPhotons Number of photons to propagate
     * @param emissionTime Vector of emission times for the photons
     * @param emissionEnergy Vector of emission energies for the photons
     */
    virtual void PropagatePhotons(
        const G4ParticleDefinition* particleDef,
        const G4String& volumeName,
        const G4ThreeVector& emissionPoint,
        const int numPhotons,
        const std::vector<double>& emissionTime,
        const std::vector<double>& emissionEnergy) = 0;

    /**
     * @brief Virtual method to initialize the fast light simulation module with configuration parameters.
     *
     * The JSON configuration object containing the parameters for 
     * the fast light simulation module.
     * The expected format of the configuration object depends on 
     * the specific implementation of the fast light simulation module.
     */
    virtual void Initialize(const rapidjson::Value& config) {}

    /**
     * @brief Virtual method to print the configuration and properties of the fast light simulation module.
     *
     * This method can be overridden by derived classes to provide specific 
     * information about the configuration and properties of the
     * fast light simulation module.
     */
    virtual void Print() const {}

    virtual void Reset() {}

  protected:
    G4String fName = {};
    SLArFastLightSimTime fTimeSim;
    EFLSType fType = kUndefined;
};

/**
 * @class SLArFastLightSimDispatcher
 * @brief Dispatcher object for Fast Light Simulation modules
 *
 * The `SLArFastLightSimDispatcher` acts as a dispatcher for multiple
 * fast light simulation modules in `SOLAr-sim`. 
 * Each module (simulator) is given a name and is associated to one or 
 * more physical volumes. 
 */
class SLArFastLightSimDispatcher {
  public:
    SLArFastLightSimDispatcher() = default;

    /**
     * @brief Register a fast light simulation module.
     */
    void RegisterSimulator(const G4String& moduleName, 
        std::unique_ptr<SLArFastLightSim> sim) {
      fSimulators[moduleName] = std::move(sim);
    }

    /**
     * @brief Set the name of the default volume where fast light simulation is applied
     */
    inline void SetDefaultVolume(const G4String& volumeName) {
      fDefaultVolume = volumeName;
    }

    /**
     * @brief Find the relevant simulator for a given position and execute it
     */
    inline void PropagatePhotons(
        const G4ParticleDefinition* particleDef,
        const G4String& volumeName,
        const G4ThreeVector& emissionPoint,
        int numPhotons,
        const std::vector<double>& emissionTime,
        const std::vector<double>& emissionWvlen) 
    {
      auto it = fVolumeToModuleMap.find(volumeName);
      if (it != fVolumeToModuleMap.end()) {
        const G4String& moduleName = it->second;

        auto sim_it = fSimulators.find(moduleName);
        if (sim_it != fSimulators.end()) {
          return sim_it->second->PropagatePhotons(
              particleDef, volumeName, emissionPoint, 
              numPhotons, emissionTime, emissionWvlen);
        }
      }
      return;
    }

  /**
   * @brief Register a volume and associate it to a given simulator
   */
  inline void RegisterVolume(
      const G4String& volumeName,
      const G4String& moduleName) {
      fVolumeToModuleMap[volumeName] = moduleName;
  }

  /**
   * @brief Print the details of all registered simulators and volumes
   */
  inline void Print() const {
    printf("----------------------------------------------------------------\n");
    printf("SLArFastLightSimDispatcher configuration:\n");
    printf("Simulator list:\n"); 
    for (const auto& sim_itr : fSimulators) {
      printf("label: %s\ttype: %s\n", sim_itr.first.data(), sim_itr.second->GetTypeName().data());
      sim_itr.second->Print();
    }
    printf("\nVolume to module mapping:\n");
    for (const auto& vol_itr : fVolumeToModuleMap) {
      printf("volume: %s\tmodule: %s\n", vol_itr.first.data(), vol_itr.second.data());
    }
  }

  private:
    std::map<G4String, std::unique_ptr<SLArFastLightSim>> fSimulators;
    std::map<std::string, std::string> fVolumeToModuleMap;
    G4String fDefaultVolume = {};

    inline G4String GetVolumeNameAt(const G4ThreeVector& point) {
      G4Navigator* navigator = 
        G4TransportationManager::GetTransportationManager()
        ->GetNavigatorForTracking();
      G4VPhysicalVolume* vol = navigator->LocateGlobalPointAndSetup(point);
      return vol ? vol->GetName() : "";
    }
};


#endif /* end of include guard SLARFASTLIGHTSIM_HH */

