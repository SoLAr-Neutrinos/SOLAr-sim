/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFLSPhotonLibrary.cc
 * @created     : Sunday Dec 14, 2025 15:36:06 CET
 */

#include "SLArFLSPhotonLibrary.hh"
#include "SLArAnalysisManager.hh"
#include "SLArEventAnode.hh"
#include "SLArDebugUtils.hh"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

void SLArFLSPhotonLibrary::Initialize(const G4String config_path) {
  FILE* fp = fopen(config_path.c_str(), "r");
  char readBuffer[65536];
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

  rapidjson::Document doc;
  doc.ParseStream(is);

  debug::require_json_member(doc, "root_file_obj");
  const auto& root_file_obj = doc["root_file_obj"];
  debug::require_json_member(root_file_obj, "filename");
  debug::require_json_member(root_file_obj, "objname");

  // setup the photon librart entry containers
  fPhotonLibraryFile = TFile::Open(root_file_obj["filename"].GetString(), "READ");
  fPhotonLibraryTree = fPhotonLibraryFile->Get<TTree>(root_file_obj["objname"].GetString());

  fPhotonLibraryTree->SetBranchAddress("x", &fEntry.x_);
  fPhotonLibraryTree->SetBranchAddress("y", &fEntry.y_);
  fPhotonLibraryTree->SetBranchAddress("z", &fEntry.z_);

  fPhotonLibraryTree->SetBranchAddress("vis_tot", &fEntry.vis_tot_);
  fPhotonLibraryTree->SetBranchAddress("vis_dir", &fEntry.vis_dir_);
  fPhotonLibraryTree->SetBranchAddress("vis_wls", &fEntry.vis_wls_);

  
  if ( doc.HasMember("tile_array_buffers") ) {
    const auto& tile_array_buffers = doc["tile_array_buffers"];
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
  auto ana_mgr = SLArAnalysisManager::Instance();

  return;
}

