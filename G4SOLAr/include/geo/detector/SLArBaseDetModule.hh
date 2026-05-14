/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArAbsModule
 * @created     : mercoledì ago 07, 2019 13:08:35 CEST
 */

#ifndef SLArABSMODULE_HH

#define SLArABSMODULE_HH

#include "material/SLArMaterial.hh"
#include "SLArGeoInfo.hh"
#include "SLArGeoUtils.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VSolid.hh"
#include "G4RotationMatrix.hh"


class SLArBaseDetModule 
{
  public:
    SLArBaseDetModule();
    SLArBaseDetModule(const SLArBaseDetModule &base);
    virtual ~SLArBaseDetModule();

    SLArGeoInfo*       GetGeoInfo();
    G4bool             ContainsGeoPar(G4String str);
    void               SetGeoPar(G4String str, G4double val);
    void               SetGeoPar(std::pair<G4String, G4double> p);
    G4double           GetGeoPar(G4String str); 

    virtual void       Init(const rapidjson::Value&) {}
    
    void               SetSolidVolume(G4VSolid* sol_vol);
    inline G4VSolid*   GetModSV() {return fModSV;}
    inline const G4VSolid* GetModSV() const {return fModSV;}

    void               SetLogicVolume(G4LogicalVolume* log_vol);
    inline G4LogicalVolume*   GetModLV() {return fModLV;}
    inline const G4LogicalVolume* GetModLV() const {return fModLV;}

    void               SetRotation(G4RotationMatrix* rot) {fRot = rot;}
    void               SetTranslation(G4ThreeVector* vec) {fTranslation = *vec;}

    void               SetMaterial(G4Material* mat);
    G4Material*        GetMaterial();

    G4RotationMatrix*  GetRotation() const {return fRot;}
    G4RotationMatrix*  GetRotation() {return fRot;}
    G4ThreeVector*     GetTranslation() {return &fTranslation;}
    
    void               SetID(const int id) {fID = id;}
    G4int              GetID() {return fID;}

    void               ResetGeometry();

    void SetModPV(G4VPhysicalVolume* pv) {fModPV = pv;}
    inline G4VPhysicalVolume* GetModPV() {return fModPV;}
    inline const G4VPhysicalVolume* GetModPV() const {return fModPV;}
    G4VPhysicalVolume* BuildAndPlacePV(
        G4String                          name, 
        G4RotationMatrix*                 rot,
        const G4ThreeVector               &vec,
        G4LogicalVolume*                  mlv,
        G4bool                            pMany = false,
        G4int                             pCopyNo = 0);

  protected:
    G4Material*        fMaterial    = {};
    SLArGeoInfo*       fGeoInfo     = {};

    G4LogicalVolume*   fModLV       = {};
    G4VSolid*          fModSV       = {};
    G4VPhysicalVolume* fModPV       = {};

    G4RotationMatrix*  fRot         = {};
    G4ThreeVector      fTranslation = {};
    G4String           fName        = {};
    G4int              fID          = {-999}; 
};


#endif /* end of include guard SLArABSMODULE_HH */

