/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetCryostat.cc
 * @created     Wed Mar 15, 2023 11:57:41 CET
 */
#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4VisAttributes.hh"
#include "G4PVParameterised.hh"
#include "G4Transform3D.hh"

#include "detector/TPC/SLArDetCryostat.hh"
#include "detector/SLArPlaneParameterisation.hpp"

SLArCryostatLayer::SLArCryostatLayer() : 
  fName("CryostatLayer"), 
  fHalfSizeX(1.), fHalfSizeY(1.), fHalfSizeZ(1.), fThickness(1.), fImportance(1),
  fMaterialName("MATERIAL_NAME"), fMaterial(nullptr), fModule(nullptr)
{}

SLArCryostatLayer::SLArCryostatLayer(
    G4String   model_name, 
    G4double*  halfSize,  
    G4double   thickness,
    G4String   material_name, 
    G4int      importance)
  : fMaterial(nullptr), fModule(nullptr)
{

  fName      = model_name;
  fHalfSizeX =  halfSize[0];
  fHalfSizeY =  halfSize[1]; 
  fHalfSizeZ =  halfSize[2]; 
  fThickness =  thickness;
  fImportance=  importance;

  fMaterialName = material_name;
}

SLArDetCryostat::SLArDetCryostat() : 
  fMatWorld(nullptr), fMatWaffle(nullptr), fMatBrick(nullptr), 
  fWaffleUnit(nullptr), fWaffleEdgeUnit(nullptr), 
  fBuildSupport(false), fAddNeutronBricks(false)
{
    
}

SLArDetCryostat::~SLArDetCryostat()
{
}


void SLArDetCryostat::BuildCryostatStructure(const rapidjson::Value& jcryo) {
  assert(jcryo.HasMember("Cryostat_structure")); 
  assert(jcryo["Cryostat_structure"].IsArray()); 

  const auto jlayers = jcryo["Cryostat_structure"].GetArray(); 
  printf("SLArDetCryostat::BuildCryostatStructure\n");

  G4double tgtZ = fGeoInfo->GetGeoPar("target_z");
  G4double tgtY = fGeoInfo->GetGeoPar("target_y");
  G4double tgtX = fGeoInfo->GetGeoPar("target_x");

  G4double cryostat_tk = 0.; 

  // compute total thickness
  for (const auto &layer : jlayers) {
    if (layer.HasMember("thickness")) 
      cryostat_tk += unit::ParseJsonVal(layer["thickness"]); 
  }

  G4double cryostat_waffle_tk = cryostat_tk; 

  if (jcryo.HasMember("Cryostat_support")) {
    fBuildSupport = true; 
    G4double support_wd = 0.; 
    const auto jsupport = jcryo["Cryostat_support"].GetObj(); 
    support_wd += unit::ParseJsonVal(jsupport["steel_major_width"]); 

    fGeoInfo->RegisterGeoPar("waffle_major_width", 
        unit::ParseJsonVal(jsupport["steel_major_width"])); 
    fGeoInfo->RegisterGeoPar("waffle_minor_width", 
        unit::ParseJsonVal(jsupport["steel_minor_width"])); 
    fGeoInfo->RegisterGeoPar("steel_thickness", 
        unit::ParseJsonVal(jsupport["steel_thickness"])); 
    fGeoInfo->RegisterGeoPar("waffle_spacing", 
        unit::ParseJsonVal(jsupport["main_spacing"])); 

    if (jsupport.HasMember("neutron_brick")) {
      const auto jbrick = jsupport["neutron_brick"].GetObj(); 
      fGeoInfo->RegisterGeoPar("brick_thickness", 
          unit::ParseJsonVal(jbrick["thickness"])); 
      fMatBrick = new SLArMaterial(); 
      fMatBrick->SetMaterialID(jbrick["material"].GetString()); 
      fAddNeutronBricks = true; 
    }
  }

  fGeoInfo->RegisterGeoPar("cryostat_tk", cryostat_tk); 

  G4double halfSize[3] = {tgtX*0.5 + cryostat_tk, 
                          tgtY*0.5 + cryostat_tk, 
                          tgtZ*0.5 + cryostat_tk}; 
  for (const auto &layer : jlayers) {
    if (layer.HasMember("thickness")) {
      G4double tk_ = unit::ParseJsonVal(layer["thickness"]);
      if (tk_ == 0.) continue;

      assert(layer.HasMember("id")); 
      assert(layer.HasMember("name")); 
      assert(layer.HasMember("material")); 
      for (size_t k=0; k<3; k++) halfSize[k] -= tk_;

      G4int importance = 1; 
      if (layer.HasMember("importance")) {
        importance = layer["importance"].GetInt(); 
      }
      
      SLArCryostatLayer* ll = new SLArCryostatLayer(
          layer["name"].GetString(), halfSize, 
          unit::ParseJsonVal(layer["thickness"]), 
          layer["material"].GetString(),importance);

      fCryostatStructure.insert(std::make_pair(layer["id"].GetInt(), ll)); 
    }
  }

  printf("SLArDetCryostat::BuildCryostatStructure: Cryostat layered structure built\n");
  for (const auto& l : fCryostatStructure) {
    printf("%i: %s %g mm %s\n", 
        l.first, l.second->fName.c_str(), l.second->fThickness, 
        l.second->fMaterialName.c_str());
  }
  return; 
}

void SLArDetCryostat::BuildSupportStructureUnit() {
  const G4double spacing     = fGeoInfo->GetGeoPar("waffle_spacing"); 
  const G4double major_width = fGeoInfo->GetGeoPar("waffle_major_width"); 
  const G4double minor_width = fGeoInfo->GetGeoPar("waffle_minor_width"); 
  const G4double majorT_width = 0.20*major_width; 
  const G4double minorT_width = 0.20*minor_width; 
  const G4double tk = fGeoInfo->GetGeoPar("steel_thickness"); 
  const G4double trnv_width  = major_width-2*tk; 
  G4double n_brick_tk  = 0.; 
  if (fGeoInfo->Contains("brick_thickness") && fAddNeutronBricks) {
    n_brick_tk = fGeoInfo->GetGeoPar("brick_thickness"); 
  }
  const G4double unit_thickness = std::max(major_width, minor_width + n_brick_tk); 
  fGeoInfo->RegisterGeoPar("waffle_total_width", unit_thickness); 

  fWaffleUnit = new SLArBaseDetModule(); 
  fWaffleUnit->SetSolidVolume( new G4Box("waffle_unit_sv", 
      0.5*(spacing), 0.5*unit_thickness, 0.5*(spacing) ) ); 
  fWaffleUnit->SetLogicVolume( new G4LogicalVolume( 
        fWaffleUnit->GetModSV(), fMatWorld->GetMaterial(), "waffle_unit_lv"));
  fWaffleUnit->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 

  // build waffle unit steel structure
  // Main frame (along the z axis)
  auto waffle_main_bar = new G4Box("waffle_main_sv", 0.25*tk, 0.5*major_width, 0.5*spacing); 
  auto waffle_main_T  = new G4Box("waffle_main_T", 0.5*majorT_width, 0.5*tk, 0.5*spacing); 
  auto waffle_main_sv = new G4UnionSolid(
      "waffle_main_sv_tmp", waffle_main_bar, waffle_main_T, 
      0, G4ThreeVector(+0.5*(majorT_width + 0.5*tk), 0.5*(major_width - tk), 0)); 
  waffle_main_sv = new G4UnionSolid(
      "waffle_main_sv", waffle_main_sv, waffle_main_T, 
      nullptr, 
      G4ThreeVector(+0.5*(majorT_width + 0.5*tk), -0.5*(major_width - tk), 0)); 

  auto waffle_main_lv = new G4LogicalVolume(
      waffle_main_sv, fMatWaffle->GetMaterial(), "waffle_main_lv"); 
  waffle_main_lv->SetVisAttributes( new G4VisAttributes( G4Colour(1, 0, 0) ) ); 
  new G4PVPlacement(0, 
      G4ThreeVector(-0.5*(spacing - 0.5*tk), -0.5*(unit_thickness - major_width), 0), 
      waffle_main_lv, "waffle_main_pv0", fWaffleUnit->GetModLV(), 0, 1); 
  auto rot = new G4RotationMatrix(0, 0, 0); 
  rot->rotateY(CLHEP::pi); 
  new G4PVPlacement(rot, 
      G4ThreeVector(+0.5*(spacing - 0.5*tk), -0.5*(unit_thickness - major_width), 0), 
      waffle_main_lv, "waffle_main_pv1", fWaffleUnit->GetModLV(), 0, 1); 
  
  // Transverse frame (along the x-axis)
  auto waffle_trnv_bar = new G4Box(
      "waffle_trnv_sv", 0.5*(spacing-tk), 0.5*trnv_width, 0.25*tk); 
  auto waffle_trnv_T  = new G4Box(
      "waffle_trnv_T", 0.5*(spacing - tk -2*majorT_width), 0.5*tk, 0.5*majorT_width); 
  auto waffle_trnv_sv = new G4UnionSolid(
      "waffle_trnv_sv_tmp", waffle_trnv_bar, waffle_trnv_T, 
      0, G4ThreeVector(0, 0.5*(trnv_width+tk), 0.5*(majorT_width - 0.5*tk) )); 
  waffle_trnv_sv = new G4UnionSolid(
      "waffle_trnv_sv", waffle_trnv_sv, waffle_trnv_T, 
      nullptr, 
      G4ThreeVector(0.0, -0.5*(trnv_width+tk), 0.5*(majorT_width - 0.5*tk) )); 
  auto waffle_trnv_lv = new G4LogicalVolume(
      waffle_trnv_sv, fMatWaffle->GetMaterial(), "waffle_trnv_lv"); 
  waffle_trnv_lv->SetVisAttributes( new G4VisAttributes( G4Colour(1, 0, 0) ) ); 
  new G4PVPlacement(0, 
      G4ThreeVector(0, -0.5*(unit_thickness-major_width), -0.5*(spacing - 0.5*tk)), 
      waffle_trnv_lv, "waffle_trnv_pv0", fWaffleUnit->GetModLV(), 0, 1); 
  new G4PVPlacement(rot, 
      G4ThreeVector(0, -0.5*(unit_thickness-major_width), +0.5*(spacing - 0.5*tk)), 
      waffle_trnv_lv, "waffle_trnv_pv1", fWaffleUnit->GetModLV(), 0, 1); 
  
  // Minor bar (along z-axis) 
  auto waffle_minor_bar = new G4Box(
      "waffle_minor_bar", 0.5*tk, 0.5*(minor_width)-tk, 0.5*(spacing-tk)); 
  auto waffle_minor_T   = new G4Box(
      "waffle_minor_T", minorT_width, 0.5*tk, 0.5*(spacing-tk)); 
  auto waffle_minor_Tdown   = new G4Box(
      "waffle_minor_Tdown", minorT_width, 0.5*tk, 0.5*(spacing-tk-2*majorT_width)); 
  auto waffle_minor_sv = new G4UnionSolid(
      "waffle_minor_tmp", waffle_minor_bar, waffle_minor_T, nullptr, 
      G4ThreeVector(0, 0.5*(minor_width-tk), 0)); 
  waffle_minor_sv = new G4UnionSolid(
      "waffle_minor_tmp2", waffle_minor_sv, waffle_minor_Tdown, nullptr, 
      G4ThreeVector(0, -0.5*(minor_width-tk), 0)); 
  G4RotationMatrix* rotY2 = new G4RotationMatrix(0, 0, 0); 
  rotY2->rotateY(0.5*CLHEP::pi); 
  waffle_minor_sv = new G4UnionSolid(
      "waffle_minor_sv", waffle_minor_sv, waffle_minor_sv,
      rotY2, 
      G4ThreeVector(0, 0, 0)); 

  auto waffle_minor_lv = new G4LogicalVolume(waffle_minor_sv, fMatWaffle->GetMaterial(),
      "waffle_minor_lv"); 
  waffle_minor_lv->SetVisAttributes( new G4VisAttributes( G4Colour(1, 0, 0) ) ); 
  new G4PVPlacement(0, G4ThreeVector(0, -0.5*(unit_thickness - minor_width), 0), 
      waffle_minor_lv, "waffle_minor_pv", fWaffleUnit->GetModLV(), 0, 1); 

  if (fAddNeutronBricks) {
    G4VSolid* brick_sv = nullptr; 
    G4double yshift = 0.; 
    if (n_brick_tk <= major_width - minor_width) {
      brick_sv = new G4Box("brick_sv", 
          0.5*(spacing-2*majorT_width-tk), 0.5*n_brick_tk, 0.5*(spacing-2*majorT_width-tk)); 
      //yshift = 0.5*(unit_thickness - n_brick_tk);
      yshift = -0.5*(unit_thickness - 2*minor_width - n_brick_tk);
    } else {
      G4double main_tk = n_brick_tk - (major_width - minor_width); 
      G4double fit_tk = major_width-minor_width;
      G4Box* brick_main = new G4Box("brick_main_sv", 
          0.5*spacing, 0.5*main_tk, 0.5*spacing); 
      G4Box* brick_fit  = new G4Box("brick_fit_sv", 
          0.5*(spacing-2*majorT_width-tk), 0.5*fit_tk, 0.5*(spacing-2*majorT_width-tk)); 
      brick_sv = new G4UnionSolid("brick_sv", brick_fit, brick_main, 
          G4TranslateY3D(0.5*(fit_tk + main_tk)) ); 
      yshift = -0.5*(unit_thickness - 2*minor_width - fit_tk);
    }
    auto brick_lv = new G4LogicalVolume(brick_sv, fMatBrick->GetMaterial(), "brick_lv"); 
    brick_lv->SetVisAttributes( G4VisAttributes( G4Colour(0, 0, 1) )); 
    new G4PVPlacement(G4TranslateY3D( yshift ), 
        brick_lv, "brick_pv", fWaffleUnit->GetModLV(), 0, 2); 
  }
}

SLArBaseDetModule* SLArDetCryostat::BuildSupportStructure(geo::EBoxFace kFace) {
  SLArBaseDetModule* waffle = new SLArBaseDetModule(); 

  // get cryostat dimensions
  G4ThreeVector dim;
  G4double cryo_tk = fGeoInfo->GetGeoPar("cryostat_tk"); 
  dim.set(
      fGeoInfo->GetGeoPar("target_x") + 2*cryo_tk,
      fGeoInfo->GetGeoPar("target_y") + 2*cryo_tk, 
      fGeoInfo->GetGeoPar("target_z") + 2*cryo_tk); 
  const auto spacing = fGeoInfo->GetGeoPar("waffle_spacing"); 

  // get normal versor for the given cryostat face
  const G4ThreeVector normal = geo::BoxFaceNormal[kFace]; 
  
  // get unit waffle dimensions 
  G4ThreeVector wffl_unit_min, wffl_unit_max;
  fWaffleUnit->GetModSV()->BoundingLimits(wffl_unit_min, wffl_unit_max); 
  const G4ThreeVector wffl_dim = wffl_unit_max - wffl_unit_min; 
  G4ThreeVector wfflNormal(0, -1, 0); 

  const G4double _angle = wfflNormal.angle(normal);
  G4ThreeVector rot_axis = wfflNormal.cross(normal);
  if (rot_axis.mag2() < 1e-6) rot_axis = wfflNormal;
  G4RotationMatrix* rot = new G4RotationMatrix(rot_axis, _angle); 

  G4ThreeVector plane_dim = dim; 
  plane_dim.transform(*rot);
  printf("Cryostat size: %.0f %.0f %.0f\n", dim.x(), dim.y(), dim.z()); 
  printf("face normal: [%.0f, %.0f, %.0f]\n", normal.x(), normal.y(), normal.z()); 
  printf("rot axis: (%.0f, %.0f, %.0f), angle = %g\n", rot_axis.x(), rot_axis.y(), rot_axis.z(), _angle*CLHEP::rad);
  printf("Face %i: %.0f mm,  %.0f mm, %.0f mm\n", kFace, plane_dim.x(), plane_dim.y(), plane_dim.z()); 

  const int nz = floor( fabs(plane_dim.z() / wffl_dim.z()) ); 
  const int nx = floor( fabs(plane_dim.x() / wffl_dim.x()) ); 
  const int ny = 0; 
  const int nReplica[3] = {nx, ny, nz};

  G4ThreeVector max_dim(nx*wffl_dim.x(), wffl_dim.y(), nz*wffl_dim.z()); 

  G4String face_name = "waffle_plane"+std::to_string(kFace)+"_sv"; 
  waffle->SetSolidVolume( new G4Box(
        face_name, 0.5*wffl_dim.x()*nx, 0.5*wffl_dim.y(), 0.5*wffl_dim.z()*nz)); 
  face_name = "waffle_face"+std::to_string(kFace)+"_lv"; 
  waffle->SetLogicVolume( new G4LogicalVolume(waffle->GetModSV(), fMatWorld->GetMaterial(), face_name)); 
  waffle->GetModLV()->SetVisAttributes( G4VisAttributes( false /*G4Colour( 1, 0.9, 0.2) */ )); 


  std::vector<SLArPlaneParameterisation*> prmtr = 
  {
    new SLArPlaneParameterisation(
        kXAxis, G4ThreeVector(-0.5*(nx-1)*wffl_dim.x(), 0, 0), wffl_dim.x()), 
    new SLArPlaneParameterisation(
        kZAxis, G4ThreeVector(0, 0, -0.5*(nz-1)*wffl_dim.z()), wffl_dim.z()), 
  }; 

  auto build_parameterised_vol = [&](
      SLArBaseDetModule* origin, 
      SLArBaseDetModule* target, 
      G4String target_prefix,
      SLArPlaneParameterisation* rpars,
      int copyNo) {

    G4ThreeVector tmp_dim = max_dim; 
    G4ThreeVector origin_dim; 
    
    G4Box* originBox = (G4Box*)origin->GetModSV();

    origin_dim[0] = 2*originBox->GetXHalfLength(); 
    origin_dim[1] = 2*originBox->GetYHalfLength(); 
    origin_dim[2] = 2*originBox->GetZHalfLength(); 

    G4ThreeVector perp_ax = wfflNormal.cross(rpars->GetReplicationAxisVector()); 
    G4double module_wdt = 0.; 
    for (int i=0; i<3; i++) {
      if ( fabs(perp_ax[i] * origin_dim[i] ) > 0 ) {
        tmp_dim[i] = origin_dim[i]; 
        module_wdt = origin_dim[i]; 
      }
    }

    target->SetSolidVolume(new G4Box(target_prefix+"_sv", 
          0.5*tmp_dim[0], 0.5*tmp_dim[1], 0.5*tmp_dim[2])); 
    target->SetLogicVolume(new G4LogicalVolume(target->GetModSV(), 
          fMatWorld->GetMaterial(), target_prefix+"_lv")); 
    target->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 
    target->SetModPV(
        new G4PVParameterised(target_prefix+"_ppv", 
          origin->GetModLV(), target->GetModLV(), 
          rpars->GetReplicationAxis(), nReplica[rpars->GetReplicationAxis()],
          rpars, true));
    target->GetModPV()->SetCopyNo(copyNo); 
  }; 

  std::vector<SLArBaseDetModule*> waffle_plane_submodule; 

  for (auto &rpars : prmtr) {
    SLArBaseDetModule* target = nullptr; 
    SLArBaseDetModule* origin = nullptr;
    G4String prefix = "";
    G4int copyNo = 666;
    if (rpars == prmtr.back()) {
      target = waffle; 
      origin = waffle_plane_submodule.back();
      prefix = "waffle_row";
      copyNo = 4; 
    } 
    else if (rpars == prmtr.front()) {
      waffle_plane_submodule.push_back( new SLArBaseDetModule() ); 
      target = waffle_plane_submodule.back();
      origin = fWaffleUnit;
      prefix = "waffle_unit";
      copyNo = 3; 
    }
    else {
      G4cout << "SLArDetSuperCellArray::BuildSuperCellArray() WARNING: " << G4endl;
      G4cout << "I should not be here!" << G4endl;
      exit( EXIT_FAILURE ); 
      waffle_plane_submodule.push_back( new SLArBaseDetModule() ); 
      target = waffle_plane_submodule.back(); 
      origin = waffle_plane_submodule.rbegin()[1];
    }

    build_parameterised_vol(origin, target, prefix, rpars, copyNo);
  }

  for (auto &subModules : waffle_plane_submodule) {
    subModules->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 
  }

  SLArBaseDetModule* total = new SLArBaseDetModule(); 
  total->SetSolidVolume( 
      new G4Box("waffle_total_sv", 
        fabs(0.5*plane_dim.x()), 0.5*wffl_dim.y(), 0.5*fabs(plane_dim.z()))); 

  total->SetLogicVolume( new G4LogicalVolume(total->GetModSV(), 
        fMatWorld->GetMaterial(), "waffle_total_lv")); 

  new G4PVPlacement(0, G4ThreeVector(0, 0, 0), waffle->GetModLV(),
      "waffle_plane", total->GetModLV(), 0, 0); 

  G4double patch_wx = fabs(plane_dim.x())-wffl_dim.x()*nx;
  G4double patch_wz = fabs(plane_dim.z())-wffl_dim.z()*nz;
  auto patchX = BuildSupportStructurePatch(0.5*patch_wx, nz*wffl_dim.z(), "x");
  auto patchZ = BuildSupportStructurePatch(0.5*patch_wz, nx*wffl_dim.x(), "z"); 
  G4String patch_name = "face"+std::to_string(kFace)+"_patch"; 
  new G4PVPlacement(0, G4ThreeVector(0.5*(fabs(plane_dim.x())-0.5*patch_wx), 0, 0), 
      patchX->GetModLV(), patch_name+"_x_0", total->GetModLV(), 0, 4); 
  new G4PVPlacement(0, G4ThreeVector(-0.5*(fabs(plane_dim.x())-0.5*patch_wx), 0, 0), 
      patchX->GetModLV(), patch_name+"_x_1", total->GetModLV(), 0, 4); 
  rot = new G4RotationMatrix(); 
  rot->rotateY(0.5*CLHEP::pi); 
  new G4PVPlacement(rot, G4ThreeVector(0, 0, 0.5*(fabs(plane_dim.z())-0.5*patch_wz)), 
      patchZ->GetModLV(), patch_name+"_z_0", total->GetModLV(), 0, 4); 
  new G4PVPlacement(rot, G4ThreeVector(0, 0,-0.5*(fabs(plane_dim.z())-0.5*patch_wz)), 
      patchZ->GetModLV(), patch_name+"_z_1", total->GetModLV(), 0, 4); 

  return total;
}

SLArBaseDetModule* SLArDetCryostat::BuildSupportStructurePatch(G4double width, G4double len, G4String name) {
  const G4double spacing     = fGeoInfo->GetGeoPar("waffle_spacing"); 
  const G4double major_width = fGeoInfo->GetGeoPar("waffle_major_width"); 
  const G4double minor_width = fGeoInfo->GetGeoPar("waffle_minor_width"); 
  const G4double majorT_width = 0.20*major_width; 
  const G4double minorT_width = 0.20*minor_width; 
  const G4double tk = fGeoInfo->GetGeoPar("steel_thickness"); 
  const G4double trnv_width  = major_width-2*tk; 

  SLArBaseDetModule* patch_unit = new SLArBaseDetModule(); 
  const G4double h = fGeoInfo->GetGeoPar("waffle_total_width"); 
  patch_unit->SetSolidVolume( 
      new G4Box("patchunit_"+name+"_sv", 0.5*width, 0.5*h, 0.5*spacing) ); 
  patch_unit->SetLogicVolume( new G4LogicalVolume(patch_unit->GetModSV(), 
        fMatWorld->GetMaterial(), "patchunit_"+name+"_lv")); 
  patch_unit->GetModLV()->SetVisAttributes( G4VisAttributes(false) ) ; 

  auto main_bar_Z = new G4Box("main_bar_Z", 0.25*tk, 0.5*major_width, 0.5*spacing); 
  //auto main_bar_T  = new G4Box("main_bar_T", 0.5*majorT_width, 0.5*tk, 0.5*spacing); 
  //auto main_bar_sv = new G4UnionSolid("main_bar_sv", main_bar_Z, main_bar_T, 0, 
      //G4ThreeVector(0.5*majorT_width, 0.5*(h-tk), 0)); 
  auto main_bar_lv = new G4LogicalVolume(main_bar_Z, fMatWaffle->GetMaterial(), 
      "main_bar_lv"); 
  main_bar_lv->SetVisAttributes( G4VisAttributes( G4Colour(1, 0, 0) ) ); 
  new G4PVPlacement(0, G4ThreeVector(0.5*(width-0.5*tk), 0, 0), main_bar_lv, "main_bar_pv", patch_unit->GetModLV(), false, 1); ; 
  auto rot = new G4RotationMatrix(0, 0, 0); 
  rot->rotateY(CLHEP::pi); 
  new G4PVPlacement(rot, G4ThreeVector(-0.5*(width-0.5*tk), 0, 0), main_bar_lv, "main_bar_pv", patch_unit->GetModLV(), false, 1); ; 
  // transverse bar
  auto patch_trnv_bar = new G4Box(
      "patch_trnv_sv", 0.5*(width-tk), 0.5*trnv_width, 0.25*tk); 
  auto patch_trnv_T  = new G4Box(
      "patch_trnv_T", 0.5*(width - tk), 0.5*tk, 0.5*majorT_width); 
  auto patch_trnv_sv = new G4UnionSolid(
      "patch_trnv_sv_tmp", patch_trnv_bar, patch_trnv_T, 
      0, G4ThreeVector(0, 0.5*(trnv_width+tk), 0.5*(majorT_width - 0.5*tk) )); 
  patch_trnv_sv = new G4UnionSolid(
      "patch_trnv_sv", patch_trnv_sv, patch_trnv_T, 
      nullptr, 
      G4ThreeVector(0.0, -0.5*(trnv_width+tk), 0.5*(majorT_width - 0.5*tk) )); 
  auto patch_trnv_lv = new G4LogicalVolume(
      patch_trnv_sv, fMatWaffle->GetMaterial(), "waffle_trnv_lv"); 
  patch_trnv_lv->SetVisAttributes( new G4VisAttributes( G4Colour(1, 0, 0) ) ); 
  new G4PVPlacement(0, 
      G4ThreeVector(0, -0.5*(h-major_width), -0.5*(spacing - 0.5*tk)), 
      patch_trnv_lv, "patch_trnv_pv0", patch_unit->GetModLV(), 0, 1); 
  new G4PVPlacement(rot, 
      G4ThreeVector(0, -0.5*(h-major_width), +0.5*(spacing - 0.5*tk)), 
      patch_trnv_lv, "patch_trnv_pv1", patch_unit->GetModLV(), 0, 1); 
  

  SLArBaseDetModule* patch = new SLArBaseDetModule(); 

  patch->SetSolidVolume( new G4Box("patch"+name+"_sv", 0.5*width, 0.5*h, 0.5*len) ); 
  patch->SetLogicVolume( 
      new G4LogicalVolume(patch->GetModSV(), fMatWorld->GetMaterial(), 
        "patch"+name+"_lv")); 
  patch->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 
  SLArPlaneParameterisation* prmtr = new SLArPlaneParameterisation(kZAxis, 
      G4ThreeVector(0, 0, -0.5*(len-spacing)), spacing); 
  patch->SetModPV( new G4PVParameterised("patch"+name+"_ppv", patch_unit->GetModLV(), 
        patch->GetModLV(), kZAxis, std::floor(len/spacing), prmtr, true) ); 


  return patch;
}

void SLArDetCryostat::BuildSupportStructureEdgeUnit() {
  const G4double spacing     = fGeoInfo->GetGeoPar("waffle_spacing"); 
  const G4double major_width = fGeoInfo->GetGeoPar("waffle_major_width"); 
  const G4double minor_width = fGeoInfo->GetGeoPar("waffle_minor_width"); 
  const G4double majorT_width = 0.20*major_width; 
  const G4double minorT_width = 0.20*minor_width; 
  const G4double tk = fGeoInfo->GetGeoPar("steel_thickness"); 
  const G4double trnv_width  = major_width-2*tk; 
  const G4double unit_thickness = fGeoInfo->GetGeoPar("waffle_total_width"); 

  auto corner_unit_sv = new G4Box("corner_unit_sv", 
      0.5*unit_thickness, 0.5*unit_thickness, 0.5*spacing); 
  fWaffleEdgeUnit = new SLArBaseDetModule(); 
  fWaffleEdgeUnit->SetSolidVolume( corner_unit_sv );
  fWaffleEdgeUnit->SetLogicVolume(
      new G4LogicalVolume(fWaffleEdgeUnit->GetModSV(), fMatWorld->GetMaterial(), 
        "corner_unit_lv")); 

  auto corner_main_sv = new G4Box("corner_main_sv", 0.5*major_width, 0.5*major_width, 0.25*tk); 
  auto corner_main_lv = new G4LogicalVolume(corner_main_sv, fMatWaffle->GetMaterial(), 
      "corner_main_lv"); 
  corner_main_lv->SetVisAttributes( G4Colour(1, 0, 0) ); 
  G4Transform3D tt = G4Translate3D(-0.5*(unit_thickness-major_width), -0.5*(unit_thickness-major_width), 0); 
  auto corner_main_pv = new G4PVPlacement(tt * G4Translate3D(0, 0, 0.5*(spacing-0.5*tk)),
      corner_main_lv, "corner_main_pv0", 
      fWaffleEdgeUnit->GetModLV(), 0, 1); 
  corner_main_pv = new G4PVPlacement(tt * G4Translate3D(0, 0,-0.5*(spacing-0.5*tk)),
      corner_main_lv, "corner_main_pv1", 
      fWaffleEdgeUnit->GetModLV(), 0, 1); 

  auto corner_tx_sv = new G4Box("corner_tx_sv", 0.5*(major_width-2*tk), 0.5*tk, 0.5*majorT_width); 
  auto corner_tx_lv = new G4LogicalVolume(corner_tx_sv, fMatWaffle->GetMaterial(), 
      "corner_tx_lv"); 
  corner_tx_lv->SetVisAttributes( G4Colour(1, 0, 0) ); 
  new G4PVPlacement(0, 
      G4ThreeVector(-0.5*(unit_thickness-major_width), -0.5*(unit_thickness-tk), 
        -0.5*(spacing-majorT_width-tk)), 
      corner_tx_lv, "corner_tx_pv3", fWaffleEdgeUnit->GetModLV(), 0, 1); 
  new G4PVPlacement(0, 
      G4ThreeVector(-0.5*(unit_thickness-major_width), -0.5*(unit_thickness-tk), 
         0.5*(spacing-majorT_width-tk)), 
      corner_tx_lv, "corner_tx_pv4", fWaffleEdgeUnit->GetModLV(), 0, 1); 
  new G4PVPlacement(0, 
      G4ThreeVector(-0.5*(unit_thickness-major_width),  
        -0.5*(unit_thickness-2*major_width+tk), 
        -0.5*(spacing-majorT_width-tk)), 
      corner_tx_lv, "corner_tx_pv5", fWaffleEdgeUnit->GetModLV(), 0, 1); 
  new G4PVPlacement(0, 
      G4ThreeVector(-0.5*(unit_thickness-major_width),  
        -0.5*(unit_thickness-2*major_width+tk), 
         0.5*(spacing-majorT_width-tk)), 
      corner_tx_lv, "corner_tx_pv6", fWaffleEdgeUnit->GetModLV(), 0, 1); 

  auto corner_ty_sv = new G4Box("corner_ty_sv", 0.5*tk, 0.5*major_width, 0.5*majorT_width); 
  auto corner_ty_lv = new G4LogicalVolume(corner_ty_sv, fMatWaffle->GetMaterial(), 
      "corner_ty_lv"); 
  corner_ty_lv->SetVisAttributes( G4Colour(1, 0, 0) ); 
  new G4PVPlacement(0, 
      G4ThreeVector(-0.5*(unit_thickness-tk), -0.5*(unit_thickness-major_width), 
        -0.5*(spacing-majorT_width-tk)), 
      corner_ty_lv, "corner_ty_pv7", fWaffleEdgeUnit->GetModLV(), 0, 1); 
  new G4PVPlacement(0, 
      G4ThreeVector(-0.5*(unit_thickness-tk), -0.5*(unit_thickness-major_width), 
         0.5*(spacing-majorT_width-tk)), 
      corner_ty_lv, "corner_ty_pv7", fWaffleEdgeUnit->GetModLV(), 0, 1); 
  new G4PVPlacement(0, 
      G4ThreeVector(
        major_width-0.5*(unit_thickness+tk),
        -0.5*(unit_thickness-major_width), 
        -0.5*(spacing-majorT_width-tk)), 
      corner_ty_lv, "corner_ty_pv7", fWaffleEdgeUnit->GetModLV(), 0, 1); 
  new G4PVPlacement(0, 
      G4ThreeVector(
        major_width-0.5*(unit_thickness+tk), -0.5*(unit_thickness-major_width), 
         0.5*(spacing-majorT_width-tk)), 
      corner_ty_lv, "corner_ty_pv8", fWaffleEdgeUnit->GetModLV(), 0, 1); 

  fWaffleEdgeUnit->GetModLV()->SetVisAttributes( G4VisAttributes(false) ) ; 

  return;
}

SLArBaseDetModule* SLArDetCryostat::BuildSupportStructureEdge(
    G4double len, G4String name = "") {
  const G4double spacing     = fGeoInfo->GetGeoPar("waffle_spacing"); 
  const G4double major_width = fGeoInfo->GetGeoPar("waffle_major_width"); 
  const G4double minor_width = fGeoInfo->GetGeoPar("waffle_minor_width"); 
  const G4double majorT_width = 0.20*major_width; 
  const G4double minorT_width = 0.20*minor_width; 
  const G4double tk = fGeoInfo->GetGeoPar("steel_thickness"); 
  const G4double trnv_width  = major_width-2*tk; 
  const G4double unit_thickness = fGeoInfo->GetGeoPar("waffle_total_width"); 


  SLArBaseDetModule* edge = new SLArBaseDetModule(); 
  edge->SetSolidVolume( new G4Box(name+"_edge_sv", 0.5*unit_thickness, 0.5*unit_thickness, 0.5*len)); 
  edge->SetLogicVolume( new G4LogicalVolume(edge->GetModSV(), 
        fMatWorld->GetMaterial(), name+"_edge_lv")); 

  const int nz = floor( fabs(len / spacing) ); 
  G4ThreeVector start_pos(0, 0, -0.5*(nz-1)*spacing); 
  SLArPlaneParameterisation* prmtr = new SLArPlaneParameterisation(kZAxis, start_pos, spacing); 
  edge->SetModPV( new G4PVParameterised(name+"edge_ppv", 
        fWaffleEdgeUnit->GetModLV(), edge->GetModLV(), kZAxis, nz, prmtr, true) ); 

  edge->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 

  return edge; 
}

void SLArDetCryostat::BuildCryostat()
{
  if (fBuildSupport) {
    BuildSupportStructureUnit();
  }
  G4cerr << "SLArDetCryostat::BuildCryostat()" << G4endl;
  G4double tgtZ         = fGeoInfo->GetGeoPar("target_z");
  G4double tgtY         = fGeoInfo->GetGeoPar("target_y");
  G4double tgtX         = fGeoInfo->GetGeoPar("target_x");
  G4double cryo_tk      = fGeoInfo->GetGeoPar("cryostat_tk"); 
  G4double waffle_tk    = fGeoInfo->GetGeoPar("waffle_total_width"); 
  const G4double cryo_tot_tk  = cryo_tk + waffle_tk; 
   
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Create Tank 
  G4double x_ = tgtX*0.5 + cryo_tot_tk;
  G4double y_ = tgtY*0.5 + cryo_tot_tk;
  G4double z_ = tgtZ*0.5 + cryo_tot_tk;
  const G4ThreeVector cryostat_dim(x_, y_, z_); 

  // Create outer box 
  G4Box* boxOut = new G4Box("fBoxOut_solid", 
      x_ + 1*CLHEP::cm, y_ + 1*CLHEP::cm, z_ + 1*CLHEP::cm); 

  // Create inner box 
  G4Box* boxInn = new G4Box("fBoxInn_solid", 
      x_-cryo_tot_tk, y_-cryo_tot_tk, z_-cryo_tot_tk);

  fModSV =  
    new G4SubtractionSolid("cryostat_solid", 
        boxOut, boxInn, 0, G4ThreeVector(0,0,0));

  // Create Cryostat container volume
  G4cerr << "Create Cryostat" << G4endl;
  fGeoInfo->SetGeoPar("cryostat_x", 2*x_);
  fGeoInfo->SetGeoPar("cryostat_y", 2*y_);
  fGeoInfo->SetGeoPar("cryostat_z", 2*z_);
  fMaterial = fMatWorld->GetMaterial(); 

  SetLogicVolume(
    new G4LogicalVolume(GetModSV(), 
      fMaterial,
      "CryostatLV", 0, 0, 0)
    );

  // -------------------------------------------------------------------------
  // create cryostat layers

  G4cerr << "create cryostat layers" << G4endl; 
  for (const auto& ll : fCryostatStructure) {
    auto layer = ll.second;
    printf("layer: %i\n", ll.first); 
    layer->fMaterial = SLArMaterial::FindInMaterialTable(layer->fMaterialName); 
    layer->fModule = BuildCryostatLayer(layer->fName, 
        layer->fHalfSizeX, layer->fHalfSizeY, layer->fHalfSizeZ, 
        layer->fThickness, layer->fMaterial); 
    printf("placing layer in LV %p\n", static_cast<void*>(fModLV)); 
    layer->fModule->GetModPV(
        layer->fName, 0, G4ThreeVector(0, 0, 0), fModLV, 
        false, ll.first);
  }

  if (fBuildSupport) {


    for (int i=0; i<6; i++) {
      auto kFace = (geo::EBoxFace)i; 
      auto waffle_face = BuildSupportStructure(kFace);
      waffle_face->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 
      const auto face_normal = geo::BoxFaceNormal[kFace]; 
      const G4ThreeVector waffle_local_normal(0, -1, 0); 
      G4ThreeVector rot_axis = face_normal.cross(waffle_local_normal); 
      G4double rot_angle = face_normal.angle(waffle_local_normal); 
      if (rot_axis.mag2() < 1e-6) rot_axis = face_normal.orthogonal(); 

      G4String face_pv_name = "waffle_face"+std::to_string(i)+"_pv"; 
      G4ThreeVector pos = -face_normal * 
        (fabs(face_normal.dot(cryostat_dim)) 
         - 0.5*fGeoInfo->GetGeoPar("waffle_total_width")); 
      G4RotationMatrix* rot = new G4RotationMatrix(rot_axis, rot_angle);
      waffle_face->GetModPV(face_pv_name, rot, pos, fModLV, false, i+1); 
      fSupportStructure.insert( std::make_pair(kFace, waffle_face) ); 
    }

    BuildSupportStructureEdgeUnit(); 

    // build support structure edges
    
    // top view 
    const auto face_top = fSupportStructure[geo::kYplus]; 
    const auto face_side = fSupportStructure[geo::kZplus]; 
    const auto sv_top = face_top->GetModLV()->GetDaughter(0)->GetLogicalVolume()->GetSolid(); 
    const auto sv_side = face_side->GetModLV()->GetDaughter(0)->GetLogicalVolume()->GetSolid(); 
    const auto len_z = static_cast<G4Box*>(sv_top)->GetZHalfLength() * 2; 
    const auto len_x = static_cast<G4Box*>(sv_top)->GetXHalfLength() * 2; 
    const auto len_y = static_cast<G4Box*>(sv_side)->GetZHalfLength() * 2; 

    auto edge_yx = BuildSupportStructureEdge(len_z, "yx"); 
    auto edge_yz = BuildSupportStructureEdge(len_x, "yz"); 
    auto edge_xz = BuildSupportStructureEdge(len_y, "xz"); 

    G4Transform3D t0 = G4Rotate3D(CLHEP::pi, G4ThreeVector(0, 1, 0)); 
    fSupportStructureEdges.push_back(
        new G4PVPlacement(0, 
          G4ThreeVector(cryostat_dim.x() - 0.5*waffle_tk, 
            cryostat_dim.y()-0.5*waffle_tk, 0), 
          edge_yx->GetModLV(),
          "edge_y+_x+_pv", fModLV, 0, 6) ); 
    fSupportStructureEdges.push_back(
        new G4PVPlacement( 
          G4Translate3D(-cryostat_dim.x() + 0.5*waffle_tk, 
            cryostat_dim.y()-0.5*waffle_tk, 0) * t0,
          edge_yx->GetModLV(),
          "edge_y+_x-_pv", fModLV, 0, 7) ); 
    G4Transform3D t1 = G4Rotate3D(CLHEP::pi, G4ThreeVector(1, 0, 0)); 
    fSupportStructureEdges.push_back( 
        new G4PVPlacement(
          G4Translate3D( cryostat_dim.x() - 0.5*waffle_tk, 
            -cryostat_dim.y()+0.5*waffle_tk, 0) * t1,
          edge_yx->GetModLV(),
          "edge_y-_x+_pv", fModLV, 0, 8) ); 
    fSupportStructureEdges.push_back( 
        new G4PVPlacement( G4Transform3D(
          G4Translate3D(-cryostat_dim.x() + 0.5*waffle_tk, 
            -cryostat_dim.y()+0.5*waffle_tk, 0) * t1 * t0),
          edge_yx->GetModLV(),
          "edge_y-_x-_pv", fModLV, 0, 9) ); 
    
    fSupportStructureEdges.push_back( 
        new G4PVPlacement( 
          G4Translate3D(0, cryostat_dim.y() - 0.5*waffle_tk, 
            -cryostat_dim.z()+0.5*waffle_tk)*
          G4Rotate3D(0.5*CLHEP::pi, G4ThreeVector(0, 1, 0)), 
          edge_yz->GetModLV(), "edge_y+z-_pv", fModLV, false, 10, true) ); 
    fSupportStructureEdges.push_back(
        new G4PVPlacement( 
          G4Translate3D(0, cryostat_dim.y() - 0.5*waffle_tk, 
            +cryostat_dim.z()-0.5*waffle_tk)*
          G4Rotate3D(0.5*CLHEP::pi, G4ThreeVector(1, 0, 0)) *
          G4Rotate3D(0.5*CLHEP::pi, G4ThreeVector(0, 1, 0)), 
          edge_yz->GetModLV(), "edge_y+z+_pv", fModLV, false, 11, true) ); 
    fSupportStructureEdges.push_back(
        new G4PVPlacement( 
          G4Translate3D(0, -cryostat_dim.y() + 0.5*waffle_tk, 
            -cryostat_dim.z()+0.5*waffle_tk)*
          G4Rotate3D(1.5*CLHEP::pi, G4ThreeVector(1, 0, 0)) *
          G4Rotate3D(.5*CLHEP::pi, G4ThreeVector(0, 1, 0)), 
          edge_yz->GetModLV(), "edge_y-z-_pv", fModLV, false, 12, true) ); 
    fSupportStructureEdges.push_back(
        new G4PVPlacement( 
          G4Translate3D(0, -cryostat_dim.y() + 0.5*waffle_tk, 
            +cryostat_dim.z()-0.5*waffle_tk)*
          G4Rotate3D(CLHEP::pi, G4ThreeVector(1, 0, 0)) *
          G4Rotate3D(0.5*CLHEP::pi, G4ThreeVector(0, 1, 0)), 
          edge_yz->GetModLV(), "edge_y-z-_pv", fModLV, false, 13, true) ); 

    fSupportStructureEdges.push_back(
        new G4PVPlacement( 
          G4Translate3D(cryostat_dim.x()-0.5*waffle_tk, 0, 
            -cryostat_dim.z()+0.5*waffle_tk)*
          G4Rotate3D(0.5*CLHEP::pi, G4ThreeVector(1, 0, 0)) * 
          G4Rotate3D(1.5*CLHEP::pi, G4ThreeVector(0, 0, 1)), 
          edge_xz->GetModLV(), "edge_x+z-_pv", fModLV, false, 14, true) ); 
    fSupportStructureEdges.push_back( 
        new G4PVPlacement( 
          G4Translate3D(-cryostat_dim.x()+0.5*waffle_tk, 0, 
            -cryostat_dim.z()+0.5*waffle_tk)*
          G4Rotate3D(0.5*CLHEP::pi, G4ThreeVector(1, 0, 0)) * 
          G4Rotate3D(1.0*CLHEP::pi, G4ThreeVector(0, 0, 1)), 
          edge_xz->GetModLV(), "edge_x-z-_pv", fModLV, false, 15, true) ); 
    fSupportStructureEdges.push_back(
        new G4PVPlacement( 
          G4Translate3D(cryostat_dim.x()-0.5*waffle_tk, 0, 
            cryostat_dim.z()-0.5*waffle_tk)*
          G4Rotate3D(-0.5*CLHEP::pi, G4ThreeVector(1, 0, 0)) * 
          G4Rotate3D(1.5*CLHEP::pi, G4ThreeVector(0, 0, 1)), 
          edge_xz->GetModLV(), "edge_x+z+_pv", fModLV, false, 16, true) ); 
    fSupportStructureEdges.push_back(
        new G4PVPlacement( 
          G4Translate3D(-cryostat_dim.x()+0.5*waffle_tk, 0, 
            cryostat_dim.z()-0.5*waffle_tk)*
          G4Rotate3D(-0.5*CLHEP::pi, G4ThreeVector(1, 0, 0)) * 
          G4Rotate3D(1.0*CLHEP::pi, G4ThreeVector(0, 0, 1)), 
          edge_xz->GetModLV(), "edge_x-z+_pv", fModLV, false, 17, true) ); 
  }

  return; 
}

SLArBaseDetModule* SLArDetCryostat::BuildCryostatLayer(
    G4String name, 
    G4double x_, G4double y_, G4double z_, G4double tk_, 
    G4Material* mat) {

  G4Box* b_out = new G4Box("b_out_"+name, x_+tk_, y_+tk_, z_+tk_); 
  G4Box* b_in  = new G4Box("b_in_" +name, x_    , y_    , z_    ); 

  G4SubtractionSolid* solid = new G4SubtractionSolid(name+"_solid", 
      b_out, b_in, 0, G4ThreeVector(0, 0, 0)); 

  SLArBaseDetModule* mod = new SLArBaseDetModule(); 
  mod->SetMaterial(mat); 
  mod->SetSolidVolume(solid); 
  mod->SetLogicVolume(new G4LogicalVolume(
        mod->GetModSV(), mod->GetMaterial(), name+"LV", 0, 0, 0)); 

/*
 *  //create a daughter volume on the -z face to be used as a sensitive detector 
 *  //for neutron shielding studies
 *
 *  G4Box* b_test_sv = new G4Box("b_test_sv_"+name, x_, y_, 0.5*tk_); 
 *  G4LogicalVolume* b_test_lv = new G4LogicalVolume(b_test_sv, mat, "b_test_lv_"+name, 
 *      0, 0, 0, 0); 
 *  new G4PVPlacement(0, G4ThreeVector(0, 0, -z_-0.5*tk_), b_test_lv, "b_test_pv_"+name, 
 *      mod->GetModLV(), false, 8, true); 
 *
 */
  return mod; 
}

void SLArDetCryostat::SetVisAttributes() {
  std::map<G4String, G4Colour> col_map; 
  col_map.insert( std::make_pair("Steel", G4Colour(0.231, 0.231, 0.227))); 
  col_map.insert( std::make_pair("Plywood", G4Colour(0.671, 0.553, 0.196))); 
  col_map.insert( std::make_pair("Water", G4Colour(0.561, 0.863, 0.91))); 
  col_map.insert( std::make_pair("BoratedPolyethilene", G4Colour(0.267, 0.671, 0.22))); 
  col_map.insert( std::make_pair("Polyurethane", G4Colour(0.867, 0.871, 0.769))); 
  G4Colour stdCol(0.611, 0.847, 0.988);
  fModLV->SetVisAttributes( G4VisAttributes(false) );
  for (auto &ll : fCryostatStructure) {
    auto lv = ll.second->fModule->GetModLV();
    printf("%s\n", lv->GetName().c_str());
    G4Colour col = stdCol; 
    if (col_map.count(lv->GetMaterial()->GetName()))
    {
      col = col_map[lv->GetMaterial()->GetName()]; 
    }
    lv->SetVisAttributes( G4VisAttributes( col ) ); 
  }
}

void SLArDetCryostat::BuildMaterials(G4String material_db) {
  for (auto &layer : fCryostatStructure) {
    SLArMaterial* mat = new SLArMaterial; 
    mat->SetMaterialID( layer.second->fMaterialName ); 
    mat->BuildMaterialFromDB(material_db); 
    layer.second->fMaterial = mat->GetMaterial(); 
  }

  fMatWaffle = new SLArMaterial(); 
  fMatWaffle->SetMaterialID("Steel"); 
  fMatWaffle->BuildMaterialFromDB(material_db); 

  if (fMatBrick) {
    fMatBrick->BuildMaterialFromDB(material_db); 
  }
  return;
}

//void SLArDetCryostat::SLArWaffleCornersParameterisation::ComputeTransformation(G4int copyNo, G4VPhysicalVolume* physVol) const {
  //const auto sv = static_cast<G4Trd*>(physVol->GetLogicalVolume()->GetSolid()); 
  //const auto hz = sv->GetZHalfLength(); 
  //const auto axis_x = G4ThreeVector(1, 0, 0); 
  //const auto axis_z = G4ThreeVector(0, 0, 1); 
  //const auto ang = (15 + copyNo*30)*CLHEP::deg;
  //const double tmp = 2*hz*sin(0.5*ang); 
  //printf("tmp is %g\n", tmp);
  //G4Transform3D trns = G4Rotate3D(-0.5*CLHEP::pi, axis_x) * 
    //G4Translate3D(0, 0, hz) * 
    //G4Translate3D(tmp*cos(0.5*ang), 0, -tmp*sin(0.5*ang)) * 
    //G4Rotate3D(ang, axis_z);
  //G4RotationMatrix* rot = new G4RotationMatrix( trns.getRotation() ); 
  //physVol->SetRotation( rot ); 
  //physVol->SetTranslation( trns.getTranslation() ); 
//}
