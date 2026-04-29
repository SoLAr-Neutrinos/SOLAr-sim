/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArDetFieldCage
 * @created     : Tuesday Apr 28, 2026 14:30:34 CEST
 */

#include "detector/TPC/SLArDetFieldCage.hh"
#include "detector/SLArPlaneParameterisation.hpp"
#include "SLArDebugUtils.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4VisAttributes.hh"
#include "G4PhysicalConstants.hh"

SLArDetFieldCage::SLArDetFieldCage()
  : SLArBaseDetModule(),
    fShape(geo::kBox),
    fShift(0., 0., 0.),
    fDriftDir(1., 0., 0.)
{}

SLArDetFieldCage::~SLArDetFieldCage() {}

void SLArDetFieldCage::ResolveDriftAxes(int& iDrift,
    int& iTrans1,
    int& iTrans2) const
{
  const G4ThreeVector ax[3] = {
    G4ThreeVector(1,0,0), G4ThreeVector(0,1,0), G4ThreeVector(0,0,1)};
  iTrans1 = -1; iTrans2 = -1; iDrift = -1;
  for (int i = 0; i < 3; i++) {
    if (std::abs(fDriftDir.dot(ax[i])) > 0.5) 
      iDrift = i;
    else if (iTrans1 < 0) 
      iTrans1 = i;
    else 
      iTrans2 = i;
  }
  assert(iDrift >= 0 && iTrans1 >= 0 && iTrans2 >= 0);
}

void SLArDetFieldCage::Init(const rapidjson::Value& jconf) 
{
  debug::require_json_member(jconf, "shape");
  fShape    = geo::get_geo_shape_code(jconf["shape"].GetString());

  debug::require_json_member(jconf, "drift_direction");
  debug::require_json_array(jconf["drift_direction"], 3);
  const auto& jdd = jconf["drift_direction"].GetArray();

  fDriftDir.set(jdd[0].GetDouble(), jdd[1].GetDouble(), jdd[2].GetDouble());
  fDriftDir /= fDriftDir.mag(); // ensure unit vector

  int iDrift, iTrans1, iTrans2;
  ResolveDriftAxes(iDrift, iTrans1, iTrans2);
  const G4String axLabel[3] = {"x", "y", "z"};

  debug::require_json_member(jconf, "tpc_x");
  debug::require_json_member(jconf, "tpc_y");
  debug::require_json_member(jconf, "tpc_z");

  // ---- Parameters shared by both shapes ----
  assert(jconf.HasMember("thickness"));
  assert(jconf.HasMember("spacing"));
  fGeoInfo->RegisterGeoPar("thickness", unit::ParseJsonVal(jconf["thickness"]));
  fGeoInfo->RegisterGeoPar("spacing",   unit::ParseJsonVal(jconf["spacing"]));

  // ---- Shift (optional, default (0,0,0)) ----
  fShift.set(0., 0., 0.);
  if (jconf.HasMember("shift")) {
    const auto& js = jconf["shift"];
    const G4double su = unit::GetJSONunit(js);
    const auto& jxyz = js["xyz"].GetArray();
    assert(jxyz.Size() == 3);
    fShift.set(jxyz[0].GetDouble()*su,
               jxyz[1].GetDouble()*su,
               jxyz[2].GetDouble()*su);
  }

  // ---- Longitudinal dimension (explicit, else TPC dim − 2 mm safety margin) ----
  G4String longKey = {};
  
  if (jconf.HasMember("longitudinal_dimension")) {
    longKey = "longitudinal_dimension";
  } else if (jconf.HasMember("length")) {
    longKey = "length";
  }

  if (longKey.empty() == false && jconf.HasMember(longKey.c_str())) {
    fGeoInfo->RegisterGeoPar("fc_drift_length",
        unit::ParseJsonVal(jconf[longKey.c_str()]));
  } else {
    const G4double tpcDrift = jconf[("tpc_"+axLabel[iDrift]).data()].GetDouble();
    fGeoInfo->RegisterGeoPar("fc_drift_length", tpcDrift - 2.*CLHEP::cm);
  }

  if (fShape == geo::kBox) {
    // -- Ring geometry --
    debug::require_json_member(jconf, "corner_radius");
    debug::require_json_member(jconf, "height_long_side");
    debug::require_json_member(jconf, "height_short_side");
    fGeoInfo->RegisterGeoPar("corner_radius",
        unit::ParseJsonVal(jconf["corner_radius"]));
    fGeoInfo->RegisterGeoPar("height_long_side",
        unit::ParseJsonVal(jconf["height_long_side"]));
    fGeoInfo->RegisterGeoPar("height_short_side",
        unit::ParseJsonVal(jconf["height_short_side"]));

    // -- Transverse dimensions: explicit pair OR wall_distance fallback --
    if (jconf.HasMember("transverse_dimensions")) {
      const auto& jt = jconf["transverse_dimensions"].GetArray();
      assert(jt.Size() == 2);
      fGeoInfo->RegisterGeoPar("fc_trans_u", unit::ParseJsonVal(jt[0]));
      fGeoInfo->RegisterGeoPar("fc_trans_v", unit::ParseJsonVal(jt[1]));
    } else {
      debug::require_json_member(jconf, "wall_distance");
      const G4double wd = unit::ParseJsonVal(jconf["wall_distance"]);
      fGeoInfo->RegisterGeoPar("wall_distance", wd);
      fGeoInfo->RegisterGeoPar("fc_trans_u",
          jconf[("tpc_"+axLabel[iTrans1]).data()].GetDouble()- wd);
      fGeoInfo->RegisterGeoPar("fc_trans_v",
          jconf[("tpc_"+axLabel[iTrans2]).data()].GetDouble() - wd);
    }
  }
  else { // kTub
    // -- Ring height --
    assert(jconf.HasMember("ring_height"));
    fGeoInfo->RegisterGeoPar("ring_height",
        unit::ParseJsonVal(jconf["ring_height"]));

    // -- Radius: explicit OR wall_distance fallback --
    if (jconf.HasMember("radius")) {
      fGeoInfo->RegisterGeoPar("fc_radius",
          unit::ParseJsonVal(jconf["radius"]));
    } else {
      debug::require_json_member(jconf, "tpc_radius");
      debug::require_json_member(jconf, "wall_distance");
      const G4double wd = unit::ParseJsonVal(jconf["wall_distance"]);
      fGeoInfo->RegisterGeoPar("wall_distance", wd);
      fGeoInfo->RegisterGeoPar("fc_radius",
          jconf["tpc_radius"].GetDouble() - wd);
    }
  }

  return;
}

void SLArDetFieldCage::Build(G4Material* matConductor, G4Material* matFill)
{
  if (fShape == geo::kBox) BuildBox(matConductor, matFill);
  else                     BuildTub(matConductor, matFill);
}

void SLArDetFieldCage::BuildBox(G4Material* matConductor, G4Material* matFill)
{
  const G4double R   = fGeoInfo->GetGeoPar("corner_radius");
  const G4double tk  = fGeoInfo->GetGeoPar("thickness");
  const G4double hl  = fGeoInfo->GetGeoPar("height_long_side");
  const G4double hs  = fGeoInfo->GetGeoPar("height_short_side");
  const G4double sp  = fGeoInfo->GetGeoPar("spacing");
  const G4double Du  = fGeoInfo->GetGeoPar("fc_trans_u");   // was Dfc[1]
  const G4double Dv  = fGeoInfo->GetGeoPar("fc_trans_v");   // was Dfc[2]
  const G4double DDriftLength = fGeoInfo->GetGeoPar("fc_drift_length");

  // Assign long/short ring heights to the two transverse axes
  const G4double hy = (Du > Dv) ? hl : hs;
  const G4double hz = (Du > Dv) ? hs : hl;

  // ---- Single electrode layer (ring cross-section) ----
  const G4ThreeVector cornerTubAxis(0., 0., 1.);
  auto fc_corner_tub = new G4Tubs("fc_corner_sv",
      R-tk, R, 0.5*std::max(hl,hs), 0., 90.*CLHEP::deg);
  auto fc_corner_lv  = new G4LogicalVolume(fc_corner_tub, matConductor, "fc_corner_lv");

  auto fc_yside_box  = new G4Box("fc_yside_box", 0.5*hy, 0.5*tk, 0.5*Du - R);
  auto fc_zside_box  = new G4Box("fc_zside_box", 0.5*hz, 0.5*tk, 0.5*Dv - R);
  auto fc_yside_lv   = new G4LogicalVolume(fc_yside_box, matConductor, "fc_yside_lv");
  auto fc_zside_lv   = new G4LogicalVolume(fc_zside_box, matConductor, "fc_zside_lv");

  const G4double deltaDim = R - (R-tk)*std::cos(45.*CLHEP::deg) + 1.*CLHEP::mm;
  auto layer_outer = new G4Box("fc_layer_outerBox", 0.5*hs, 0.5*Du, 0.5*Dv);
  auto layer_inner = new G4Box("fc_layer_innerBox",
      0.5*hs, 0.5*Du-deltaDim, 0.5*Dv-deltaDim);
  auto fc_layer_sv = new G4SubtractionSolid("fc_layer_sv",
      layer_outer, layer_inner);
  auto fc_layer_lv = new G4LogicalVolume(fc_layer_sv, matFill, "fieldCage_layer_lv");
  fc_layer_lv->SetVisAttributes(G4VisAttributes(false));

  // Place four rounded corners
  const G4double angStep[4] = {0., -0.5, -1.0, -1.5};
  const G4double cy[4] = { 1., 1., -1., -1.};
  const G4double cz[4] = { 1., -1., -1., 1.};
  G4double delta = cornerTubAxis.angle(G4ThreeVector(1,0,0));
  auto rot_common = new G4RotationMatrix(
      cornerTubAxis.cross(G4ThreeVector(1,0,0)), delta);
  for (int ic = 0; ic < 4; ic++) {
    auto rot = new G4RotationMatrix(*rot_common);
    rot->rotateZ(angStep[ic]*CLHEP::pi);
    new G4PVPlacement(rot,
        G4ThreeVector(0., cy[ic]*(0.5*Du-R), cz[ic]*(0.5*Dv-R)),
        fc_corner_lv, "fc_corner"+std::to_string(ic)+"_pv",
        fc_layer_lv, false, 10+ic);
  }

  // Place four sides
  const G4ThreeVector xAxis(1.,0.,0.);
  new G4PVPlacement(new G4RotationMatrix(xAxis,  0.5*CLHEP::pi),
      G4ThreeVector(0., 0., -0.5*(Dv-tk)),
      fc_yside_lv, "fc_yside0_pv", fc_layer_lv, false, 14);
  new G4PVPlacement(new G4RotationMatrix(xAxis,  0.5*CLHEP::pi),
      G4ThreeVector(0., 0., +0.5*(Dv-tk)),
      fc_yside_lv, "fc_yside1_pv", fc_layer_lv, false, 15);
  new G4PVPlacement(nullptr, G4ThreeVector(0., -0.5*(Du-tk), 0.),
      fc_zside_lv, "fc_zside0_pv", fc_layer_lv, false, 16);
  new G4PVPlacement(nullptr, G4ThreeVector(0., +0.5*(Du-tk), 0.),
      fc_zside_lv, "fc_zside1_pv", fc_layer_lv, false, 17);

  // Container volume (hollow box spanning drift length)
  auto vol_outer = new G4Box("fcvolume_outer_box",
      0.5*DDriftLength, 0.5*Du, 0.5*Dv);
  auto vol_inner = new G4Box("fcvolume_inner_box",
      0.5*DDriftLength, 0.5*Du-deltaDim, 0.5*Dv-deltaDim);
  auto fc_volume_sv = new G4SubtractionSolid("fc_volume_sv", vol_outer, vol_inner);
  auto fc_volume_lv = new G4LogicalVolume(fc_volume_sv, matFill, "fc_volume_lv");
  fc_volume_lv->SetVisAttributes(G4VisAttributes(false));

  SetSolidVolume(fc_volume_sv);
  SetLogicVolume(fc_volume_lv);

  // Parameterise along local X (BuildTPC rotates to fElectronDriftDir)
  G4double len = hs, len_tmp = hs;
  G4int n_replica = 0;
  while (len_tmp <= DDriftLength) { len = len_tmp; n_replica++; len_tmp += sp; }

  auto param = new SLArPlaneParameterisation(kXAxis,
      G4ThreeVector(-0.5*(len - sp + hl), 0., 0.), sp);
  SetModPV(new G4PVParameterised("fieldCage_ppv",
        fc_layer_lv, fc_volume_lv,
        param->GetReplicationAxis(), n_replica-1, param));
  GetModPV()->SetCopyNo(20);
}

void SLArDetFieldCage::BuildTub(G4Material* matConductor, G4Material* matFill)
{
  const G4double tk  = fGeoInfo->GetGeoPar("thickness");
  const G4double sp  = fGeoInfo->GetGeoPar("spacing");
  const G4double rh  = fGeoInfo->GetGeoPar("ring_height");
  const G4double R_fc = fGeoInfo->GetGeoPar("fc_radius");
  const G4double DDriftLength = fGeoInfo->GetGeoPar("fc_drift_length");

  // ---- Single ring (one field-cage electrode) ----
  auto fc_ring_sv = new G4Tubs("fc_ring_sv", R_fc - tk, R_fc, 0.5*rh, 0., CLHEP::twopi);
  auto fc_ring_lv = new G4LogicalVolume(fc_ring_sv, matConductor, "fc_ring_lv");

  // ---- Container: cylindrical shell spanning the drift length ----
  // rMin/rMax bracket the ring so the parameterised placement is valid.
  auto fc_volume_sv = new G4Tubs("fc_volume_sv",
      R_fc - tk, R_fc, 0.5*DDriftLength, 0., CLHEP::twopi);
  auto fc_volume_lv = new G4LogicalVolume(fc_volume_sv, matFill, "fc_volume_lv");
  fc_volume_lv->SetVisAttributes(G4VisAttributes(false));

  SetSolidVolume(fc_volume_sv);
  SetLogicVolume(fc_volume_lv);

  // ---- Compute number of ring replicas (same logic as box cage) ----
  G4double len = rh, len_tmp = rh;
  G4int n_replica = 0;
  while (len_tmp <= DDriftLength) { len = len_tmp; n_replica++; len_tmp += sp; }

  // Parameterise along Z (natural tubs axis; BuildTPC() will rotate to fElectronDriftDir)
  auto parameterisation = new SLArPlaneParameterisation(kZAxis,
      G4ThreeVector(0, 0, -0.5*(len - sp + rh)), sp);

  SetModPV(new G4PVParameterised("fieldCage_ppv",
      fc_ring_lv, fc_volume_lv,
      parameterisation->GetReplicationAxis(), n_replica - 1, parameterisation));
  GetModPV()->SetCopyNo(20);
}


void SLArDetFieldCage::SetVisAttributes(G4bool visible)
{
  G4Colour col(0.3, 0.3, 0.3, 0.45);
  G4VisAttributes vis;
  visible ? vis.SetColor(col) : vis.SetVisibility(false);

  const size_t nd = GetModLV()->GetNoDaughters();
  for (size_t i = 0; i < nd; i++) {
    G4LogicalVolume* lv = GetModLV()->GetDaughter(i)->GetLogicalVolume();
    for (size_t j = 0; j < lv->GetNoDaughters(); j++)
      lv->GetDaughter(j)->GetLogicalVolume()->SetVisAttributes(vis);
  }
}
