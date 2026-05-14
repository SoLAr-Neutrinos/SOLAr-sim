/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetTPC.hh
 * @created     Thur Nov 03, 2022 12:22:00 CET
 */

#ifndef SLARDETTPC_HH

#define SLARDETTPC_HH

#include "detector/SLArBaseDetModule.hh"
#include "detector/TPC/SLArDetFieldCage.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4PVPlacement.hh"

class SLArDetTPC : public SLArBaseDetModule {

public:
  SLArDetTPC          ();
  virtual ~SLArDetTPC () = default;

  void          BuildTPC();

  void          BuildMaterial(G4String);
  void          BuildDefalutGeoParMap();

  geo::EGeoShape GetShape() const { return fShape; }
  void SetShape(geo::EGeoShape shape) { fShape = shape; }
  const G4ThreeVector GetTPCcenter();
  inline const G4ThreeVector& GetElectronDriftDir() {return fElectronDriftDir;}
  inline const G4double& GetElectricField() {return fElectricField;}
  inline SLArDetFieldCage* GetFieldCage() {return fFieldCage;}
  virtual void  Init(const rapidjson::Value& jconf) override; 
  void  InitFieldCage(const rapidjson::Value& jcon); 
  void  SetVisAttributes();
  inline void  SetFieldCageVisibility(const G4bool vis) 
    { if (fFieldCage) fFieldCage->SetVisAttributes(vis); }

private:
  SLArMaterial* fMatTarget = {};
  SLArMaterial* fMatFieldCage = {};
  SLArDetFieldCage* fFieldCage = {}; 
  geo::EGeoShape fShape = geo::kBox;
  G4double      fElectricField = {}; 
  G4ThreeVector fElectronDriftDir = {}; 
  G4bool fFieldCageVisibility = true;

};


#endif /* end of include guard SLARDETTPC_HH */
