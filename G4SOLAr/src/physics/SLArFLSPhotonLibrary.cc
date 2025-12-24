/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFLSPhotonLibrary.cc
 * @created     : Sunday Dec 14, 2025 15:36:06 CET
 */

#include <set>
#include "SLArFLSPhotonLibrary.hh"
#include "SLArAnalysisManager.hh"
#include "SLArEventAnode.hh"
#include "SLArDebugUtils.hh"
#include "SLArUnit.hpp"

#include "G4Poisson.hh"

#include "TPRegexp.h"
#include "TObjArray.h"
#include "TObjString.h"

#include "rapidjson/document.h"

void SLArFLSPhotonLibrary::Initialize(const rapidjson::Value& config) {
  const auto& doc = config.GetObject();

  debug::require_json_member(doc, "root_file_obj");
  const auto& root_file_obj = doc["root_file_obj"];
  debug::require_json_member(root_file_obj, "filename");
  debug::require_json_member(root_file_obj, "objname");

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


  fPhotonLibraryTree->SetBranchAddress("vis_tot", &fEntry.vis_tot_);
  fPhotonLibraryTree->SetBranchAddress("vis_dir", &fEntry.vis_dir_);
  fPhotonLibraryTree->SetBranchAddress("vis_wls", &fEntry.vis_wls_);

  TPRegexp rgx_anode("anode_(\\d+)");
  TPRegexp rgx_scarray("xarray_(\\d+)");

  SLArAnalysisManager* ana_mgr = SLArAnalysisManager::Instance();
  
  if ( doc.HasMember("array_buffers") ) {
    const auto& tile_array_buffers = doc["array_buffers"];
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
        if ( buffer_obj.HasMember("pds_module") ) {
          TString pds_module = buffer_obj["pds_module"].GetString();
          if ( rgx_anode.MatchB(pds_module) ) {
            TObjArray* matches = rgx_anode.MatchS(pds_module);
            TObjString* match_str = static_cast<TObjString*>( matches->At(1) );
            G4int anode_id = std::stoi( match_str->GetString().Data() );
            fBranchTargetAnodeMap[name] = &(ana_mgr->GetEventAnode().GetEventAnodeByID(anode_id));
            
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
            TObjString* match_str = static_cast<TObjString*>( matches->At(1) );
            G4int scarray_id = std::stoi( match_str->GetString().Data() );
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

  if ( doc.HasMember("sipm_buffers") ) {
    const auto& sipm_buffers = doc["sipm_buffers"];
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
      }
    }
  }

  fclose(fp);
};


void SLArFLSPhotonLibrary::PropagatePhotons(
    const G4ThreeVector& emissionPoint,
    const int numPhotons,
    const double emissionTime) 
{
  // Find the voxel corresponding to the emission point
  const Long64_t iX = static_cast<Long64_t>(std::round(fVoxelSize[0] > 0 ? emissionPoint.x() / fVoxelSize[0] : 0));
  const Long64_t iY = static_cast<Long64_t>(std::round(fVoxelSize[1] > 0 ? emissionPoint.y() / fVoxelSize[1] : 0));
  const Long64_t iZ = static_cast<Long64_t>(std::round(fVoxelSize[2] > 0 ? emissionPoint.z() / fVoxelSize[2] : 0));

  // Compute the entry index
  const Long64_t entryIndex = iX * (fNumVoxelsY * fNumVoxelsZ) + iY * fNumVoxelsZ + iZ;

  // Retrieve the photon library entry
  fPhotonLibraryTree->GetEntry(entryIndex);

  for (auto& anode_ev_itr : fBranchTargetAnodeMap) {
    const std::string& branch_name = anode_ev_itr.first;
    SLArEventAnode* anode_ev = anode_ev_itr.second;

    const auto& vis_sipm = fEntry.sipmBuffers_.at(branch_name);

    for (size_t idx = 0; idx < vis_sipm.size(); idx++) {
      const float& visibility = vis_sipm[idx];
      const int detected_photons = G4Poisson(numPhotons * visibility * 0.127);

      const auto& base_ = fAnodeNComponentMap[branch_name];

      const int sipm_idx = static_cast<int>(idx % base_.at(2));
      const int t_idx = static_cast<int>(idx / base_.at(2)) % base_.at(1);
      const int mt_idx = static_cast<int>(idx / (base_.at(2) * base_.at(1)));

      if (detected_photons > 0) {
        for (int p = 0; p < detected_photons; p++) {
          SLArEventPhotonHit hit( emissionTime, 0 ); 
          anode_ev->RegisterHit(hit, mt_idx, t_idx);
        }
      }
    }
  }

  return;
}

