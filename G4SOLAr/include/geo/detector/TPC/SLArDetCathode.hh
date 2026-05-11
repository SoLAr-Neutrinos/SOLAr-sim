/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetCathode.hh
 * @created     Mon Mar 20, 2023 16:26:39 CET
 */

#ifndef SLARDETCATHODE_HH

#define SLARDETCATHODE_HH

#include "detector/SLArBaseDetModule.hh"

#include "G4ThreeVector.hh"

class SLArDetCathode : public SLArBaseDetModule {

public:
  SLArDetCathode          ();
  virtual ~SLArDetCathode ();

  void BuildCathode();

  void BuildMaterial(G4String);
  void BuildDefalutGeoParMap();
  geo::EGeoShape GetGeoShape() const { return fShape; }
  void SetGeoShape(geo::EGeoShape shape) { fShape = shape; }

  G4ThreeVector GetCathodeCenter() const;
  G4RotationMatrix* GetRotation() const { return fRotation; }
  virtual void  Init(const rapidjson::Value&) override; 
  void SetVisAttributes();

private:
  // Some useful global variables
  SLArMaterial* fMatCathode;
  G4RotationMatrix* fRotation = {};
  geo::EGeoShape fShape = geo::kBox;

};


#endif /* end of include guard SLARDETCATHODE_HH */
