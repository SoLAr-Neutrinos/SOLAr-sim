/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArSunDirectionGenerator.hh
 * @created     : 2025-02-12 13:00:00
 */

#ifndef SLARSUNDIRECTIONGENERATOR_HH

#define SLARSUNDIRECTIONGENERATOR_HH

#include "SLArDirectionGenerator.hh"
#include "SLArRandomExtra.hh"
#include "SLArRootUtilities.hh"
#include "SLArGeneratorConfig.hh"
#include "SLArRunAction.hh"

#include "TMath.h"
#include "G4RunManager.hh"

namespace gen{
namespace direction{
  /**
   * @class SLArSunDirectionGenerator
   * @brief Samples the direction of primary particles from the nadir distribution of the Sun
   *
   * This class generates the direction of primary particles based on the
   * Sun's nadir exposure at SURF. The exposure is passed as a TH1D histogram 
   * through the `nadir_hist` field in the configuration JSON object, 
   * where one must specify the `filename` and `objname` fields.
   *
   */
  class SLArSunDirectionGenerator : public SLArDirectionGenerator {
    public: 
      inline SLArSunDirectionGenerator() : SLArDirectionGenerator() {}

      inline ~SLArSunDirectionGenerator() {}

      inline G4String GetType() const override {return "sun_dir_generator";}

      inline EDirectionGenerator GetDirectionGeneratorEnum() const override {return kSunDir;}

      inline void Config(const rapidjson::Value& config) override;

      void ShootDirection(G4ThreeVector& direction) override;

      const TH1D* GetNadirDistribution() const {return fNadirDistribution.get();}

      inline void SetNadirDistribution(TH1D* nadir) {
        fNadirDistribution = std::make_unique<TH1D>( *nadir );}

      void Print() const override {
        printf("SLArSunDirectionGenerator configuration dump:\n"); 
        printf("Sun direction\n\n");

        printf("nadir distribution info:\n");
        printf("  - filename: %s\n", fNadirHistInfo.filename.data());
        printf("  - objname: %s\n", fNadirHistInfo.objname.data());
        return;
      }

      const rapidjson::Document ExportConfig() const override;
      

    private: 
      std::unique_ptr<TH1D> fNadirDistribution = {};
      ExtSourceInfo_t fNadirHistInfo = {};
  };
}
}


#endif /* end of include guard SLARISOTROPICDIRECTIONGENERATOR_HH */

