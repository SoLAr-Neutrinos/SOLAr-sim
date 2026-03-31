/*
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGPSDirectionGenerator.hh
 * @created     : Wednesday Feb 12, 2025 11:40:20 CET
 */

#ifndef SLARGPSDIRECTIONGENERATOR_HH

#define SLARGPSDIRECTIONGENERATOR_HH

#include "SLArDirectionGenerator.hh"
#include "SLArRandomExtra.hh"
#include "G4SPSAngDistribution.hh"
#include "G4SPSRandomGenerator.hh"

namespace gen{
namespace direction{

  /**
   * @class SLArGPSDirectionGenerator
   * @brief Interface to the G4SPSAngDistribution class
   *
   * This class is a wrapper around the G4SPSAngDistribution class, which is used to generate
   * the direction of primary particles. It provides methods to configure the
   * direction generator, shoot directions, and export the configuration in JSON format.
   *
   * The configuration is done using a JSON object, which is passed to the Config method.
   * The arguments are supposed to be in the same format used in GPS macro commands
   * and are:
   * - `type`: the type of the source 
   *   (`iso` [default], `cos`, `planar`, `beam1d`, `beam2d`, `focused`, `user`)
   * - `rot1`, `rot2`: Defines the rotation vectors AR1 [default (1,0,0)] and 
   *   AR2 [default (0,1,0)] for the angular distribution and are not necessarily 
   *   unit vectors.  
   * - `mintheta`, `maxtheta`: the minimum and maximum theta angles (theta = 0 -> -z direction). 
   *   Values must be given in JSON object with key "val", and optionally with key 
   *   "unit" for the unit.
   * - `minphi`, `maxphi`: the minimum and maximum phi angles. Values must be given in
   *   JSON object with key "val", and optionally with key "unit" for the unit.
   * - `sigma_x`, `sigma_y`, `sigma_r`: Sets the standard deviation [default 0 rad] 
   *   of beam directional profile in the radial, x- and y- direction. 
   * - `focus_point`: the focus point for a beam source. Its coordinates are given
   *   in a JSON object with key "val" associated to a three-dimensional array,
   *   and optionally with key "unit" for the unit.
   * - `theta_hist`, `phi_hist`: the name of the histogram file and object to be used
   *   for the angular distribution. The histogram must be a TH1D object with 
   *   angles in radians. The configuration is done using a JSON object with the
   *   `type` field set to "user", and the `filename` and `objname` fields.
   */


  class SLArGPSDirectionGenerator : public SLArDirectionGenerator {
    public: 
      struct GPSAngConfig_t {
        G4ThreeVector rot1 = G4ThreeVector();
        G4ThreeVector rot2 = G4ThreeVector();
        G4ThreeVector focus_point = G4ThreeVector();
        G4double mintheta = 0.0;
        G4double maxtheta = M_PI;
        G4double minphi = 0.0;
        G4double maxphi = 2.0*M_PI;
        G4double sigma_x = 0.0;
        G4double sigma_y = 0.0;
        G4double sigma_r = 0.0;
        ExtSourceInfo_t theta_hist = {};
        ExtSourceInfo_t phi_hist = {};
      };

      inline SLArGPSDirectionGenerator() : SLArDirectionGenerator() {
        fAngDist = std::make_unique<G4SPSAngDistribution>();
        fRandGen = std::make_unique<G4SPSRandomGenerator>();
        fAngDist->SetBiasRndm( fRandGen.get() );
        fAngDist->SetPosDistribution( new G4SPSPosDistribution() );
      }

      inline ~SLArGPSDirectionGenerator() {}

      inline G4String GetType() const override {return "gps_dir_generator";}

      inline EDirectionGenerator GetDirectionGeneratorEnum() const override {return kGPSDir;}

      inline void ShootDirection(G4ThreeVector& direction) override {
        if (fAngDist == nullptr) {
          fprintf(stderr, "SLArGPSDirectionGenerator ERROR: Missing angular distribution\n");
          exit(EXIT_FAILURE);
        }
        G4ParticleMomentum p = fAngDist->GenerateOne();
        fTmpDirection.set( p.x(), p.y(), p.z() );
        direction.set( fTmpDirection.x(), fTmpDirection.y(), fTmpDirection.z() );
        return;
      }

      inline void Config(const rapidjson::Value& config) override;

      void Print() const override;

      const rapidjson::Document ExportConfig() const override;
      

    protected: 
      std::unique_ptr<G4SPSAngDistribution> fAngDist = nullptr;
      std::unique_ptr<G4SPSRandomGenerator> fRandGen = nullptr;
      GPSAngConfig_t fGPSConfig = {};

  };

}
}


#endif /* end of include guard SLARGPSDIRECTIONGENERATOR_HH */

