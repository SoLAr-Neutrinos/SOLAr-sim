/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetSupperCell.cc
 * @created     : Tue May 24, 2022 11:54:17 CEST
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

#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4VisAttributes.hh"
#include "G4MaterialPropertyVector.hh"

SLArDetSuperCell::SLArDetSuperCell() : SLArBaseDetModule(),
  fPerfectQE(false),
  fLightGuide(nullptr), fCoating(nullptr),
  fMatSuperCell(nullptr), fMatLightGuide(nullptr), fMatCoating(nullptr), 
  fSkinSurface(nullptr)
{  
  fGeoInfo = new SLArGeoInfo();
}

SLArDetSuperCell::SLArDetSuperCell(const SLArDetSuperCell &detSuperCell) 
  : SLArBaseDetModule(detSuperCell)
{
  fPerfectQE   = detSuperCell.fPerfectQE;
  fGeoInfo     = detSuperCell.fGeoInfo;
  fMatSuperCell= detSuperCell.fMatSuperCell;

  fMatSuperCell = new SLArMaterial(*detSuperCell.fMatSuperCell); 
  fMatLightGuide = new SLArMaterial(*detSuperCell.fMatLightGuide);
  fMatCoating   = new SLArMaterial(*detSuperCell.fMatCoating);

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
  fLightGuide->GetModPV("SuperCellLightGuide", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 101);

  h = 0.5*fhTot 
      - 0.5*fCoating->GetGeoPar("coating_y");
  G4cout<<"GetModPV coating..." << G4endl; 
  fCoating->GetModPV("SuperCellCoating", 0, 
      G4ThreeVector(0, h, 0),
      fModLV, false, 102);

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


SLArMaterial* SLArDetSuperCell::GetCoatingMaterial()
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
    G4cout << "SLArDetReadoutTile::SetPerfectQE: Setting 100% QE between "
           << "1 and 5 eV" << G4endl;
    G4double phEne[5] = {2*CLHEP::eV, 3*CLHEP::eV, 4*CLHEP::eV, 5*CLHEP::eV, 13*CLHEP::eV};
    G4double eff  [5] = {0.15, 0.15, 0.15, 0.03, 0.03};
    
    fMatCoating->GetMaterialOpticalSurf()
            ->GetMaterialPropertiesTable()
            ->AddProperty("EFFICIENCY", phEne, eff, 6);
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
  fGeoInfo->RegisterGeoPar("cell_z"   , 50.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("cell_x"   , 10.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("coating_y",  0.5*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("cell_y"   ,  4.0*CLHEP::mm);
}


void SLArDetSuperCell::BuildMaterial(G4String materials_db)
{
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
/*
  auto SCproperties = new G4MaterialPropertiesTable();
  
  G4double phEne[5] = {2.0*CLHEP::eV, 3.0*CLHEP::eV, 4.0*CLHEP::eV, 5.0*CLHEP::eV, 6.0*CLHEP::eV};
  G4double eff  [5] = {0.0, 0.0, 0.0, 1.0, 1.0};
  
  SCproperties->AddProperty("EFFICIENCY", phEne, eff, 5);
  fMatCoating->GetMaterialOpticalSurf()->SetMaterialPropertiesTable(SCproperties);
*/
  fSkinSurface = new G4LogicalSkinSurface("PTP_LgSkin", fCoating->GetModLV(), fMatCoating->GetMaterialOpticalSurf());

  return fSkinSurface;
}


