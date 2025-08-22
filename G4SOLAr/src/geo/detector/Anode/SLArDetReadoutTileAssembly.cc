/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetReadoutTileAssembly.cc
 * @created     : Mon Jul 18, 2022 11:10:23 CEST
 */

#include "detector/Anode/SLArDetReadoutTileAssembly.hh"
#include "detector/Anode/SLArDetReadoutTile.hh"
#include "detector/SLArPlaneParameterisation.hpp"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVParameterised.hh"
#include "G4VPhysicalVolume.hh"

#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4VisAttributes.hh"
#include "G4MaterialPropertyVector.hh"

SLArDetReadoutTileAssembly::SLArDetReadoutTileAssembly() 
  : SLArBaseDetModule(), fMatReadoutPlane(nullptr), fTileRow(nullptr), fTileModel("")
{
  fGeoInfo = new SLArGeoInfo();  
}

SLArDetReadoutTileAssembly::SLArDetReadoutTileAssembly(const SLArDetReadoutTileAssembly& detReadoutPlane) 
  : SLArBaseDetModule(detReadoutPlane), fMatReadoutPlane(nullptr), fTileRow(nullptr), fTileModel(detReadoutPlane.fTileModel)
{
  fMatReadoutPlane = new SLArMaterial(*detReadoutPlane.fMatReadoutPlane);
}

SLArDetReadoutTileAssembly::~SLArDetReadoutTileAssembly()
{}

void SLArDetReadoutTileAssembly::BuildDefalutGeoParMap() 
{
  G4cout  << "SLArDetReadoutTileAssembly::BuildGeoParMap()" << G4endl;
  
  fGeoInfo->RegisterGeoPar("rdoutplane_z"   ,  1.0*CLHEP::m);
  fGeoInfo->RegisterGeoPar("rdoutplane_x"   ,  2.0*CLHEP::m);
}

void SLArDetReadoutTileAssembly::BuildMaterial(G4String materials_db)
{
  fMatReadoutPlane = new SLArMaterial(); 
  fMatReadoutPlane->SetMaterialID("LAr");
  fMatReadoutPlane->BuildMaterialFromDB(materials_db);
}

void SLArDetReadoutTileAssembly::BuildTileRow(SLArDetReadoutTile* tile) {
  fTileRow = new SLArBaseDetModule();
  fTileRow->SetMaterial(fMatReadoutPlane->GetMaterial());
  G4double tile_x  = tile->GetGeoPar("tile_x"); 
  G4double tile_y  = tile->GetGeoPar("tile_y")+tile->GetUnitCell()->GetGeoPar("cell_y");
  G4double tile_z  = tile->GetGeoPar("tile_z"); 
  G4double plane_z = fGeoInfo->GetGeoPar("rdoutplane_z"); 

  G4int n_z = std::floor(plane_z / tile_z); 

  G4double true_plane_z = tile_z*n_z;
  
  G4Box* tile_row_z_box = new G4Box("tile_row_z_box", 
      0.5*tile_x, 0.5*tile_y, 0.5*true_plane_z); 
  fTileRow->SetSolidVolume(tile_row_z_box);
  fTileRow->SetLogicVolume(
      new G4LogicalVolume(tile_row_z_box, 
      fMatReadoutPlane->GetMaterial(), "tile_row_z_lv") 
      );
  fTileRow->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 

  SLArPlaneParameterisation* rowTileParametrization = 
    new SLArPlaneParameterisation(
          kZAxis, G4ThreeVector(0, 0, -0.5*tile_z*(n_z-1)), tile_z);
  
  fTileRow->SetModPV(
      //new G4PVReplica("tile_row_z", tile->fModLV, fTileRow->GetModLV(), kZAxis, n_z, tile_z)
      new G4PVParameterised("tile_row_z", tile->GetModLV(), fTileRow->GetModLV(),
        kZAxis, n_z, rowTileParametrization, true) 
  );
  fTileRow->SetGeoPar("tilerow_x", tile_x);
  fTileRow->SetGeoPar("tilerow_y", tile_y);
  fTileRow->SetGeoPar("tilerow_z", true_plane_z);
}

void SLArDetReadoutTileAssembly::BuildReadoutPlane(SLArDetReadoutTile* tile) 
{
  G4double tile_x  = tile->GetGeoPar("tile_x"); 
  G4double tile_y  = tile->GetGeoPar("tile_y") + tile->GetUnitCell()->GetGeoPar("cell_y");
  G4double tile_z  = tile->GetGeoPar("tile_z"); 
  G4double plane_x = fGeoInfo->GetGeoPar("rdoutplane_x"); 
  G4double plane_z = fGeoInfo->GetGeoPar("rdoutplane_z"); 
  G4double plane_y = tile_y; 

  G4int n_z = std::floor(plane_z / tile_z); 
  G4int n_x = std::floor(plane_x / tile_x); 

  G4double true_plane_x = tile_x*n_x;
  G4double true_plane_z = tile_z*n_z;
  
  BuildTileRow(tile);

  fModSV = new G4Box("ReadoutPlane_box", 
      true_plane_x*0.5, plane_y*0.5, true_plane_z*0.5);
  fModLV = new G4LogicalVolume(fModSV, 
      fMatReadoutPlane->GetMaterial(), "ReadoutPlaneLV", 0, 0, 0, 1); 

  
  SLArPlaneParameterisation* planeParametrization =
    new SLArPlaneParameterisation(
        kXAxis, G4ThreeVector(-0.5*tile_x*(n_x-1), 0., 0.), tile_x); 

  SetModPV(
      //new G4PVReplica("ReadoutPlane", fTileRow->GetModLV(), fModLV, kXAxis, n_x, tile_x)
      new G4PVParameterised("ReadoutPlane", fTileRow->GetModLV(), fModLV,
        kXAxis, n_x, planeParametrization, true) 
  );
}

void SLArDetReadoutTileAssembly::SetVisAttributes(const int depth) {
  if (depth == 0) {
    fModLV->SetVisAttributes( G4VisAttributes( G4Colour(0.82, 0.82, 0.82) ) ); 
  }
  else {
    fModLV->SetVisAttributes( G4VisAttributes(false) ); 
  }
  return;
}







