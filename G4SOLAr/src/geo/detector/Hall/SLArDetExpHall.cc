/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetExpHall.cc
 * @created     : Tuesday Nov 12, 2024 16:22:08 CET
 */
#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"

#include "SLArUnit.hpp"
#include "detector/Hall/SLArDetExpHall.hh"
#include "TMath.h"

SLArDetExpHall::SLArDetExpHall() : SLArBaseDetModule()
{
    
}

SLArDetExpHall::~SLArDetExpHall()
{
}

void SLArDetExpHall::Init(const rapidjson::Value& config)
{

  if (config.HasMember("layout")) {
    fLayout = GetExpHallLayout(config["layout"].GetString());
  }
  else {
    fLayout = kBox;
  }

  if (config.HasMember("wall_layers")) {
    const rapidjson::Value& wall_layers = config["wall_layers"];
    if (wall_layers.IsArray() == false) {
      throw std::runtime_error("SLArDetExpHall::Init: wall_layers is not an array");
    }
    for (rapidjson::SizeType i = 0; i < wall_layers.Size(); i++) {
      const rapidjson::Value& layer = wall_layers[i];
      SLArWallLayer wall_layer;
      wall_layer.fName = layer["name"].GetString();
      printf("reading layer %s\n", wall_layer.fName.c_str());
      wall_layer.fThickness = unit::ParseJsonVal( layer["thickness"] );
      printf("\tthickness %g\n", wall_layer.fThickness);
      wall_layer.fMaterial = SLArMaterial(layer["material"].GetString());
      printf("\tmaterial %s\n", wall_layer.fMaterial.GetMaterialID().data());
      fWallLayers.insert(std::make_pair(i+1, wall_layer));
    }
  }

  assert(config.HasMember("dimensions"));
  fGeoInfo->ReadFromJSON(config["dimensions"].GetArray());

  if (config.HasMember("base_material")) {
    SLArMaterial mat(config["base_material"].GetString());
  }
}

void SLArDetExpHall::BuildMaterials(G4String db_file)
{
  for (auto& layer_itr : fWallLayers) {
    auto& layer = layer_itr.second;
    layer.fMaterial.BuildMaterialFromDB(db_file);
  }

  fBaseMaterial.BuildMaterialFromDB(db_file);

  return;
}

void SLArDetExpHall::BuildLayers(const G4VPhysicalVolume* world_pv)
{
  const G4Box* world_box = dynamic_cast<const G4Box*>(world_pv->GetLogicalVolume()->GetSolid());
  const G4double wall_thickness = GetWallThickness();

  G4Box* exp_hall_outer_vol = new G4Box("exp_hall_outer_vol", 
      world_box->GetXHalfLength(),
      world_box->GetYHalfLength(),
      world_box->GetZHalfLength());

  G4VSolid* exp_hall_inner_vol = BuildLayerSolid(0); 
  exp_hall_inner_vol->SetName("exp_hall_inner_vol");
  if (fLayout == kBarrelVault) {
    fHallBoxShift = GetSolidDisplacement(static_cast<G4SubtractionSolid*>(exp_hall_inner_vol));
  }

  fModSV = new G4SubtractionSolid("exp_hall_sv", exp_hall_outer_vol, exp_hall_inner_vol, 0, 
      G4ThreeVector(0, 0.5*fHallBoxShift, 0));

  fModLV = new G4LogicalVolume(fModSV, fBaseMaterial.GetMaterial() , "exp_hall_lv");

  double d = wall_thickness;
  printf("Building Experimental Hall with %lu layers and wall thickness %g\n", 
      fWallLayers.size(), wall_thickness);
  for (auto& layer_itr : fWallLayers) {
    const uint16_t& order = layer_itr.first;
    SLArWallLayer& layer = layer_itr.second;
    G4VSolid* solid_out = BuildLayerSolid(d, &layer);
    solid_out->SetName(layer.fName+"_sv_out");

    G4VSolid* solid_in = BuildLayerSolid(d - layer.fThickness, &layer);
    solid_in->SetName(layer.fName+"_sv_in");

    G4SubtractionSolid* solid = new G4SubtractionSolid(
        layer.fName+"_solid", solid_out, solid_in, nullptr, G4ThreeVector(0, 0, 0));
    G4LogicalVolume* logic = new G4LogicalVolume(
        solid, layer.fMaterial.GetMaterial(), layer.fName+"_lv");
    G4PVPlacement* phisical = new G4PVPlacement(0, G4ThreeVector(0, 0.5*fHallBoxShift, 0),
        logic, layer.fName, fModLV, false, 88800+order, true);

    d -= layer.fThickness;
  }

}

G4double SLArDetExpHall::GetSolidDisplacement(const G4BooleanSolid* solid) const
{
  const G4DisplacedSolid* disp = dynamic_cast<const G4DisplacedSolid*>(solid->GetConstituentSolid(1));
  return (disp->GetObjectTranslation().y());
}

G4double SLArDetExpHall::GetSolidDisplacement(G4BooleanSolid* solid) 
{
  const G4DisplacedSolid* disp = dynamic_cast<const G4DisplacedSolid*>(solid->GetConstituentSolid(1));
  return (disp->GetObjectTranslation().y());
}

G4VSolid* SLArDetExpHall::BuildLayerSolid(const G4double& d, SLArWallLayer* layer)
{
  if (fLayout == kBox) {
    return new G4Box("exp_hall_layer", 
        0.5*fGeoInfo->GetGeoPar("inner_size_x") + d,
        0.5*fGeoInfo->GetGeoPar("inner_size_y") + d,
        0.5*fGeoInfo->GetGeoPar("inner_size_z") + d);
  }
  else if (fLayout == kBarrelVault) {
    printf("Building barrel with d = %g\n", d);

    const G4double dh0 = fGeoInfo->GetGeoPar("height_vault") - fGeoInfo->GetGeoPar("inner_size_y");
    const G4double alpha = std::atan( 0.5*fGeoInfo->GetGeoPar("inner_size_x") / dh0);
    const G4double gamma = TMath::Pi() - 2*alpha;
    const G4double radius0 = 0.5*fGeoInfo->GetGeoPar("inner_size_x") / std::sin(gamma);
    const G4double radius = radius0 + d / std::sin(gamma);
    const G4double dd = d / std::tan(gamma);

    G4Box* box = new G4Box("box", 
        0.5*fGeoInfo->GetGeoPar("inner_size_x") + d, 
        0.5*fGeoInfo->GetGeoPar("inner_size_y") + 0.5*(d+dd),
        0.5*fGeoInfo->GetGeoPar("inner_size_z") + d);
 
    G4Tubs* barrel = new G4Tubs("barrel", 
        0.0, radius, 0.5*fGeoInfo->GetGeoPar("inner_size_z") + d,
        0.5*TMath::Pi() - gamma, 2*gamma);

    const G4double dy = box->GetYHalfLength() - radius*std::cos(gamma);
    printf("Box info: HalfYLen: %g\n", box->GetYHalfLength());
    printf("Barrel Vault info: γ = %.2f deg, r = %.2f, y = %.2f - 0.5*dd = %g\n", 
        gamma/CLHEP::deg, radius, dy, -0.5*dd);
    G4UnionSolid* solid = new G4UnionSolid("barrel_vault", barrel, box, 
        nullptr, G4ThreeVector(0, -dy, 0));

    return solid;
  }

  return nullptr;
}


SLArDetExpHall::SLArWallLayer::SLArWallLayer() :
  fName(""),
  fThickness(0),
  fMaterial("Air") {}

SLArDetExpHall::SLArWallLayer::SLArWallLayer(
    G4String   layer_name, 
    G4double   thickness,
    G4String   material_name) :
  fName(layer_name),
  fThickness(thickness),
  fMaterial(material_name) {}

