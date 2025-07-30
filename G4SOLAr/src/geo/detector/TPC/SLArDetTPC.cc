/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetTPC.cc
 * @created     Thur Nov 03, 2022 12:23:21 CET
 */


#include "detector/SLArDetectorConstruction.hh"
#include "detector/TPC/SLArDetTPC.hh"
#include "detector/SLArPlaneParameterisation.hpp"

#include "SLArAnalysisManager.hh"

#include "G4PhysicalConstants.hh"
#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Tubs.hh"
#include "G4Ellipsoid.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4VisAttributes.hh"
#include "G4PVParameterised.hh"


SLArDetTPC::SLArDetTPC() : SLArBaseDetModule(),
  fMatTarget(nullptr), fMatFieldCage(nullptr), fFieldCage(nullptr), fFieldCageVisibility(true)
{
  fGeoInfo = new SLArGeoInfo();
}


SLArDetTPC::~SLArDetTPC() {
  std::cerr << "Deleting SLArDetTPC..." << std::endl;
  std::cerr << "SLArDetTPC DONE" << std::endl;
}

void SLArDetTPC::BuildMaterial(G4String db_file) 
{
  // TODO: IMPLEMENT PROPER MATERIALS IN /materials
  fMatTarget = new SLArMaterial();
  fMatFieldCage = new SLArMaterial(); 

  fMatTarget->SetMaterialID("LAr");
  fMatTarget->BuildMaterialFromDB(db_file);

  fMatFieldCage->SetMaterialID("Steel"); 
  fMatFieldCage->BuildMaterialFromDB(db_file); 
}

void SLArDetTPC::BuildDefalutGeoParMap() 
{
  G4cerr << "SLArDetTPC::BuildGeoParMap()" << G4endl;
  fGeoInfo->RegisterGeoPar("tpc_y"       , 150.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("tpc_z"       , 200.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("tpc_x"       ,  60.0*CLHEP::cm); 
  G4cerr << "Exit method\n" << G4endl;
}

void SLArDetTPC::BuildFieldCage() {
  const G4ThreeVector axis[3] = {
    G4ThreeVector(1, 0, 0), 
    G4ThreeVector(0, 1, 0), 
    G4ThreeVector(0, 0, 1)};
  const G4String axis_label[3] = {"x", "y", "z"};

  auto fcGeoInfo = fFieldCage->GetGeoInfo(); 
  const G4double R = fcGeoInfo->GetGeoPar("corner_radius"); 
  const G4double tk = fcGeoInfo->GetGeoPar("thickness");
  const G4double hl = fcGeoInfo->GetGeoPar("height_long_side"); 
  const G4double hs = fcGeoInfo->GetGeoPar("height_short_side"); 
  const G4double wd = fcGeoInfo->GetGeoPar("wall_distance"); 
  const G4double sp = fcGeoInfo->GetGeoPar("spacing"); 
  // field cage dimensions: in this (arbitrary) frame x = drift direction
  G4ThreeVector Dfc(0., 0., 0.); 
  G4double DDriftLength = 0; 
  Dfc.setX( fFieldCage->GetGeoPar("height_short_side") ); 
  G4int idim = 1;
  for (int i=0; i<3; i++) {
    if (fElectronDriftDir.dot(axis[i]) == 0) {
      Dfc[idim] = fGeoInfo->GetGeoPar("tpc_"+axis_label[i]) - wd; 
      idim++;
    } else {
      DDriftLength = fGeoInfo->GetGeoPar("tpc_"+axis_label[i]) - 2*CLHEP::cm; 
    }
  }
  
  G4double hy = 0.; 
  G4double hz = 0.; 
  if ( Dfc[1] > Dfc[2] ) {hy = hl; hz = hs;} else {hy = hs; hz = hl;}
  

  // build solid and logical volumes
  auto fc_corner_tub = new G4Tubs("fc_corner_sv", 
      R-tk, R, 0.5*std::max(hl, hs), 0, 90*CLHEP::deg); 
  auto fc_corner_lv = new G4LogicalVolume(fc_corner_tub, fMatFieldCage->GetMaterial(), 
      "fc_corner_lv"); 
  G4double dly = R; G4double dlz = R; 
  auto fc_yside_box = new G4Box("fc_lside_box", 
      0.5*hy, 0.5*tk, 0.5*Dfc[1] - dly); 
  auto fc_zside_box = new G4Box("fc_sside_box", 
      0.5*hz, 0.5*tk, 0.5*Dfc[2] - dlz);
  auto fc_zside_lv = new G4LogicalVolume(fc_zside_box, fMatFieldCage->GetMaterial(), 
      "fc_lside_lv"); 
  auto fc_yside_lv = new G4LogicalVolume(fc_yside_box, fMatFieldCage->GetMaterial(), 
      "fc_sside_lv"); 

  G4ThreeVector _cornerTubAxis = G4ThreeVector(0, 0, 1); 

  fFieldCage->GetGeoInfo()->DumpParMap(); 
  G4Box* layer_outer_box = new G4Box("fc_layer_outerBox", 
      0.5*Dfc[0], 0.5*Dfc[1], 0.5*Dfc[2]); 
  G4double deltaDim = R-(R-tk)*cos( 45*CLHEP::deg ) + 1*CLHEP::mm; 
  G4Box* layer_inner_box = new G4Box("fc_layer_innerBox", 
      0.5*Dfc[0], 0.5*Dfc[1]-deltaDim, 0.5*Dfc[2]-deltaDim); 

  auto fc_layer_sv = 
    new G4SubtractionSolid("fc_layer_sv", layer_outer_box, layer_inner_box);  
  auto fc_layer_lv = new G4LogicalVolume(fc_layer_sv, fMatTarget->GetMaterial(), 
      "fieldCage_layer_lv"); 
  fc_layer_lv->SetVisAttributes( G4VisAttributes(false) ); 
  // place the corners
  G4double delta = _cornerTubAxis.angle( G4ThreeVector(1, 0, 0) ); 
  G4RotationMatrix* rot_common = 
    new G4RotationMatrix(_cornerTubAxis.cross(G4ThreeVector(1, 0, 0)), delta); 
  G4RotationMatrix* rot0 = new G4RotationMatrix(*rot_common); 
  G4RotationMatrix* rot1 = new G4RotationMatrix(*rot_common); 
  rot1->rotateZ(-0.5*CLHEP::pi); 
  G4RotationMatrix* rot2 = new G4RotationMatrix(*rot_common); 
  rot2->rotateZ(-1.0*CLHEP::pi); 
  G4RotationMatrix* rot3 = new G4RotationMatrix(*rot_common); 
  rot3->rotateZ(-1.5*CLHEP::pi); 
  /*auto corner_0 = */
  new G4PVPlacement(rot0, G4ThreeVector(0, 0.5*Dfc[1]-R, 0.5*Dfc[2]-R),
      fc_corner_lv, "fc_corner0_pv", fc_layer_lv, false, 10); 
  /*auto corner_1 = */
  new G4PVPlacement(rot1, G4ThreeVector(0, 0.5*Dfc[1]-R, -(0.5*Dfc[2]-R)),
      fc_corner_lv, "fc_corner1_pv", fc_layer_lv, false, 11); 
  /*auto corner_2 = */
  new G4PVPlacement(rot2, G4ThreeVector(0, -(0.5*Dfc[1]-R), -(0.5*Dfc[2]-R)),
      fc_corner_lv, "fc_corner2_pv", fc_layer_lv, false, 12);   
  /*auto corner_3 = */
  new G4PVPlacement(rot3, G4ThreeVector(0, -(0.5*Dfc[1]-R), +(0.5*Dfc[2]-R)),
      fc_corner_lv, "fc_corner3_pv", fc_layer_lv, false, 13);   

  // place the sides
  /*auto yside_0 = */
  new G4PVPlacement(new G4RotationMatrix(axis[0], 0.5*CLHEP::pi), 
      G4ThreeVector(0, 0, -0.5*(Dfc[2]-tk)),
      fc_yside_lv, "fc_yside0_pv", fc_layer_lv, false, 14);   
  /*auto yside_1 = */
  new G4PVPlacement(new G4RotationMatrix(axis[0], 0.5*CLHEP::pi), 
      G4ThreeVector(0, 0, +0.5*(Dfc[2]-tk)),
      fc_yside_lv, "fc_yside1_pv", fc_layer_lv, false, 15);   
  /*auto zside_0 = */
  new G4PVPlacement(nullptr, 
      G4ThreeVector(0, -0.5*(Dfc[1]-tk), 0.),
      fc_zside_lv, "fc_zside0_pv", fc_layer_lv, false, 16);   
  /*auto zside_1 = */
  new G4PVPlacement(0, 
      G4ThreeVector(0, +0.5*(Dfc[1]-tk), 0.),
      fc_zside_lv, "fc_zside1_pv", fc_layer_lv, false, 17);   

  auto fcvolume_outer_box = new G4Box("fcvolume_outer_box", 
      0.5*DDriftLength, 0.5*Dfc[1], 0.5*Dfc[2] ); 
  auto fcvolume_inner_box = new G4Box("fcvolume_inner_box", 
      0.5*DDriftLength, 0.5*Dfc[1]-deltaDim, 0.5*Dfc[2]-deltaDim ); 
  auto fc_volume_sv = new G4SubtractionSolid("fc_volume_sv", 
      fcvolume_outer_box, fcvolume_inner_box); 
  auto fc_volume_lv = new G4LogicalVolume(fc_volume_sv, fMatTarget->GetMaterial(), 
      "fc_volume_lv"); 
  fc_volume_lv->SetVisAttributes( G4VisAttributes(false) ); 
  fFieldCage->SetSolidVolume(fc_volume_sv); 
  fFieldCage->SetLogicVolume(fc_volume_lv); 


  // compute field cage true length
  G4double len = hs;
  G4double len_tmp = len;
  G4int n_replica = 0; 
  while (len_tmp <= DDriftLength) {
    len = len_tmp; 
    n_replica++;
    len_tmp += sp; 
  } 

  auto parameterisation = new SLArPlaneParameterisation(kXAxis, 
      G4ThreeVector(-0.5*(len - sp + hl) , 0, 0), sp); 

  G4String fc_name = G4String("fieldCage_") + std::to_string(fID) + "_ppv";
  fFieldCage->SetModPV( new G4PVParameterised(fc_name, 
      fc_layer_lv, fc_volume_lv, 
      parameterisation->GetReplicationAxis(), n_replica, parameterisation) 
      );
  fFieldCage->GetModPV()->SetCopyNo(20);

  return;
}

void SLArDetTPC::BuildTPC() 
{

  G4cerr << "SLArDetTPC::BuildTPC()" << G4endl;
  //* * * * * * * * * * * * * * * * * * * * * * * * * * *//
  // Building the Target                                 //
  G4cerr << "\tBuilding TPC LAr volume" << G4endl;
  G4double tpcX= fGeoInfo->GetGeoPar("tpc_x");
  G4double tpcY= fGeoInfo->GetGeoPar("tpc_y");
  G4double tpcZ= fGeoInfo->GetGeoPar("tpc_z");
  
  // Create and fill fTarget
  G4double x_ = tpcX*0.5;
  G4double y_ = tpcY*0.5;
  G4double z_ = tpcZ*0.5;

  fModSV = new G4Box("TPC", x_, y_, z_);

  SetLogicVolume(
    new G4LogicalVolume(fModSV, 
      fMatTarget->GetMaterial(),
      "TPC"+std::to_string(fID)+"_lv", 0, 0, 0)
    );

  if (fFieldCage) {
    BuildFieldCage(); 

    G4RotationMatrix* rot = new G4RotationMatrix(); 
    const auto _fcAxis = G4ThreeVector(1, 0, 0); 
    const auto _fieldDir   = fElectronDriftDir;
    const auto _angle = _fieldDir.angle(_fcAxis);
    auto rot_axis = _fieldDir.cross(_fcAxis); 
    if (rot_axis.mag2() < 1e-6) rot_axis = _fcAxis;
    rot->set(rot_axis, _angle); 
    fFieldCage->GetModPV("field_cage", rot, G4ThreeVector(0, 0, 0), this->GetModLV(), 0, 99); 
  }
}

const G4ThreeVector SLArDetTPC::GetTPCcenter() {
  if ( fGeoInfo->Contains("tpc_pos_x") && 
       fGeoInfo->Contains("tpc_pos_y") &&
       fGeoInfo->Contains("tpc_pos_z") ) {
    return G4ThreeVector(fGeoInfo->GetGeoPar("tpc_pos_x"), 
        fGeoInfo->GetGeoPar("tpc_pos_y"), 
        fGeoInfo->GetGeoPar("tpc_pos_z")); 
  }

  return G4ThreeVector(0., 0., 0.); 
}

void SLArDetTPC::SetVisAttributes()
{
  G4cout << "SLArDetTPC::SetVisAttributes()" << G4endl;

  G4VisAttributes* visAttributes = new G4VisAttributes();

  visAttributes->SetVisibility(true); 
  visAttributes->SetColor(0.607, 0.847, 0.992, 0.1);
  fModLV->SetVisAttributes( G4VisAttributes(false) );

  return;
}

void SLArDetTPC::SetFieldCageVisibility(const G4bool val) {
  G4Colour col(0.3, 0.3, 0.3, 0.45);
  G4VisAttributes vis;

  fFieldCageVisibility = val;

  (fFieldCageVisibility) ? vis.SetColor( col ) : vis.SetVisibility( false );

  const size_t ndaughters = fFieldCage->GetModLV()->GetNoDaughters();
  for (size_t i = 0; i < ndaughters; i++) {
    G4LogicalVolume* lv = fFieldCage->GetModLV()->GetDaughter(i)->GetLogicalVolume(); 
    const size_t lv_nd = lv->GetNoDaughters();
    for (size_t j = 0; j < lv_nd; j++) {
      G4LogicalVolume* llv = lv->GetDaughter(j)->GetLogicalVolume();
      llv->SetVisAttributes( vis ); 
    }
  }

  return;
}

void SLArDetTPC::Init(const rapidjson::Value& jconf) {
  assert(jconf.IsObject()); 
  auto jtpc = jconf.GetObject(); 
  assert(jtpc.HasMember("dimensions")); 
  assert(jtpc.HasMember("position"  )); 
  assert(jtpc.HasMember("copyID"    )); 

  SetID(jtpc["copyID"].GetInt()); 

  fGeoInfo->ReadFromJSON(jtpc["dimensions"].GetArray()); 

  const auto jposition = jtpc["position"].GetObj(); 
  G4double vunit = 1.0; 
  if (jposition.HasMember("unit")) {
    vunit = G4UIcommand::ValueOf(jposition["unit"].GetString());
  }
  G4String xvar[3] = {"x", "y", "z"}; 
  assert(jposition.HasMember("xyz")); 
  int ii=0; 
  for (const auto &v : jposition["xyz"].GetArray()) {
    G4double tmp = v.GetDouble() * vunit; 
    fGeoInfo->RegisterGeoPar("tpc_pos_"+xvar[ii], tmp); 
    ++ii; 
  }

  if (jtpc.HasMember("electric_field")) {
    fElectricField = unit::ParseJsonVal( jtpc["electric_field"] ); 
    printf("electric_field is %g kV/cm\n", fElectricField/(CLHEP::kilovolt/CLHEP::cm)); 
    auto jfield = jtpc["electric_field"].GetObj(); 
    auto jdir   = jfield["direction"].GetArray(); 
    assert(jdir.Size() == 3); 
    int idim = 0; 
    for (const auto &v : jdir) {
      fElectronDriftDir[idim] = v.GetDouble();
      idim++; 
    }
  } 
  else {
    printf("SLArDetTPC::Init WARNING: Electric field properties are not specified. "); 
    printf("Setting default values: E = 0.5 kV/cm, drift dir (1, 0, 0)\n"); 
    fElectricField = 0.5*(CLHEP::kilovolt/CLHEP::cm); 
    fElectronDriftDir = G4ThreeVector(1, 0, 0); 
  } 

  if (jtpc.HasMember("field_cage")) {
    InitFieldCage(jtpc["field_cage"]); 
  }
}

void SLArDetTPC::InitFieldCage(const rapidjson::Value& jconf) {
  fFieldCage = new SLArBaseDetModule(); 
  auto fc_geoInfo = fFieldCage->GetGeoInfo(); 
  fc_geoInfo->RegisterGeoPar("corner_radius", unit::ParseJsonVal(jconf["corner_radius"]));
  fc_geoInfo->RegisterGeoPar("thickness", unit::ParseJsonVal(jconf["thickness"]));
  fc_geoInfo->RegisterGeoPar("wall_distance", unit::ParseJsonVal(jconf["wall_distance"])); 
  fc_geoInfo->RegisterGeoPar("spacing", unit::ParseJsonVal(jconf["spacing"])); 
  fc_geoInfo->RegisterGeoPar("height_long_side", unit::ParseJsonVal(jconf["height_long_side"])); 
  fc_geoInfo->RegisterGeoPar("height_short_side", unit::ParseJsonVal(jconf["height_short_side"]));
  
  return;
}
