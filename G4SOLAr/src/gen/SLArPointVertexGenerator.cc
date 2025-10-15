/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArPointVertexGenerator.cc
 * @created     : Monday Jun 23, 2025 16:28:52 CEST
 */

#include "gen/SLArPointVertexGenerator.hh"
#include "SLArPrimaryGeneratorAction.hh"
#include "G4RunManager.hh"

namespace gen {
  namespace vertex {

    void SLArPointVertexGenerator::Config(const rapidjson::Value& config) {
      if ( config.HasMember("time") ) {
        fTimeGen.SourceConfiguration( config["time"] );
      }
      if ( config.HasMember("volume") ) {
        fReferenceVolumeName = config["volume"].GetString(); 

        G4PhysicalVolumeStore* pvstore = G4PhysicalVolumeStore::GetInstance();
        auto ref = pvstore->GetVolume(fReferenceVolumeName);
        const auto to_global = geo::GetTransformToGlobal(ref);
        fTransform = to_global;
      }
      if ( !config.HasMember("xyz") ) {
        throw std::invalid_argument("point vtx gen missing mandatory \"xyz\" field\n");
      }

      const auto& jxyz = config["xyz"];
      if (jxyz.HasMember("val") == false) {
        throw std::invalid_argument("field \"val\" not found in \"xyz\" field\n");
      }
      const auto& jxyz_val = jxyz["val"];
      if (jxyz_val.IsArray() == false) {
        throw std::invalid_argument("field \"value\" must be a rapidjson::Array\n");
      }
      G4double vunit = unit::GetJSONunit(jxyz);

      if (jxyz.HasMember("n_events")) {
        if (jxyz["n_events"].IsInt() == false) {
          throw std::invalid_argument("field \"n_events\" must be an integer\n");
        }
        fNrOfEvPerVertex = jxyz["n_events"].GetInt();
      }

      if (jxyz_val[0].IsArray()) {
        for (const auto& jjxyz : jxyz_val.GetArray()) {
          if (jjxyz.IsArray() == false) {
            throw std::invalid_argument("elements of \"val\" must be a arrays of size 3.\n");
          }
          const auto& jxyz_elem = jjxyz.GetArray();
          if (jxyz_elem.Size() != 3) {
            throw std::invalid_argument("elements of \"val\" must be a arrays of size 3.\n");
          }

          fVertexList.emplace_back(
              jxyz_elem[0].GetDouble() * vunit ,
              jxyz_elem[1].GetDouble() * vunit ,
              jxyz_elem[2].GetDouble() * vunit );

        }
      }
      else {
        if (jxyz_val.Size() != 3) {
          throw std::invalid_argument("field \"val\" must be an array of size 3.\n");
        }
        fVertex.setX( jxyz_val.GetArray()[0].GetDouble() * vunit ); 
        fVertex.setY( jxyz_val.GetArray()[1].GetDouble() * vunit ); 
        fVertex.setZ( jxyz_val.GetArray()[2].GetDouble() * vunit ); 
      }

      return;
    }

    const rapidjson::Document SLArPointVertexGenerator::ExportConfig() const {
      rapidjson::Document vtx_info; 
      vtx_info.SetObject(); 

      G4String gen_type = GetType();
      char buffer[50];
      int len = snprintf(buffer, sizeof(buffer), "%s", gen_type.data());
      rapidjson::Value str_gen_type;
      str_gen_type.SetString(buffer, len, vtx_info.GetAllocator());

      vtx_info.AddMember("type", str_gen_type, vtx_info.GetAllocator()); 
      rapidjson::Value vtx_coord(rapidjson::kArrayType); 
      vtx_coord.PushBack( fVertex.x(), vtx_info.GetAllocator() ); 
      vtx_coord.PushBack( fVertex.y(), vtx_info.GetAllocator() ); 
      vtx_coord.PushBack( fVertex.z(), vtx_info.GetAllocator() ); 
      vtx_info.AddMember("vertex", vtx_coord, vtx_info.GetAllocator()); 

      if (fReferenceVolumeName.empty() == false) {
        rapidjson::Value jvol; 
        jvol.SetString(fReferenceVolumeName.data(), vtx_info.GetAllocator());
        vtx_info.AddMember("volume", jvol, vtx_info.GetAllocator());
      }

      auto dtime = fTimeGen.ExportConfig();
      rapidjson::Value jtime; jtime.SetObject();
      jtime.CopyFrom( dtime, vtx_info.GetAllocator() );
      vtx_info.AddMember("time", jtime, vtx_info.GetAllocator());

      return vtx_info; 
    }

    void SLArPointVertexGenerator::ShootVertex(G4ThreeVector& vertex)
    {
      HepGeom::Point3D<G4double> vtx; 

      static bool print_warning = true;

      if (fVertexList.empty()) {
        vtx.set(fVertex.x(), fVertex.y(), fVertex.z());
      }
      else {
        G4int idx = 0;
        G4RunManager* run_manager = G4RunManager::GetRunManager();
        const SLArPrimaryGeneratorAction* primary_gen_action = 
          dynamic_cast<const SLArPrimaryGeneratorAction*>(run_manager->GetUserPrimaryGeneratorAction());
        const int event_nr = primary_gen_action->GetEventID();
        idx = (event_nr / fNrOfEvPerVertex) % (int)fVertexList.size();

        vtx.set(fVertexList[idx].x(),
            fVertexList[idx].y(),
            fVertexList[idx].z());

      }
      vtx = fTransform * vtx;
      vertex.set( vtx.x(), vtx.y(), vtx.z() );
      return;
    }

  }
}


