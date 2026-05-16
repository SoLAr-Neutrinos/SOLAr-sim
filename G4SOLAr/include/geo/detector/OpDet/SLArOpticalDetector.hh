/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArOpticalDetector.hh
 * @created     : Tuesday May 12, 2026 19:43:30 CEST
 */

#ifndef SLAROPTICALDETECTOR_HH

#define SLAROPTICALDETECTOR_HH

#include "geo/detector/SLArBaseDetModule.hh"
#include "G4LogicalSkinSurface.hh"
#include "core/SLArDebugUtils.hh"

/**
 * @class SLArOpticalDetector
 * @brief Base abstract class for optical detectors in SOLAr-sim
 *
 */
class SLArOpticalDetector : public SLArBaseDetModule
{
  public: 
    enum class EOpDetType {kSuperCell, kSiPM, kUndefined};
    inline SLArOpticalDetector() = default;
    inline SLArOpticalDetector(const G4String& model_name) : SLArOpticalDetector() {
      fOpDetModelName = model_name;
      fName = model_name;
    }
    inline SLArOpticalDetector(const SLArOpticalDetector& base) : SLArBaseDetModule(base) {
      fOpDetType = base.fOpDetType;
      fOpDetTypeName = base.fOpDetTypeName;
      fOpDetModelName = base.fOpDetModelName;
      fPerfectEfficiency = base.fPerfectEfficiency;
    }
    inline virtual ~SLArOpticalDetector() = default;

    inline EOpDetType GetOpDetType() const { return fOpDetType; }
    inline G4String GetOpDetTypeName() const { return fOpDetTypeName; }
    inline G4String GetOpDetModelName() const { return fOpDetModelName; }
    virtual void BuildMaterial(G4String materials_db) = 0;
    virtual void BuildOpticalDetector() = 0;
    inline virtual void Init(const rapidjson::Value& config) override {
      debug::require_json_type(config, rapidjson::kObjectType);
      debug::require_json_member(config, "module_type");
      debug::require_json_member(config, "name");

      fOpDetTypeName = config["module_type"].GetString();
      fName = config["name"].GetString();
      fOpDetModelName = fName;
    }
    G4LogicalSkinSurface* GetOpDetSkinSurface() const { return fOpDetSkinSurface; }
    G4LogicalSkinSurface* GetOpDetSkinSurface() { return fOpDetSkinSurface; }
    inline void SetPerfectEfficiency(G4bool kQE) { 
      if (fPerfectEfficiency==kQE) return;
      else {     
        fPerfectEfficiency = kQE;
        G4cout << "SLArDetSuperCell::SetPerfectQE: Setting 100% QE between 2 and 12 eV" << G4endl;
        G4double phEne[2] = {2*CLHEP::eV, 12*CLHEP::eV};
        G4double eff  [2] = {1.0 , 1.0 };
        G4Material* active_material = fOpDetSkinSurface->GetLogicalVolume()->GetMaterial();
        active_material->GetMaterialPropertiesTable()->AddProperty("EFFICIENCY", phEne, eff, 2);  
      }
    }
    inline G4bool IsPerfectEfficiency() const { return fPerfectEfficiency; }
    virtual void SetVisAttributes(const int&) = 0;


  protected:
    EOpDetType fOpDetType = EOpDetType::kUndefined;
    G4String fOpDetTypeName = "UndefinedOpticalDetector";
    G4String fOpDetModelName;
    G4bool fPerfectEfficiency = false;
    G4LogicalSkinSurface* fOpDetSkinSurface = {};
};



#endif /* end of include guard SLAROPTICALDETECTOR_HH */

