#include "SLArDetectorConstruction.hh"
//#include "detector/TPC/SLArPENFilm.hh"

#include "SLArAnalysisManager.hh"
#include "SLArMaterial.hh"

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
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4Material.hh"


SLArPENFilm::SLArPENFilm() :SLArBaseDetModule(),
  fMatPEN(nullptr), fSkinPENSurface(nullptr)
{
  fGeoInfo = new SLArGeoInfo();
}

SLArPENFilm::~SLArPENFilm() {
  std::cerr << "Deleting SLArPENFilm..." << std::endl;
  std::cerr << "SLArPENFilm DONE" << std::endl;

}

//G4VPhysicalVolume *SLArPENFilm::Construct()

void SLArPENFilm::BuildMaterial(G4String db_file)
{
  fMatPEN = new SLArMaterial();
  
  fMatPEN->SetMaterialID("PEN");
  fMatPEN->BuildMaterialFromDB(db_file);
}

void SLArPENFilm::BuildDefalutGeoParMap()
{
  G4cerr << "SLArPENFilm::BuildGeoParMap()" << G4endl;
  fGeoInfo->RegisterGeoPar("pos_x"       ,   0.5*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("pos_y"       ,   0.0*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("pos_z"       ,   0.0*CLHEP::mm);
  fGeoInfo->RegisterGeoPar("dim_y"       ,   150.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("dim_z"       ,   200.0*CLHEP::cm);
  fGeoInfo->RegisterGeoPar("dim_x"       ,   5.0*CLHEP::mm);
  G4cerr << "Exit method\n" << G4endl;
}
  
void SLArPENFilm::BuildPEN()
{

  G4cerr << "SLArPENFilm::BuildPEN()" << G4endl;
  //Building Target
  G4cerr << "\tBuilding PEN Film" << G4endl;
  G4double PENX= fGeoInfo->GetGeoPar("dim_x");
  G4double PENY= fGeoInfo->GetGeoPar("dim_y");
  G4double PENZ= fGeoInfo->GetGeoPar("dim_z");
  
  //Create and fill fTarget
  G4double x_ = PENX*0.5;
  G4double y_ = PENY*0.5;
  G4double z_ = PENZ*0.5;
  
  fModSV = new G4Box("penfilm"+std::to_string(fID)+"_sv", x_, y_, z_);
  
  SetLogicVolume(new G4LogicalVolume(fModSV, fMatPEN->GetMaterial(), "penfilm"+std::to_string(fID)+"_lv"));
 }

G4ThreeVector SLArPENFilm::GetPENCenter() {

  if (fGeoInfo->Contains("pos_x") &&
       fGeoInfo->Contains("pos_y") &&
       fGeoInfo->Contains("pos_z") ) {
    return G4ThreeVector(fGeoInfo->GetGeoPar("pos_x"),
        fGeoInfo->GetGeoPar("pos_y"),
        fGeoInfo->GetGeoPar("pos_z"));
  }

  return G4ThreeVector(0., 0., 0.);
}

void SLArPENFilm::SetVisAttributes()
{
  G4cout << "SLArPENFilm::SetVisAtrributes()" << G4endl;
  
  G4VisAttributes* visAttributes = new G4VisAttributes();
  
  visAttributes->SetVisibility(true);
  visAttributes->SetColor(0.534, 0.521, 0.185, 0.1);
  fModLV->SetVisAttributes( visAttributes );
  
  return;
}

void SLArPENFilm::Init(const rapidjson::Value& jconf) {
  assert(jconf.IsObject());
  auto jpenfilm = jconf.GetObject();
  
  assert(jpenfilm.HasMember("dimensions"));
  assert(jpenfilm.HasMember("position"));
  assert(jpenfilm.HasMember("copyID"));
  
  fID = jpenfilm["copyID"].GetInt();
  fGeoInfo->ReadFromJSON(jpenfilm["position"].GetObj(), "pos");
  fGeoInfo->ReadFromJSON(jpenfilm["dimensions"].GetArray());
  
  return;
}
 
