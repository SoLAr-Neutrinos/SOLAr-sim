#ifndef SLARPENFILM_HH

#define SLARDPENFILM_HH

#include "detector/SLArBaseDetModule.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"

class SLArPENFilm : public SLArBaseDetModule {

public:
  SLArPENFilm          ();
  virtual ~SLArPENFilm ();
  
  G4LogicalSkinSurface* BuildLogicalSkinSurface();
  
  void BuildPEN();
  
  void BuildMaterial(G4String);
  void BuildDefalutGeoParMap();
  
  G4ThreeVector GetPENCenter(); 
  virtual void  Init(const rapidjson::Value&) override;
  void          SetVisAttributes();
 
private:
  SLArMaterial* fMatPEN;
  G4LogicalSkinSurface* fSkinPENSurface;
  G4OpticalSurface* OpPENSkin;

};

#endif
         
