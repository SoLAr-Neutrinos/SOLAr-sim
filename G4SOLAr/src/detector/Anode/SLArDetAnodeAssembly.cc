/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetAnodeAssembly.cc
 * @created     Tue Mar 21, 2023 12:00:33 CET
 */

#include <detector/Anode/SLArDetAnodeAssembly.hh>
#include <detector/Anode/SLArDetReadoutTileAssembly.hh>
#include <detector/SLArPlaneParameterisation.hpp>
#include <detector/SLArGeoUtils.hh>

#include <config/SLArCfgAnode.hh>

#include <G4VisAttributes.hh>
#include <G4PVParameterised.hh>
#include <G4Box.hh>
#include <G4RotationMatrix.hh>

SLArDetAnodeAssembly::SLArDetAnodeAssembly() : 
  SLArBaseDetModule(), 
  fTPCID(0), 
  fMatAnode(nullptr), fAnodeRow(nullptr), 
  fPosition(0, 0, 0), fNormal(1, 0, 0),
  fRotation(0), fTileAssemblyModel("")
{
  fGeoInfo = new SLArGeoInfo();     
}

SLArDetAnodeAssembly::~SLArDetAnodeAssembly() {}

void SLArDetAnodeAssembly::BuildMaterial(G4String materials_db)
{
  fMatAnode = new SLArMaterial(); 
  fMatAnode->SetMaterialID("LAr");
  fMatAnode->BuildMaterialFromDB(materials_db);
}

void SLArDetAnodeAssembly::Init(const rapidjson::Value& jconf) {
  G4String suffix[3] = {"x", "y", "z"}; 

  assert(jconf.IsObject()); 
  auto janode = jconf.GetObject(); 

  assert(janode.HasMember("copyID")); 
  assert(janode.HasMember("position")); 
  assert(janode.HasMember("dimensions")); 
  assert(janode.HasMember("rot")); 
  assert(janode.HasMember("normal")); 
  assert(janode.HasMember("tile_assembly")); 
  assert(janode.HasMember("tpcID")); 

  SetID( janode["copyID"].GetInt() ); 

  fTileAssemblyModel = janode["tile_assembly"].GetString(); 

  fTPCID = janode["tpcID"].GetInt();

  auto jpos = janode["position"].GetObject(); 
  int idim = 0; 
  G4double vunit = unit::Unit2Val(jpos["unit"]); 
  for (const auto &v : jpos["xyz"].GetArray()) {
    fPosition[idim] = (v.GetDouble() * vunit); 
    fGeoInfo->RegisterGeoPar("pos_"+suffix[idim], fPosition[idim]); 
    idim++; 
  }

  auto jrot = janode["rot"].GetObject(); 
  vunit = unit::Unit2Val(jrot["unit"]); 
  assert(jrot.HasMember("val")); 
  assert(jrot["val"].IsArray()); 
  assert(jrot["val"].GetArray().Size() == 3);
  double eulerAngles[3] = {0.}; 
  idim = 0; 
  for (const auto &v : jrot["val"].GetArray()) {
     eulerAngles[idim] = v.GetDouble()*vunit; 
     idim++; 
  }
  fRotation = new G4RotationMatrix(eulerAngles[0], eulerAngles[1], eulerAngles[2]); 
  fGeoInfo->RegisterGeoPar("anode_phi", eulerAngles[0]); 
  fGeoInfo->RegisterGeoPar("anode_theta", eulerAngles[1]); 
  fGeoInfo->RegisterGeoPar("anode_psi", eulerAngles[2]); 
 
  auto jdim = janode["dimensions"].GetArray(); 
  fGeoInfo->ReadFromJSON(jdim); 

  idim = 0;
  for (const auto &v : janode["normal"].GetArray()) {
    fNormal[idim] = v.GetDouble(); 
    idim++; 
  }
  
}

void SLArDetAnodeAssembly::BuildAnodeAssembly(SLArDetReadoutTileAssembly* megatile) {
  G4Box* megatileBox = (G4Box*)megatile->GetModSV(); 
  if (!megatileBox) printf("MEGATILE BOX IS NULL!\n"); 
  G4double mt_x = 2*megatileBox->GetXHalfLength(); 
  G4double mt_y = 2*megatileBox->GetYHalfLength(); 
  G4double mt_z = 2*megatileBox->GetZHalfLength(); 

  G4double anode_x = fGeoInfo->GetGeoPar("dim_x"); 
  G4double anode_y = mt_y; 
  G4double anode_z = fGeoInfo->GetGeoPar("dim_z");

  G4int n_z = std::floor(anode_z / mt_z); 
  anode_z = n_z * mt_z; 
  G4int n_x = std::floor(anode_x / mt_x); 
  anode_x = n_x * mt_x; 

  fAnodeRow = new SLArBaseDetModule(); 
  fAnodeRow->SetSolidVolume(
        new G4Box("anode_row_sv", 0.5*mt_x, 0.5*mt_y, 0.5*anode_z));
  fAnodeRow->SetLogicVolume(
      new G4LogicalVolume(fAnodeRow->GetModSV(), fMatAnode->GetMaterial(),
        "anode_row_lv")); 
  fAnodeRow->GetModLV()->SetVisAttributes( G4VisAttributes(false) ); 

  SLArPlaneParameterisation* anodeRowParametrization =
      new SLArPlaneParameterisation(kZAxis, G4ThreeVector(0, 0, -0.5*mt_z*(n_z-1)), mt_z); 
 
  fAnodeRow->SetModPV(
      new G4PVParameterised("anode_row_pv", 
        megatile->GetModLV(), fAnodeRow->GetModLV(),kZAxis, n_z, 
        anodeRowParametrization, true)
      );

  fModSV = new G4Box("anode_sv", 0.5*anode_x, 0.5*anode_y, 0.5*anode_z); 
  fModLV = new G4LogicalVolume(fModSV, fMatAnode->GetMaterial(), "anode_lv"); 
  fModLV->SetVisAttributes( G4VisAttributes(false) ); 

  SLArPlaneParameterisation* anodeParameterization = 
      new SLArPlaneParameterisation(kXAxis, G4ThreeVector(-0.5*mt_x*(n_x-1), 0, 0), mt_x); 
  
  SetModPV(new G4PVParameterised("anode_pv", fAnodeRow->GetModLV(), fModLV,
        kXAxis, n_x, anodeParameterization, true)); 

}

SLArCfgAnode SLArDetAnodeAssembly::BuildAnodeConfig() {
  SLArCfgAnode anodeCfg("Anode_"+std::to_string(fID)); 
  anodeCfg.SetIdx( fID ); 
  anodeCfg.SetTPCID( fTPCID ); 
  anodeCfg.SetNormal( fNormal.x(), fNormal.y(), fNormal.z() ); 
  anodeCfg.SetupAxes(); 
  anodeCfg.SetPhi( fGeoInfo->GetGeoPar("anode_phi") ); 
  anodeCfg.SetTheta( fGeoInfo->GetGeoPar("anode_theta") ); 
  anodeCfg.SetPsi( fGeoInfo->GetGeoPar("anode_psi") ); 


  auto anode_parameterised = (G4PVParameterised*)fModLV->GetDaughter(0); 
  auto mtrow_parameterised = (G4PVParameterised*)fAnodeRow->GetModLV()->GetDaughter(0); 

  auto megatile_lv = fAnodeRow->GetModLV()->GetDaughter(0)->GetLogicalVolume(); 
  auto trow_lv = megatile_lv->GetDaughter(0)->GetLogicalVolume(); 
  auto tile_lv = trow_lv->GetDaughter(0)->GetLogicalVolume(); 
  
  auto mt_parameterised  = (G4PVParameterised*)megatile_lv->GetDaughter(0); 
  auto trow_parameterised  = (G4PVParameterised*)trow_lv->GetDaughter(0); 

  if (anode_parameterised->IsParameterised() == false) {
    printf("SLArDetAnodeAssembly::BuildAnodeConfig() "); 
    printf("Anode is not a parameterised volume! Quit.\n"); 
    throw std::runtime_error("SLArDetAnodeAssembly::BuildAnodeConfig() ERROR: Anode is not a parameterized volume.\n"); 
  }

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

  auto rpl_mt_row = get_replication_data(anode_parameterised); 
  auto rpl_mt_clm = get_replication_data(mtrow_parameterised); 
  auto rpl_t_row  = get_replication_data(mt_parameterised); 
  auto rpl_t_clm  = get_replication_data(trow_parameterised); 

  auto rot_inv = new G4RotationMatrix(*fRotation); 
  rot_inv->invert(); 

  for (int i_mt_row = 0; i_mt_row < rpl_mt_row.fNreplica; i_mt_row++) {
    G4ThreeVector pos_mt_row  = 
      rpl_mt_row.fStartingPos + rpl_mt_row.fWidth*(i_mt_row)*rpl_mt_row.fReplicaAxisVec;

    for (int i_mt_clm = 0; i_mt_clm < rpl_mt_clm.fNreplica; i_mt_clm++) {
      G4int mt_id = (i_mt_row+1)*1000 + i_mt_clm;
      G4String mtName = Form("%s_%i_%i", 
          fTileAssemblyModel.data(), anodeCfg.GetID(), mt_id); 
      SLArCfgMegaTile mtCfg(mtName, mt_id); 

      G4ThreeVector mt_local_pos = pos_mt_row + 
        rpl_mt_clm.fStartingPos + rpl_mt_clm.fWidth*(i_mt_clm)*rpl_mt_clm.fReplicaAxisVec;
      mtCfg.SetX(mt_local_pos.x()); 
      mtCfg.SetY(mt_local_pos.y()); 
      mtCfg.SetZ(mt_local_pos.z()); 

      G4ThreeVector mt_abs_pos = fPosition + mt_local_pos.transform(*rot_inv); 
      mtCfg.SetPhysX( mt_abs_pos.x() ); 
      mtCfg.SetPhysY( mt_abs_pos.y() ); 
      mtCfg.SetPhysZ( mt_abs_pos.z() ); 

      mtCfg.SetPhi( anodeCfg.GetPhi() ); 
      mtCfg.SetTheta( anodeCfg.GetTheta() ); 
      mtCfg.SetPsi( anodeCfg.GetPsi() ); 

      mtCfg.SetNormal( anodeCfg.GetNormal() ); 
      mtCfg.SetupAxes(); 
      
      mtCfg.SetSize( 
          2*((G4Box*)megatile_lv->GetSolid())->GetXHalfLength(),
          2*((G4Box*)megatile_lv->GetSolid())->GetYHalfLength(),
          2*((G4Box*)megatile_lv->GetSolid())->GetZHalfLength() ); 
      

      //printf("megatile %i: local (%.2f, %.2f, %.2f) - abs (%.2f, %.2f, %.2f)\n", 
          //mtCfg.GetIdx(), mtCfg.GetX(), mtCfg.GetY(), mtCfg.GetZ(), 
          //mtCfg.GetPhysX(), mtCfg.GetPhysY(), mtCfg.GetPhysZ());

      for (int i_t_row = 0; i_t_row < rpl_t_row.fNreplica; i_t_row++) {
        G4ThreeVector pos_t_row = 
          rpl_t_row.fStartingPos + rpl_t_row.fWidth*(i_t_row)*rpl_t_row.fReplicaAxisVec;
        printf("pos_t_row: %.2f, %.2f, %.2f\n", pos_t_row.x(), pos_t_row.y(), pos_t_row.z()); 
        for (int i_t_clm = 0; i_t_clm < rpl_t_clm.fNreplica; i_t_clm++) {
          G4ThreeVector t_pos = 
            rpl_t_clm.fStartingPos + rpl_t_clm.fWidth*(i_t_clm)*rpl_t_clm.fReplicaAxisVec;
          G4ThreeVector t_local_pos = pos_t_row + 
            rpl_t_clm.fStartingPos + rpl_t_clm.fWidth*(i_t_clm)*rpl_t_clm.fReplicaAxisVec;
          //printf("\tt_pos: %.2f, %.2f, %.2f\n", t_pos.x(), t_pos.y(), t_pos.z()); 
          //printf("\tt_local_pos: %.2f, %.2f, %.2f\n", t_local_pos.x(), t_local_pos.y(), t_local_pos.z()); 

          SLArCfgReadoutTile tileCfg( 100*(i_t_row+1) + i_t_clm ); 
          G4String tileName = Form("ReadoutTile_%i_%i_%i", anodeCfg.GetID(), 
              mtCfg.GetID(), tileCfg.GetID()); 
          tileCfg.SetName( tileName.data() ); 
          //printf("tile name: %s\n", tileCfg->GetName()); 

          tileCfg.SetPhi( anodeCfg.GetPhi() ); 
          tileCfg.SetTheta( anodeCfg.GetTheta() ); 
          tileCfg.SetPsi( anodeCfg.GetPsi() ); 

          tileCfg.SetX( t_local_pos.x() ); 
          tileCfg.SetY( t_local_pos.y() ); 
          tileCfg.SetZ( t_local_pos.z() ); 
          
          G4ThreeVector t_abs_pos = mt_abs_pos + t_local_pos.transform(*rot_inv); 

          tileCfg.SetPhysX( t_abs_pos.x() ); 
          tileCfg.SetPhysY( t_abs_pos.y() ); 
          tileCfg.SetPhysZ( t_abs_pos.z() ); 

          tileCfg.SetNormal( mtCfg.GetNormal() ); 
          tileCfg.SetupAxes(); 

          tileCfg.SetSize( 
              2*((G4Box*)tile_lv->GetSolid())->GetXHalfLength(),
              2*((G4Box*)tile_lv->GetSolid())->GetYHalfLength(),
              2*((G4Box*)tile_lv->GetSolid())->GetZHalfLength() ); 

          mtCfg.RegisterElement( tileCfg ); 
        }
      }

      auto h2 = mtCfg.BuildPolyBinHist(SLArCfgAssembly<SLArCfgReadoutTile>::ESubModuleReferenceFrame::kRelative);
      delete h2;

      //for (const auto& t : mtCfg.GetConstMap()) {
        //printf("Tile %i - pos: %.2f, %.2f, %.2f\n", t.GetIdx(), 
            //t.GetX(), t.GetY(), t.GetZ());
      //}
      //getchar();

      anodeCfg.RegisterElement( mtCfg ); 
    }

    anodeCfg.SetSize(
        2*((G4Box*)fModSV)->GetXHalfLength(),
        2*((G4Box*)fModSV)->GetYHalfLength(),
        2*((G4Box*)fModSV)->GetZHalfLength() ); 
  }

  printf("%s has %lu elements registered\n", 
      anodeCfg.GetName(), anodeCfg.GetMap().size());

  return anodeCfg; 
}


