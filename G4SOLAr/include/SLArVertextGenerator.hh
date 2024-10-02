/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArVertextGenerator
 * @created     : Friday Mar 29, 2024 14:58:43 CET
 * @brief       : Vertex generator interface class
 */

#ifndef SLARVERTEXTGENERATOR_HH

#define SLARVERTEXTGENERATOR_HH

#include <G4ThreeVector.hh>
#include <rapidjson/document.h>
#include <SLArGeoUtils.hh>
#include <SLArUnit.hpp>

namespace gen {
  enum EVertexGenerator {kUndefinedVtxGen = -1, kPoint = 0, kBulk = 1, kBoxVolSurface = 2};
  static const std::map<G4String, EVertexGenerator> vtxGenMap = {
    {"point", EVertexGenerator::kPoint}, 
    {"bulk", EVertexGenerator::kBulk}, 
    {"boxsurface", EVertexGenerator::kBoxVolSurface}
  };

  static inline EVertexGenerator getVtxGenIndex(G4String str) {
    EVertexGenerator kGen = EVertexGenerator::kUndefinedVtxGen;
    if (vtxGenMap.find(str) != vtxGenMap.end()) {
      kGen = vtxGenMap.find(str)->second;
    }
    return kGen;
  }

  static inline void printVtxGeneratorType() {
    printf("Available vertex generators:\n");
    for (const auto& vgen : vtxGenMap) {
      printf("\t- %s\n", vgen.first.data());
    }
    return;
  }

class SLArVertexGenerator {
  public:
    SLArVertexGenerator() = default;

    virtual ~SLArVertexGenerator() = default;

    /// Check if the generator can provide at least one more vertex (default: return true)
    inline virtual bool HasNextVertex() const {return true;}
    virtual G4String GetType() const = 0; 
    virtual void ShootVertex(G4ThreeVector &  vertex_) = 0;
    virtual void Config(const rapidjson::Value&) = 0;
    virtual const rapidjson::Document ExportConfig() const = 0;  
    virtual void Print() const = 0;
};

class SLArPointVertexGenerator : public SLArVertexGenerator {
  public: 
    inline SLArPointVertexGenerator() : fVertex(0.0, 0.0, 0.0) {}
    
    inline SLArPointVertexGenerator(const G4ThreeVector& v) {
      fVertex.set( v.x(), v.y(), v.z() ); 
    }
    
    inline ~SLArPointVertexGenerator() {}
    
    inline G4String GetType() const override {return "point_vertex_generator";}

    inline G4ThreeVector GetVertex() const {return fVertex;}

    inline void ShootVertex(G4ThreeVector& vertex) override {
      vertex.set( fVertex.x(), fVertex.y(), fVertex.z() );
      return;
    }
    
    inline void Config(const rapidjson::Value& config) override {
      if ( !config.HasMember("xyz") ) {
        throw std::invalid_argument("point vtx gen missing mandatory \"xyz\" field\n");
      }
      if (config["xyz"].IsArray() == false) {
        throw std::invalid_argument("field \"xyz\" must be a rapidjson::Array\n");
      }
      auto jxyz = config["xyz"].GetArray(); 
      assert(jxyz.Size() == 3);
      G4double vunit = unit::GetJSONunit(config); 

      fVertex.setX( jxyz[0].GetDouble() * vunit ); 
      fVertex.setY( jxyz[1].GetDouble() * vunit ); 
      fVertex.setZ( jxyz[2].GetDouble() * vunit ); 
      return;
    }

    void Print() const override {
      printf("SLArPointVertexGenerator configuration dump:\n"); 
      printf("vertex set to %g, %g, %g mm\n\n", 
          fVertex.x(), fVertex.y(), fVertex.z()); 
      return;
    }

    const rapidjson::Document ExportConfig() const override {
      rapidjson::Document vtx_info; 
      vtx_info.SetObject(); 

      G4String gen_type = GetType();
      char buffer[50];
      int len = sprintf(buffer, "%s", gen_type.data());
      rapidjson::Value str_gen_type;
      str_gen_type.SetString(buffer, len, vtx_info.GetAllocator());

      vtx_info.AddMember("type", str_gen_type, vtx_info.GetAllocator()); 
      rapidjson::Value vtx_coord(rapidjson::kArrayType); 
      vtx_coord.PushBack( fVertex.x(), vtx_info.GetAllocator() ); 
      vtx_coord.PushBack( fVertex.y(), vtx_info.GetAllocator() ); 
      vtx_coord.PushBack( fVertex.z(), vtx_info.GetAllocator() ); 
      vtx_info.AddMember("vertex", vtx_coord, vtx_info.GetAllocator()); 

      return vtx_info; 
    }

  private: 
      G4ThreeVector fVertex;

};
  
}


#endif /* end of include guard SLARVERTEXTGENERATOR_HH */

