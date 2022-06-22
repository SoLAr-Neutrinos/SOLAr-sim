/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSupperCell
 * @created     : martedì mag 24, 2022 11:54:17 CEST
 */

#include "detector/SuperCell/SLArDetSuperCell.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4VisAttributes.hh"
#include "G4MaterialPropertyVector.hh"

SLArDetSuperCell::SLArDetSuperCell() : SLArBaseDetModule(),
  fPerfectQE(false),
  fLightGuide(nullptr), fCoating(nullptr),
  fMatSuperCell(nullptr), fMatLightGuide(nullptr), fMatCoating(nullptr)
{  
  fGeoInfo = new SLArGeoInfo();
}

SLArDetSuperCell::SLArDetSuperCell(const SLArDetSuperCell &detSuperCell) 
  : SLArBaseDetModule(detSuperCell)
{
  fPerfectQE   = detSuperCell.fPerfectQE;
  fGeoInfo     = detSuperCell.fGeoInfo;
  fMatSuperCell= detSuperCell.fMatSuperCell;

  fMatSuperCell = new SLArMaterialInfo(*detSuperCell.fMatSuperCell); 
  fMatLightGuide = new SLArMaterialInfo(*detSuperCell.fMatLightGuide);
  fMatCoating   = new SLArMaterialInfo(*detSuperCell.fMatCoating);

}

SLArDetSuperCell::~SLArDetSuperCell() {
  G4cerr << "Deleting SLArDetSuperCell... " <<  G4endl;

  if (fLightGuide)   {delete fLightGuide; fLightGuide = 0;}
  if (fCoating)      {delete fCoating; fCoating = 0;}
  if (fMatSuperCell) {delete fMatSuperCell; fMatSuperCell = 0;}
  if (fMatLightGuide){delete fMatLightGuide; fMatLightGuide = 0;}
  if (fMatCoating)   {delete fMatCoating; fMatCoating = 0;}
  G4cerr << "SLArDetSuperCell DONE" <<  G4endl;
}

void SLArDetSuperCell::SetPhotoDetPos(EPhotoDetPosition kPos)
{
  fPos = kPos;
}

EPhotoDetPosition SLArDetSuperCell::GetSuperCellPos()
{
  return fPos;
}



void SLArDetSuperCell::BuildLightGuide()
{
  G4cerr << "Building SuperCell Lightguide" << G4endl;

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

void SLArDetSuperCell::BuildSuperCell()
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

  G4VSolid* SuperCell_box = new G4Box("SuperCell",
      fGeoInfo->GetGeoPar("cell_x")*0.5,
      fhTot,
      fGeoInfo->GetGeoPar("cell_z")*0.5
      );


  fModLV
    = new G4LogicalVolume(SuperCell_box, 
        fMatSuperCell->GetMaterial(),
        "SuperCellLV",0,0,0);

  /*  *  *  *  *  *  *  *  *  *  *  *  * 
   * Place SuperCell components
   *  *  *  *  *  *  *  *  *  *  *  *  */

  G4double h = 0*mm;
  h = 0.5*fhTot - 0.5*fCoating->GetGeoPar("coating_y");

  G4cout<<"GetModPV light guide..." << G4endl; 
  fLightGuide->GetModPV("SuperCellLightGuide", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 101);

  h = fhTot 
      - 0.5*fCoating->GetGeoPar("coating_y");
  G4cout<<"GetModPV coating..." << G4endl; 
  fCoating->GetModPV("SuperCellCoating", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 102);

   return;
}

void SLArDetSuperCell::ResetSuperCellGeometry() 
{
  G4cout<< "Reset SuperCell Geometry" << G4endl;
  
  return; 
}

void SLArDetSuperCell::SetVisAttributes()
{
  G4VisAttributes* visAttributes = new G4VisAttributes();
  visAttributes->SetColor(0.862, 0.952, 0.976, 0.5);
  fLightGuide->GetModLV()->SetVisAttributes( visAttributes );

  visAttributes = new G4VisAttributes( G4Color(0.968, 0.494, 0.007) );
  fCoating->GetModLV()->SetVisAttributes( visAttributes );

  visAttributes = new G4VisAttributes();
  visAttributes->SetColor(0.305, 0.294, 0.345, 0.0);
  fModLV->SetVisAttributes( visAttributes );

  return;
}

SLArBaseDetModule* SLArDetSuperCell::GetCoating()
{
  return fCoating;
}


SLArMaterialInfo* SLArDetSuperCell::GetCoatingMaterial()
{
  return fMatCoating;
}

G4double SLArDetSuperCell::GetTotalHeight()
{
  return fhTot;
}


void SLArDetSuperCell::SetPerfectQE(G4bool kQE)
{
  if (fPerfectQE==kQE) return;
  else 
  {     
    fPerfectQE = kQE;
    G4cout << "SLArDetSuperCell::SetPerfectQE: Setting 100% QE between "
           << "2 and 5 eV" << G4endl;
    G4double phEne[2] = {2*eV, 5*eV};
    G4double eff  [2] = {1.0 , 1.0 };
    
    fMatCoating->GetMaterialBuilder()->GetSurface()
               ->GetMaterialPropertiesTable()
               ->AddProperty("EFFICIENCY", phEne, eff, 2);  
  }

  return;
}

void SLArDetSuperCell::BuildDefalutGeoParMap() 
{
  G4cout  << "SLArDetSuperCell::BuildGeoParMap()" << G4endl;
  
  // Light guide and Coating layer size indicates:
  // * the side width 
  // * side length 
  // * light guide thickness
  // * coating layer thikness
  fGeoInfo->RegisterGeoPar("cell_z"   , 50.0*cm);
  fGeoInfo->RegisterGeoPar("cell_x"   , 10.0*cm);
  fGeoInfo->RegisterGeoPar("coating_y",  0.5*mm);
  fGeoInfo->RegisterGeoPar("cell_y"   ,  4.0*mm);
}


void SLArDetSuperCell::BuildMaterial()
{
  fMatLightGuide   = new SLArMaterialInfo();
  fMatCoating      = new SLArMaterialInfo();
  fMatSuperCell    = new SLArMaterialInfo();

  fMatSuperCell->SetMaterialID("Vacuum");
  fMatSuperCell->GetMaterialBuilder()->BuildMaterial();

  fMatLightGuide->SetMaterialID("Plastic");
  fMatLightGuide->GetMaterialBuilder()->BuildMaterial();

  fMatCoating->SetMaterialID("Bialkali");
  fMatCoating->GetMaterialBuilder()->BuildMaterial();
}


