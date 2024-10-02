/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetCathode.hh
 * @created     Mon Mar 20, 2023 16:26:39 CET
 */

#ifndef SLARDETCATHODE_HH

#define SLARDETCATHODE_HH

#include "detector/SLArBaseDetModule.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"



class SLArDetCathode : public SLArBaseDetModule {

public:
  SLArDetCathode          ();
  virtual ~SLArDetCathode ();
  
  G4LogicalSkinSurface* BuildLogicalSkinSurface();

  void          BuildCathode();

  void          BuildMaterial(G4String);
  void          BuildDefalutGeoParMap();
  
  //G4LogicalSkinSurface* GetESRSkin() {return fSkinESRSurface;}

  G4ThreeVector GetCathodeCenter();
  virtual void  Init(const rapidjson::Value&) override; 
  void          SetVisAttributes();
  
private:
  // Some useful global variables
  SLArMaterial* fMatCathode;
  SLArMaterial* fMatESR;
  G4LogicalSkinSurface* fSkinESRSurface;
  G4OpticalSurface* OpESRFilm;
  //SLArBaseDetModule* fCathodeBuild; 
 
};


#endif /* end of include guard SLARDETCATHODE_HH */
