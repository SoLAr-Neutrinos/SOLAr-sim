/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFLSPhotonLibrary.cc
 * @created     : Sunday Dec 14, 2025 15:36:06 CET
 */

#include <set>

#include "SLArDebugUtils.hh"
#include "physics/SLArFLSPhotonLibrary.hh"
#include "analysis/SLArAnalysisManager.hh"
#include "event/SLArEventAnode.hh"
#include "action/SLArRunAction.hh"
#include "geo/detector/SLArDetectorConstruction.hh"
#include "geo/SLArUnit.hpp"

#include "G4ParticleDefinition.hh"
#include "G4RunManager.hh"
#include "G4Poisson.hh"

#include "TPRegexp.h"
#include "TObjArray.h"
#include "TObjString.h"

#include "rapidjson/document.h"

/**
 * Source the configuration file and initialize the Photon Library simulator
 * by associating the specified branch address to the appropriate data structure
 * variables.
 *
 * The configuration file is expected to be in JSON format and contain the following fields: 
 *
 * | Field | Required/Opt   | Type | Description |
 * |-------|----------------|------|-------------|
 * | `root_file_obj` | Required | Object | Object containing the ROOT file information: `filename` (string, path to the ROOT file containing the photon library) and `objname` (string, name of the TTree object in the ROOT file containing the photon library data) |
 * | `material` | Required | String | Name of the material for which the photon library is defined (used for time of flight simulation) |
 * | `time` | Optional | Object | Configuration for the time of flight simulation (see `SLArFLSTimeOfFlightSim::Initialize` for details) |
 * | `voxel_size` | Optional | Array of 3 numbers or Object | Size of the voxels in the photon library along each axis (in mm). Can be specified as an array of 3 numbers (e.g. `[10, 10, 10]`) or as an object with a `val` field containing the array and a unit field (e.g. `{ "val": [1, 1, 1], "unit": "cm" }`) |
 * | `shift` | Optional | Array of 3 numbers or Object | Shift to be applied to the emission point coordinates before retrieving the corresponding voxel from the photon library (in mm). Can be specified as an array of 3 numbers (e.g. `[0, 0, 0]`) or as an object with a `val` field containing the array and a unit field (e.g. `{ "val": [0, 0, 0], "unit": "cm" }`) |
 * | `n_voxels` | Optional | Array of 3 integers | Number of voxels along each axis in the photon library. If not specified, the number of voxels will be inferred from the data in the ROOT file |
 * | `tile_array_branches` | Optional | Array of Objects | Array of objects specifying the configuration for tile array branches. Each object should contain the following fields: `name` (string, name of the branch in the photon library TTree), `size` (integer, size of the visibility buffer for this branch), `enabled` (boolean, whether to enable this branch or not), `pde_scale_factor` (float, optional, scale factor to be applied to the PDE when simulating detected photons for this branch), `pds_module` (string, optional, name of the PDS module associated with this branch, used to determine the target data structure for registering detected photons), and `n_elements` (array of integers, required if `pds_module` is specified and corresponds to an anode, specifies the number of components along each axis for the anode geometry) |
 * | `sipm_array_branches` | Optional | Array of Objects | Array of objects specifying the configuration for SiPM array branches. Each object should contain the following fields: `name` (string, name of the branch in the photon library TTree), `size` (integer, size of the visibility buffer for this branch), `enabled` (boolean, whether to enable this branch or not), `pde_scale_factor` (float, optional, scale factor to be applied to the PDE when simulating detected photons for this branch), `pds_module` (string, optional, name of the PDS module associated with this branch, used to determine the target data structure for registering detected photons), and `n_elements` (array of integers, required if `pds_module` is specified and corresponds to an anode, specifies the number of components along each axis for the anode geometry) |
 *
 * @param config configuration file path
 */
void SLArFLSPhotonLibrary::Initialize(const rapidjson::Value& config) 
{
  const auto& doc = config.GetObject();

  debug::require_json_member(doc, "root_file_obj");
  const auto& root_file_obj = doc["root_file_obj"];
  debug::require_json_member(root_file_obj, "filename");
  debug::require_json_member(root_file_obj, "objname");
  debug::require_json_member(doc, "material");

  fTimeSim.SetMaterial(doc["material"].GetString());
  if (doc.HasMember("time")) {
    fTimeSim.Initialize(doc["time"]);
  }

  if ( doc.HasMember("voxel_size") ) {
    const auto& jvoxel_size = doc["voxel_size"];
    if ( jvoxel_size.IsArray() && jvoxel_size.Size() == 3 ) {
      fVoxelSize[0] = jvoxel_size[0].GetDouble();
      fVoxelSize[1] = jvoxel_size[1].GetDouble();
      fVoxelSize[2] = jvoxel_size[2].GetDouble();
    }
    else if ( jvoxel_size.IsObject() ) {
      G4double vunit = unit::GetJSONunit(jvoxel_size);
      debug::require_json_member(jvoxel_size, "val");
      const auto& jval = jvoxel_size["val"];
      debug::require_json_type(jval, rapidjson::kArrayType);
      for ( rapidjson::SizeType i=0; i<jval.Size(); ++i ) {
        fVoxelSize[i] = jval[i].GetDouble() * vunit;
      }
    }
  }

  if ( doc.HasMember("shift") ) {
    const auto& jshift = doc["shift"];
    if ( jshift.IsArray() && jshift.Size() == 3 ) {
      fShift.setX(jshift[0].GetDouble()) ;
      fShift.setY(jshift[1].GetDouble());
      fShift.setZ(jshift[2].GetDouble());
    }
    else if ( jshift.IsObject() ) {
      G4double vunit = unit::GetJSONunit(jshift);
      debug::require_json_member(jshift, "val");
      const auto& jval = jshift["val"];
      debug::require_json_type(jval, rapidjson::kArrayType);
      const auto& jarray = jval.GetArray();
      fShift.set( jarray[0].GetDouble() * vunit,
                  jarray[1].GetDouble() * vunit,
                  jarray[2].GetDouble() * vunit );
    }
    else {
      G4Exception(
          "SLArFLSPhotonLibrary::Initialize",
          "ConfigError",
          JustWarning,
          "Invalid format for 'shift' parameter in photon library configuration."
          );
    }
  }

  // setup the photon librart entry containers
  fPhotonLibraryFile = TFile::Open(root_file_obj["filename"].GetString(), "READ");
  fPhotonLibraryTree = fPhotonLibraryFile->Get<TTree>(root_file_obj["objname"].GetString());

  fPhotonLibraryTree->SetBranchAddress("x", &fEntry.x_);
  fPhotonLibraryTree->SetBranchAddress("y", &fEntry.y_);
  fPhotonLibraryTree->SetBranchAddress("z", &fEntry.z_);
  
  // -- Set the number of voxels along each axis
  if ( doc.HasMember("n_voxels") ) {
    const auto& jn_voxels = doc["n_voxels"];
    if ( jn_voxels.IsArray() && jn_voxels.Size() == 3 ) {
      fNumVoxelsX = jn_voxels[0].GetInt();
      fNumVoxelsY = jn_voxels[1].GetInt();
      fNumVoxelsZ = jn_voxels[2].GetInt();
    }
    else {
      G4Exception(
          "SLArFLSPhotonLibrary::Initialize",
          "ConfigError",
          JustWarning,
          "Invalid format for 'n_voxels' parameter in photon library configuration."
          );
    }
  }
  else {
    std::set<float> xValues;
    std::set<float> yValues;
    std::set<float> zValues;
    Long64_t nEntries = fPhotonLibraryTree->GetEntries();
    for (Long64_t i=0; i<nEntries; ++i) {
      fPhotonLibraryTree->GetEntry(i);
      xValues.insert(fEntry.x_);
      yValues.insert(fEntry.y_);
      zValues.insert(fEntry.z_);
    }
    fNumVoxelsX = xValues.size();
    fNumVoxelsY = yValues.size();
    fNumVoxelsZ = zValues.size();
  }

  fPhotonLibraryTree->SetBranchAddress("vis_tot", &fEntry.vis_tot_);
  fPhotonLibraryTree->SetBranchAddress("vis_dir", &fEntry.vis_dir_);
  fPhotonLibraryTree->SetBranchAddress("vis_wls", &fEntry.vis_wls_);

  TPRegexp rgx_anode("anode_(\\d+)");
  TPRegexp rgx_scarray("^(apex|scarray|xarapuca|arapuca)_([0-9]+)$");

  SLArAnalysisManager* ana_mgr = SLArAnalysisManager::Instance();
  
  if ( doc.HasMember("tile_array_branches") ) {
    const auto& tile_array_buffers = doc["tile_array_branches"];
    debug::require_json_type(tile_array_buffers, rapidjson::kArrayType);
    for (const auto& buffer_obj : tile_array_buffers.GetArray()) {
      debug::require_json_member(buffer_obj, "name");
      debug::require_json_member(buffer_obj, "size");

      G4bool enabled = ( buffer_obj.HasMember("enabled") ) ?
        buffer_obj["enabled"].GetBool() : true;

      if ( enabled ) {
        std::string name = buffer_obj["name"].GetString();
        size_t size = buffer_obj["size"].GetUint();
        auto& vis_buffer = fEntry.tilearrayBuffers_[name];
        vis_buffer.resize(size, 0.0f);
        fPhotonLibraryTree->SetBranchAddress(name.c_str(), vis_buffer.data());

        if ( buffer_obj.HasMember("pde_scale_factor") ) {
          fPDEScaleFactor[name] = buffer_obj["pde_scale_factor"].GetFloat();
        }
        else {
          fPDEScaleFactor[name] = 1.0;
        }

        if ( buffer_obj.HasMember("pds_module") ) {
          TString pds_module = buffer_obj["pds_module"].GetString();
          if ( rgx_anode.MatchB(pds_module) ) {
            TObjArray* matches = rgx_anode.MatchS(pds_module);
            TObjString* match_str = static_cast<TObjString*>( matches->At(1) );
            G4int tpc_id = std::stoi( match_str->GetString().Data() );
            fBranchTargetAnodeTileMap[name] = &(ana_mgr->GetEventAnode().GetEventAnodeByTPCID(tpc_id));
            
            debug::require_json_member(buffer_obj, "n_elements"); 
            const auto& jn_elements = buffer_obj["n_elements"];
            debug::require_json_type(jn_elements, rapidjson::kArrayType);
            auto& base_map = fAnodeNComponentMap[name];
            for ( rapidjson::SizeType i=0; i<jn_elements.Size(); ++i ) {
              base_map.push_back( jn_elements[i].GetUint() );
            }

            delete matches;
          } else if ( rgx_scarray.MatchB(pds_module) ) {
            TObjArray* matches = rgx_scarray.MatchS(pds_module);
            G4int scarray_id = std::stoi( static_cast<TObjString*>(matches->At(2))->GetString().Data() );
            fBranchTargetSCArrayMap[name] = &(ana_mgr->GetEventPDS().GetOpDetArrayByID(scarray_id));

            delete matches;
          } else {
            G4Exception(
                "SLArFLSPhotonLibrary::Initialize",
                "ConfigError",
                JustWarning,
                Form("Unknown pds_module format '%s' for tile array buffer '%s'.",
                  pds_module.Data(), name.c_str())
                );
          }
        }
      }
    }
  }

  if ( doc.HasMember("sipm_array_branches") ) {
    const auto& sipm_buffers = doc["sipm_array_branches"];
    debug::require_json_type(sipm_buffers, rapidjson::kArrayType);
    for (const auto& buffer_obj : sipm_buffers.GetArray()) {
      debug::require_json_member(buffer_obj, "name");
      debug::require_json_member(buffer_obj, "size");

      G4bool enabled = ( buffer_obj.HasMember("enabled") ) ?
        buffer_obj["enabled"].GetBool() : true;

      if ( enabled ) {
        std::string name = buffer_obj["name"].GetString();
        size_t size = buffer_obj["size"].GetUint();
        auto& vis_buffer = fEntry.sipmBuffers_[name];
        vis_buffer.resize(size, 0.0f);
        fPhotonLibraryTree->SetBranchAddress(name.c_str(), vis_buffer.data());
        if ( buffer_obj.HasMember("pds_module") ) {
          TString pds_module = buffer_obj["pds_module"].GetString();
          if ( rgx_anode.MatchB(pds_module) ) {
            TObjArray* matches = rgx_anode.MatchS(pds_module);
            TObjString* match_str = static_cast<TObjString*>( matches->At(1) );
            G4int tpc_id = std::stoi( match_str->GetString().Data() );
            if ( fBranchTargetAnodeSiPMMap.find(name) != fBranchTargetAnodeSiPMMap.end() ) {
              delete matches; 
              continue;
            }
            fBranchTargetAnodeSiPMMap[name] = &(ana_mgr->GetEventAnode().GetEventAnodeByTPCID(tpc_id));
            
            debug::require_json_member(buffer_obj, "n_elements"); 
            const auto& jn_elements = buffer_obj["n_elements"];
            debug::require_json_type(jn_elements, rapidjson::kArrayType);
            auto& base_map = fAnodeNComponentMap[name];
            for ( rapidjson::SizeType i=0; i<jn_elements.Size(); ++i ) {
              base_map.push_back( jn_elements[i].GetUint() );
            }

            if ( buffer_obj.HasMember("pde_scale_factor") ) {
              fPDEScaleFactor[name] = buffer_obj["pde_scale_factor"].GetFloat();
            }
            else {
              fPDEScaleFactor[name] = 1.0;
            }

            delete matches;
          }  
          else {
            G4Exception(
                "SLArFLSPhotonLibrary::Initialize",
                "ConfigError",
                JustWarning,
                Form("Unknown pds_module format '%s' for tile array buffer '%s'.",
                  pds_module.Data(), name.c_str())
                );
          }
        }
      }
    }
  }

};

void SLArFLSPhotonLibrary::Print() const {
  printf("%s SLArFLSPhotonLibrary configuration:\n", fName.c_str());
  printf("Photon library file: %s\n", fPhotonLibraryFile->GetName());
  printf("Voxel size [mm]: (%.2f, %.2f, %.2f)\n", fVoxelSize[0], fVoxelSize[1], fVoxelSize[2]);
  printf("Number of voxels: (%lld, %lld, %lld)\n", fNumVoxelsX, fNumVoxelsY, fNumVoxelsZ);
  printf("Shift [mm]: (%.2f, %.2f, %.2f) mm\n", fShift.x(), fShift.y(), fShift.z());
  if (fAnodeNComponentMap.size() > 0) {
    printf("Anode components map:\n");
    for (const auto& anode_itr : fAnodeNComponentMap) {
      printf("\tAnode: %s - NComponents: ", anode_itr.first.c_str());
      for (const auto& ncomp : anode_itr.second) {
        printf("%d ", ncomp);
      }
      printf("\n");
    }
  }

  if (fSCArrayNComponentMap.size() > 0) {
    printf("SuperCellArray components map:\n");
    for (const auto& scarray_itr : fSCArrayNComponentMap) {
      printf("\tSuperCellArray: %s - NComponents: ", scarray_itr.first.c_str());
      for (const auto& ncomp : scarray_itr.second) {
        printf("%d ", ncomp);
      }
      printf("\n");
    }
  }

  if (fBranchTargetAnodeSiPMMap.size() > 0) {
    printf("\nAnode SiPM branch targets:\n");
    for (const auto& anode_itr : fBranchTargetAnodeSiPMMap) {
      printf("\tBranch: %s -> Anode TPC ID: %d (PDE scaling: %g)\n",
          anode_itr.first.c_str(),
          anode_itr.second->GetID(), 
          GetPDEScaleFactor(anode_itr.first));
    }
  }
  if (fBranchTargetAnodeTileMap.size() > 0) {
    printf("\nAnode Tile branch targets:\n");
    for (const auto& anode_itr : fBranchTargetAnodeTileMap) {
      printf("\tBranch: %s -> Anode TPC ID: %d (PDE scaling: %g)\n",
          anode_itr.first.c_str(),
          anode_itr.second->GetID(), 
          GetPDEScaleFactor(anode_itr.first));
    }
  }

  return;

}

/**
 * Propagate photons from the emission point to the optical detectors
 * by retrieving the corresponding visibility from the photon library
 * and simulating the detection process (including time of flight and PDE)
 * to register hits on the target data structures.
 *
 */
void SLArFLSPhotonLibrary::PropagatePhotons(
    const G4ParticleDefinition *particleDef,
    const G4String& volumeName,
    const G4ThreeVector& emissionPoint,
    const int numPhotons,
    const std::vector<double>& emissionTime,
    const std::vector<double>& emissionEnergy) 
{
  if (emissionTime.size() == 0 || emissionEnergy.size() == 0) {
    G4Exception(
        "SLArFLSPhotonLibrary::PropagatePhotons", "ConfigError", JustWarning,
        "Emission time and energy vectors must not be empty for photon propagation. Skipping photon propagation."
        );
    return;
  }

  // Find the voxel corresponding to the emission point
  //printf("SLArFLSPhotonLibrary::PropagatePhotons: volume %s, emissionPoint (%.2f, %.2f, %.2f) cm, numPhotons %d, emissionTime %.2f ns\n",
      //volumeName.c_str(),
      //emissionPoint.x(), emissionPoint.y(), emissionPoint.z(),
      //numPhotons,
      //emissionTime);
  //printf("num voxels: (%lld, %lld, %lld), voxel size (%.2f, %.2f, %.2f) mm, shift (%.2f, %.2f, %.2f) mm\n",
      //fNumVoxelsX, fNumVoxelsY, fNumVoxelsZ,
      //fVoxelSize[0], fVoxelSize[1], fVoxelSize[2],
      //fShift.x(), fShift.y(), fShift.z());

  G4ThreeVector shiftedPoint = emissionPoint + fShift;
  const Long64_t iX = static_cast<Long64_t>(std::round(fVoxelSize[0] > 0 ? shiftedPoint.x() / fVoxelSize[0] : 0));
  const Long64_t iY = static_cast<Long64_t>(std::round(fVoxelSize[1] > 0 ? shiftedPoint.y() / fVoxelSize[1] : 0));
  const Long64_t iZ = static_cast<Long64_t>(std::round(fVoxelSize[2] > 0 ? shiftedPoint.z() / fVoxelSize[2] : 0));

  // Compute the entry index
  const Long64_t entryIndex = iX * (fNumVoxelsY * fNumVoxelsZ) + iY * fNumVoxelsZ + iZ;
  
  // Retrieve the photon library entry
  fPhotonLibraryTree->GetEntry(entryIndex);

  //float dx = 0; float dy = 0; float dz = 0;
  //dx = emissionPoint.x() - fEntry.x_;
  //dy = emissionPoint.y() - fEntry.y_;
  //dz = emissionPoint.z() - fEntry.z_;
  //printf("SLArFLSPhotonLibrary::PropagatePhotons: emissionPoint (%.2f, %.2f, %.2f) cm -> voxel indices (%lld, %lld, %lld) -> entryIndex %lld\n",
      //emissionPoint.x(), emissionPoint.y(), emissionPoint.z(),
      //iX, iY, iZ,
      //entryIndex);
  //printf("voxel coords: (%.2f, %.2f, %.2f) mm - delta (%.2f, %.2f, %.2f) mm\n\n",
      //fEntry.x_, fEntry.y_, fEntry.z_,
      //dx, dy, dz);
  //getchar(); 

  SLArAnalysisManager* ana_mgr = SLArAnalysisManager::Instance();
  auto detector = 
    static_cast<const SLArDetectorConstruction*>
    (G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  for (auto& anode_ev_itr : fBranchTargetAnodeSiPMMap) {
    const std::string& branch_name = anode_ev_itr.first;
    SLArEventAnode* anode_ev = anode_ev_itr.second;
    auto& anode_cfg = ana_mgr->GetAnodeCfgByID( anode_ev->GetID() );
    G4ThreeVector tpc_pos = detector->GetDetTPCs().at(anode_cfg.GetTPCID())->GetModPV()->GetObjectTranslation();
    G4ThreeVector anode_pos(anode_cfg.GetX(), anode_cfg.GetY(), anode_cfg.GetZ());
    G4RotationMatrix* anode_rot = new G4RotationMatrix(
        anode_cfg.GetPhi(), anode_cfg.GetTheta(), anode_cfg.GetPsi() );
    G4Transform3D anode_transform(*anode_rot, tpc_pos);

    const auto& vis_sipm = fEntry.sipmBuffers_.at(branch_name);
    size_t photon_entry = 0; 
    size_t max_photon_entry = emissionTime.size() - 1;

    const float pde_scale_factor = GetPDEScaleFactor(branch_name);

    for (size_t idx = 0; idx < vis_sipm.size(); idx++) {
      const float& visibility = vis_sipm[idx];
      const int detected_photons = G4Poisson(numPhotons * visibility * pde_scale_factor);
      //printf("Anode %s, idx %zu, visibility %g, detected photons %d\n", 
          //branch_name.c_str(), idx, visibility, detected_photons);

      if (detected_photons > 0) {
        const auto& base_ = fAnodeNComponentMap[branch_name];

        const int sipm_idx = static_cast<int>(idx % base_.at(2));
        const int t_idx = static_cast<int>(idx / base_.at(2)) % base_.at(1);
        const int mt_idx = static_cast<int>(idx / (base_.at(2) * base_.at(1)));

        const auto& mt_cfg = anode_cfg.GetConstMap().at(mt_idx);
        const auto& t_cfg = mt_cfg.GetConstMap().at(t_idx);
        const auto& n_sipm_rows = t_cfg.GetNCellRows(); 
        const auto& n_sipm_cols = t_cfg.GetNCellCols();
        const int row = static_cast<int>(sipm_idx / n_sipm_cols);
        const int col = static_cast<int>(sipm_idx % n_sipm_cols);

        G4ThreeVector mt_pos(mt_cfg.GetX(), mt_cfg.GetY(), mt_cfg.GetZ());
        G4ThreeVector t_pos(t_cfg.GetX(), t_cfg.GetY(), t_cfg.GetZ());
        G4ThreeVector sipm_pos(
            (row + 0.5) * 30.0 - t_cfg.GetSizeX()*0.5, 
            0, 
            (col + 0.5) * 30.0 - t_cfg.GetSizeZ()*0.5);
        //printf("anode pos [%g, %g, %g] mm\n", anode_pos.x(), anode_pos.y(), anode_pos.z());
        //printf("mt pos [%g, %g, %g] mm\n", mt_pos.x(), mt_pos.y(), mt_pos.z());
        //printf("t pos [%g, %g, %g] mm\n", t_pos.x(), t_pos.y(), t_pos.z());
        //printf("sipm pos [%g, %g, %g] mm\n", sipm_pos.x(), sipm_pos.y(), sipm_pos.z());

        HepGeom::Point3D<double> local_sipm_pos = anode_pos + mt_pos + t_pos + sipm_pos;
        HepGeom::Point3D<double> global_sipm_pos = anode_transform * local_sipm_pos;

        //printf("local sipm pos [%g, %g, %g] mm\n", local_sipm_pos.x(), local_sipm_pos.y(), local_sipm_pos.z());
        //printf("global sipm pos [%g, %g, %g] mm\n", global_sipm_pos.x(), global_sipm_pos.y(), global_sipm_pos.z());
        //getchar(); 

        const G4ThreeVector opDetPoint = global_sipm_pos;

        for (int p = 0; p < detected_photons; p++) {
          // sample one entry from the emissionTime/wavelength vector
          photon_entry = static_cast<size_t>(G4UniformRand() * max_photon_entry);

          //printf("Detected photon on anode %s, mt_idx %d, t_idx %d\n", 
              //branch_name.c_str(), mt_idx, t_idx);
          G4double timeOfFlight = fTimeSim.SamplePropagationTime(emissionPoint, opDetPoint, emissionEnergy.at(photon_entry)); 

          SLArEventPhotonHit hit( (emissionTime.at(photon_entry) + timeOfFlight)/CLHEP::ps , 0 ); 
          hit.SetCellNr( sipm_idx );
          anode_ev->RegisterHit(hit, mt_idx, t_idx);
        }
      }
    }
    delete anode_rot;
  }

  TPRegexp rgx_xa_wall_nr(".*(\\d).*");

  for (auto& pds_ev_itr : fBranchTargetSCArrayMap) {
    const std::string& branch_name = pds_ev_itr.first;
    SLArEventSuperCellArray* pds_ev = pds_ev_itr.second;
    // read number from the branch name to get the corresponding OpDet array configuration
    auto matches = rgx_xa_wall_nr.MatchS(branch_name.c_str());
    int opdet_array_id = -1;
    if (matches && matches->GetEntries() > 1) {
      TObjString* match_str = dynamic_cast<TObjString*>( matches->At(1) );
      if (match_str) {
        opdet_array_id = 40 + std::stoi( match_str->GetString().Data() );
      }
      else {
        G4Exception(
            "SLArFLSPhotonLibrary::PropagatePhotons", "ConfigError", JustWarning,
            Form("Could not extract OpDet array ID from branch name '%s'. Expected format: '...<number>'.",
            branch_name.c_str()));
      }
    } else {
      G4Exception(
          "SLArFLSPhotonLibrary::PropagatePhotons", "ConfigError", JustWarning,
          Form("Could not extract OpDet array ID from branch name '%s'. Expected format: '...<number>'.", branch_name.c_str())
          );
    }
    delete matches;

    auto& opdet_array_cfg = ana_mgr->GetPDSCfg().GetMap().at( opdet_array_id );
    G4ThreeVector tpc_pos = detector->GetDetTPCs().at(opdet_array_cfg.GetTPCID())->GetModPV()->GetObjectTranslation();
    G4ThreeVector opdet_wall_pos(opdet_array_cfg.GetX(), opdet_array_cfg.GetY(), opdet_array_cfg.GetZ());
    G4RotationMatrix* opdet_wall_rot = new G4RotationMatrix(
        opdet_array_cfg.GetPhi(), opdet_array_cfg.GetTheta(), opdet_array_cfg.GetPsi() );

    G4RotationMatrix* opdet_wall_rot_inv = new G4RotationMatrix(*opdet_wall_rot);
    opdet_wall_rot_inv->invert();
    G4Transform3D opdet_wall_transform(*opdet_wall_rot_inv, tpc_pos + opdet_wall_pos); 

    const auto& vis_opdet_array = fEntry.tilearrayBuffers_.at(branch_name);
    size_t photon_entry = 0; 
    size_t max_photon_entry = emissionTime.size() - 1;

    const float pde_scale_factor = GetPDEScaleFactor(branch_name);

    for (size_t idx = 0; idx < vis_opdet_array.size(); idx++) {
      const float& visibility = vis_opdet_array[idx];
      const int detected_photons = G4Poisson(numPhotons * visibility * pde_scale_factor);
      const auto& base_ = fAnodeNComponentMap.at(branch_name);
      const int t_idx = idx;
      if (detected_photons > 0) {
        const auto& t_cfg = opdet_array_cfg.GetConstMap().at(t_idx);
        G4ThreeVector t_pos(t_cfg.GetX(), t_cfg.GetY(), t_cfg.GetZ());
        G4ThreeVector t_size( t_cfg.GetSizeX(), t_cfg.GetSizeY(), t_cfg.GetSizeZ() );
        HepGeom::Point3D<G4double> local_hit_pos(t_size.x()*(G4UniformRand() - 0.5), 0.0, t_size.z()*(G4UniformRand() - 0.5));
        local_hit_pos += HepGeom::Point3D<G4double>(t_pos.x(), t_pos.y(), t_pos.z());
        HepGeom::Point3D<double> global_hit_pos = opdet_wall_transform* local_hit_pos;

        //printf("Anode %s, idx %zu, visibility %g, detected photons %d\n", 
        //branch_name.c_str(), idx, visibility, detected_photons);
        //printf("Rot: phi %g, theta %g, psi %g deg\n", 
        //opdet_array_cfg.GetPhi(), opdet_array_cfg.GetTheta(), opdet_array_cfg.GetPsi());
        //printf("Rot matrix:\n");
        //opdet_wall_rot->print(std::cout);
        //printf("TPC %i pos [%g, %g, %g] mm\n", opdet_array_cfg.GetTPCID(), tpc_pos.x(), tpc_pos.y(), tpc_pos.z());
        //printf("opdet wall pos [%g, %g, %g] mm\n", opdet_wall_pos.x(), opdet_wall_pos.y(), opdet_wall_pos.z());
        //printf("t pos [%g, %g, %g] mm\n", t_pos.x(), t_pos.y(), t_pos.z());
        //printf("t pos (phys): [%g, %g, %g] mm\n", t_cfg.GetPhysX(), t_cfg.GetPhysY(), t_cfg.GetPhysZ());
        //printf("local hit pos [%g, %g, %g] mm\n", local_hit_pos.x(), local_hit_pos.y(), local_hit_pos.z());
        //printf("global hit pos [%g, %g, %g] mm\n", global_hit_pos.x(), global_hit_pos.y(), global_hit_pos.z());


        //printf("----------------------------------------------------------------------------\n");
        //printf("%i DETECTED PHOTONS on OpDet array %s, tile idx %zu, visibility %g\n", 
        //detected_photons, branch_name.c_str(), idx, visibility);
        //printf("\n----------------------------------------------------------------------------\n\n");

        for (int p = 0; p < detected_photons; p++) {
          // sample one entry from the emissionTime/wavelength vector
          photon_entry = static_cast<size_t>(G4UniformRand() * max_photon_entry);

          //printf("Detected photon on anode %s, mt_idx %d, t_idx %d\n", 
              //branch_name.c_str(), mt_idx, t_idx);
          //printf("emission point (%.2f, %.2f, %.2f) cm\n", emissionPoint.x(), emissionPoint.y(), emissionPoint.z());
          //printf("opdet point (%.2f, %.2f, %.2f) cm\n", opDetPoint.x(), opDetPoint.y(), opDetPoint.z());
          G4double timeOfFlight = fTimeSim.SamplePropagationTime(emissionPoint, global_hit_pos, emissionEnergy.at(photon_entry)); 

          SLArEventPhotonHit hit( (emissionTime.at(photon_entry) + timeOfFlight)/CLHEP::ps, 0 ); 
          pds_ev->RegisterHit(hit, t_idx);
        }
      }
    }
    delete opdet_wall_rot;
    delete opdet_wall_rot_inv;
  }

  return;
}

