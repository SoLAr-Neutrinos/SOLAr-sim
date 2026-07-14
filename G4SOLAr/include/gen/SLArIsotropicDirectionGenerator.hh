/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArIsotropicDirectionGenerator.hh
 * @created     : 2025-02-12 13:00:00
 */

#ifndef SLARISOTROPICDIRECTIONGENERATOR_HH

#define SLARISOTROPICDIRECTIONGENERATOR_HH

#include "SLArDirectionGenerator.hh"
#include "SLArRandomExtra.hh"

namespace gen{
namespace direction{
  /**
   * @class SLArIsotropicDirectionGenerator
   * @brief Generates isotropic directions for primary particles
   *
   * This class generates isotropic directions for primary particles. The direction is
   * sampled uniformly from a sphere.
   */
  class SLArIsotropicDirectionGenerator : public SLArDirectionGenerator {
    public: 
      inline SLArIsotropicDirectionGenerator() {}

      inline ~SLArIsotropicDirectionGenerator() {}

      inline EDirectionGenerator GetDirectionGeneratorEnum() const override {return kIsotropicDir;}

      inline G4String GetType() const override {return "isotropic_dir_generator";}

      inline void ShootDirection(G4ThreeVector& direction) override {
        fTmpDirection = SLArRandom::SampleRandomDirection();
        direction.set( fTmpDirection.x(), fTmpDirection.y(), fTmpDirection.z() );
        return;
      }

      inline void Config(const rapidjson::Value& config) override {
        return;
      }

      void Print() const override {
        printf("SLArIsotropicDirectionGenerator configuration dump:\n"); 
        printf("Isotropic direction\n\n");
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
        return dir_info; 
      }
  };
}
}


#endif /* end of include guard SLARISOTROPICDIRECTIONGENERATOR_HH */

