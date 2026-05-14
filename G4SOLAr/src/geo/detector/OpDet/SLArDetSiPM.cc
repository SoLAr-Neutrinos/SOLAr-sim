/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSiPM.cc
 * @created     : Wednesday May 13, 2026 10:41:56 CEST
 */

#include "detector/OpDet/SLArDetSiPM.hh"
#include "SLArDebugUtils.hh"

#include "G4Box.hh"
#include "G4SubtractionSolid.hh"
#include "G4VisAttributes.hh"

void SLArDetSiPM::BuildMaterial(G4String materials_db) {
  fBaseMaterial = new SLArMaterial("LAr");
  fBaseMaterial->BuildMaterialFromDB(materials_db);

  fMatSiPM = new SLArMaterial("SiliconActive");
  fMatSiPM->BuildMaterialFromDB(materials_db); 
  
  fMatSiPMCapsule = new SLArMaterial("SiliconPassive");
  fMatSiPMCapsule->BuildMaterialFromDB(materials_db);
  return;
}

void SLArDetSiPM::BuildOpticalDetector() {
  G4cout << "Building SiPM..." << G4endl;
  double fill_factor = fGeoInfo->GetGeoPar("sipm_fill_factor"); 
  double x_ = fGeoInfo->GetGeoPar("sipm_x"); 
  double y_ = fGeoInfo->GetGeoPar("sipm_y"); 
  double z_ = fGeoInfo->GetGeoPar("sipm_z"); 
  double d_ = 0.25*((x_ + z_) - sqrt(pow(x_ + z_, 2) - 4*x_*z_*(1-fill_factor))); 
  double x  = x_ - 2*d_; 
  double z  = z_ - 2*d_; 

  fSiPMActive = new SLArBaseDetModule();

  fSiPMActive->SetGeoPar("active_sipm_y", y_); 
  fSiPMActive->SetGeoPar("active_sipm_x", x ); 
  fSiPMActive->SetGeoPar("active_sipm_z", z ); 
  
  SetMaterial(fBaseMaterial->GetMaterial());
  fSiPMActive->SetMaterial(fMatSiPM->GetMaterial()); 

  SetSolidVolume(new G4Box("SiPMBox", 0.5*x_, 0.5*y_, 0.5*z_));
 
  SetLogicVolume( new G4LogicalVolume(
        fModSV, fBaseMaterial->GetMaterial(), "SiPM_lv", 0, 0, 0) 
      );

  fSiPMActive->SetSolidVolume(new G4Box("SiPMActiveBox", 0.5*x, 0.5*y_, 0.5*z) );

  fSiPMActive->SetLogicVolume( new G4LogicalVolume(
        fSiPMActive->GetModSV(), fMatSiPM->GetMaterial(), "SiPMActive_lv", 0, 0, 0) 
      );

  G4SubtractionSolid* sipm_passive_box = new G4SubtractionSolid("SiPMCapsuleBox", 
      fModSV, fSiPMActive->GetModSV(), 0, G4ThreeVector(0, 0, 0)); 

  G4LogicalVolume* capsule_lv = 
    new G4LogicalVolume(sipm_passive_box, 
        fMatSiPMCapsule->GetMaterial(), "SiPMCapsule_lv", 0, 0, 0); 

  new G4PVPlacement(0, G4ThreeVector(0, 0, 0), capsule_lv, "SiPMCapsulePV",
      fModLV, 0, 251, true);

  fSiPMActive->BuildAndPlacePV("SiPMActivePV", 0, G4ThreeVector(0, 0, 0), 
      fModLV, 0, 250); 
}
 
void SLArDetSiPM::SetVisAttributes(const int& depth) {
  G4Color col_gray(0.753, 0.753, 0.753);
  G4VisAttributes* visAttributes = new G4VisAttributes( ); 
  visAttributes->SetColor(col_gray);

  if ( depth > 1 ) visAttributes->SetVisibility(true);
  else visAttributes->SetVisibility(false);

  fModLV->SetVisAttributes( visAttributes );

  for (size_t ii=0; ii < fModLV->GetNoDaughters(); ii++) {
    fModLV->GetDaughter(ii)->GetLogicalVolume()->SetVisAttributes( visAttributes ); 
  }
}

G4LogicalSkinSurface* SLArDetSiPM::BuildLogicalSkinSurface() {
  fOpDetSkinSurface = 
    new G4LogicalSkinSurface(
        "SiPM_LgSkin", 
        fSiPMActive->GetModLV(), 
        fMatSiPM->GetMaterialOpticalSurf());

  return fOpDetSkinSurface;
}

