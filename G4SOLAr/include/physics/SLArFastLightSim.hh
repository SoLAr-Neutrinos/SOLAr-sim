/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFastLightSim.hh
 * @created     : Sunday Dec 14, 2025 14:55:24 CET
 */

#ifndef SLARFASTLIGHTSIM_HH

#define SLARFASTLIGHTSIM_HH

#include "G4TransportationManager.hh"
#include "G4ThreeVector.hh"

#include "rapidjson/document.h"

class SLArFastLightSim {
  public:
    enum EFLSType {kUndefined = 0, kLUT = 1};
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

    virtual void PropagatePhotons(
        const G4String& volumeName,
        const G4ThreeVector& emissionPoint,
        const int numPhotons,
        const double emissionTime) = 0;

    virtual void Initialize(const rapidjson::Value& config) {}

    virtual void Print() const {}

    virtual void Reset() {}

  protected:
    G4String fName = {};
    EFLSType fType = kUndefined;

};

class SLArFastLightSimDispatcher : public SLArFastLightSim {
  public:
    SLArFastLightSimDispatcher() = default;

    void RegisterSimulator(const G4String& moduleName, 
        std::unique_ptr<SLArFastLightSim> sim) {
      fSimulators[moduleName] = std::move(sim);
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

