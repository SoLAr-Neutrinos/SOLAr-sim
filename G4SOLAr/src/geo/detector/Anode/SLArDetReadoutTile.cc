/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetReadoutTile.cc
 * @created     : giovedì lug 14, 2022 09:33:56 CEST
 */


#include "geo/detector/Anode/SLArDetReadoutTile.hh"
#include "geo/detector/SLArPlaneParameterisation.hpp"
#include "geo/SLArGeoUtils.hh"
#include "detector/SLArPlaneParameterisation.hpp"
#include "config/SLArCfgReadoutTile.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4PVParameterised.hh"
#include "G4VPhysicalVolume.hh"

#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4VisAttributes.hh"
#include "G4MaterialPropertyVector.hh"

#include "TH2Poly.h"

SLArDetReadoutTile::SLArDetReadoutTile() : SLArBaseDetModule(),
  fPerfectQE(false),
  fBasePCB(nullptr), fBaseTile(nullptr), fChargePix(nullptr), 
  fSiPM(nullptr), fSiPMActive(nullptr), fUnitCell(nullptr),
  fMatReadoutTile(nullptr), fMatPCB(nullptr), fMatCopper(nullptr), 
  fMatSiPM(nullptr), fMatSiPMCapsule(nullptr), 
  fSkinSurface(nullptr)
{  
  fGeoInfo = new SLArGeoInfo();
}

SLArDetReadoutTile::SLArDetReadoutTile(const SLArDetReadoutTile &detReadoutTile) 
  : SLArBaseDetModule(detReadoutTile)
{
  fPerfectQE   = detReadoutTile.fPerfectQE;
  fGeoInfo     = detReadoutTile.fGeoInfo;

  fMatReadoutTile = new SLArMaterial(*detReadoutTile.fMatReadoutTile); 
  fMatPCB = new SLArMaterial(*detReadoutTile.fMatPCB);
  fMatCopper = new SLArMaterial(*detReadoutTile.fMatCopper);
  fMatSiPM   = new SLArMaterial(*detReadoutTile.fMatSiPM);
  fMatSiPMCapsule = new SLArMaterial(*detReadoutTile.fMatSiPMCapsule); 

  fBasePCB = new SLArBaseDetModule(*detReadoutTile.fBasePCB); 
  fBaseTile = new SLArBaseDetModule(*detReadoutTile.fBaseTile); 
  fSiPM = new SLArBaseDetModule(*detReadoutTile.fSiPM); 
  fSiPMActive = new SLArBaseDetModule(*detReadoutTile.fSiPMActive); 
  fUnitCell = new SLArBaseDetModule(*detReadoutTile.fUnitCell); 
  fChargePix = new SLArBaseDetModule(*detReadoutTile.fChargePix); 
}

SLArDetReadoutTile::~SLArDetReadoutTile() {
  G4cerr << "Deleting SLArDetReadoutTile... " <<  G4endl;

  if (fBasePCB)   {delete fBasePCB; fBasePCB = 0;}
  if (fBaseTile)   {delete fBaseTile; fBaseTile = 0;}
  if (fChargePix) {delete fChargePix; fChargePix = 0;}
  if (fSiPM)      {delete fSiPM; fSiPM = 0;}
  if (fSiPMActive){delete fSiPMActive; fSiPMActive = 0;} 
  if (fMatReadoutTile) {delete fMatReadoutTile; fMatReadoutTile = 0;}
  if (fMatPCB){delete fMatPCB; fMatPCB = 0;}
  if (fMatCopper){delete fMatCopper; fMatCopper = 0;}
  if (fMatSiPM)   {delete fMatSiPM; fMatSiPM = 0;}
  if (fMatSiPMCapsule)   {delete fMatSiPMCapsule; fMatSiPMCapsule = 0;}
  G4cerr << "SLArDetReadoutTile DONE" <<  G4endl;
}

void SLArDetReadoutTile::BuildPCB()
{
  G4cerr << "Building ReadoutTile PCB base" << G4endl;

  fBasePCB = new SLArBaseDetModule();
  fBasePCB->SetGeoPar(fGeoInfo->GetGeoPair("tile_z"));
  fBasePCB->SetGeoPar(fGeoInfo->GetGeoPair("tile_x"));
  fBasePCB->SetGeoPar("tile_y", fGeoInfo->GetGeoPar("tile_y") - 1*CLHEP::mm);

  fBasePCB->SetMaterial(fMatPCB->GetMaterial());

  G4VSolid* lgbox = 
    new G4Box("PCBBaseSolid", 
        0.5*fBasePCB->GetGeoPar("tile_x"),
        0.5*fBasePCB->GetGeoPar("tile_y"),
        0.5*fBasePCB->GetGeoPar("tile_z"));

  fBasePCB->SetSolidVolume(lgbox);
  fBasePCB->SetLogicVolume(
    new G4LogicalVolume(fBasePCB->GetModSV(), 
      fBasePCB->GetMaterial(),
      "PCBBaseLV", 0, 0, 0)
    );

  fBaseTile = new SLArBaseDetModule(); 
  fBaseTile->SetGeoPar(fGeoInfo->GetGeoPair("tile_z"));
  fBaseTile->SetGeoPar(fGeoInfo->GetGeoPair("tile_x"));
  fBaseTile->SetGeoPar("tile_y", 1*CLHEP::mm);

  fBaseTile->SetMaterial(fMatCopper->GetMaterial()); 

  fBaseTile->SetSolidVolume(new G4Box("TileBaseSolid", 
      0.5*fBaseTile->GetGeoPar("tile_x"), 
      0.5*fBaseTile->GetGeoPar("tile_y"), 
      0.5*fBaseTile->GetGeoPar("tile_z")
      )); 
  fBaseTile->SetLogicVolume( new G4LogicalVolume(
      fBaseTile->GetModSV(), fBaseTile->GetMaterial(), 
      "TileBaseLV", 0, 0, 0));

  //printf("tile dimensions: %.2f %.2f, %.2f mm\n", 
      //fGeoInfo->GetGeoPar("tile_x"), fGeoInfo->GetGeoPar("tile_y"), fGeoInfo->GetGeoPar("tile_z"));
}

void SLArDetReadoutTile::BuildSiPM()
{
  G4cout << "Building ReadoutTile SiPMs..." << G4endl;
  double fill_factor = fSiPM->GetGeoPar("sipm_fill_factor"); 
  double x_ = fSiPM->GetGeoPar("sipm_x"); 
  double y_ = fSiPM->GetGeoPar("sipm_y"); 
  double z_ = fSiPM->GetGeoPar("sipm_z"); 
  double d_ = 0.25*((x_ + z_) - sqrt(pow(x_ + z_, 2) - 4*x_*z_*(1-fill_factor))); 
  double x  = x_ - 2*d_; 
  double z  = z_ - 2*d_; 

  fSiPMActive = new SLArBaseDetModule(); 

  fSiPMActive = new SLArBaseDetModule(); 
  fSiPMActive->SetGeoPar("active_sipm_y", y_); 
  fSiPMActive->SetGeoPar("active_sipm_x", x ); 
  fSiPMActive->SetGeoPar("active_sipm_z", z ); 
  
  fSiPM->SetMaterial(fMatReadoutTile->GetMaterial());
  fSiPMActive->SetMaterial(fMatSiPM->GetMaterial()); 

  fSiPM->SetSolidVolume(
        new G4Box("SiPMBox", 
          0.5*x_, 0.5*y_, 0.5*z_));
 
  fSiPM->SetLogicVolume(
      new G4LogicalVolume(fSiPM->GetModSV(), 
        fMatReadoutTile->GetMaterial(), "SiPM_lv", 0, 0, 0)
      );

  fSiPMActive->SetSolidVolume(
      new G4Box("SiPMActiveBox", 0.5*x, 0.5*y_, 0.5*z) );

  fSiPMActive->SetLogicVolume(
    new G4LogicalVolume(fSiPMActive->GetModSV(), 
        fMatSiPM->GetMaterial(), "SiPMActive_lv", 0, 0, 0)
      );

  G4SubtractionSolid* sipm_passive_box = new G4SubtractionSolid("SiPMCapsuleBox", 
      fSiPM->GetModSV(), fSiPMActive->GetModSV(), 0, G4ThreeVector(0, 0, 0)); 

  G4LogicalVolume* capsule_lv = 
    new G4LogicalVolume(sipm_passive_box, 
        fMatSiPMCapsule->GetMaterial(), "SiPMCapsule_lv", 0, 0, 0); 

  new G4PVPlacement(0, G4ThreeVector(0, 0, 0), capsule_lv, "SiPMCapsulePV",
      fSiPM->GetModLV(), 0, 251, true);

  fSiPMActive->GetModPV("SiPMActivePV", 0, G4ThreeVector(0, 0, 0), 
      fSiPM->GetModLV(), 0, 250); 
}

void SLArDetReadoutTile::BuildChargePix()
{
  G4cout << "Building ReadoutTile ChargePixel..." << G4endl;
  fChargePix->SetMaterial(fMatSiPM->GetMaterial());

  fChargePix->SetSolidVolume(
        new G4Box("pixel_box", 
          0.5*fChargePix->GetGeoPar("pix_x"),
          0.5*fChargePix->GetGeoPar("pix_y"),
          0.5*fChargePix->GetGeoPar("pix_z"))
        );
 
  fChargePix->SetLogicVolume(
      new G4LogicalVolume(fChargePix->GetModSV(), 
        fChargePix->GetMaterial(), "pixel_lv", 0, 0, 0)
      );
}

void SLArDetReadoutTile::BuildUnitCell() {
  //--------------------------  Build Unit Cell components
  BuildSiPM();
  BuildChargePix(); 

  G4double hl = 0, hq = 0;

  if (fSiPM) hl = fSiPM->GetGeoPar("sipm_y"); 
  if (fChargePix) hq = fChargePix->GetGeoPar("pix_y"); 
  fUnitCell->SetGeoPar("cell_y", std::max(hl, hq)); 
  fUnitCell->SetSolidVolume(
   new G4Box("tileCellBox", 
      0.5*fUnitCell->GetGeoPar("cell_x"), 
      0.5*fUnitCell->GetGeoPar("cell_y"), 
      0.5*fUnitCell->GetGeoPar("cell_z"))); 
  fUnitCell->SetLogicVolume( 
      new G4LogicalVolume(
        fUnitCell->GetModSV(), 
        fMatReadoutTile->GetMaterial(), 
        "rdtile_cell_lv")); 
  fUnitCell->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 

  
  for (const auto& comp : fCellStructure) {
    G4ThreeVector yshift(0, 0, 0); 
    //if (comp.fMod == fChargePix && hq < hl) yshift.setY(0.5*(hq-hl));
    if (comp.fMod == fChargePix && hq < hl) continue; // spare some RAM 
                                                      // for DUNE-size module
    else if (comp.fMod == fSiPM && hl < hq) yshift.setY(0.5*(hl-hq));
    comp.fMod->GetModPV(comp.fName, 0, comp.fPos+yshift, 
        fUnitCell->GetModLV(),true, comp.fCopyNo); 
  }

}

void SLArDetReadoutTile::BuildReadoutTile()
{
  //--------------------------  Build ReadoutTile components
  BuildPCB();

  BuildUnitCell(); 

  //--------- Building a "empty" LV as ReadoutTile container
  G4cout << "SLArDetReadoutTile::BuildReadoutTile()" << G4endl;
  fhTot = fGeoInfo->GetGeoPar("tile_y") + fUnitCell->GetGeoPar("cell_y"); 

  fModSV = new G4Box("ReadoutTile",
      fGeoInfo->GetGeoPar("tile_x")*0.5,
      fhTot*0.5,
      fGeoInfo->GetGeoPar("tile_z")*0.5
      );

  fModLV
    = new G4LogicalVolume(fModSV, 
        fMatReadoutTile->GetMaterial(),
        "ReadoutTileLV",0,0,0);

  //----------------------------Place ReadoutTile components
  // 1a. The PCB base
  G4double h = 0*CLHEP::mm;
  G4cout<<"GetModPV PCB base..." << G4endl; 
  fBasePCB->GetModPV("ReadoutTilePCB", 0, 
      G4ThreeVector(0, -0.5*(fhTot-fBasePCB->GetGeoPar("tile_y")), 0),
      fModLV, false, 200);
  // 1a. The copper base
  fBaseTile->GetModPV("ReadoutTileBase", 0, 
      G4ThreeVector(0, -0.5*(fhTot-2*fBasePCB->GetGeoPar("tile_y")-1*CLHEP::mm), 0), 
      fModLV, false, 201); 
  
  
  // 3. Create a volume parametrization instance 
  G4double cell_z = fUnitCell->GetGeoPar("cell_z"); 
  G4double cell_y = fUnitCell->GetGeoPar("cell_y"); 
  G4double cell_x = fUnitCell->GetGeoPar("cell_x"); 
  G4double tile_z = fGeoInfo->GetGeoPar("tile_z"); 
  G4double tile_x = fGeoInfo->GetGeoPar("tile_x"); 

  G4int n_z = floor(tile_z / cell_z); 
  G4int n_x = floor(tile_x / cell_x); 

  SLArPlaneParameterisation* rowParametrization = 
    new SLArPlaneParameterisation(kZAxis, 
        G4ThreeVector(0, 0, -0.5*cell_z*(n_z-1)), 
        cell_z); 
  printf("Creating a row of %i sensor cells...\n", n_z);

  G4Box* cell_row_box = new G4Box("tileCellRow",0.5*cell_x,0.5*cell_y,0.5*cell_z*n_z); 
  G4LogicalVolume* cell_row_lv = new G4LogicalVolume(cell_row_box, fMatReadoutTile->GetMaterial(), "rdtile_cell_row_lv"); 
  cell_row_lv->SetVisAttributes( G4VisAttributes(false) ); 
  new G4PVParameterised("cell_row", fUnitCell->GetModLV(), cell_row_lv, kZAxis, n_z,
      rowParametrization, true); 
  
  // 4. Full sensor plane
  printf("Creating %i repilacas of rows...\n", n_x);
  SLArPlaneParameterisation* tplaneParametrization = 
    new SLArPlaneParameterisation(kXAxis, 
        G4ThreeVector(-0.5*cell_x*(n_x-1), 0, 0), 
        cell_x); 

  G4Box* cell_plane_box = new G4Box("tileCellPlane", 
      0.5*cell_x*n_x, 0.5*cell_y, 0.5*cell_z*n_z); 
  G4LogicalVolume* cell_plane_lv = new G4LogicalVolume(cell_plane_box, 
      fMatReadoutTile->GetMaterial(), "rdtile_cell_plane_lv"); 
  cell_plane_lv->SetVisAttributes( G4VisAttributes(false) ); 
  //new G4PVReplica("cell_plane", cell_row_lv, cell_plane_lv, kXAxis, 10, 3*dx); 
  new G4PVParameterised("cell_plane", cell_row_lv, cell_plane_lv, kXAxis, n_x, tplaneParametrization, true); 

  // 5. Final assembly (PCB + sensor plane)
  G4cout<<"Final placement..." << G4endl; 
  SetModPV( new G4PVPlacement(
      0, G4ThreeVector(0., 0.5*(fhTot-cell_y), 0.), 
      cell_plane_lv, "ReadoutTileSensors",fModLV, false, 50, false) );

   return;
}


void SLArDetReadoutTile::SetVisAttributes(const int depth)
{
  G4VisAttributes* visAttributes = new G4VisAttributes();
  visAttributes->SetColor(0.0824, 0.635, 0.019);
  if (depth > 0) {
    fBasePCB->GetModLV()->SetVisAttributes( visAttributes );
    fBaseTile->GetModLV()->SetVisAttributes( G4VisAttributes(true, G4Color(0.988, 0.906, 0.6)) ); 
  }
  else {
    fBasePCB->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    fBaseTile->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
  }

  visAttributes = new G4VisAttributes( G4Color(0.753, 0.753, 0.753) );
  if (depth > 1) {
    fSiPM->GetModLV()->SetVisAttributes( visAttributes );
  }
  else {
    fSiPM->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
  }
  for (size_t ii=0; ii < fSiPM->GetModLV()->GetNoDaughters(); ii++) {
    if (depth > 1) {
      fSiPM->GetModLV()->GetDaughter(ii)->GetLogicalVolume()->SetVisAttributes( visAttributes ); 
    }
    else {
      fSiPM->GetModLV()->GetDaughter(ii)->GetLogicalVolume()->SetVisAttributes( G4VisAttributes(false) ); 
    }
  }

  if (fChargePix) {
    visAttributes = new G4VisAttributes( G4Color(0.921, 0.659, 0.007) );
    if (depth > 1) {
      fChargePix->GetModLV()->SetVisAttributes( visAttributes );
    }
    else {
      fChargePix->GetModLV()->SetVisAttributes( G4VisAttributes(false) );
    }
  }

  visAttributes = new G4VisAttributes();
  visAttributes->SetColor(0.80, 0.80, 0.80);
  fModLV->SetVisAttributes( G4VisAttributes(false) );

  for (size_t ii=0; ii < fModLV->GetNoDaughters(); ii++) {
    auto lv = fModLV->GetDaughter(ii)->GetLogicalVolume(); 
    if ( std::strcmp(lv->GetName().data(), "rdtile_cell_plane_lv") == 0) {
      lv->SetVisAttributes( G4VisAttributes(false) ); 
    }
  }

  return;
}

SLArBaseDetModule* SLArDetReadoutTile::GetSiPMActive()
{
  return fSiPMActive;
}


SLArMaterial* SLArDetReadoutTile::GetSiPMActiveMaterial()
{
  return fMatSiPM;
}


void SLArDetReadoutTile::SetPerfectQE(G4bool kQE)
{
  if (fPerfectQE==kQE) return;
  else 
  {     
    fPerfectQE = kQE;
    G4cout << "SLArDetReadoutTile::SetPerfectQE: Setting 100% QE between "
           << "1 and 5 eV" << G4endl;
    G4double phEne[2] = {1*CLHEP::eV, 13*CLHEP::eV};
    G4double eff  [2] = {1.0 , 1.0 };
    
    fMatSiPM->GetMaterialOpticalSurf()
               ->GetMaterialPropertiesTable()
                  ->AddProperty("EFFICIENCY", phEne, eff, 2);  
  }

  return;
}

void SLArDetReadoutTile::BuildMaterial(G4String materials_db)
{
  fMatPCB         = new SLArMaterial();
  fMatCopper      = new SLArMaterial();
  fMatSiPM        = new SLArMaterial();
  fMatChargePix   = new SLArMaterial(); 
  fMatReadoutTile = new SLArMaterial();
  fMatSiPMCapsule = new SLArMaterial(); 

  fMatReadoutTile->SetMaterialID("LAr");
  fMatReadoutTile->BuildMaterialFromDB(materials_db);

  fMatPCB->SetMaterialID("PCB");
  fMatPCB->BuildMaterialFromDB(materials_db);

  fMatCopper->SetMaterialID("CopperPassive");
  fMatCopper->BuildMaterialFromDB(materials_db);

  fMatChargePix->SetMaterialID("Steel");
  fMatChargePix->BuildMaterialFromDB(materials_db);

  fMatSiPM->SetMaterialID("SiliconActive");
  fMatSiPM->BuildMaterialFromDB(materials_db);

  fMatSiPMCapsule->SetMaterialID("SiliconPassive");
  fMatSiPMCapsule->BuildMaterialFromDB(materials_db);
}

void SLArDetReadoutTile::BuildComponentsDefinition(const rapidjson::Value& comps) 
{
  assert(comps.IsArray()); 
  for (const auto& comp : comps.GetArray()) {
    assert(comp.HasMember("name")); 
    assert(comp.HasMember("dimensions")); 
    SLArBaseDetModule* mod = nullptr;
    if (std::strcmp(comp["name"].GetString(), "pixel") == 0) {
      if (fChargePix) mod = fChargePix; 
      else {fChargePix = new SLArBaseDetModule(); mod = fChargePix;}
      printf("SLArDetReadoutTile::BuildComponentsDefinition: %s [%p]\n", 
          comp["name"].GetString(), static_cast<void*>(mod));
    } 
    else if (std::strcmp(comp["name"].GetString(), "sipm") == 0) {
      if (fSiPM) mod = fSiPM; 
      else {fSiPM = new SLArBaseDetModule(); mod = fSiPM;}
      printf("SLArDetReadoutTile::BuildComponentsDefinition: %s [%p]\n", 
          comp["name"].GetString(), static_cast<void*>(mod));
    }
    else {
      printf("SLArDetReadoutTile::BuildComponentsDefinition: I don't know what a %s is.\n", comp["name"].GetString());
    }

    assert(comp["dimensions"].IsArray()); 
    mod->GetGeoInfo()->ReadFromJSON(comp["dimensions"].GetArray()); 
  }
}

void SLArDetReadoutTile::BuildUnitCellStructure(const rapidjson::Value& celldef) {
  assert(celldef.HasMember("dimensions")); 
  assert(celldef.HasMember("lineup")); 
  if (!fUnitCell) fUnitCell = new SLArBaseDetModule(); 
  fUnitCell->GetGeoInfo()->ReadFromJSON(celldef["dimensions"].GetArray());

  assert(celldef["lineup"].IsArray()); 
  for (const auto& comp : celldef["lineup"].GetArray()) {
    SLArBaseDetModule* mod = nullptr; 
    if (std::strcmp(comp["component"].GetString(), "pixel") == 0) mod = fChargePix; 
    else if (std::strcmp(comp["component"].GetString(), "sipm") == 0) mod = fSiPM; 
    else {
      printf("SLArDetReadoutTile::BuildUnitCellStructure: Unknown component %s in lineup\n", comp["component"].GetString());
    }

    G4String name_ = comp["name"].GetString(); 
    G4int copy_ = comp["copy"].GetInt(); 
    G4ThreeVector pos_ = G4ThreeVector(0, 0, 0); 
    if (comp.HasMember("pos_x")) pos_.setX(unit::ParseJsonVal(comp["pos_x"]));
    if (comp.HasMember("pos_y")) pos_.setY(unit::ParseJsonVal(comp["pos_y"]));
    if (comp.HasMember("pos_z")) pos_.setZ(unit::ParseJsonVal(comp["pos_z"]));

    fCellStructure.push_back( SUnitCellComponent(name_, copy_, mod, pos_) );
  }

  assert(celldef.HasMember("pixelmap")); 
  BuildUnitCellPixMap(celldef["pixelmap"]); 
}


void SLArDetReadoutTile::BuildUnitCellPixMap(const rapidjson::Value& pixblueprint) {
  assert(pixblueprint.IsArray()); 
  for (const auto& pix : pixblueprint.GetArray()) {
    assert(pix.HasMember("name")); 
    assert(pix.HasMember("edges")); 
    assert(pix["edges"].IsArray());
    SUnitCellPixelArea pixArea(pix["name"].GetString()); 

    for (const auto &edge : pix["edges"].GetArray()) {
      G4ThreeVector pos_ = G4ThreeVector(0, 0, 0); 
      if (edge.HasMember("x")) pos_.setX(unit::ParseJsonVal(edge["x"]));
      if (edge.HasMember("y")) pos_.setY(unit::ParseJsonVal(edge["y"]));
      if (edge.HasMember("z")) pos_.setZ(unit::ParseJsonVal(edge["z"]));

      pixArea.fEdges.push_back(pos_); 
    }

    fCellPixelMap.push_back(pixArea); 
  }
}

G4LogicalSkinSurface* SLArDetReadoutTile::BuildLogicalSkinSurface() {
  fSkinSurface = 
    new G4LogicalSkinSurface(
        "SiPM_LgSkin", 
        fSiPMActive->GetModLV(), 
        fMatSiPM->GetMaterialOpticalSurf());

  return fSkinSurface;
}

TH2Poly* SLArDetReadoutTile::BuildTileChgPixelMap(
    const G4ThreeVector& xAxis, const G4ThreeVector& yAxis, 
    const G4ThreeVector* _shift, const G4RotationMatrix* _rot) {

#ifdef SLAR_DEBUG
  printf("SLArDetReadoutTile::BuildTileChgPixelMap():\n");
  if (_shift) G4cout<< "shift: " << *_shift << G4endl;
  if (_rot) G4cout << "rot: " << *_rot << G4endl; 
#endif

  TH2Poly* h2 = new TH2Poly("h2TileChgPixMap", 
      "Tile charge pixel map", 
      -0.5*fGeoInfo->GetGeoPar("tile_z"), + 0.5*fGeoInfo->GetGeoPar("tile_z"), 
      -0.5*fGeoInfo->GetGeoPar("tile_x"), +0.5*fGeoInfo->GetGeoPar("tile_x")); 
  h2->SetFloat(); 

  auto tilesens_pv = fModLV->GetDaughter(2)->GetLogicalVolume()->GetDaughter(0);
  G4int n_row = 0; G4int n_cell = 0; 
  G4double crow_x = 0; G4double cell_z = 0.; 
  G4ThreeVector crow_pos0;  G4ThreeVector crow_vaxis;
  G4ThreeVector cell_pos0;  G4ThreeVector cell_vaxis; 


  if (tilesens_pv->IsParameterised()) {
    auto trow_parametrization = 
      (SLArPlaneParameterisation*)tilesens_pv->GetParameterisation(); 
    EAxis axis_ = kUndefined; G4double offset_ = 0.; G4bool cons_;
    tilesens_pv->GetReplicationData(axis_, n_row, crow_x, offset_, cons_); 
    crow_x = trow_parametrization->GetSpacing(); 
    crow_vaxis = trow_parametrization->GetReplicationAxisVector(); 
    crow_pos0 = trow_parametrization->GetStartPos(); 

    // getting cell_row_pv
    auto crow_pv = tilesens_pv->GetLogicalVolume()->GetDaughter(0); 
    if (crow_pv->IsParameterised()) {
      auto crow_parametrization = 
        (SLArPlaneParameterisation*)crow_pv->GetParameterisation(); 
      EAxis caxis_ = kUndefined; G4double coffset_ = 0.; G4bool ccons_;
      crow_pv->GetReplicationData(caxis_, n_cell, cell_z, coffset_, ccons_); 
      cell_z = crow_parametrization->GetSpacing(); 
      cell_vaxis = crow_parametrization->GetReplicationAxisVector(); 
      cell_pos0 = crow_parametrization->GetStartPos();

    } else {
      printf("%s is not a parameterised volume\n", crow_pv->GetName().c_str());
    }

    //now build the pixel map
    for (G4int ix=0; ix<n_row; ix++) {
      G4ThreeVector row_pos = crow_pos0 + ix*crow_x*crow_vaxis;
      for (G4int iz=0; iz<n_cell; iz++) {
        G4ThreeVector cell_pos = row_pos + cell_pos0 + iz*cell_z*cell_vaxis; 

        //printf("cell_pos:[%.2f, %.2f, %.2f]\n",cell_pos.x(),cell_pos.y(),cell_pos.z());
        //printf("----------------------------------------------------------------\n");
        for (const auto &cc : fCellPixelMap) {
          std::vector<SLArCfgReadoutTile::xypoint> xypoints; 
          //G4ThreeVector pix_pos = cc.fPos + cell_pos + pos_tile; 
          //G4ThreeVector pix_pos_= pix_pos; pix_pos_.transform(*mtile_rot_inv); 
          //printf("pix_pos  : (%.2f, %.2f, %.2f)\n", pix_pos .x(), pix_pos .y(), pix_pos .z());
          //printf("pix_pos_r: (%.2f, %.2f, %.2f)\n", pix_pos_.x(), pix_pos_.y(), pix_pos_.z());
          //printf("phi_x: %g, theta_x: %g, phi_y = %g, theta_y = %g, phi_z= %g, theta_z = %g\n", 
          //TMath::RadToDeg()*mtile_rot_inv->phiX(), TMath::RadToDeg()*mtile_rot_inv->thetaX(), 
          //TMath::RadToDeg()*mtile_rot_inv->phiY(), TMath::RadToDeg()*mtile_rot_inv->thetaY(), 
          //TMath::RadToDeg()*mtile_rot_inv->phiZ(), TMath::RadToDeg()*mtile_rot_inv->thetaZ());
          //G4ThreeVector vpoint[5]; 
          //vpoint[0] = pix_pos + G4ThreeVector(-0.5*pix_x, 0, -0.5*pix_z);
          //vpoint[1] = pix_pos + G4ThreeVector(-0.5*pix_x, 0,  0.5*pix_z);
          //vpoint[2] = pix_pos + G4ThreeVector( 0.5*pix_x, 0,  0.5*pix_z);
          //vpoint[3] = pix_pos + G4ThreeVector( 0.5*pix_x, 0, -0.5*pix_z);
          //vpoint[4] = pix_pos + G4ThreeVector(-0.5*pix_x, 0, -0.5*pix_z);

          //for (int ipix = 0; ipix<5; ipix++) {
          //G4ThreeVector pix_phys = mtile_pos + vpoint[ipix].transform(*mtile_rot_inv);
          ////printf("pix[%i]: %.2f, %.2f, %.2f mm\n", ipix, pix_phys.x(), pix_phys.y(), pix_phys.z());
          //SLArCfgReadoutTile::xypoint p = { pix_phys.z(), pix_phys.y() }; 
          //xypoints.push_back( p ); 
          //}

          TGraph* g = new TGraph(); 
          int ipoint = 0; 
//#ifdef SLAR_DEBUG
          //printf("Registering pixel collection area:\n");
//#endif

          for (const auto &edge : cc.fEdges) {
            G4ThreeVector edge_pos = edge + cell_pos; 
            G4ThreeVector edge_phys(0, 0, 0); 
            if (_shift) edge_pos += *(_shift); 
            if (_rot) {
              edge_phys = edge_pos.transform(*(_rot));
            } else {
              edge_phys = edge_pos;
            }

            G4double x = edge_phys.dot(xAxis); 
            G4double y = edge_phys.dot(yAxis); 
//#ifdef SLAR_DEBUG
            //printf("Adding point: %g, %g mm\n", x, y);
//#endif
            g->SetPoint(ipoint, x, y); 
            ipoint++; 
          }

          h2->AddBin( std::move(g) ); 
//#ifdef SLAR_DEBUG
          //printf("----------------------------------------------------------------\n");
//#endif
        }
      }
    }
  } else {
    printf("SLArDetReadoutTile::BuildTileChgPixelMap WARNING\n");
    printf("Selected physical volume (%s) is not parameterised!\n", tilesens_pv->GetName().c_str());
    exit(EXIT_FAILURE);
  }

  return h2;
}

G4int SLArDetReadoutTile::GetNumberOfCellRows() const {
  if (fModLV == nullptr) {
    fprintf(stdout, "SLArDetReadoutTile::GetNumberOfCellRows: WARNING fModLV is null\n");
  }

  G4VPhysicalVolume* pv = nullptr; 
  // get cell plane from daughter volumes
  for (size_t ii=0; ii < fModLV->GetNoDaughters(); ii++) {
    auto pv_tmp = fModLV->GetDaughter(ii);
    if ( pv_tmp->GetName() == "ReadoutTileSensors") {
      pv = pv_tmp;
      break; 
    }
  }
  if (pv == nullptr) {
    fprintf(stdout, "SLArDetReadoutTile::GetNumberOfCellRows: ERROR ReadoutTileSensors not found\n");
    exit(EXIT_FAILURE);
  }

  G4int n_row = 0;
  pv = pv->GetLogicalVolume()->GetDaughter(0); // access cell_plane
  if (pv->IsParameterised()) {
    const G4PVParameterised* parpv = static_cast<const G4PVParameterised*>(pv);
    auto rep_data = get_plane_replication_data(parpv);
    n_row = rep_data.fNreplica;
  }

  return n_row;
}

G4int SLArDetReadoutTile::GetNumberOfCellCols() const {
  if (fModLV == nullptr) {
    fprintf(stdout, "SLArDetReadoutTile::GetNumberOfCellCols: WARNING fModLV is null\n");
  }

  G4VPhysicalVolume* plane_pv = nullptr; 
  // get plane pv from daughter volumes
  for (size_t ii=0; ii < fModLV->GetNoDaughters(); ii++) {
    auto pv_tmp = fModLV->GetDaughter(ii);
    if ( pv_tmp->GetName() == "ReadoutTileSensors") {
      plane_pv = pv_tmp;
      break; 
    }
  }
  if (plane_pv == nullptr) {
    fprintf(stdout, "SLArDetReadoutTile::GetNumberOfCellCols: ERROR ReadoutTileSensors not found\n");
    exit(EXIT_FAILURE);
  }

  G4int n_col = 0;
  const auto plane_lv = plane_pv->GetLogicalVolume()->GetDaughter(0)->GetLogicalVolume(); 

  G4VPhysicalVolume* row_pv = nullptr;
  for (size_t ii=0; ii < plane_lv->GetNoDaughters(); ii++) {
    auto pv_tmp = plane_lv->GetDaughter(ii);
    if ( pv_tmp->GetName() == "cell_row") {
      row_pv = pv_tmp;
      break; 
    }
  }
  if (row_pv == nullptr) {
    fprintf(stdout, "SLArDetReadoutTile::GetNumberOfCellCols: ERROR cell_row not found\n");
    exit(EXIT_FAILURE);
  }


  if (row_pv->IsParameterised()) {
    const G4PVParameterised* parpv = static_cast<const G4PVParameterised*>(row_pv);
    auto rep_data = get_plane_replication_data(parpv);
    n_col = rep_data.fNreplica;
  }

  return n_col;
}
