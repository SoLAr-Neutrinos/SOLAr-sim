/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFastLightSim.hh
 * @created     : Sunday Dec 14, 2025 14:55:24 CET
 */

#ifndef SLARFASTLIGHTSIM_HH

#define SLARFASTLIGHTSIM_HH

#include "G4TransportationManager.hh"
#include "G4ThreeVector.hh"

class SLArFastLightSim {
  public:
    virtual ~SLArFastLightSim() = default;

    virtual void PropagatePhotons(
        const G4ThreeVector& emissionPoint,
        const int numPhotons,
        const double emissionTime) = 0;

    virtual void Initialize() {}

    virtual void Reset() {}
};

class SLArFastLightSimDispatcher : public SLArFastLightSim {
  public:
    SLArFastLightSimDispatcher() = default;

    void RegisterSimulator(const G4String& volumeName, 
        std::unique_ptr<SLArFastLightSim> sim) {
      fSimulators[volumeName] = std::move(sim);
    }

    void SetDefaultVolume(const G4String& volumeName) {
      fDefaultVolume = volumeName;
    }

    inline void PropagatePhotons(
        const G4ThreeVector& emissionPoint,
        int numPhotons,
        double emissionTime) override {

      G4String volumeName = GetVolumeNameAt(emissionPoint);

      auto it = fSimulators.find(volumeName);
      if (it != fSimulators.end()) {
        return it->second->PropagatePhotons(
            emissionPoint, numPhotons, emissionTime);
      }

      return;
    }

    void Initialize() override {
      for (auto& pair : fSimulators) {
        pair.second->Initialize();
      }
    }

  private:
    std::map<G4String, std::unique_ptr<SLArFastLightSim>> fSimulators;
    G4String fDefaultVolume = {};

    G4String GetVolumeNameAt(const G4ThreeVector& point) {
      G4Navigator* navigator = 
        G4TransportationManager::GetTransportationManager()
        ->GetNavigatorForTracking();
      G4VPhysicalVolume* vol = navigator->LocateGlobalPointAndSetup(point);
      return vol ? vol->GetName() : "";
    }
};


#endif /* end of include guard SLARFASTLIGHTSIM_HH */

