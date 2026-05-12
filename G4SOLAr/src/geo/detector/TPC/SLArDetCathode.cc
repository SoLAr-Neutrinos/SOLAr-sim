/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetCathode.cc
 * @created     Mon Mar 20, 2023 16:28:35 CET
 */


#include "detector/TPC/SLArDetCathode.hh"
#include "SLArDebugUtils.hh"
#include "geo/SLArUnit.hpp"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisAttributes.hh"


SLArDetCathode::SLArDetCathode() : SLArBaseDetModule(),
  fMatCathode(nullptr)
{
  fGeoInfo = new SLArGeoInfo();
}


SLArDetCathode::~SLArDetCathode() {
  std::cerr << "Deleting SLArDetCathode..." << std::endl;
  std::cerr << "SLArDetCathode DONE" << std::endl;
}

void SLArDetCathode::BuildMaterial(G4String db_file) 
{
  // TODO: IMPLEMENT PROPER MATERIALS IN /materials
  fMatCathode = new SLArMaterial();

  fMatCathode->SetMaterialID("FR4");
  fMatCathode->BuildMaterialFromDB(db_file);
}

void SLArDetCathode::BuildDefalutGeoParMap() 
{
  G4cerr << "SLArDetCathode::BuildGeoParMap()" << G4endl;
  fGeoInfo->RegisterGeoPar("pos_x"       ,   0.0*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("pos_y"       ,   0.0*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("pos_z"       ,   0.0*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("dim_y"       , 150.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("dim_z"       , 200.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("dim_x"       ,  60.0*CLHEP::cm); 
  G4cerr << "Exit method\n" << G4endl;
}

void SLArDetCathode::BuildCathode() 
{

  G4cerr << "SLArDetCathode::BuildCathode()" << G4endl;
  //* * * * * * * * * * * * * * * * * * * * * * * * * * *//
  // Building the Target                                 //
  G4cout << "\tBuilding TPC Cathode" << G4endl;
  if (fShape == geo::kTub) {
    G4double cathR= fGeoInfo->GetGeoPar("dim_r");
    G4double cathY= fGeoInfo->GetGeoPar("dim_y");

    // Create and fill fTarget
    G4double r_ = cathR;
    G4double y_ = cathY*0.5;

    fModSV = new G4Tubs("cathode"+std::to_string(fID)+"_sv", 0., r_, y_, 0., CLHEP::twopi);
  } 
  else {
    G4double cathX= fGeoInfo->GetGeoPar("dim_x");
    G4double cathY= fGeoInfo->GetGeoPar("dim_y");
    G4double cathZ= fGeoInfo->GetGeoPar("dim_z");

    // Create and fill fTarget
    G4double x_ = cathX*0.5;
    G4double y_ = cathY*0.5;
    G4double z_ = cathZ*0.5;

    fModSV = new G4Box("cathode"+std::to_string(fID)+"_sv", x_, y_, z_);
  }
  SetLogicVolume(
      new G4LogicalVolume(fModSV, 
        fMatCathode->GetMaterial(),
        "cathode"+std::to_string(fID)+"_lv") 
      );
}

G4ThreeVector SLArDetCathode::GetCathodeCenter() const {
  if ( fGeoInfo->Contains("pos_x") && 
       fGeoInfo->Contains("pos_y") &&
       fGeoInfo->Contains("pos_z") ) {
    return G4ThreeVector(fGeoInfo->GetGeoPar("pos_x"), 
        fGeoInfo->GetGeoPar("pos_y"), 
        fGeoInfo->GetGeoPar("pos_z")); 
  }

  return G4ThreeVector(0., 0., 0.); 
}

void SLArDetCathode::SetVisAttributes()
{
  G4cout << "SLArDetCathode::SetVisAttributes()" << G4endl;

  G4VisAttributes* visAttributes = new G4VisAttributes();

  visAttributes->SetVisibility(true); 
  visAttributes->SetColor(0.607, 0.847, 0.992, 0.4);
  fModLV->SetVisAttributes( visAttributes );

  return;
}

void SLArDetCathode::Init(const rapidjson::Value& jconf) {
  assert(jconf.IsObject()); 
  auto jcathode = jconf.GetObject(); 

  debug::require_json_member(jcathode, "dimensions"); 
  debug::require_json_member(jcathode, "position");
  debug::require_json_member(jcathode, "copyID");

  fID = jcathode["copyID"].GetInt(); 
  fGeoInfo->ReadFromJSON(jcathode["position"].GetObj(), "pos"); 
  fGeoInfo->ReadFromJSON(jcathode["dimensions"].GetArray()); 
  
  if (jcathode.HasMember("shape")) {
    std::string shape_str = jcathode["shape"].GetString();
    if (shape_str == "box") {
      fShape = geo::kBox;
    } else if (shape_str == "tub") {
      fShape = geo::kTub;
    } else {
      G4cerr << "Unknown shape: " << shape_str << G4endl;
      G4cerr << "Defaulting to box." << G4endl;
      fShape = geo::kBox;
    }
  }

  if (jcathode.HasMember("rot")) {
    auto jrot = jcathode["rot"].GetObject(); 
    G4double vunit = unit::Unit2Val(jrot["unit"]); 
    assert(jrot.HasMember("val")); 
    assert(jrot["val"].IsArray()); 
    assert(jrot["val"].GetArray().Size() == 3);
    double eulerAngles[3] = {0.}; 
    G4int idim = 0; 
    for (const auto &v : jrot["val"].GetArray()) {
      eulerAngles[idim] = v.GetDouble()*vunit; 
      idim++; 
    }
    fRotation = new G4RotationMatrix(eulerAngles[0], eulerAngles[1], eulerAngles[2]); 
    fGeoInfo->RegisterGeoPar("cathode_phi", eulerAngles[0]); 
    fGeoInfo->RegisterGeoPar("cathode_theta", eulerAngles[1]); 
    fGeoInfo->RegisterGeoPar("cathode_psi", eulerAngles[2]); 
  }

  return;
}
