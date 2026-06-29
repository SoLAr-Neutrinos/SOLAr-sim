/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArDetSupperCell.cc
 * @created     : Tue May 24, 2022 11:54:17 CEST
 */

#include "detector/OpDet/SLArDetSuperCell.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"

SLArDetSuperCell::~SLArDetSuperCell() {
  G4cout << "Deleting " << fName << " SLArDetSuperCell... " <<  G4endl;
  if (fLightGuide)   {delete fLightGuide; fLightGuide = 0;}
  if (fCoating)      {delete fCoating; fCoating = 0;}
  if (fMatSuperCell) {delete fMatSuperCell; fMatSuperCell = 0;}
  if (fMatLightGuide){delete fMatLightGuide; fMatLightGuide = 0;}
  if (fMatCoating)   {delete fMatCoating; fMatCoating = 0;}
  if (fMatWLSCoating){delete fMatWLSCoating; fMatWLSCoating = 0;}
  G4cout << "SLArDetSuperCell DONE" <<  G4endl;
}

void SLArDetSuperCell::BuildLightGuide()
{
  G4cout << "Building " << fName << " SuperCell Lightguide" << G4endl;

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
  G4cout << "Building " << fName << " SuperCell sensitive coating..." << G4endl;
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

void SLArDetSuperCell::BuildWLSCoating()
{
  G4cout << "Building " << fName << " SuperCell wavelength-shifting Coating..." << G4endl;
  fWLSCoating = new SLArBaseDetModule();
  fWLSCoating->SetGeoPar(fGeoInfo->GetGeoPair("cell_z"  ));
  fWLSCoating->SetGeoPar(fGeoInfo->GetGeoPair("cell_x"  ));
  fWLSCoating->SetGeoPar(fGeoInfo->GetGeoPair("wlscoating_y"));

  fWLSCoating->SetMaterial(fMatWLSCoating->GetMaterial());

  fWLSCoating->SetSolidVolume(
        new G4Box("WLSCoatingSV", 
          0.5*fWLSCoating->GetGeoPar("cell_x"),
          0.5*fWLSCoating->GetGeoPar("wlscoating_y"),
          0.5*fWLSCoating->GetGeoPar("cell_z"))
        );
 
  fWLSCoating->SetLogicVolume(
      new G4LogicalVolume(fWLSCoating->GetModSV(), 
        fWLSCoating->GetMaterial(), "WLSCoatingLV", 0, 0, 0)
      );
}

void SLArDetSuperCell::BuildOpticalDetector() 
{
  /*  *  *  *  *  *  *  *  *  *  *  *  * 
   * Build all the SuperCell components
   *  *  *  *  *  *  *  *  *  *  *  *  */
  BuildLightGuide();
  BuildCoating();
  if (fGeoInfo->Contains("wlscoating_y")) {
    BuildWLSCoating();
  }

  //* * * * * * * * * * * * * * * * * * * * * * * * * * *//
  // Building a "empty" LV as SuperCell container        //
  //* * * * * * * * * * * * * * * * * * * * * * * * * * *//

  G4cout << "SLArDetSuperCell::BuildSuperCell() " << fName << G4endl;

  fhTot = fGeoInfo->GetGeoPar("cell_y") 
    + fGeoInfo->GetGeoPar("coating_y");
  if (fWLSCoating) {
    fhTot += fWLSCoating->GetGeoPar("wlscoating_y");
  }

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
  if (fWLSCoating) {
    h -= 0.5*fWLSCoating->GetGeoPar("wlscoating_y");
  }


  printf("SLArDetSuperCell::BuildSuperCell: placing components...\n");
  G4cout<<"GetModPV light guide..." << G4endl; 
  fLightGuide->BuildAndPlacePV("SuperCellLightGuide", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 101);

  h = 0.5*fhTot 
    - 0.5*fCoating->GetGeoPar("coating_y")
    - (fWLSCoating ? fWLSCoating->GetGeoPar("wlscoating_y") : 0);

  G4cout<<"GetModPV coating..." << G4endl; 
  fCoating->BuildAndPlacePV("SuperCellCoating", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 102);

  if (fWLSCoating) {
    h = 0.5*fhTot 
      - 0.5*fWLSCoating->GetGeoPar("wlscoating_y");
    printf("  WLS coating: %s\n", fWLSCoating->GetModLV()->GetName().c_str());
    fWLSCoating->BuildAndPlacePV("SuperCellWLSCoating", 0, 
        G4ThreeVector(0, h, 0), 
        fModLV, false, 102);
  }

  return;
}


void SLArDetSuperCell::SetVisAttributes(const int& level)
{
  if (level > 1) {
    G4VisAttributes* LGvisAttributes = new G4VisAttributes();
    LGvisAttributes->SetColor(0.862, 0.952, 0.976, 0.5);
    fLightGuide->GetModLV()->SetVisAttributes( LGvisAttributes );

    G4Color col_sens; 
    if (fMatCoatingName == "PTP_sensitive") {
      col_sens = G4Color(0.776, 0.0, 1.0, 0.5);
    }
    else {
      col_sens = G4Color(0.0, 0.0, 1.0); 
    }
    G4VisAttributes* CoatingvisAttributes = new G4VisAttributes( col_sens );
    fCoating->GetModLV()->SetVisAttributes( CoatingvisAttributes );

    if (fWLSCoating) {
      G4VisAttributes* WLSvisAttributes = new G4VisAttributes( G4Color(0.0, 0.8, 0.0) );
      fWLSCoating->GetModLV()->SetVisAttributes( WLSvisAttributes );
    }
  }
  else if (level == 1){
    fLightGuide->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    fCoating->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    if (fWLSCoating) {
      fWLSCoating->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    }
    G4VisAttributes* global_vis_attr = new G4VisAttributes();
    global_vis_attr->SetColor(0.305, 0.294, 0.345, 0.0);
    fModLV->SetVisAttributes( global_vis_attr );
  }
  else {
    fLightGuide->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    fCoating->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    if (fWLSCoating) {
      fWLSCoating->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    }
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
  fMatWLSCoating   = new SLArMaterial();

  fMatSuperCell->SetMaterialID("LAr");
  fMatSuperCell->BuildMaterialFromDB(materials_db);

  fMatLightGuide->SetMaterialID("Plastic");
  fMatLightGuide->BuildMaterialFromDB(materials_db);

  fMatCoating->SetMaterialID( fMatCoatingName );
  fMatCoating->BuildMaterialFromDB(materials_db);

  fMatWLSCoating->SetMaterialID("PTP_wls");
  fMatWLSCoating->BuildMaterialFromDB(materials_db);
}

G4LogicalSkinSurface* SLArDetSuperCell::BuildLogicalSkinSurface() {
  fOpDetSkinSurface = 
    new G4LogicalSkinSurface(
        "PTP_LgSkin", 
        fCoating->GetModLV(), 
        fMatCoating->GetMaterialOpticalSurf());

  return fOpDetSkinSurface;
}

G4LogicalBorderSurface* SLArDetSuperCell::BuildWLSLogicalBorderSurface() {
  G4LogicalBorderSurface* wlsSurf = 
    new G4LogicalBorderSurface(
        "WLSCoatingBorder", 
        fModPV, 
        fWLSCoating->GetModPV(), 
        fMatWLSCoating->GetMaterialOpticalSurf());

  return wlsSurf;
}

