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
  fMatSuperCell(nullptr), fMatLightGuide(nullptr), fMatCoating(nullptr), fMatWLSCoating(nullptr),
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
  fMatWLSCoating = new SLArMaterial(*detSuperCell.fMatWLSCoating);

}

SLArDetSuperCell::~SLArDetSuperCell() {
  G4cerr << "Deleting SLArDetSuperCell... " <<  G4endl;
  if (fLightGuide)   {delete fLightGuide; fLightGuide = 0;}
  if (fCoating)      {delete fCoating; fCoating = 0;}
  if (fMatSuperCell) {delete fMatSuperCell; fMatSuperCell = 0;}
  if (fMatLightGuide){delete fMatLightGuide; fMatLightGuide = 0;}
  if (fMatCoating)   {delete fMatCoating; fMatCoating = 0;}
  if (fMatWLSCoating){delete fMatWLSCoating; fMatWLSCoating = 0;}
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
  G4cout << "Building SuperCell sensitive coating..." << G4endl;
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
  G4cout << "Building SuperCell wavelength-shifting Coating..." << G4endl;
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
void SLArDetSuperCell::BuildSuperCell()
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

  G4cout << "SLArDetSuperCell::BuildSuperCell()" << G4endl;

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
  printf("SLArDetSuperCell::BuildSuperCell: placing components...\n");
  printf("  LightGuide: %s\n", fLightGuide->GetModLV()->GetName().c_str());
  G4ThreeVector pos(0, -0.5*(fhTot-fLightGuide->GetGeoPar("cell_y")), 0);
  fLightGuide->GetModPV("SuperCellLightGuide", 0, pos, fModLV, false, 100);

  printf("  Photo-sensitive coating: %s\n", fCoating->GetModLV()->GetName().c_str());
  pos[1] += 0.5*(fLightGuide->GetGeoPar("cell_y") + fCoating->GetGeoPar("coating_y"));
  fCoating->GetModPV("SuperCellCoating", 0, pos, fModLV, false, 101);

  if (fWLSCoating) {
    printf("  WLS coating: %s\n", fWLSCoating->GetModLV()->GetName().c_str());
    pos[1] += 0.5*(fCoating->GetGeoPar("coating_y") + fWLSCoating->GetGeoPar("wlscoating_y"));
    fWLSCoating->GetModPV("SuperCellWLSCoating", 0, pos, fModLV, false, 102);
  }
  return;
}


void SLArDetSuperCell::SetVisAttributes()
{
  G4VisAttributes* LGvisAttributes = new G4VisAttributes();
  LGvisAttributes->SetColor(0.862, 0.952, 0.976, 0.5);
  fLightGuide->GetModLV()->SetVisAttributes( LGvisAttributes );

  G4VisAttributes* CoatingvisAttributes = new G4VisAttributes( G4Color(0.968, 0.494, 0.007) );
  fCoating->GetModLV()->SetVisAttributes( CoatingvisAttributes );

  if (fWLSCoating) {
    G4VisAttributes* WLSvisAttributes = new G4VisAttributes( G4Color(0.0, 0.8, 0.0) );
    fWLSCoating->GetModLV()->SetVisAttributes( WLSvisAttributes );
  }

  fModLV->SetVisAttributes( G4VisAttributes( false ) ); ;

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
    G4cout << "SLArDetSuperCell::SetPerfectQE: Setting 100% QE between "
           << "2 and 5 eV" << G4endl;
    G4double phEne[2] = {2*CLHEP::eV, 5*CLHEP::eV};
    G4double eff  [2] = {1.0 , 1.0 };
    
    //fMatCoating->GetMaterialBuilder()->GetSurface()
               //->GetMaterialPropertiesTable()
               //->AddProperty("EFFICIENCY", phEne, eff, 2);  
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
  fMatWLSCoating   = new SLArMaterial();

  fMatSuperCell->SetMaterialID("LAr");
  fMatSuperCell->BuildMaterialFromDB(materials_db);

  fMatLightGuide->SetMaterialID("Plastic");
  fMatLightGuide->BuildMaterialFromDB(materials_db);

  fMatCoating->SetMaterialID("PTP_sensitive");
  fMatCoating->BuildMaterialFromDB(materials_db);

  fMatWLSCoating->SetMaterialID("PTP_wls");
  fMatWLSCoating->BuildMaterialFromDB(materials_db);
}

G4LogicalSkinSurface* SLArDetSuperCell::BuildLogicalSkinSurface() {
  fSkinSurface = 
    new G4LogicalSkinSurface(
        "PTP_LgSkin", 
        fCoating->GetModLV(), 
        fMatCoating->GetMaterialOpticalSurf());

  return fSkinSurface;
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

