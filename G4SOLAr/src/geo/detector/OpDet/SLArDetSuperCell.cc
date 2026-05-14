/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSupperCell.cc
 * @created     : Tue May 24, 2022 11:54:17 CEST
 */

#include "detector/OpDet/SLArDetSuperCell.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4VisAttributes.hh"
#include "G4MaterialPropertyVector.hh"

SLArDetSuperCell::~SLArDetSuperCell() {
  G4cout << "Deleting SLArDetSuperCell... " <<  G4endl;
  if (fLightGuide)   {delete fLightGuide; fLightGuide = 0;}
  if (fCoating)      {delete fCoating; fCoating = 0;}
  if (fMatSuperCell) {delete fMatSuperCell; fMatSuperCell = 0;}
  if (fMatLightGuide){delete fMatLightGuide; fMatLightGuide = 0;}
  if (fMatCoating)   {delete fMatCoating; fMatCoating = 0;}
  G4cout << "SLArDetSuperCell DONE" <<  G4endl;
}


void SLArDetSuperCell::BuildLightGuide()
{
  G4cout << "Building SuperCell Lightguide" << G4endl;

  fLightGuide = new SLArBaseDetModule();
  fLightGuide->SetGeoPar(fGeoInfo->GetGeoPair("cell_z"));
  fLightGuide->SetGeoPar(fGeoInfo->GetGeoPair("cell_x"));
  fLightGuide->SetGeoPar(fGeoInfo->GetGeoPair("cell_y"));

  fLightGuide->SetMaterial(fMatLightGuide->GetMaterial());

  G4VSolid* lgbox = 
    new G4Box("LightGuideSlab", 
        0.5*fLightGuide->GetGeoPar("cell_x"),
        0.5*fLightGuide->GetGeoPar("cell_y"),
        0.5*fLightGuide->GetGeoPar("cell_z"));

  fLightGuide->SetSolidVolume(lgbox);
  fLightGuide->SetLogicVolume(
    new G4LogicalVolume(fLightGuide->GetModSV(), 
      fLightGuide->GetMaterial(),
      "LightGuideLV", 0, 0, 0)
    );
  
}

void SLArDetSuperCell::BuildCoating()
{
  G4cout << "Building SuperCell Coating..." << G4endl;
  fCoating = new SLArBaseDetModule();
  fCoating->SetGeoPar(fGeoInfo->GetGeoPair("cell_z"  ));
  fCoating->SetGeoPar(fGeoInfo->GetGeoPair("cell_x"  ));
  fCoating->SetGeoPar(fGeoInfo->GetGeoPair("coating_y"));

  fCoating->SetMaterial(fMatCoating->GetMaterial());

  fCoating->SetSolidVolume(
        new G4Box("Coating", 
          0.5*fCoating->GetGeoPar("cell_x"),
          0.5*fCoating->GetGeoPar("coating_y"),
          0.5*fCoating->GetGeoPar("cell_z"))
        );
 
  fCoating->SetLogicVolume(
      new G4LogicalVolume(fCoating->GetModSV(), 
        fCoating->GetMaterial(), "Coating", 0, 0, 0)
      );
}

void SLArDetSuperCell::BuildOpticalDetector() 
{
  /*  *  *  *  *  *  *  *  *  *  *  *  * 
   * Build all the SuperCell components
   *  *  *  *  *  *  *  *  *  *  *  *  */

  BuildLightGuide();
  BuildCoating();


  //* * * * * * * * * * * * * * * * * * * * * * * * * * *//
  // Building a "empty" LV as SuperCell container        //
  //* * * * * * * * * * * * * * * * * * * * * * * * * * *//

  G4cout << "SLArDetSuperCell::BuildSuperCell()" << G4endl;

  fhTot = fGeoInfo->GetGeoPar("cell_y") 
    + fGeoInfo->GetGeoPar("coating_y");

  fModSV = new G4Box("SuperCell",
      fGeoInfo->GetGeoPar("cell_x")*0.5,
      fhTot*0.5,
      fGeoInfo->GetGeoPar("cell_z")*0.5
      );


  fModLV
    = new G4LogicalVolume(fModSV, 
        fMatSuperCell->GetMaterial(),
        "SuperCellLV",0,0,0);

  /*  *  *  *  *  *  *  *  *  *  *  *  * 
   * Place SuperCell components
   *  *  *  *  *  *  *  *  *  *  *  *  */

  G4double h = 0*CLHEP::mm;
  h = -0.5*fCoating->GetGeoPar("coating_y");

  G4cout<<"GetModPV light guide..." << G4endl; 
  fLightGuide->BuildAndPlacePV("SuperCellLightGuide", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 101);

  h = 0.5*fhTot 
      - 0.5*fCoating->GetGeoPar("coating_y");
  G4cout<<"GetModPV coating..." << G4endl; 
  fCoating->BuildAndPlacePV("SuperCellCoating", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 102);

   return;
}


void SLArDetSuperCell::SetVisAttributes(const int& level)
{
  if (level > 0) {
    G4VisAttributes* visAttributes = new G4VisAttributes();
    visAttributes->SetColor(0.862, 0.952, 0.976, 0.5);
    fLightGuide->GetModLV()->SetVisAttributes( visAttributes );

    visAttributes = new G4VisAttributes( G4Color(0.968, 0.494, 0.007) );
    fCoating->GetModLV()->SetVisAttributes( visAttributes );

    visAttributes = new G4VisAttributes();
    visAttributes->SetColor(0.305, 0.294, 0.345, 0.0);
    fModLV->SetVisAttributes( visAttributes );
  }
  else {
    fLightGuide->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    fCoating->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    fModLV->SetVisAttributes( G4VisAttributes(false) );
  }

  return;
}


void SLArDetSuperCell::BuildMaterial(G4String materials_db)
{
  if (fMatLightGuide) {delete fMatLightGuide; fMatLightGuide = {};} 
  if (fMatCoating) {delete fMatCoating; fMatCoating = {};} 
  if (fMatSuperCell) {delete fMatSuperCell; fMatSuperCell = {};} 

  fMatLightGuide   = new SLArMaterial();
  fMatCoating      = new SLArMaterial();
  fMatSuperCell    = new SLArMaterial();

  fMatSuperCell->SetMaterialID("Vacuum");
  fMatSuperCell->BuildMaterialFromDB(materials_db);

  fMatLightGuide->SetMaterialID("Plastic");
  fMatLightGuide->BuildMaterialFromDB(materials_db);

  fMatCoating->SetMaterialID("PTP");
  fMatCoating->BuildMaterialFromDB(materials_db);
}

G4LogicalSkinSurface* SLArDetSuperCell::BuildLogicalSkinSurface() {
  fOpDetSkinSurface = 
    new G4LogicalSkinSurface(
        "PTP_LgSkin", 
        fCoating->GetModLV(), 
        fMatCoating->GetMaterialOpticalSurf());

  return fOpDetSkinSurface;
}


