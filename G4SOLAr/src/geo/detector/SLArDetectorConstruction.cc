/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArDetectorConstruction.cc
 * @created     Wed Nov 16, 2022 09:44:58 CET
 */

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

#include "action/SLArRunAction.hh"
#include "SLArAnalysisManager.hh"
#include "physics/SLArCrossSectionBiasing.hh"

#include "detector/SLArDetectorConstruction.hh"
#include "geo/SLArGeoUtils.hh"

#include "detector/SLArBaseDetModule.hh"
#include "detector/TPC/SLArDetTPC.hh"
#include "SensitiveDetectors/SLArLArSD.hh"
#include "SensitiveDetectors/SLArExtScorerSD.hh"

#include "detector/Anode/SLArDetReadoutTile.hh"
#include "SensitiveDetectors/SLArReadoutTileSD.hh"

#include "detector/SuperCell/SLArDetSuperCellArray.hh"
#include "SensitiveDetectors/SLArSuperCellSD.hh"

#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgBaseSystem.hh"
#include "config/SLArCfgReadoutTile.hh"
#include "config/SLArCfgMegaTile.hh"

#include "G4SubtractionSolid.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4PVParameterised.hh"

#include "G4SDManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VSensitiveDetector.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4RunManager.hh"
#include "G4GeometryManager.hh"
#include "G4PSTermination.hh"
#include "G4PSNofSecondary.hh"
#include "G4SDParticleFilter.hh"
#include "G4SDParticleWithEnergyFilter.hh"
#include "G4UnitsTable.hh"

#include <fstream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/**
 * @details Standard constructor of the SLArDetectorConstruction class, 
 * setting the geometry configuration file and the material description table
 *
 * @param geometry_cfg_file Geometry configuration file 
 * @param material_db_file Material description table
 */
SLArDetectorConstruction::SLArDetectorConstruction(
    G4String geometry_cfg_file, G4String material_db_file)
  : G4VUserDetectorConstruction(),
  fGeometryCfgFile(""), 
  fMaterialDBFile(""),
  fExpHall(nullptr),
  fSuperCell(nullptr),
  fWorldLog(nullptr), 
  fWorldPhys(nullptr) 
{ 
  fGeometryCfgFile = geometry_cfg_file; 
  fMaterialDBFile  = material_db_file; 
#ifdef SLAR_DEBUG
  printf("SLArDetectorConstruction Build with\ngeometry %s\nmaterials %s\n",
      fGeometryCfgFile.c_str(), fMaterialDBFile.c_str());
#endif
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArDetectorConstruction::~SLArDetectorConstruction(){
  G4cerr << "Deleting SLArDetectorConstruction... " << G4endl;
  for (G4int i=0; i<G4int(fVisAttributes.size()); ++i) 
  {
    delete fVisAttributes[i];
  }

  G4cerr << "SLArDetectorConstruction DONE" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/**
 * @details Detector initilization. 
 * Parse the geometry configuration of the world and of all the other 
 * detector components. Configuration of the cryostat and LAr target is 
 * passed to fTPC together with the materials table. 
 * The configuration of the readout system is treated by the InitPix 
 * and InitPDS functions. 
 */
void SLArDetectorConstruction::Init() {
#ifdef SLAR_DEBUG
  printf("SLArDetectorConstruction::Init\ngeometry: %s\nmaterials: %s\n", 
      fGeometryCfgFile.c_str(), fMaterialDBFile.c_str());
#endif
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  SLArAnaMgr->fAnaMsgr->AssignDetectorConstruction(this);

  // open geometry configuration file
  FILE* geo_cfg_file = std::fopen(fGeometryCfgFile, "r");
  if (geo_cfg_file == nullptr) {
    fprintf(stderr, "Unable to open geometry configuration file %s\n", 
        fGeometryCfgFile.c_str()); 
    exit( EXIT_FAILURE ); 
  }
  char readBuffer[65536];
  rapidjson::FileReadStream is(geo_cfg_file, readBuffer, sizeof(readBuffer));

  rapidjson::Document d;
  d.ParseStream<rapidjson::kParseCommentsFlag>(is);
  assert(d.IsObject());
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  // Parse world dimensions
  if (d.HasMember("World")) {
    const rapidjson::Value& wrld = d["World"]; 
    assert(wrld.HasMember("dimensions")); 
    fWorldGeoPars.ReadFromJSON(wrld["dimensions"].GetArray());
  } else {
    fWorldGeoPars.RegisterGeoPar("size_x", 5*CLHEP::m); 
    fWorldGeoPars.RegisterGeoPar("size_y", 8*CLHEP::m); 
    fWorldGeoPars.RegisterGeoPar("size_z",20*CLHEP::m); 
  }

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  // Parse carvern dimensions
  rapidjson::Value* jhall = nullptr;
  if (d.HasMember("ExperimentalHall")) {
    jhall = &d["ExperimentalHall"];
  } else if (d.HasMember("Cavern")) {
    jhall = &d["Cavern"];
  } else if (d.HasMember("ExpHall")) {
    jhall = &d["ExpHall"];
  } else {
    fprintf(stderr, "No cavern or experimental hall description found\n");
    exit( EXIT_FAILURE ); 
  }
  InitExpHall(*jhall);

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  // Parse shielding description
  if (d.HasMember("Shielding")) {
    G4cout << "SLArDetectorConstruction::Init Shielding" << G4endl;
    InitShielding(d["Shielding"]); 
  }

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Initialize TPC objects
  G4cout << "SLArDetectorConstruction::Init TPC" << G4endl;
  InitTPC(d["TPC"]); 
  InitCathode(d["Cathode"]);
  ConstructTarget(); 

  fCryostat = new SLArDetCryostat(); 
  fCryostat->SetGeoPar( "target_size_x", fDetector->GetGeoPar("det_size_x") ); 
  fCryostat->SetGeoPar( "target_size_y", fDetector->GetGeoPar("det_size_y") ); 
  fCryostat->SetGeoPar( "target_size_z", fDetector->GetGeoPar("det_size_z") ); 

  if (d.HasMember("Cryostat")) {
    fCryostat->BuildCryostatStructure(d["Cryostat"]);
    G4cout << "SLArDetectorConstruction::Init Cryostat DONE" << G4endl;
  }

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  // Initialize Photodetectors
  if (d.HasMember("SuperCell") && d.HasMember("PhotoDetectionSystem")) {
    G4cout << "SLArDetectorConstruction::Init SuperCells" << G4endl;
    InitSuperCell( d["SuperCell"].GetObj() ); 
    G4cout << "SLArDetectorConstruction::Init SuperCells DONE" << G4endl;
    InitPDS(d["PhotoDetectionSystem"]); 
    G4cout << "SLArDetectorConstruction::Init PDS DONE" << G4endl;
  }

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  // Initialize ReadoutTile
  if (d.HasMember("ReadoutTile") && d.HasMember("Anode")) {
    G4cout << "SLArDetectorConstruction::Init Pix..." << G4endl;
    InitReadoutTile(d["ReadoutTile"].GetObj()); 
    InitAnode(d["Anode"]); 
    G4cout << "SLArDetectorConstruction::Init Pix DONE" << G4endl;
  }

  std::fclose(geo_cfg_file);
}

void SLArDetectorConstruction::InitExpHall(const rapidjson::Value& jexp) {
  fExpHall = new SLArDetExpHall(); 
  fExpHall->Init(jexp); 
}

void SLArDetectorConstruction::InitShielding(const rapidjson::Value& jshield) {
  if (jshield.IsArray()) {
    for (const auto& shielding_wall : jshield.GetArray()) {
      fShielding.emplace_back( new SLArDetShielding() ); 
      auto& shield = fShielding.back();
      shield->Init(shielding_wall);
    }
  }
  else if (jshield.IsObject()) {
    fShielding.emplace_back( new SLArDetShielding() ); 
    auto& shield = fShielding.back();
    shield->Init(jshield);
  }
  else {
    G4Exception("SLArDetectorConstruction::InitShielding()",
        "InvalidGeometryConfig",
        FatalException,
        "Shielding configuration must be an object or an array of objects");
  }
  return;
}

void SLArDetectorConstruction::InitTPC(const rapidjson::Value& jtpc) {
  assert(jtpc.IsArray()); 

  //loop over TPC modules
  for (auto &tpc : jtpc.GetArray()) {
    SLArDetTPC* detTPC = new SLArDetTPC(); 
    detTPC->Init(tpc); 
    fTPC.insert(std::make_pair(detTPC->GetID(), detTPC)); 
  }

}

void SLArDetectorConstruction::InitCathode(const rapidjson::Value& jcathode) {
  assert(jcathode.IsArray()); 

  for (const auto &jcath : jcathode.GetArray()) {
    SLArDetCathode* detCathode = new SLArDetCathode(); 
    detCathode->Init(jcath); 
    fCathode.insert(std::make_pair(detCathode->GetID(), detCathode)); 
  }
} 

void SLArDetectorConstruction::InitSuperCell(const rapidjson::Value& jsupercell) {
  fSuperCell = new SLArDetSuperCell(); 
  assert(jsupercell.HasMember("dimensions")); 
  fSuperCell->GetGeoInfo()->ReadFromJSON(jsupercell["dimensions"].GetArray()); 
  return;
}

/**
 * @details Construct the fSuperCell object and parse the supercell 
 * geometry from the pds object described in the geometry configuration
 * file. 
 * After this first step, the method creates a SuperCell system configuration 
 * object and registers all the SuperCell arrays defined in the pds["modules"]
 * section and finally the configuration is registered by the analysis manager
 *
 * @param pds supercell system description
 */
void SLArDetectorConstruction::InitPDS(const rapidjson::Value& jconf) {

  assert(jconf.IsArray()); 

  for (const auto &jarray : jconf.GetArray()) {
    SLArDetSuperCellArray* detSCArray = new SLArDetSuperCellArray(); 
    detSCArray->Init(jarray); 
    fSCArray.insert( std::make_pair(detSCArray->GetID(), detSCArray) ); 
  }

  return;
}

/*
 *  printf("Building SuperCell configuration object...\n");
 *  SLArCfgSystemSuperCell* pdsCfg = new SLArCfgSystemSuperCell("PDSCfg"); 
 *  if (pds.HasMember("modules")) {
 *    printf("is modules an array?\n");
 *    assert(pds["modules"].IsArray());
 *    printf("yes\n"); 
 *    for (const auto &mdl : pds["modules"].GetArray()) {
 *      printf("[%i] Module %s\n", mdl["id"].GetInt(), mdl["name"].GetString());
 *
 *      SLArCfgSuperCellArray* sc_array = 
 *        new SLArCfgSuperCellArray(mdl["name"].GetString(), mdl["id"].GetInt());
 *      sc_array->SetIdx(mdl["id"].GetInt()); 
 *
 *      if (mdl.HasMember("positions")) {
 *        assert(mdl["positions"].IsArray());
 *        for (const auto &isc : mdl["positions"].GetArray()) {
 *          SLArCfgSuperCell* scCfg = new SLArCfgSuperCell(); 
 *          const char* cunit = isc["unit"].GetString(); 
 *          auto xyz = isc["xyz"].GetArray(); 
 *          auto rot = isc["rot"].GetArray(); 
 *
 *          scCfg->SetIdx(isc["copy"].GetInt()); 
 *
 *          scCfg->SetX(xyz[0].GetDouble()*G4UIcommand::ValueOf(cunit)); 
 *          scCfg->SetY(xyz[1].GetDouble()*G4UIcommand::ValueOf(cunit)); 
 *          scCfg->SetZ(xyz[2].GetDouble()*G4UIcommand::ValueOf(cunit)); 
 *
 *          scCfg->SetPhi  (rot[0].GetDouble()*TMath::DegToRad()); 
 *          scCfg->SetTheta(rot[1].GetDouble()*TMath::DegToRad()); 
 *          scCfg->SetPsi  (rot[2].GetDouble()*TMath::DegToRad()); 
 *
 *          scCfg->Set2DSize_X(fSuperCell->GetGeoPar("cell_z")); 
 *          scCfg->Set2DSize_Y(fSuperCell->GetGeoPar("cell_x")); 
 *
 *          if (isc.HasMember("norm")) {
 *            assert(isc["norm"].IsArray()); 
 *            scCfg->SetNormal(
 *                  isc["norm"].GetArray()[0].GetDouble(),
 *                  isc["norm"].GetArray()[1].GetDouble(),
 *                  isc["norm"].GetArray()[2].GetDouble()
 *                ); 
 *          }
 *
 *          sc_array->RegisterElement(scCfg); 
 *        }
 *      }
 *
 *      pdsCfg->RegisterElement(sc_array);
 *    }
 *  }
 *
 *
 *  SLArAnalysisManager::Instance()->LoadPDSCfg(pdsCfg);
 *
 *}
 *
 */
/**
 * @details Parse the description of the pixelated anode readout system. 
 * Build the fReadoutTile object, setup the anode readout configuration
 * according to the description provided in pixsys["modules"] and finally 
 * source the configuration to the analysis manager. 
 *
 * @param pixsys Pixelated anode readout description
 */
void SLArDetectorConstruction::InitReadoutTile(const rapidjson::Value& pixsys) {
  fReadoutTile = new SLArDetReadoutTile();

  assert(pixsys.HasMember("dimensions")); 
  assert(pixsys.HasMember("components")); 
  assert(pixsys.HasMember("unit_cell")); 

  fReadoutTile->GetGeoInfo()->ReadFromJSON(pixsys["dimensions"].GetArray()); 
  fReadoutTile->BuildComponentsDefinition(pixsys["components"]); 
  fReadoutTile->BuildUnitCellStructure(pixsys["unit_cell"]); 
  fReadoutTile->BuildMaterial(fMaterialDBFile);

  if (pixsys.HasMember("tile_assembly")) {
    assert(pixsys["tile_assembly"].IsArray()); 

    for (const auto &mtile : pixsys["tile_assembly"].GetArray()) {
      // Setup megatile
      SLArDetReadoutTileAssembly* megatile = new SLArDetReadoutTileAssembly(); 
      assert(mtile.HasMember("dimensions")); 
      megatile->GetGeoInfo()->ReadFromJSON(mtile["dimensions"].GetArray()); 
      megatile->BuildMaterial(fMaterialDBFile); 
      fReadoutMegaTile.insert(std::make_pair(mtile["name"].GetString(),megatile)); 
    } // end of Megatile models loop
  } // endif pixsys.HasMember("tile_assembly")
}

void SLArDetectorConstruction::InitAnode(const rapidjson::Value& jconf) {
  assert(jconf.IsArray()); 

  for (const auto &janode : jconf.GetArray()) {
    SLArDetAnodeAssembly* detAnode = new SLArDetAnodeAssembly(); 
    detAnode->Init(janode); 
    fAnodes.insert( std::make_pair(detAnode->GetID(), detAnode) ); 
  }

}

void SLArDetectorConstruction::ConstructTarget() {
  G4ThreeVector target_min   (0, 0, 0); 
  G4ThreeVector target_max   (0, 0, 0); 

  G4double eps = 1*CLHEP::mm;

  for (const auto &tpc_ : fTPC) {
    auto& tpc = tpc_.second; 
    G4ThreeVector local_center; 
    G4ThreeVector local_dim;  
    local_center.setX(tpc->GetGeoPar("tpc_pos_x")); 
    local_center.setY(tpc->GetGeoPar("tpc_pos_y")); 
    local_center.setZ(tpc->GetGeoPar("tpc_pos_z"));
    local_dim   .setX(tpc->GetGeoPar("tpc_x")); 
    local_dim   .setY(tpc->GetGeoPar("tpc_y")); 
    local_dim   .setZ(tpc->GetGeoPar("tpc_z"));

    G4ThreeVector local_min = local_center - 0.5*local_dim; 
    G4ThreeVector local_max = local_center + 0.5*local_dim; 

    G4cout << local_center << G4endl; 
    G4cout << local_min << G4endl; 
    G4cout << local_max << G4endl; 

    for (int idim = 0; idim <3; idim++) {
      if (local_min[idim] < target_min[idim]) target_min[idim] = local_min[idim]; 
      if (local_max[idim] > target_max[idim]) target_max[idim] = local_max[idim]; 

      //printf("target_max: [%g, %g, %g], target_min: [%g, %g, %g]\n", 
      //target_max[0], target_max[1], target_max[2], 
      //target_min[0], target_min[1], target_min[2]); 
    }
  }

  G4ThreeVector target_center = 0.5*(target_min + target_max); 
  G4double target_dim[3] = {0.}; 
  for (int idim = 0; idim < 3; idim++) {
    target_dim[idim] = target_max[idim] - target_min[idim] + 2*eps; 
  }

  fDetector = new SLArBaseDetModule(); 
  fDetector->SetGeoPar("det_pos_x", target_center.x()); 
  fDetector->SetGeoPar("det_pos_y", target_center.y()); 
  fDetector->SetGeoPar("det_pos_z", target_center.z()); 

  fDetector->SetGeoPar("det_size_x", target_dim[0]); 
  fDetector->SetGeoPar("det_size_y", target_dim[1]); 
  fDetector->SetGeoPar("det_size_z", target_dim[2]); 

  printf("LAr target: pos (%g, %g, %g) mm - size %g x %g x %g\n", 
      target_center.x(), target_center.y(), target_center.z(), 
      target_dim[0], target_dim[1], target_dim[2]); 

  fDetector->SetSolidVolume( new G4Box("target_lar_solid", 
        0.5*target_dim[0], 0.5*target_dim[1], 0.5*target_dim[2]) ); 
  SLArMaterial* matTarget = new SLArMaterial("LAr"); 
  matTarget->BuildMaterialFromDB(fMaterialDBFile); 
  fDetector->SetLogicVolume( new G4LogicalVolume(fDetector->GetModSV(), 
        matTarget->GetMaterial(), "target_lar_lv") ); 
  fDetector->GetModLV()->SetVisAttributes( G4Colour( 1, 0, 0 ) ); 

}

void SLArDetectorConstruction::ConstructCryostat() {
  fCryostat->BuildMaterials(fMaterialDBFile); 
  fCryostat->BuildCryostat(); 

  fCryostat->GetModPV("cryostat_pv", 0, 
      fDetector->GetModPV()->GetTranslation(), 
      fWorldLog, 0) ; 

  fCryostat->SetVisAttributes(); 
}

void SLArDetectorConstruction::ConstructCathode() {
  for (auto &cathode : fCathode) {
    cathode.second->BuildMaterial(fMaterialDBFile); 
    cathode.second->BuildCathode(); 
    auto geoinfo = cathode.second->GetGeoInfo(); 
    cathode.second->GetModPV(
        "cathode_pv_"+std::to_string(cathode.first), 0, 
        G4ThreeVector(
          geoinfo->GetGeoPar("pos_x"), 
          geoinfo->GetGeoPar("pos_y"), 
          geoinfo->GetGeoPar("pos_z")), 
        fDetector->GetModLV(), 0, cathode.first); 
  }
}

/**
 * @details Construct the world volume, build and place the 
 * SLArDetectorConstruction::fTPC object. 
 * After this first step, the method calls BuildAndPlaceSuperCells() and
 * BuildAndPlaceAnode() to place the SuperCell and the Readout Tile
 * detector system. 
 *
 * @return 
 */
G4VPhysicalVolume* SLArDetectorConstruction::Construct()
{
#ifdef SLAR_DEBUG
  printf("SLArDetectorConstruction::Construct\ngeometry: %s\nmaterials: %s\n", 
      fGeometryCfgFile.c_str(), fMaterialDBFile.c_str());
#endif
  Init();

  // ------------- Volumes --------------
  // 1. Build and place WORLD volume
  G4Box* expHall_box = new G4Box("World", 
      0.5*fWorldGeoPars.GetGeoPar("size_x"), 
      0.5*fWorldGeoPars.GetGeoPar("size_y"), 
      0.5*fWorldGeoPars.GetGeoPar("size_z"));  

  SLArMaterial* matWorld = new SLArMaterial("Air"); 
  matWorld->BuildMaterialFromDB(fMaterialDBFile); 

  fWorldLog   
    = new G4LogicalVolume(expHall_box, matWorld->GetMaterial(), "World",0,0,0);

  fWorldPhys
    = new G4PVPlacement(0,G4ThreeVector(),fWorldLog,"World",0,false,0);

  // 2. Build and place the Experimental Hall
  ConstructExperimentalHall();

  // 3. Build and place the detector's shielding (just in case)
  ConstructShielding();

  // Compute the position of the TPCs including cryostat dimensions
  const G4double target_size_y = fDetector->GetGeoPar("det_size_y"); 
  G4double cryostat_tk = fCryostat->GetGeoPar("cryostat_tk");
  G4double shielding_tk = 0.0; 
  for (const auto &shield : fShielding) {
    if (shield->GetFace() == geo::EBoxFace::kYminus) {
      shielding_tk = shield->GetGeoPar("shielding_thickness");
    }
  }

  if (fCryostat->HasSupportStructure()) cryostat_tk += fCryostat->GetGeoPar("waffle_total_width");

  // 4. Build and place the LAr target
  G4cout << "\nSLArDetectorConstruction: Building the Detector Volume" << G4endl;
  G4ThreeVector target_center( 
      fDetector->GetGeoPar("det_pos_x"), 
      fDetector->GetGeoPar("det_pos_y"), 
      fDetector->GetGeoPar("det_pos_z") );

  G4ThreeVector hall_center = fExpHall->GetBoxCenter();
  G4ThreeVector hall_halfsize = fExpHall->GetBoxHalfSize();

  G4double detector_floor_spacing = 0*CLHEP::cm;
  G4ThreeVector target_y_shift = G4ThreeVector(0, cryostat_tk + shielding_tk + 0.5*target_size_y - hall_halfsize.y() + detector_floor_spacing, 0);
  G4ThreeVector target_pos = hall_center + target_center + target_y_shift; 
  G4cout << "target_center: " << target_center << G4endl;
  G4cout << "target_halfsize: " << 0.5*target_size_y << G4endl;
  G4cout << "hall_center: " << hall_center << G4endl;
  G4cout << "cryostat thickness: " << cryostat_tk << G4endl; 
  G4cout << "shielding thickness: " << shielding_tk << G4endl;
  G4cout << "target_y: " << target_pos << G4endl;

  fDetector->SetModPV( new G4PVPlacement(
        0, 
        target_pos,
        fDetector->GetModLV(), "target_lar_pv", fWorldLog, 0, 9) ); 

  // 5. Build and place the Cryostat
  G4cout << "\nSLArDetectorConstruction: Building the Cryostat" << G4endl;
  fCryostat->SetWorldMaterial(matWorld); 
  ConstructCryostat(); 

  G4cout << "\nSLArDetectorConstruction: Building the Cathode" << G4endl;
  ConstructCathode(); 

  G4cout << "\nSLArDetectorConstruction: Building the TPCs" << G4endl;
  for (auto &tpc : fTPC) {
    tpc.second->BuildMaterial(fMaterialDBFile); 
    tpc.second->BuildTPC();
    tpc.second->GetModPV("TPC"+std::to_string(tpc.first), 0,
        tpc.second->GetTPCcenter(), 
        fDetector->GetModLV(), false, tpc.first);
    tpc.second->SetVisAttributes(); 
  }

  // 6. Build and place the "conventional" Photon Detection System 
  if (fSuperCell) BuildAndPlaceSuperCells();

  // 7. Build and place the "pixel-based" readout system 
  BuildAndPlaceAnode(); 

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  //Visualization attributes
  if (fSuperCell) fSuperCell->SetVisAttributes();

  G4VisAttributes* visAttributes = new G4VisAttributes();
  visAttributes->SetColor(0.25,0.54,0.79, 0.0);
  fWorldLog->SetVisAttributes(visAttributes);


  SLArAnalysisManager::Instance()->CreateEventStructure();

  //always return the physical World
  return fWorldPhys;
}

/**
 * @details Create Sensitive Detector objects for the readout systems 
 * (SLArDetectorConstruction::fReadoutTile, SLArDetectorConstruction::fSuperCell), 
 * for the LAr TPC active volume. The method then calls 
 * SLArDetectorConstruction::ConstructCryostatScorer() to set a fraction of the
 * cryostat wall as Sensitive Detectors to implement some simple scorer used for 
 * background shielding studies.
 */
void SLArDetectorConstruction::ConstructSDandField()
{
  // sensitive detectors 
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String SDname;

#ifndef SLAR_EXTERNAL
  auto anaMngr = SLArAnalysisManager::Instance(); 
  if (anaMngr->GetPhysicsBiasingMap().size() > 0) {
    for (const auto &biasing : anaMngr->GetPhysicsBiasingMap() ) {
      auto xsecBias = new SLArCrossSectionBiasing(biasing.first, "biasing_"+biasing.first); 
      xsecBias->SetBiasingFactor( biasing.second ); 
      for (const auto &tpc : fTPC) {
        xsecBias->AttachTo( tpc.second->GetModLV() ); 
      }
    }
  }

  //Set ReadoutTile SD
  if (fReadoutTile) {
    G4VSensitiveDetector* sipmSD
      = new SLArReadoutTileSD(SDname="/tile/sipm");
    SDman->AddNewDetector(sipmSD);
    SetSensitiveDetector(
        fReadoutTile->GetSiPMActive()->GetModLV(), sipmSD );
  }

  //Set SuperCell SD
  if (fSuperCell) {
    G4VSensitiveDetector* superCellSD
      = new SLArSuperCellSD(SDname="/supercell"); 
    SDman->AddNewDetector(superCellSD); 
    SetSensitiveDetector(
        fSuperCell->GetCoating()->GetModLV(), superCellSD );
  }

  // Set LAr-volume SD
  G4int iTPC = 0; 
  for (const auto tpc : fTPC) {
    auto tpcSD = 
      new SLArLArSD("/TPC/LArTPC"+std::to_string(tpc.first), tpc.first);
    SDman->AddNewDetector(tpcSD);
    SetSensitiveDetector(tpc.second->GetModLV(), tpcSD);
    iTPC++; 
  }

#endif

}

void SLArDetectorConstruction::AddExternalScorer(const G4String phys_volume_name, const G4String alias)
{
  // sensitive detectors 
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String SDname;
#ifdef SLAR_EXTERNAL
  auto external_scorer_pv = G4PhysicalVolumeStore::GetInstance()->GetVolume(phys_volume_name); 
  if (external_scorer_pv) {
    G4String ext_scorer_name = "/Ext/scorer/" + alias;
    printf("SLArDetectorConstruction::AddExternalScorer(): "); 
    printf("Making %s a scorer volume (%s)\n", phys_volume_name.data(), ext_scorer_name.data()); 

    const G4double surface = 
      geo::get_bounding_volume_surface(external_scorer_pv->GetLogicalVolume()->GetSolid());
    printf("external scorer surface: %g cm^2\n", surface/(CLHEP::cm2));

    auto ext_scorer_sd = 
      new SLArExtScorerSD(ext_scorer_name); 
    SDman->AddNewDetector(ext_scorer_sd); 
    SetSensitiveDetector(external_scorer_pv->GetLogicalVolume(), ext_scorer_sd); 
    auto runAction = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction();
    runAction->RegisterExtScorerLV( external_scorer_pv->GetLogicalVolume() ); 
    fExtScorerPV.push_back( external_scorer_pv ); 
  }
  else {
    printf("SLArDetectorConstruction::AddExternalScorer(): ERROR "); 
    printf("Unable to fond physical volume %s in physical volume store\n", phys_volume_name.data()); 
    throw std::invalid_argument("AddExternalScorer ERROR: cannot find %s in Physical Volume Store"); 
  }
#else
  G4cout << "SLArDetectorConstruction::AddExternalScorer WARNING: " << G4endl;
  G4cout << "This method is active only when solar_sim is compiled with SLAR_EXTERNAL option, and this is not the case." << G4endl;
#endif

  return;
}


SLArDetTPC* SLArDetectorConstruction::GetDetTPC(int copyid) 
{
  int count = fTPC.count(copyid); 
  if (!count) {
    printf("SLArDetectorConstruction::GetDetTPC(%i) ERROR:", copyid); 
    printf("Unable to find TPC with copy id %i\n", copyid); 
    return nullptr;
  }

  return fTPC.find(copyid)->second;
}

//SLArDetPMT* SLArDetectorConstruction::GetDetPMT(const char* mod) 
//{
//SLArDetPMT* pmt = nullptr;
//pmt = fPMTs.find(mod)->second;
//return pmt;
//}

//std::map<G4String,SLArDetPMT*>& SLArDetectorConstruction::GetDetPMTs()
//{
//return fPMTs;
//}


G4LogicalVolume* SLArDetectorConstruction::GetLogicWorld()
{
  return fWorldLog;
}

G4String SLArDetectorConstruction::GetFirstChar(G4String line)
{
  int length = line.length();
  G4String ch = "";
  int i = 0;
  while (isspace(line[i])) i++;

  if (i==length) return "#";
  else           return &line[i];
}


/**
 * @details Build the SLArDetectorConstruction::fSuperCell object and 
 * the photon detector's logical skin surface. 
 * Then place the individual SuperCell according to the configuration 
 * stored in the analysis manager. 
 */
void SLArDetectorConstruction::BuildAndPlaceSuperCells()
{
  fSuperCell->BuildMaterial(fMaterialDBFile);
  fSuperCell->BuildSuperCell();
  fSuperCell->BuildLogicalSkinSurface(); 

  // Get PMTSystem Configuration
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  SLArCfgSystemSuperCell&  pdsCfg = SLArAnaMgr->GetPDSCfg();

  printf("-- Building SuperCell arrays\n");
  for (auto &array_ : fSCArray) {
    auto scarray = array_.second; 
    auto scarray_id = array_.first; 
    scarray->BuildMaterial(fMaterialDBFile); 
    printf("---- Building SC array volume\n");
    scarray->BuildSuperCellArray( fSuperCell );
    auto pos = scarray->GetPosition(); 
    auto rot = scarray->GetRotation();

    auto tpc = fTPC.find(scarray->GetTPCID())->second; 
    auto glb_pos = tpc->GetTPCcenter() + pos; 
    scarray->SetGlobalPos( glb_pos ); 

    printf("---- Placing SC Array %i in TPC %i\n", scarray_id, tpc->GetID());
    scarray->GetModPV("pds_"+std::to_string(scarray_id), 
        rot, pos, tpc->GetModLV(), 0, scarray_id); 

    auto array_cfg = scarray->BuildSuperCellArrayCfg(); 
    array_cfg.SetX( pos.x() ); array_cfg.SetPhysX( glb_pos.x() );
    array_cfg.SetY( pos.y() ); array_cfg.SetPhysY( glb_pos.y() );
    array_cfg.SetZ( pos.z() ); array_cfg.SetPhysZ( glb_pos.z() );

    TH2Poly* h2 = array_cfg.BuildPolyBinHist(SLArCfgSuperCellArray::ESubModuleReferenceFrame::kWorld, true);

    delete h2;

    pdsCfg.RegisterElement( array_cfg ); 
  }


  SLArAnaMgr->LoadPDSCfg(pdsCfg); 
  G4cout << "SLArDetectorConstruction::BuildAndPlaceSuperCell DONE" << G4endl;
  return;
}

/**
 * @details Build the SLArDetectorConstruction::fReadoutTile object and 
 * construct the logical skin surface of the active photon detector volume.
 *
 */
void SLArDetectorConstruction::BuildAndPlaceAnode() {

  printf("SLArDetectorConstruction::BuildAndPlaceAnode()...\n");
  printf("-- Building readout tile\n");
  fReadoutTile->BuildReadoutTile(); 
  fReadoutTile->BuildLogicalSkinSurface(); 

  printf("-- Building readout tile assemblies\n");
  for (auto &mt : fReadoutMegaTile) {
    mt.second->BuildReadoutPlane(fReadoutTile); 
  }

  printf("-- Building anode assemblies\n");
  auto ana_mgr = SLArAnalysisManager::Instance();

  for (auto &anode_ : fAnodes) {
    auto anode = anode_.second; 
    auto anode_id = anode_.first; 
    anode->BuildMaterial(fMaterialDBFile); 
    printf("---- Building anode volume\n");
    anode->BuildAnodeAssembly( 
        fReadoutMegaTile.find(anode->GetTileAssemblyModel())->second );
    auto pos = anode->GetPosition(); 
    auto rot = anode->GetRotation();

    auto tpc = fTPC.find(anode->GetTPCID())->second; 
    auto glb_pos = fDetector->GetModPV()->GetTranslation() + tpc->GetTPCcenter() + pos; 

    printf("---- Placing Anode %i in TPC %i\n", anode_id, tpc->GetID());
    anode->GetModPV("anode"+std::to_string(anode_id), 
        rot, pos, tpc->GetModLV(), 0, anode_id); 

    auto anode_cfg = anode->BuildAnodeConfig(); 
    anode_cfg.SetX( pos.x() ); anode_cfg.SetPhysX( glb_pos.x() ); 
    anode_cfg.SetY( pos.y() ); anode_cfg.SetPhysY( glb_pos.y() ); 
    anode_cfg.SetZ( pos.z() ); anode_cfg.SetPhysZ( glb_pos.z() ); 
    ana_mgr->LoadAnodeCfg(anode_cfg); 
  }

  ConstructAnodeMap(); 
}

void SLArDetectorConstruction::SetAnodeVisAttributes(const int depth) {
  fReadoutTile->SetVisAttributes(depth); 
  for (auto& mt : fReadoutMegaTile) {
    mt.second->SetVisAttributes(depth);
  }

  return;
}

void SLArDetectorConstruction::ConstructAnodeMap() {
  printf("SLArDetectorConstruction::ConstructAnodeMap()\n");
  auto ana_mgr = SLArAnalysisManager::Instance(); 

  for (auto &anodeCfg_ : ana_mgr->GetAnodeCfg()) {
    auto& anodeCfg = anodeCfg_.second; 
    // access the first megatile to extract the map of the tiles 
    // (which is replicated for all the megatiles in the anode). 
    int megatile_nr = anodeCfg.GetMap().size(); 
    printf("%s has %i elements registered\n", anodeCfg.GetName(), megatile_nr); 
#ifdef SLAR_DEBUG
    anodeCfg.DumpMap(); 
#endif

    const size_t n_megatiles = anodeCfg.GetMap().size(); 
    if (n_megatiles == 0) {
      printf("SLArDetectorConstruction::ConstructAnodeMap WARNING: Anode %i has no megatiles registered.\n", 
          anodeCfg.GetIdx()); 
      return;
    }

    printf("getting front megatile\n"); 
    SLArCfgMegaTile& mtileCfg = anodeCfg.GetMap().front(); 

    printf("creating TH2Poly for a megatile...\n"); 
    auto hMapMegaTile = anodeCfg.BuildPolyBinHist(SLArCfgAnode::kWorld, true);
    printf("mapMegaTile\n");
    auto hMapTile     = mtileCfg.BuildPolyBinHist(
        SLArCfgAssembly<SLArCfgReadoutTile>::ESubModuleReferenceFrame::kRelative, 
        true); 
    printf("mapTile\n");
    G4RotationMatrix* mtile_rot = new G4RotationMatrix(
        mtileCfg.GetPhi(), 
        mtileCfg.GetTheta(), 
        mtileCfg.GetPsi());
    G4RotationMatrix* mtile_rot_inv = new G4RotationMatrix(*mtile_rot); 
    mtile_rot_inv->invert(); // FIXME: Why do I need to use the inverse rotation????? 

    auto hMapPixel = fReadoutTile->BuildTileChgPixelMap(
        G4ThreeVector(anodeCfg.GetAxis0().x(), anodeCfg.GetAxis0().y(), anodeCfg.GetAxis0().z()), 
        G4ThreeVector(anodeCfg.GetAxis1().x(), anodeCfg.GetAxis1().y(), anodeCfg.GetAxis1().z()), 
        nullptr, mtile_rot_inv);
    printf("mapPixel\n");

    anodeCfg.RegisterMap(0, hMapMegaTile); 
    anodeCfg.RegisterMap(1, hMapTile); 
    anodeCfg.RegisterMap(2, hMapPixel); 

    delete mtile_rot;
    delete mtile_rot_inv; 
  }

  printf("SLArDetectorConstruction::ConstructAnodeMap() DONE \n");
  return; 
}

G4VIStore* SLArDetectorConstruction::CreateImportanceStore() {

  printf("World volume ------------------------------------\n");
  G4IStore *istore = G4IStore::GetInstance();
  istore->SetWorldVolume(); 

  G4double imp =1;
  istore->AddImportanceGeometryCell(1, *fWorldPhys);
  printf("\nCavern ----------------------------------------\n");
  auto cavern_pv = fExpHall->GetModPV();
  printf("fCavern PV ptr: %p\n", static_cast<void*>(cavern_pv));
  istore->AddImportanceGeometryCell(
      1, *cavern_pv, cavern_pv->GetCopyNo()); 
  size_t n_cavern_layers = cavern_pv->GetLogicalVolume()->GetNoDaughters(); 
  for (int i = 0; i<n_cavern_layers; i++) {
    auto vol = cavern_pv->GetLogicalVolume()->GetDaughter(i);
    auto cell = G4GeometryCell(*vol, vol->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("Adding %s to istore with importance %g (rep nr. %i, %p)\n", 
          cell.GetPhysicalVolume().GetName().data(), 
          imp, cell.GetReplicaNumber(), static_cast<void*>(vol) );
      istore->AddImportanceGeometryCell(imp, cell); 
    }
  }

  printf("\nCryostat ----------------------------------------\n");
  printf("fCryostat PV ptr: %p\n", static_cast<void*>(fCryostat->GetModPV())); 
  istore->AddImportanceGeometryCell(
      1, *(fCryostat->GetModPV()), fCryostat->GetModPV()->GetCopyNo());

  printf("Support structure\n");
  for (const auto &face_ : fCryostat->GetCryostatSupportStructure() ) {
    auto face = face_.second;
    G4GeometryCell cell(*face->GetModPV(), face->GetModPV()->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("SUPPORT FACE: Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(),
          cell.GetReplicaNumber(), imp);              
      istore->AddImportanceGeometryCell(imp, cell); 
    }

    const auto pv = face->GetModLV()->GetDaughter(0); 
    const auto lv = pv->GetLogicalVolume();

    auto vol_row = (G4PVParameterised*)lv->GetDaughter(0);
    printf("vol_row: %s[%s]\n", 
        vol_row->GetName().data(), 
        vol_row->GetLogicalVolume()->GetName().data()); ;

    auto repl = get_plane_replication_data(static_cast<G4PVParameterised*>(vol_row)); 
    for (int i=0; i<repl.fNreplica; i++) {

      cell = G4GeometryCell(*vol_row, i); 

      if (istore->IsKnown(cell) == false) {
        printf("Adding %s to istore with importance %g (rep nr. %i, %p)\n", 
            vol_row->GetName().data(), imp, vol_row->GetCopyNo(), 
            static_cast<void*>(vol_row) );

        istore->AddImportanceGeometryCell(imp, cell); 
      }
    }



    auto vol_unit = vol_row->GetLogicalVolume()->GetDaughter(0); 
    auto row_repl = get_plane_replication_data(static_cast<G4PVParameterised*>(vol_unit)); 
    for (int iunit = 0; iunit<row_repl.fNreplica; iunit++) {
      cell = G4GeometryCell(*vol_unit, iunit); 
      if ( istore->IsKnown( cell ) == false) {
        printf("Adding %s [%s] to istore with importance %g (rep nr. %i, %p)\n", 
            vol_unit->GetName().data(), 
            vol_unit->GetLogicalVolume()->GetName().data(), 
            imp, iunit, static_cast<void*>(vol_unit));
        istore->AddImportanceGeometryCell(imp, cell); 
      }

    } // waffle row 

    for (int i=1; i<face->GetModLV()->GetNoDaughters(); i++) {
      auto pv_patch = face->GetModLV()->GetDaughter(i); 
      auto lv_patch = pv_patch->GetLogicalVolume(); 

      G4GeometryCell cell(*pv_patch, pv_patch->GetCopyNo()); 
      if (istore->IsKnown(cell) == false) {
        printf("PATCH MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
            cell.GetPhysicalVolume().GetName().data(), 
            cell.GetReplicaNumber(), imp);              
        istore->AddImportanceGeometryCell(imp, cell);  

        const auto ppv_patch = (G4PVParameterised*)lv_patch->GetDaughter(0); 

        if (ppv_patch->IsParameterised()) {
          const auto unit_lv = ppv_patch->GetLogicalVolume(); 
          const auto rpl = get_plane_replication_data(ppv_patch); 

          for (int k=0; k<rpl.fNreplica; k++) {
            cell = G4GeometryCell(*ppv_patch, k); 
            if (istore->IsKnown(cell) == false) {
              printf("PATCH MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
                  cell.GetPhysicalVolume().GetName().data(),
                  cell.GetReplicaNumber(), imp);              

              istore->AddImportanceGeometryCell(imp, cell);  

              for (int k=0; k<unit_lv->GetNoDaughters(); k++) {
                const auto component_pv = unit_lv->GetDaughter(k); 
                cell = G4GeometryCell(*component_pv, component_pv->GetCopyNo()); 
                if (istore->IsKnown(cell) == false) {
                  printf("PATCH UNIT: Adding %s (rp nr %i) to istore with importance %g\n",
                      cell.GetPhysicalVolume().GetName().data(),
                      cell.GetReplicaNumber(), imp);              

                  istore->AddImportanceGeometryCell(imp, cell);  
                }
              }
            }
          }
        }
      }
    } // end of patch
  } // end of waffle face 

  for (const auto edge : fCryostat->GetCryostatSupportStructureEdges()) {
    const auto edge_lv = edge->GetLogicalVolume(); 
    const auto edge_ppv = edge_lv->GetDaughter(0); 
    const auto edge_unit_lv = edge_ppv->GetLogicalVolume(); 

    auto cell = G4GeometryCell(*edge, edge->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("EDGE MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(),
          cell.GetReplicaNumber(), imp);              
      istore->AddImportanceGeometryCell(imp, cell); 

      const auto repl = get_plane_replication_data(static_cast<G4PVParameterised*>(edge_ppv)); 
      for (int k=0; k<repl.fNreplica; k++) {
        cell = G4GeometryCell(*edge_ppv, k); 
        if (istore->IsKnown(cell) == false) {
          printf("EDGE UNIT: Adding %s (rp nr %i) to istore with importance %g\n",
              cell.GetPhysicalVolume().GetName().data(),
              cell.GetReplicaNumber(), imp);              
          istore->AddImportanceGeometryCell(imp, cell); 
        }
      }
    }


  }

  printf("\nWaffle unit\n");
  const auto waffle = fCryostat->GetWaffleUnit(); 
  const auto waffle_lv = waffle->GetModLV(); 

  for (G4int k=0; k<waffle_lv->GetNoDaughters(); k++) {
    auto vol = waffle_lv->GetDaughter(k); 
    auto cell = G4GeometryCell(*vol, vol->GetCopyNo()); 
    if (istore->IsKnown(cell)==false) {
      printf("Adding %s to istore with importance %g (rep nr. %i, %p)\n", 
          cell.GetPhysicalVolume().GetName().data(), 
          imp, cell.GetReplicaNumber(), static_cast<void*>(vol) );
      istore->AddImportanceGeometryCell(imp, cell); 
    }
    getchar();
  }

  printf("\nWaffle edge unit\n");
  const auto edgeunit = fCryostat->GetWaffleCornerUnit(); 
  const auto edgeunit_lv = edgeunit->GetModLV(); 
  for (G4int k=0; k<edgeunit_lv->GetNoDaughters(); k++) {
    auto vol = edgeunit_lv->GetDaughter(k); 
    auto cell = G4GeometryCell(*vol, vol->GetCopyNo()); 
    if (istore->IsKnown(cell)==false) {
      printf("Adding %s to istore with importance %g (rep nr. %i, %p)\n", 
          cell.GetPhysicalVolume().GetName().data(), 
          imp, cell.GetReplicaNumber(), static_cast<void*>(vol) );
      istore->AddImportanceGeometryCell(imp, cell); 
    }
  }



  printf("\nCryostat layers\n");
  for (const auto &layer : fCryostat->GetCryostatStructure())
  {
    imp = layer.second.fImportance;
    const auto vol = layer.second.fModule->GetModPV();
    G4cout << "Going to assign importance: " << imp << ", to volume: " 
      << vol->GetName() << " rep nr: " << vol->GetCopyNo() << G4endl;
    istore->AddImportanceGeometryCell(imp, *vol, vol->GetCopyNo());
  }

  // the remaining part pf the geometry (rest) gets the same
  // importance as the innermost cryostat layer
  //
  printf("\nActive volume -----------------------------------\n");
  istore->AddImportanceGeometryCell(
      imp,*fDetector->GetModPV(), fDetector->GetModPV()->GetCopyNo());
  for (int i=0; i<fDetector->GetModLV()->GetNoDaughters(); i++) {
    auto vol = fDetector->GetModLV()->GetDaughter(i); 
    auto cell = G4GeometryCell(*vol, vol->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("Adding %s (replica nr %i) to istore with importance %g\n", 
          cell.GetPhysicalVolume().GetName().data(), cell.GetReplicaNumber(), imp);
      istore->AddImportanceGeometryCell(imp, cell); 
    }
  }

  printf("\nTPCs --------------------------------------------\n");
  for (const auto &tpc_ : fTPC) {
    auto tpc = tpc_.second;

    auto field_cage = tpc->GetFieldCage(); 
    if (field_cage) {
      auto field_cage_vol = field_cage->GetModLV()->GetDaughter(0); 

      auto cell = G4GeometryCell(*field_cage->GetModPV(), field_cage->GetModPV()->GetCopyNo()); 
      if (istore->IsKnown(cell) == false) {
        printf("Adding %s (replica nr %i) to istore with importance %g\n", 
            cell.GetPhysicalVolume().GetName().data(), cell.GetReplicaNumber(), imp);
        istore->AddImportanceGeometryCell(imp, cell); 
      }

      auto fc_repl = get_plane_replication_data((G4PVParameterised*)field_cage_vol); 
      for (int i=0; i<fc_repl.fNreplica; i++) {
        cell = G4GeometryCell(*field_cage_vol, i); 
        if (istore->IsKnown(cell) == false) {
          printf("Adding %s with Replica nr %i\n", 
              cell.GetPhysicalVolume().GetName().data(), i);

          istore->AddImportanceGeometryCell(imp, cell); 
        }
      }

      for (int i=0; i<field_cage_vol->GetLogicalVolume()->GetNoDaughters(); i++) {
        auto vol = field_cage_vol->GetLogicalVolume()->GetDaughter(i); 
        auto cell = G4GeometryCell(*vol, vol->GetCopyNo()); 
        if (istore->IsKnown(cell) == false) {
          printf("Adding %s (rp nr %i) to istore with importance %g\n", 
              vol->GetName().data(), vol->GetCopyNo(), imp);
          istore->AddImportanceGeometryCell(imp, cell); 
        }
      }
    }
    printf("TPC ID = %i\n", tpc->GetID());
    SLArDetAnodeAssembly* anode = nullptr; 
    for (const auto &a : fAnodes) {
      if (a.second->GetTPCID() == tpc->GetID()) {
        anode = a.second; 
        break;
      }
    }
    printf("\nAnode -----------------------------------------\n");
    auto anode_vol = anode->GetModLV()->GetDaughter(0); 
    auto mt_row = anode->GetTileAssemblyRow(); 
    auto mt_row_vol = mt_row->GetModLV()->GetDaughter(0); 
    for (int i=0; i<anode_vol->GetLogicalVolume()->GetNoDaughters(); i++) {
      const auto vol = (G4PVParameterised*)anode_vol->GetLogicalVolume()->GetDaughter(i);
      const auto plane_repl = get_plane_replication_data(vol); 
      for (int j=0; j<plane_repl.fNreplica; j++) {
        auto cell = G4GeometryCell(*vol, j); 
        if (istore->IsKnown(cell) == false) {
          printf("Adding %s (rp nr %i) to istore with importance %g\n",
              vol->GetName().data(), j, imp);
          istore->AddImportanceGeometryCell(imp, cell);
        }

        auto vol_mt = vol->GetLogicalVolume()->GetDaughter(0); 
        const auto mt_repl = get_plane_replication_data((G4PVParameterised*)vol); 
        for (int k=0; k<mt_repl.fNreplica; k++) {
          cell = G4GeometryCell(*vol_mt, k); 
          if (istore->IsKnown(cell) == false) {
            printf("MT MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
                cell.GetPhysicalVolume().GetName().data(), 
                cell.GetReplicaNumber(), imp);              
            istore->AddImportanceGeometryCell(imp, cell); 
          }
        }
      }
    }


  }

  printf("\nReadout Tile System -------------------------\n");
  const auto megatile_vol = (G4PVParameterised*)fReadoutMegaTile.begin()->second->GetModPV(); 
  const auto tile_row_vol = (G4PVParameterised*)megatile_vol->GetLogicalVolume()->GetDaughter(0); 
  const auto pcb_vol     = (G4PVParameterised*)tile_row_vol->GetLogicalVolume()->GetDaughter(0); 
  const auto base_vol  = (G4PVParameterised*)tile_row_vol->GetLogicalVolume()->GetDaughter(1); 
  const auto sensor_vol  = (G4PVParameterised*)tile_row_vol->GetLogicalVolume()->GetDaughter(2); 

  const auto tile_row_repl = get_plane_replication_data(megatile_vol); 
  const auto tile_repl     = get_plane_replication_data(tile_row_vol); 

  for (int i=0; i<tile_row_repl.fNreplica; i++) {
    auto cell = G4GeometryCell(*tile_row_vol, i); 
    if (istore->IsKnown(cell) == false) {
      printf("TILE ROW: Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(), 
          cell.GetReplicaNumber(), imp);              
      istore->AddImportanceGeometryCell(imp, cell); 
    }
  }

  for (int i=0; i<tile_repl.fNreplica; i++) {
    auto cell = G4GeometryCell(*pcb_vol, pcb_vol->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("TILE MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(), 
          cell.GetReplicaNumber(), imp);              
      istore->AddImportanceGeometryCell(imp, cell); 
    }
    cell = G4GeometryCell(*base_vol, base_vol->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("TILE MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(), 
          cell.GetReplicaNumber(), imp);              
      istore->AddImportanceGeometryCell(imp, cell); 
    }
    cell = G4GeometryCell(*sensor_vol, sensor_vol->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("TILE MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(), 
          cell.GetReplicaNumber(), imp);              
      istore->AddImportanceGeometryCell(imp, cell); 
    }
  }

  const auto sensor_plane_vol = sensor_vol->GetLogicalVolume()->GetDaughter(0); 
  const auto cell_row_vol = sensor_plane_vol->GetLogicalVolume()->GetDaughter(0); 
  const auto cell_row_repl = get_plane_replication_data( (G4PVParameterised*)sensor_plane_vol); 
  for (int i=0; i<cell_row_repl.fNreplica; i++) {
    auto cell = G4GeometryCell(*cell_row_vol, i); 
    if (istore->IsKnown(cell) == false) {
      printf("CELL ROW: Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(), 
          cell.GetReplicaNumber(), imp);              
      istore->AddImportanceGeometryCell(imp, cell); 
    }
  }


  const auto cell_repl = get_plane_replication_data((G4PVParameterised*)cell_row_vol); 
  for (int i=0; i<cell_row_repl.fNreplica; i++) {
    for (int j=0; j<cell_row_vol->GetLogicalVolume()->GetNoDaughters(); j++) {
      const auto vol = cell_row_vol->GetLogicalVolume()->GetDaughter(j); 
      auto cell = G4GeometryCell(*vol, i); 
      if (istore->IsKnown(cell) == false) {
        printf("CELL MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
            cell.GetPhysicalVolume().GetName().data(), 
            cell.GetReplicaNumber(), imp);              
        istore->AddImportanceGeometryCell(imp, cell); 
      }

      if (vol->GetLogicalVolume()->GetNoDaughters() > 0) {
        for (int n=0; n<vol->GetLogicalVolume()->GetNoDaughters(); n++) {
          const auto sub_vol = vol->GetLogicalVolume()->GetDaughter(n); 
          cell = G4GeometryCell(*sub_vol, sub_vol->GetCopyNo());
          if (istore->IsKnown(cell) == false) {
            printf("CELL SUBVOLUME: Adding %s (rp nr %i) to istore with importance %g\n",
                cell.GetPhysicalVolume().GetName().data(), 
                cell.GetReplicaNumber(), imp);              
            istore->AddImportanceGeometryCell(imp, cell); 
          }
        }

      }
    }

  }

  printf("\nPhoton Detection System -------------------------\n");
  for (const auto &pdsplane_ : fSCArray) {
    const auto pdsplane = pdsplane_.second; 
    printf("pdsplane name: %s - parameterised %i\n",
        pdsplane->GetModPV()->GetName().data(), 
        pdsplane->GetModPV()->IsParameterised());
    auto cell = G4GeometryCell(*pdsplane->GetModPV(), pdsplane->GetModPV()->GetCopyNo()); 
    if (istore->IsKnown(cell) == false) {
      printf("Adding %s (rp nr %i) to istore with importance %g\n",
          cell.GetPhysicalVolume().GetName().data(), cell.GetReplicaNumber(), imp);
      istore->AddImportanceGeometryCell(imp, cell); 
    }

    auto row_vol = (G4PVParameterised*)pdsplane->GetModLV()->GetDaughter(0); 
    printf("SC ROW: %s - replicated: %i\n", row_vol->GetName().data(), row_vol->IsParameterised());
    auto row_repl = get_plane_replication_data(row_vol); 
    for (int i=0; i<row_repl.fNreplica; i++) {
      cell = G4GeometryCell(*row_vol, i); 
      if (istore->IsKnown(cell) == false) {
        printf("SC ROW: Adding %s (rp nr %i) to istore with importance %g\n",
            cell.GetPhysicalVolume().GetName().data(), cell.GetReplicaNumber(), imp);
        istore->AddImportanceGeometryCell(imp, cell); 
      }

      auto sc_vol = row_vol->GetLogicalVolume()->GetDaughter(0); 
      printf("SC MODULE: %s - replicated: %i\n", 
          sc_vol->GetName().data(), sc_vol->IsReplicated());
      const auto sc_repl = get_plane_replication_data((G4PVParameterised*)sc_vol); 

      for (int j=0; j<sc_repl.fNreplica; j++) {
        cell = G4GeometryCell(*sc_vol, j); 
        if (istore->IsKnown(cell) == false) {
          printf("SC MODULE: Adding %s (rp nr %i) to istore with importance %g\n",
              cell.GetPhysicalVolume().GetName().data(), 
              cell.GetReplicaNumber(), imp); 
          istore->AddImportanceGeometryCell(imp, cell); 
        }
      }
    }

    for (int k=0; k<fSuperCell->GetModLV()->GetNoDaughters(); k++) {
      auto vol = fSuperCell->GetModLV()->GetDaughter(k); 
      cell = G4GeometryCell(*vol, vol->GetCopyNo()); 
      if (istore->IsKnown(cell) == false) {
        printf("SC OBJECT: Adding %s (rp nr %i) to istore with importance %g\n",
            cell.GetPhysicalVolume().GetName().data(), 
            cell.GetReplicaNumber(), imp); 
        istore->AddImportanceGeometryCell(imp, cell); 
      }
    }


  }

  return istore;
}

/**
 * @details Construct some scorers to evaluate the cryostat shielding performance. 
 * The method assigns a G4PSTermination scorer
 * to the borated polyethilene layers to count the nr of neutrons stopped in 
 * those volumes. Similarly, a G4PSFlatSurfaceCurrent is assigned to the cryostat
 * outer and inner walls to check the number of neutrons entering the cryostat and
 * the TPC active volume. 
 *
 * TODO: replace the hardcoded CopyIDs of the interested cryostat layers with 
 * a more flaxible solution. 
 */
void SLArDetectorConstruction::ConstructCryostatScorer() {
  // create particle filters
  G4SDParticleFilter* neutronFilter = new G4SDParticleFilter("neutronFilter"); 
  G4SDParticleFilter* gammaFilter = new G4SDParticleFilter("gammaFilter"); 
  G4SDParticleWithEnergyFilter* gammaEnergyFilter = 
    new G4SDParticleWithEnergyFilter("gamma", 0.5); 

  neutronFilter->add("neutron"); 
  gammaFilter->add("gamma"); 


  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  std::vector<G4MultiFunctionalDetector*> MFDetectors; 

  int ilayer = 0;
  for (const auto &layer_ : fCryostat->GetCryostatStructure()) {
    auto layer = layer_.second;
    auto mfd = new G4MultiFunctionalDetector("Cryostat/"+layer.fName); 

    // create scorers 
    G4PSTermination* captureCntsNeutrons = 
      new G4PSTermination("terminationNeutrons"+layer.fName); 
    captureCntsNeutrons->SetFilter(neutronFilter); 
    G4PSNofSecondary* secondaries = 
      new G4PSNofSecondary("secondaryGammas"+layer.fName); 
    secondaries->SetFilter(gammaEnergyFilter);
    G4PSTermination* captureCntsGamma = 
      new G4PSTermination("terminationGamma"+std::to_string(ilayer)); 

    captureCntsNeutrons->SetFilter(gammaFilter);  
    mfd->RegisterPrimitive(captureCntsGamma); 

    SDman->AddNewDetector( mfd );  

    SetSensitiveDetector(layer.fModule->GetModLV(), mfd); 
    ilayer++; 
  }

}

void SLArDetectorConstruction::ConstructExperimentalHall() {
  fExpHall->BuildMaterials(fMaterialDBFile); 
  fExpHall->BuildLayers( fWorldPhys ); 
  fExpHall->GetModPV("exp_hall_pv", nullptr, G4ThreeVector(0, 0, 0), fWorldLog, false, 88800);

  return;
}

void SLArDetectorConstruction::ConstructShielding() {
  if (fShielding.empty())  return;
  G4cout << "\nSLArDetectorConstruction: Building the Shielding" << G4endl;
  const G4ThreeVector hall_center = fExpHall->GetBoxCenter();
  const G4ThreeVector hall_halfsize = fExpHall->GetBoxHalfSize();
  const G4double target_dim_x = fDetector->GetGeoPar("det_size_x");
  const G4double target_dim_y = fDetector->GetGeoPar("det_size_y");
  const G4double target_dim_z = fDetector->GetGeoPar("det_size_z");
  const G4double target_pos_x = fDetector->GetGeoPar("det_pos_x");
  const G4double target_pos_y = fDetector->GetGeoPar("det_pos_y");
  const G4double target_pos_z = fDetector->GetGeoPar("det_pos_z");
  G4double cryostat_tk  = fCryostat->GetGeoPar("cryostat_tk");
  if (fCryostat->HasSupportStructure()) {
    cryostat_tk += fCryostat->GetGeoPar("waffle_total_width");
  }
  
  const G4ThreeVector target_pos(target_pos_x, target_pos_y, target_pos_z);
  const G4ThreeVector target_halfsize(0.5*target_dim_x, 0.5*target_dim_y, 0.5*target_dim_z);
  const G4ThreeVector cryostat_halfsize = target_halfsize + 
    G4ThreeVector( cryostat_tk, cryostat_tk, cryostat_tk );

  const G4ThreeVector gap_cavern_cryostat = hall_halfsize - (cryostat_halfsize + target_pos);

  for (auto& shield : fShielding) {
    const geo::EBoxFace face = shield->GetFace();
    const G4String face_name = geo::get_face_name( face ); 
    G4RotationMatrix* shield_rot = nullptr;

    G4ThreeVector shield_pos(0.0, 0.0, 0.0);
    G4double shield_thickness = shield->GetGeoPar("shielding_thickness");
    G4ThreeVector gap_shield_cryostat = gap_cavern_cryostat - 
      G4ThreeVector( shield_thickness, shield_thickness, shield_thickness );
    G4ThreeVector shield_halfsize(shield->GetGeoPar("dim_x")*0.5,
        shield_thickness*0.5, // this value is unused 
        shield->GetGeoPar("dim_z")*0.5);

    G4double gap = 0.0;

    switch (face) {
      case geo::EBoxFace::kYminus:
        shield_pos.set(0.0, 
            hall_center.y() - hall_halfsize.y() + shield_thickness*0.5,
            hall_center.z());
        break;
      case geo::EBoxFace::kYplus:
        shield_rot = new G4RotationMatrix();
        shield_rot->rotateZ(CLHEP::pi);
        gap = fabs( gap_shield_cryostat.y() ); 
        shield_pos.set(0.0, 
            hall_center.y() + hall_halfsize.y() - shield_thickness*0.5,
            hall_center.z());
        break;
      case geo::EBoxFace::kXminus:
        shield_rot = new G4RotationMatrix();
        shield_rot->rotateZ(-CLHEP::halfpi);
        gap = fabs( gap_shield_cryostat.x() );
        shield_pos.set(
            hall_center.x() - hall_halfsize.x() + (shield_thickness+gap)*0.5,
            hall_center.y() - hall_halfsize.y() + shield_halfsize.x(),
            hall_center.z());
        break;
      case geo::EBoxFace::kXplus:
        shield_rot = new G4RotationMatrix();
        shield_rot->rotateZ(CLHEP::halfpi);
        gap = fabs( gap_shield_cryostat.x() );
        shield_pos.set(
            hall_center.x() + hall_halfsize.x() - (shield_thickness+gap)*0.5,
            hall_center.y() - hall_halfsize.y() + shield_halfsize.x(),
            hall_center.z());
        break;
      case geo::EBoxFace::kZminus:
        shield_rot = new G4RotationMatrix();
        shield_rot->rotateX(-CLHEP::halfpi);
        gap = fabs( gap_shield_cryostat.z() );
        shield_pos.set(
            hall_center.x(),
             hall_center.y() - hall_halfsize.y() + shield_halfsize.z(),
            hall_center.z() - hall_halfsize.z() + (shield_thickness+gap)*0.5);
        break;
      case geo::EBoxFace::kZplus:
        shield_rot = new G4RotationMatrix();
        shield_rot->rotateX(CLHEP::halfpi);
        gap = fabs( gap_shield_cryostat.z() );
        shield_pos.set(
            hall_center.x(),
            hall_center.y() - hall_halfsize.y() + shield_halfsize.z(),
            hall_center.z() + hall_halfsize.z() - (shield_thickness+gap)*0.5);
        break;
      default:
        G4Exception("SLArDetectorConstruction::Construct()",
            "InvalidGeometryConfig", FatalException, 
            "Invalid shielding face orientation");
        break;
    }

    if (gap > 0.0) {
      shield->AddLayer("airgap", gap, "Air", 
          shield->GetShieldingLayers().end()->second.fImportance);
    }
    shield->SetMaterial( fWorldLog->GetMaterial() );
    shield->BuildMaterials(fMaterialDBFile);
    shield->BuildShielding();

    shield->GetModPV(face_name+"_shielding_pv", shield_rot, shield_pos, 
        fWorldLog, false, static_cast<int>(face) );
  }
}


