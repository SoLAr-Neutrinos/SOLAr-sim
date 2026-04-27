/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetTPC.hh
 * @created     Thur Nov 03, 2022 12:22:00 CET
 */

#ifndef SLARDETTPC_HH

#define SLARDETTPC_HH

#include "detector/SLArBaseDetModule.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

class SLArDetTPC : public SLArBaseDetModule {

public:
  SLArDetTPC          ();
  virtual ~SLArDetTPC ();

  void          BuildTPC();

  void          BuildMaterial(G4String);
  void          BuildFieldCage(); 
  void          BuildFieldCageTub(); 
  void          BuildFieldCageBox();
  void          BuildDefalutGeoParMap();

  geo::EGeoShape GetShape() const { return fShape; }
  void SetShape(geo::EGeoShape shape) { fShape = shape; }
  const G4ThreeVector GetTPCcenter();
  const G4ThreeVector& GetElectronDriftDir() {return fElectronDriftDir;}
  const G4double& GetElectricField() {return fElectricField;}
  SLArBaseDetModule* GetFieldCage() {return fFieldCage;}
  virtual void  Init(const rapidjson::Value& jconf) override; 
  void  InitFieldCage(const rapidjson::Value& jcon); 
  void  SetVisAttributes();
  void  SetFieldCageVisibility(const G4bool); 


private:
  // Some useful global variables
  SLArMaterial* fMatTarget;
  SLArMaterial* fMatFieldCage;
  SLArBaseDetModule* fFieldCage; 
  geo::EGeoShape     fShape{geo::kBox};
  G4double      fElectricField; 
  G4ThreeVector fElectronDriftDir; 
  G4bool fFieldCageVisibility;

};


#endif /* end of include guard SLARDETTPC_HH */
