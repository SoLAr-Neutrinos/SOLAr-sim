/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetSuperCellArray.cc
 * @created     Fri Mar 24, 2023 16:08:29 CET
 */

#include "detector/SuperCell/SLArDetSuperCellArray.hh"
#include "config/SLArCfgSuperCellArray.hh"
#include "config/SLArCfgSuperCell.hh"
#include "G4PVParameterised.hh"
#include "G4Box.hh"
#include "G4VisAttributes.hh"

SLArDetSuperCellArray::SLArDetSuperCellArray() : 
  SLArBaseDetModule(), 
  fTPCID(0), 
  fMaterialBase(nullptr), fSuperCell(nullptr),  
  fPosition(0, 0, 0), fGlobalPosition(0, 0, 0), fNormal(0, 1, 0),
  fRotation(nullptr), fPhotoDetModel("")
{
  fGeoInfo = new SLArGeoInfo(); 
}

SLArDetSuperCellArray::~SLArDetSuperCellArray() {}

void SLArDetSuperCellArray::BuildMaterial(G4String materials_db) {
  fMaterialBase = new SLArMaterial("LAr"); 
  fMaterialBase->BuildMaterialFromDB(materials_db); 
  return;
}

void SLArDetSuperCellArray::Init(const rapidjson::Value& jconf) {
  G4String xyz_suffix[3] = {"x", "y", "z"}; 
  G4String ang_suffix[3] = {"phi", "theta", "psi"}; 

  assert(jconf.IsObject()); 
  auto jarray = jconf.GetObject(); 

  assert(jarrry.HasMember("name"));
  assert(jarray.HasMember("copyID")); 
  assert(jarray.HasMember("position")); 
  assert(jarray.HasMember("dimensions")); 
  assert(jarray.HasMember("rot")); 
  assert(jarray.HasMember("normal")); 
  assert(jarray.HasMember("tpcID")); 
  assert(jarray.HasMember("photodet_model"));

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
     fGeoInfo->RegisterGeoPar("scarray_"+ang_suffix[idim], eulerAngles[idim]); 
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

  if (jarray.HasMember("replication_data")) {
    if (jarray["replication_data"].IsObject()) {
      auto parameterisation = new SLArPlaneParameterisation(jarray["replication_data"]); 
      fParameterisation.push_back(parameterisation); 
    } else if (jarray["replication_data"].IsArray()) {
      for (const auto &rdata : jarray["replication_data"].GetArray()) {
        auto parameterisation = new SLArPlaneParameterisation(rdata); 
        fParameterisation.push_back(parameterisation);         
      }
    } else {
      printf("WARNING: CANNOT PARSE SUPERCELL ARRAY REPLICATION DATA\n");
    }
  }
  
  return; 
}

void SLArDetSuperCellArray::BuildSuperCellArray(SLArDetSuperCell* superCell) {
  G4ThreeVector max_dim; 
  fSuperCell = superCell;
  max_dim[0] = fGeoInfo->GetGeoPar("dim_x");
  max_dim[1] = superCell->GetTotalHeight();
  max_dim[2] = fGeoInfo->GetGeoPar("dim_z");

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
    printf("tmp_dim: %g, %g, %g\n", tmp_dim[0], tmp_dim[1], tmp_dim[2]);
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
      target_prefix = "SC_row";
    } 
    else if (rpars == fParameterisation.front()) {
      fSubModules.push_back( new SLArBaseDetModule() ); 
      target = fSubModules.back();
      origin = superCell;
      target_prefix = "SC_module";
    }
    else {
      G4cout << "SLArDetSuperCellArray::BuildSuperCellArray() WARNING: " << G4endl;
      G4cout << "I should not be here!" << G4endl;
      fSubModules.push_back( new SLArBaseDetModule() ); 
      target = fSubModules.back(); 
      origin = fSubModules.rbegin()[1];
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

std::pair<int, G4double> SLArDetSuperCellArray::ComputeArrayTrueLength(
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

SLArCfgSuperCellArray SLArDetSuperCellArray::BuildSuperCellArrayCfg() {
  SLArCfgSuperCellArray arrayCfg("SC_array_"+std::to_string(fID), fID); 

  arrayCfg.SetIdx( fID ); 
  arrayCfg.SetNormal( fNormal.x(), fNormal.y(), fNormal.z() ); 
  arrayCfg.SetupAxes(); 
  arrayCfg.SetPhi  ( fGeoInfo->GetGeoPar("scarray_phi") ); 
  arrayCfg.SetTheta( fGeoInfo->GetGeoPar("scarray_theta") ); 
  arrayCfg.SetPsi  ( fGeoInfo->GetGeoPar("scarray_psi") ); 

  auto sc_array = (G4PVParameterised*)fModLV->GetDaughter(0); 
  auto sc_row   = (G4PVParameterised*)fSubModules.front()->GetModLV()->GetDaughter(0);  

  auto get_replication_data = [](G4PVParameterised* pv) {
    SLArPlaneParameterisation::PlaneReplicationData_t data; 
    pv->GetReplicationData(data.fReplicaAxis, data.fNreplica, 
        data.fWidth, data.fOffset, data.fConsuming); 
    auto parameterisation = (SLArPlaneParameterisation*)pv->GetParameterisation(); 
    data.fReplicaAxisVec = parameterisation->GetReplicationAxisVector(); 
    data.fStartingPos = parameterisation->GetStartPos(); 
    data.fWidth = parameterisation->GetSpacing(); 
    return data;
  };

  auto rpl_sc_row = get_replication_data(sc_array); 
  auto rpl_sc_clm = get_replication_data(sc_row); 
  auto rot_inv = new G4RotationMatrix(*fRotation); 
  rot_inv->invert(); 

  for (int i_sc_row = 0; i_sc_row<rpl_sc_row.fNreplica; i_sc_row++) {
    G4ThreeVector pos_sc_row = 
      rpl_sc_row.fStartingPos + rpl_sc_row.fWidth*i_sc_row*rpl_sc_row.fReplicaAxisVec;

    for (int i_sc_clm = 0; i_sc_clm < rpl_sc_clm.fNreplica; i_sc_clm++) {
      G4int sc_id = (i_sc_row+1)*100 + i_sc_clm;
      G4String scName = Form("%s_%i_%i", 
          fPhotoDetModel.data(), arrayCfg.GetIdx(), sc_id); 
      SLArCfgSuperCell scCfg(sc_id);
      scCfg.SetName(scName);

      G4ThreeVector sc_local_pos = pos_sc_row + 
        rpl_sc_clm.fStartingPos + rpl_sc_clm.fWidth*i_sc_clm*rpl_sc_clm.fReplicaAxisVec;
      scCfg.SetX(sc_local_pos.x()); 
      scCfg.SetY(sc_local_pos.y()); 
      scCfg.SetZ(sc_local_pos.z()); 

      G4ThreeVector sc_abs_pos = fGlobalPosition + sc_local_pos.transform(*rot_inv); 

      scCfg.SetPhysX( sc_abs_pos.x() ); 
      scCfg.SetPhysY( sc_abs_pos.y() ); 
      scCfg.SetPhysZ( sc_abs_pos.z() ); 

      scCfg.SetPhi( arrayCfg.GetPhi() ); 
      scCfg.SetTheta( arrayCfg.GetTheta() ); 
      scCfg.SetPsi( arrayCfg.GetPsi() ); 

      scCfg.SetNormal( arrayCfg.GetNormal() ); 
      scCfg.SetupAxes(); 

      const auto scBox = (G4Box*)fSuperCell->GetModSV();
      scCfg.SetSize( 2*scBox->GetXHalfLength(),
                     2*scBox->GetYHalfLength(), 
                     2*scBox->GetZHalfLength() ); 

      arrayCfg.RegisterElement( scCfg );
    }
  }

  return arrayCfg;
}
