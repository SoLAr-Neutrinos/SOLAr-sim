/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArExternalGeneratorAction.hh
 * @created     Tue Apr 11, 2023 09:43:54 CEST
 */

#ifndef SLAREXTERNALGENERATORACTION_HH

#define SLAREXTERNALGENERATORACTION_HH

#include <SLArVertextGenerator.hh>
#include <SLArBaseGenerator.hh>
#include <SLArPGunGeneratorAction.hh>

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>

#include <TH1D.h>
#include <TRandom3.h>

class G4ParticleTable;

namespace gen {

class SLArExternalGeneratorAction : public SLArBaseGenerator 
{
  public: 
    struct ExtGenConfig_t {
      G4String ext_config_path {};
      G4String ext_spectrum_path {}; 
      G4String ext_spectrum_key {}; 
      G4String ext_primary_particle {};
      G4double ext_particle_energy {};
      G4int    n_particles = 1;
    };
    SLArExternalGeneratorAction(const G4String label = "");
    virtual ~SLArExternalGeneratorAction(); 

    G4String GetGeneratorType() const override {return "external";}
    EGenerator GetGeneratorEnum() const override {return kExternalGen;}
    
    void Configure(const rapidjson::Value& config) override;
    virtual void GeneratePrimaries(G4Event* ev) override; 
    //G4double SourceExternalConfig(const G4String ext_cfg_path); 

    G4String WriteConfig() const override;

  protected:
    ExtGenConfig_t fConfig;
    std::unique_ptr<G4ParticleGun> fParticleGun; 
    G4ParticleDefinition* fParticleDef; 
    std::unique_ptr<TH1D> fEnergySpectrum; 
    std::unique_ptr<TRandom3> fRandomEngine; 

}; 

}
#endif /* end of include guard SLAREXTERNALGENERATORACTION_HH */

