/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArDetOpDetArray.cc
 * @created     Fri Mar 24, 2023 16:08:29 CET
 */

#include "detector/OpDet/SLArDetOpDetArray.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgSuperCell.hh"
#include "core/SLArDebugUtils.hh"
#include "G4PVParameterised.hh"
#include "G4Box.hh"
#include "G4VisAttributes.hh"
#include "G4Exception.hh"

SLArDetOpDetArray::SLArDetOpDetArray() : 
  SLArBaseDetModule(), 
  fTPCID(0), 
  fMaterialBase(nullptr), fOpDetModuleBase(nullptr),  
  fPosition(0, 0, 0), fGlobalPosition(0, 0, 0), fNormal(0, 1, 0),
  fRotation(nullptr), fPhotoDetModel("")
{}

SLArDetOpDetArray::~SLArDetOpDetArray() {}

void SLArDetOpDetArray::BuildMaterial(G4String materials_db) {
  fMaterialBase = new SLArMaterial("LAr"); 
  fMaterialBase->BuildMaterialFromDB(materials_db); 
  return;
}

void SLArDetOpDetArray::Init(const rapidjson::Value& jconf) {
  G4String xyz_suffix[3] = {"x", "y", "z"}; 
  G4String ang_suffix[3] = {"phi", "theta", "psi"}; 

  debug::require_json_type(jconf, rapidjson::kObjectType);
  const auto& jarray = jconf.GetObject(); 

  debug::require_json_member(jarray, "name");
  debug::require_json_member(jarray, "copyID"); 
  debug::require_json_member(jarray, "position"); 
  debug::require_json_member(jarray, "dimensions"); 
  debug::require_json_member(jarray, "rot"); 
  debug::require_json_member(jarray, "normal"); 
  debug::require_json_member(jarray, "tpcID"); 
  debug::require_json_member(jarray, "photodet_model");

  fName = jarray["name"].GetString();

  SetID( jarray["copyID"].GetInt() ); 

  fPhotoDetModel = jarray["photodet_model"].GetString(); 

  fTPCID = jarray["tpcID"].GetInt();

  auto jpos = jarray["position"].GetObject(); 
  int idim = 0; 
  G4double vunit = unit::Unit2Val(jpos["unit"]); 
  for (const auto &v : jpos["xyz"].GetArray()) {
    fPosition[idim] = (v.GetDouble() * vunit); 
    fGeoInfo->RegisterGeoPar("pos_"+xyz_suffix[idim], fPosition[idim]); 
    idim++; 
  }

  auto jrot = jarray["rot"].GetObject(); 
  vunit = unit::Unit2Val(jrot["unit"]); 
  assert(jrot.HasMember("val")); 
  assert(jrot["val"].IsArray()); 
  assert(jrot["val"].GetArray().Size() == 3);
  double eulerAngles[3] = {0.}; 
  idim = 0; 
  for (const auto &v : jrot["val"].GetArray()) {
     eulerAngles[idim] = v.GetDouble()*vunit; 
     fGeoInfo->RegisterGeoPar("opdetarray_"+ang_suffix[idim], eulerAngles[idim]); 
     idim++; 
  }
  fRotation = new G4RotationMatrix(eulerAngles[0], eulerAngles[1], eulerAngles[2]); 
 
  auto jdim = jarray["dimensions"].GetArray(); 
  fGeoInfo->ReadFromJSON(jdim); 

  idim = 0;
  for (const auto &v : jarray["normal"].GetArray()) {
    fNormal[idim] = v.GetDouble(); 
    idim++; 
  }

  const bool has_replication = jarray.HasMember("replication_data");
  const bool has_explicit    = jarray.HasMember("opdet_positions");
 
  if (has_replication && has_explicit) {
    G4Exception("SLArDetOpDetArray::Init", "ConfigError001", FatalException,
        "OpDetArray config contains both 'replication_data' and 'opdet_positions'. "
        "Only one placement mode may be specified.");
  }
 
  if (has_replication) {
    fPlacementMode = EPlacementMode::kParameterised;
    if (jarray["replication_data"].IsObject()) {
      fParameterisation.push_back(
          new SLArPlaneParameterisation(jarray["replication_data"]));
    } else if (jarray["replication_data"].IsArray()) {
      for (const auto& rdata : jarray["replication_data"].GetArray()) {
        fParameterisation.push_back(new SLArPlaneParameterisation(rdata));
      }
    } else {
      G4Exception("SLArDetOpDetArray::Init", "ConfigError002", FatalException,
          "Invalid format for 'replication_data': expected an object or array of objects.");
    }
  } else if (has_explicit) {
    fPlacementMode = EPlacementMode::kExplicit;
    debug::require_json_array(jarray["opdet_positions"]);
 
    for (const auto& jentry : jarray["opdet_positions"].GetArray()) {
      debug::require_json_type(jentry, rapidjson::kObjectType);
      debug::require_json_member(jentry, "id");
      debug::require_json_member(jentry, "xyz");
      debug::require_json_array(jentry["xyz"], 3);
 
      SExplicitOpDetPos entry;
      entry.id = jentry["id"].GetInt();
 
      // Optional per-entry unit; falls back to 1.0 (== mm in G4 units) if absent
      const G4double pos_unit = unit::GetJSONunit(jentry);
      const auto& jxyz = jentry["xyz"].GetArray();
      entry.position.set(
          jxyz[0].GetDouble() * pos_unit,
          jxyz[1].GetDouble() * pos_unit,
          jxyz[2].GetDouble() * pos_unit);
 
      fExplicitPositions.push_back(entry);
    }
 
    if (fExplicitPositions.empty()) {
      G4Exception("SLArDetOpDetArray::Init", "ConfigError003", JustWarning,
          "OpDetArray 'opdet_positions' list is empty. No detectors will be placed.");
    }
  } else {
    G4Exception("SLArDetOpDetArray::Init", "ConfigError004", FatalException,
        "OpDetArray configuration must contain either 'replication_data' or 'opdet_positions'.");
  }
 
  return;
}

void SLArDetOpDetArray::BuildOpDetArray(SLArOpticalDetector* opdet) {
  if (fPlacementMode == EPlacementMode::kExplicit) {
    BuildOpDetArrayExplicit(opdet);
  } else {
    BuildOpDetArrayParameterised(opdet);
  }
}

void SLArDetOpDetArray::BuildOpDetArrayParameterised(SLArOpticalDetector* opdet) {
  fOpDetModuleBase = opdet;

  G4ThreeVector max_dim( 
      fGeoInfo->GetGeoPar("dim_x"), 
      fGeoInfo->GetGeoPar("dim_y"),
      fGeoInfo->GetGeoPar("dim_z")
      ) ;

  G4ThreeVector localNormal = G4ThreeVector(0, 1, 0); 

  auto build_parameterised_vol = [&](
      SLArBaseDetModule* origin, 
      SLArBaseDetModule* target, 
      G4String target_prefix,
      SLArPlaneParameterisation* rpars) {

    G4ThreeVector tmp_dim = max_dim; 
    G4ThreeVector origin_dim; 
    
    G4Box* originBox = (G4Box*)origin->GetModSV();
    //origin->GetGeoInfo()->DumpParMap(); 
    //target->GetGeoInfo()->DumpParMap(); 

    origin_dim[0] = 2*originBox->GetXHalfLength(); 
    origin_dim[1] = 2*originBox->GetYHalfLength(); 
    origin_dim[2] = 2*originBox->GetZHalfLength(); 

    G4ThreeVector perp_ax = localNormal.cross(rpars->GetReplicationAxisVector()); 
    G4double module_wdt = 0.; 
    for (int i=0; i<3; i++) {
      if ( fabs(perp_ax[i] * origin_dim[i] ) > 0 ) {
        tmp_dim[i] = origin_dim[i]; 
        module_wdt = origin_dim[i]; 
      } else {
        tmp_dim[i] += 1*CLHEP::mm;
      }
    }

    target->SetSolidVolume(new G4Box(target_prefix+"_sv", 
          0.5*tmp_dim[0], 0.5*tmp_dim[1], 0.5*tmp_dim[2])); 
    target->SetLogicVolume(new G4LogicalVolume(target->GetModSV(), 
          fMaterialBase->GetMaterial(), target_prefix+"_lv")); 
    auto start_ = ComputeArrayTrueLength(
        origin_dim.dot(rpars->GetReplicationAxisVector()), 
        rpars->GetSpacing(), 
        rpars->GetReplicationAxisVector().dot(max_dim)); 
    rpars->SetStartPos( 
        0.5*rpars->GetReplicationAxisVector()
        *(-start_.second + origin_dim.dot(rpars->GetReplicationAxisVector())) 
        );
    printf("SLArDetSuperCellArray::build_parameterised_vol: origin %s -> target %s\n", 
        origin->GetModLV()->GetName().data(), 
        target->GetModLV()->GetName().data());

    G4String pvp_name = target_prefix + "_ppv"; 
    target->SetModPV(
        new G4PVParameterised(pvp_name, 
          origin->GetModLV(), target->GetModLV(), 
          rpars->GetReplicationAxis(), start_.first,
          rpars, true)); 
  }; 

  for (auto &rpars : fParameterisation) {
    SLArBaseDetModule* target = nullptr; 
    SLArBaseDetModule* origin = nullptr;
    G4String target_prefix = "";
    if (rpars == fParameterisation.back()) {
      target = this; 
      origin = fSubModules.back();
      target_prefix = "opdet_row";
    } 
    else if (rpars == fParameterisation.front()) {
      fSubModules.push_back( new SLArBaseDetModule() ); 
      target = fSubModules.back();
      origin = opdet;
      target_prefix = "opdet";
    }
    else {
      G4ExceptionDescription ed;
      ed << "Unexpected parameterisation in SLArDetOpDetArray::BuildOpDetArray! ";
      ed << "Only two parameterisations are expected (for rows and columns), but found more.";
      G4Exception("SLArDetOpDetArray::BuildOpDetArray", "ConfigError002", FatalException, ed);
    }

    build_parameterised_vol(origin, target, target_prefix, rpars);
  }

  fModPV->SetCopyNo(800+fID); 
  auto vol = fModLV->GetDaughter(0); 
  //printf("vol name: %s (%lu daughters)\n", vol->GetName().data(), fModLV->GetNoDaughters());
  vol->SetCopyNo(800+fID); 

  for (auto &subModules : fSubModules) {
    subModules->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 
  }

  fModLV->SetVisAttributes( G4VisAttributes(false) ); 
}

void SLArDetOpDetArray::BuildOpDetArrayExplicit(SLArOpticalDetector* opdet) {
  // Build a bounding-box volume for the whole array using the dimensions
  // declared in the JSON configuration.
  const G4ThreeVector max_dim(
      fGeoInfo->GetGeoPar("dim_x"),
      fGeoInfo->GetGeoPar("dim_y"),
      fGeoInfo->GetGeoPar("dim_z"));
 
  SetSolidVolume(new G4Box(
      fName + "_sv",
      0.5 * max_dim.x(), 0.5 * max_dim.y(), 0.5 * max_dim.z()));
  SetLogicVolume(new G4LogicalVolume(
      fModSV, fMaterialBase->GetMaterial(), fName + "_lv"));
 
  // Place each detector at its explicitly specified local position.
  for (const SExplicitOpDetPos& entry : fExplicitPositions) {
    const G4String pv_name =
        Form("%s_%i_%i_pv", fPhotoDetModel.data(), fID, entry.id);
 
    new G4PVPlacement(
        nullptr,          // inherit array rotation — no additional rotation
        entry.position,   // position in array-local frame
        opdet->GetModLV(),
        pv_name,
        fModLV,
        false,            // pMany
        entry.id,         // copy number
        true);            // check overlaps
  }
 
#ifdef SLAR_DEBUG
  printf("SLArDetOpDetArray::BuildOpDetArrayExplicit: "
         "placed %zu detector(s) in array %s (id %i)\n",
         fExplicitPositions.size(), fName.data(), fID);
#endif // SLAR_DEBUG

  fModLV->SetVisAttributes(G4VisAttributes(false));
}

std::pair<int, G4double> SLArDetOpDetArray::ComputeArrayTrueLength(
    G4double sc_dim, G4double spacing, G4double max_len) {
  G4double len = sc_dim;
  G4double len_tmp = len;
  G4int n_replica = 0; 
  while (len_tmp <= max_len) {
    len = len_tmp; 
    n_replica++;
    len_tmp += (spacing); 
  } 

  return std::make_pair(n_replica, fabs(len));
};

SLArCfgSuperCellArray SLArDetOpDetArray::BuildOpDetArrayCfg() {
  SLArCfgSuperCellArray arrayCfg("OpDet_array_" + std::to_string(fID), fID);
 
  arrayCfg.SetIdx(fID);
  arrayCfg.SetNormal(fNormal.x(), fNormal.y(), fNormal.z());
  arrayCfg.SetupAxes();
  arrayCfg.SetPhi  (fGeoInfo->GetGeoPar("opdetarray_phi"));
  arrayCfg.SetTheta(fGeoInfo->GetGeoPar("opdetarray_theta"));
  arrayCfg.SetPsi  (fGeoInfo->GetGeoPar("opdetarray_psi"));
 
  if (fPlacementMode == EPlacementMode::kExplicit) {
    FillCfgExplicit(arrayCfg);
  } else {
    FillCfgParameterised(arrayCfg);
  }
 
  return arrayCfg;
}

/*
 *SLArCfgSuperCellArray SLArDetOpDetArray::BuildOpDetArrayCfg() {
 *  SLArCfgSuperCellArray arrayCfg("OpDet_array_"+std::to_string(fID), fID); 
 *
 *  arrayCfg.SetIdx( fID ); 
 *  arrayCfg.SetNormal( fNormal.x(), fNormal.y(), fNormal.z() ); 
 *  arrayCfg.SetupAxes(); 
 *  arrayCfg.SetPhi  ( fGeoInfo->GetGeoPar("opdetarray_phi") ); 
 *  arrayCfg.SetTheta( fGeoInfo->GetGeoPar("opdetarray_theta") ); 
 *  arrayCfg.SetPsi  ( fGeoInfo->GetGeoPar("opdetarray_psi") ); 
 *
 *  auto sc_array = (G4PVParameterised*)fModLV->GetDaughter(0); 
 *  auto sc_row   = (G4PVParameterised*)fSubModules.front()->GetModLV()->GetDaughter(0);  
 *
 *  auto get_replication_data = [](G4PVParameterised* pv) {
 *    SLArPlaneParameterisation::PlaneReplicationData_t data; 
 *    pv->GetReplicationData(data.fReplicaAxis, data.fNreplica, 
 *        data.fWidth, data.fOffset, data.fConsuming); 
 *    auto parameterisation = (SLArPlaneParameterisation*)pv->GetParameterisation(); 
 *    data.fReplicaAxisVec = parameterisation->GetReplicationAxisVector(); 
 *    data.fStartingPos = parameterisation->GetStartPos(); 
 *    data.fWidth = parameterisation->GetSpacing(); 
 *    return data;
 *  };
 *
 *  auto rpl_sc_row = get_replication_data(sc_array); 
 *  auto rpl_sc_clm = get_replication_data(sc_row); 
 *  auto rot_inv = new G4RotationMatrix(*fRotation); 
 *  rot_inv->invert(); 
 *
 *  for (int i_sc_row = 0; i_sc_row<rpl_sc_row.fNreplica; i_sc_row++) {
 *    G4ThreeVector pos_sc_row = 
 *      rpl_sc_row.fStartingPos + rpl_sc_row.fWidth*i_sc_row*rpl_sc_row.fReplicaAxisVec;
 *
 *    for (int i_sc_clm = 0; i_sc_clm < rpl_sc_clm.fNreplica; i_sc_clm++) {
 *      G4int sc_id = (i_sc_row+1)*100 + i_sc_clm;
 *      G4String scName = Form("%s_%i_%i", 
 *          fPhotoDetModel.data(), arrayCfg.GetIdx(), sc_id); 
 *      SLArCfgSuperCell scCfg(sc_id);
 *      scCfg.SetName(scName);
 *
 *      G4ThreeVector sc_local_pos = pos_sc_row + 
 *        rpl_sc_clm.fStartingPos + rpl_sc_clm.fWidth*i_sc_clm*rpl_sc_clm.fReplicaAxisVec;
 *      scCfg.SetX(sc_local_pos.x()); 
 *      scCfg.SetY(sc_local_pos.y()); 
 *      scCfg.SetZ(sc_local_pos.z()); 
 *
 *      G4ThreeVector sc_abs_pos = fGlobalPosition + sc_local_pos.transform(*rot_inv); 
 *
 *      scCfg.SetPhysX( sc_abs_pos.x() ); 
 *      scCfg.SetPhysY( sc_abs_pos.y() ); 
 *      scCfg.SetPhysZ( sc_abs_pos.z() ); 
 *
 *      scCfg.SetPhi( arrayCfg.GetPhi() ); 
 *      scCfg.SetTheta( arrayCfg.GetTheta() ); 
 *      scCfg.SetPsi( arrayCfg.GetPsi() ); 
 *
 *      scCfg.SetNormal( arrayCfg.GetNormal() ); 
 *      scCfg.SetupAxes(); 
 *
 *      const auto scBox = (G4Box*)fOpDetModuleBase->GetModSV();
 *      scCfg.SetSize( 2*scBox->GetXHalfLength(),
 *                     2*scBox->GetYHalfLength(), 
 *                     2*scBox->GetZHalfLength() ); 
 *
 *      arrayCfg.RegisterElement( scCfg );
 *    }
 *  }
 *
 *  return arrayCfg;
 *}
 */


void SLArDetOpDetArray::FillCfgParameterised(SLArCfgSuperCellArray& arrayCfg) const {
  auto sc_array = static_cast<G4PVParameterised*>(fModLV->GetDaughter(0));
  auto sc_row   = static_cast<G4PVParameterised*>(
      fSubModules.front()->GetModLV()->GetDaughter(0));

  auto rpl_sc_row = get_plane_replication_data(sc_array);
  auto rpl_sc_clm = get_plane_replication_data(sc_row);

  auto rot_inv = new G4RotationMatrix(*fRotation);
  rot_inv->invert();

  for (int i_sc_row = 0; i_sc_row < rpl_sc_row.fNreplica; ++i_sc_row) {
    const G4ThreeVector pos_sc_row =
        rpl_sc_row.fStartingPos
        + rpl_sc_row.fWidth * i_sc_row * rpl_sc_row.fReplicaAxisVec;

    for (int i_sc_clm = 0; i_sc_clm < rpl_sc_clm.fNreplica; ++i_sc_clm) {
      const G4int sc_id = (i_sc_row + 1) * 100 + i_sc_clm;
      const G4String scName =
          Form("%s_%i_%i", fPhotoDetModel.data(), arrayCfg.GetIdx(), sc_id);

      SLArCfgSuperCell scCfg(sc_id);
      scCfg.SetName(scName);

      G4ThreeVector sc_local_pos =
          pos_sc_row
          + rpl_sc_clm.fStartingPos
          + rpl_sc_clm.fWidth * i_sc_clm * rpl_sc_clm.fReplicaAxisVec;

      scCfg.SetX(sc_local_pos.x());
      scCfg.SetY(sc_local_pos.y());
      scCfg.SetZ(sc_local_pos.z());
 
      G4ThreeVector sc_abs_pos = fGlobalPosition + sc_local_pos.transform(*rot_inv);
      scCfg.SetPhysX(sc_abs_pos.x());
      scCfg.SetPhysY(sc_abs_pos.y());
      scCfg.SetPhysZ(sc_abs_pos.z());
 
      scCfg.SetPhi  (arrayCfg.GetPhi());
      scCfg.SetTheta(arrayCfg.GetTheta());
      scCfg.SetPsi  (arrayCfg.GetPsi());
      scCfg.SetNormal(arrayCfg.GetNormal());
      scCfg.SetupAxes();

      const auto* scBox = static_cast<const G4Box*>(fOpDetModuleBase->GetModSV());
      scCfg.SetSize(2 * scBox->GetXHalfLength(),
                    2 * scBox->GetYHalfLength(),
                    2 * scBox->GetZHalfLength());

      arrayCfg.RegisterElement(scCfg);
    }
  }
}

 

void SLArDetOpDetArray::FillCfgExplicit(SLArCfgSuperCellArray& arrayCfg) const {
  if (!fOpDetModuleBase) {
    G4Exception("SLArDetOpDetArray::FillCfgExplicit", "ConfigError010",
        FatalException,
        "fOpDetModuleBase is null. Call BuildOpDetArray() before BuildOpDetArrayCfg().");
  }

  auto rot_inv = new G4RotationMatrix(*fRotation);
  rot_inv->invert();

  const auto* scBox = static_cast<const G4Box*>(fOpDetModuleBase->GetModSV());
  const G4double sx = 2 * scBox->GetXHalfLength();
  const G4double sy = 2 * scBox->GetYHalfLength();
  const G4double sz = 2 * scBox->GetZHalfLength();

  for (const SExplicitOpDetPos& entry : fExplicitPositions) {
    const G4String scName =
        Form("%s_%i_%i", fPhotoDetModel.data(), arrayCfg.GetIdx(), entry.id);

    SLArCfgSuperCell scCfg(entry.id);
    scCfg.SetName(scName);
 
    // Local position (in the array frame)
    scCfg.SetX(entry.position.x());
    scCfg.SetY(entry.position.y());
    scCfg.SetZ(entry.position.z());

    // Absolute (world-frame) position: rotate local → world, then offset by
    // the array's global anchor point
    G4ThreeVector local_copy = entry.position;
    const G4ThreeVector sc_abs_pos = fGlobalPosition + local_copy.transform(*rot_inv);
    scCfg.SetPhysX(sc_abs_pos.x());
    scCfg.SetPhysY(sc_abs_pos.y());
    scCfg.SetPhysZ(sc_abs_pos.z());

    // All detectors in one array share the same orientation and normal
    scCfg.SetPhi  (arrayCfg.GetPhi());
    scCfg.SetTheta(arrayCfg.GetTheta());
    scCfg.SetPsi  (arrayCfg.GetPsi());
    scCfg.SetNormal(arrayCfg.GetNormal());
    scCfg.SetupAxes();
    scCfg.SetSize(sx, sy, sz);

    arrayCfg.RegisterElement(scCfg);
  }
}
