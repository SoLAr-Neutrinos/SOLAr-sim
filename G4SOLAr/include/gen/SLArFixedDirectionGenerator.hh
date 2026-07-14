/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArFixedDirectionGenerator.hh
 * @created     : Wednesday Feb 12, 2025 12:09:32 CET
 */

#ifndef SLARFIXEDDIRECTIONGENERATOR_HH

#define SLARFIXEDDIRECTIONGENERATOR_HH

#include "SLArDirectionGenerator.hh"

namespace gen{
  namespace direction {

  /**
   * @class SLArFixedDirectionGenerator
   * @brief Generates a fixed direction for primary particles
   *
   * This class generates a fixed direction for primary particles. The direction is
   * specified in the configuration JSON object using the "axis" field, 
   * which can be a three-dimensional array or an object with a "val" field
   * set to a three-dimensional array. 
   * One can also specify a reference physical volume using the "volume" field.
   */
  class SLArFixedDirectionGenerator : public SLArDirectionGenerator {
    public: 
      inline SLArFixedDirectionGenerator() : fDirection(0.0, 0.0, 0.0) {}

      inline SLArFixedDirectionGenerator(const G4ThreeVector& v) {
        fDirection.set( v.x(), v.y(), v.z() ); 
      }

      inline ~SLArFixedDirectionGenerator() {}

      inline G4String GetType() const override {return "fixed_dir_generator";}

      inline EDirectionGenerator GetDirectionGeneratorEnum() const override {
        return EDirectionGenerator::kFixedDir;
      }

      inline G4ThreeVector GetDirection() const {return fDirection;}

      inline void ShootDirection(G4ThreeVector& direction) override {
        HepGeom::Point3D<G4double> dir(fDirection.x(), fDirection.y(), fDirection.z());
        dir = fTransform * dir;
        direction.set( dir.x(), dir.y(), dir.z() );
        fTmpDirection.set( dir.x(), dir.y(), dir.z() );
        return;
      }

      inline void Config(const rapidjson::Value& config) override {
        if ( config.HasMember("volume") ) {
          fReferenceVolumeName = config["volume"].GetString(); 

          G4PhysicalVolumeStore* pvstore = G4PhysicalVolumeStore::GetInstance();
          auto ref = pvstore->GetVolume(fReferenceVolumeName);
          const auto to_global = geo::GetTransformToGlobal(ref);
          fTransform = to_global;
        }
        if ( !config.HasMember("axis") ) {
          throw std::invalid_argument("fixed dir gen missing mandatory \"axis\" field\n");
        }

        const auto& jxyz = config["axis"];
        if ( jxyz.IsObject() ) {
          if (jxyz.HasMember("val") == false) {
            throw std::invalid_argument("field \"val\" not found in \"xyz\" field\n");
          }
          const auto& jxyz_val = jxyz["val"];
          if (jxyz_val.IsArray() == false) {
            throw std::invalid_argument("field \"value\" must be a rapidjson::Array\n");
          }
          if (jxyz_val.Size() != 3) {
            throw std::invalid_argument("field \"value\" must be a rapidjson::Array of size 3\n");
          }

          G4double vunit = unit::GetJSONunit(jxyz);
          
          fDirection.setX( jxyz_val.GetArray()[0].GetDouble() * vunit ); 
          fDirection.setY( jxyz_val.GetArray()[1].GetDouble() * vunit ); 
          fDirection.setZ( jxyz_val.GetArray()[2].GetDouble() * vunit ); 
        }
        else if (jxyz.IsArray()) {
          const auto& jxyz_array = jxyz.GetArray();
          if (jxyz_array.Size() != 3) {
            throw std::invalid_argument("field \"axis\" must be a rapidjson::Array of size 3\n");
          }
          fDirection.setX( jxyz_array[0].GetDouble() ); 
          fDirection.setY( jxyz_array[1].GetDouble() ); 
          fDirection.setZ( jxyz_array[2].GetDouble() ); 
        }
        else {
          throw std::invalid_argument("field \"axis\" must be a rapidjson::Array or a rapidjson::Object\n");
        }
        return;
      }

      void Print() const override {
        printf("SLArFixedDirectionGenerator configuration dump:\n"); 
        printf("Direction set to %g, %g, %g mm\n\n", 
            fDirection.x(), fDirection.y(), fDirection.z()); 
        if (fReferenceVolumeName.size() > 0) {
          printf("Reference volume: %s\n", fReferenceVolumeName.data());
        }
        return;
      }

      const rapidjson::Document ExportConfig() const override {
        rapidjson::Document dir_info; 
        dir_info.SetObject(); 
        auto& allocator = dir_info.GetAllocator();

        G4String gen_type = GetType();
        char buffer[50];
        int len = snprintf(buffer, sizeof(buffer), "%s", gen_type.data());
        rapidjson::Value str_gen_type;
        str_gen_type.SetString(buffer, len, allocator);

        dir_info.AddMember("mode", str_gen_type, allocator); 
        rapidjson::Value dir_coord(rapidjson::kArrayType); 
        dir_coord.PushBack( fDirection.x(), allocator ); 
        dir_coord.PushBack( fDirection.y(), allocator ); 
        dir_coord.PushBack( fDirection.z(), allocator ); 
        dir_info.AddMember("direction", dir_coord, allocator); 

        if (fReferenceVolumeName.empty() == false) {
          rapidjson::Value jvol; 
          jvol.SetString(fReferenceVolumeName.data(), allocator);
          dir_info.AddMember("volume", jvol, allocator);
        }

        return dir_info; 
      }

    private: 
      G4ThreeVector fDirection;
      G4String fReferenceVolumeName = {};
      G4Transform3D fTransform = {};
  };
}
}


#endif /* end of include guard SLARFIXEDDIRECTIONGENERATOR_HH */

