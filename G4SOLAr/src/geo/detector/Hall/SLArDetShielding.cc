/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetShielding.hh
 * @created     : Thursday Oct 30, 2025 14:00:34 CET
 */

#include "SLArDebugUtils.hh"
#include "G4Exception.hh"
#include "G4VisAttributes.hh"
#include "detector/SLArBaseDetModule.hh"
#include "detector/Hall/SLArDetShielding.hh"

SLArDetShielding::SLArDetShielding()
{}

SLArDetShielding::~SLArDetShielding()
{}

void SLArDetShielding::AddLayer(
    G4String layer_name, G4double thickness,
    G4String material_name, G4int importance)
{
  unsigned int layer_id = fShieldingLayers.size() + 1;
  fShieldingLayers.emplace(layer_id, 
      SLArShieldingLayer(layer_name, thickness, material_name, importance));
  if (fGeoInfo->Contains("shielding_thickness") == false) {
    fGeoInfo->SetGeoPar("shielding_thickness", thickness);
  }
  else {
    G4double shielding_tk = fGeoInfo->GetGeoPar("shielding_thickness") + thickness;
    fGeoInfo->SetGeoPar("shielding_thickness", shielding_tk);
  }
  return;
}

void SLArDetShielding::Init(const rapidjson::Value& config)
{
  debug::require_json_member(config, "name");
  debug::require_json_member(config, "side");
  debug::require_json_member(config, "dimensions");
  debug::require_json_member(config, "layers");

  debug::require_json_type(config["dimensions"], rapidjson::kArrayType);
  debug::require_json_type(config["layers"], rapidjson::kArrayType);

  fName = config["name"].GetString();
  fFace = geo::get_face_from_name( config["side"].GetString() );

  fGeoInfo->ReadFromJSON( config["dimensions"].GetArray() );

  unsigned int layer_id = 1;
  G4double shielding_tk = 0.0; 
  for (const auto &jlayer : config["layers"].GetArray()) {
    debug::require_json_type(jlayer, rapidjson::kObjectType);
    debug::require_json_member(jlayer, "name");
    debug::require_json_member(jlayer, "material");
    debug::require_json_member(jlayer, "thickness");
    G4String mat_name = jlayer["material"].GetString();
    G4double tk = unit::ParseJsonVal(jlayer["thickness"]);
    G4double importance = 
      (jlayer.HasMember("importance")) ? jlayer["importance"].GetInt() : 1;
    
    AddLayer(jlayer["name"].GetString(), tk, mat_name, importance);

    layer_id++;
  }
}

void SLArDetShielding::BuildMaterials(G4String db_file)
{
  printf("Building Shielding materials...\n");
  for (auto& layer_itr : fShieldingLayers) {
    auto& layer = layer_itr.second;
    SLArMaterial* mat = new SLArMaterial(); 
    mat->SetMaterialID(layer.fMaterialName);
    mat->BuildMaterialFromDB(db_file);
    layer.fMaterial = mat->GetMaterial();
  }
  fBaseMaterial.BuildMaterialFromDB(db_file);
  return;
}


void SLArDetShielding::BuildShielding() 
{
  fModSV = new G4Box( (fName + "_sv").c_str(),
      0.5*fGeoInfo->GetGeoPar("dim_x"),
      0.5*fGeoInfo->GetGeoPar("shielding_thickness"),
      0.5*fGeoInfo->GetGeoPar("dim_z") );

  fModLV = new G4LogicalVolume(
      fModSV,
      fBaseMaterial.GetMaterial(),
      (fName + "_lv").c_str() ); 

  unsigned int layer_id = 1; 
  G4double curr_y = -0.5*fGeoInfo->GetGeoPar("shielding_thickness");
  for (const auto &layer_itr : fShieldingLayers) {
    const auto &layer = layer_itr.second;
    G4double layer_tk = layer.fThickness;
    const G4String layer_name = layer.fName;

    curr_y += 0.5*layer_tk;

    G4Box* layer_sv = new G4Box( (fName + "_layer_" + std::to_string(layer_id) + "_sv").c_str(),
        0.5*fGeoInfo->GetGeoPar("dim_x"),
        0.5*layer_tk,
        0.5*fGeoInfo->GetGeoPar("dim_z") );
    G4LogicalVolume* layer_lv = new G4LogicalVolume(
        layer_sv,
        layer.fMaterial,
        (fName + "_" + layer_name + "_" + std::to_string(layer_id) + "_lv").c_str() );
    new G4PVPlacement(
        0, G4ThreeVector(0, curr_y, 0),
        layer_lv,
        (fName + "_" + layer_name + "_" + std::to_string(layer_id) + "_pv").c_str(),
        fModLV, 0, layer_id, true );
    curr_y += 0.5*layer_tk;

    layer_id++;
  }

  return;
}

void SLArDetShielding::SetVisAttributes() {
  std::map<G4String, G4Colour> col_map; 
  col_map.insert( std::make_pair("Steel", G4Colour(0.231, 0.231, 0.227))); 
  col_map.insert( std::make_pair("Plywood", G4Colour(0.671, 0.553, 0.196))); 
  col_map.insert( std::make_pair("Water", G4Colour(0.561, 0.863, 0.91))); 
  col_map.insert( std::make_pair("BoratedPolyethilene", G4Colour(0.267, 0.671, 0.22))); 
  col_map.insert( std::make_pair("Polyurethane", G4Colour(0.867, 0.871, 0.769))); 
  col_map.insert( std::make_pair("Air", G4Colour(0.53, 0.80, 0.98)));
  col_map.insert( std::make_pair("HDPE", G4Colour(0.82, 0.41, 0.12)));
  col_map.insert( std::make_pair("Lead", G4Colour(0.43, 0.50, 0.56)));
  G4Colour stdCol(0.611, 0.847, 0.988);

  for (auto &ll : fShieldingLayers) {
    printf("setting vis for shielding layer %s\n", ll.second.fName.c_str());
    auto lv = ll.second.fModule->GetModLV();
    printf("%s\n", lv->GetName().c_str());
    G4Colour col = stdCol; 
    if (col_map.count(lv->GetMaterial()->GetName()))
    {
      col = col_map[lv->GetMaterial()->GetName()]; 
    }
    lv->SetVisAttributes( G4VisAttributes( col ) ); 
  }

  fModLV->SetVisAttributes( G4VisAttributes(false) );
}

