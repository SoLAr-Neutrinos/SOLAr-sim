/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPGunGeneratorAction.hh
 * @created     Mon Jan 02, 2023 15:49:57 CET
 */

#ifndef SLARPGUNGENERATORACTION_HH

#define SLARPGUNGENERATORACTION_HH

#include <string>
#include <SLArBaseGenerator.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>
class G4ParticleTable; 

namespace gen {
class SLArPGunGeneratorAction : public SLArBaseGenerator
{
  public: 
    struct PGunConfig_t {
      G4String particle_name = "e-"; 
      G4double particle_energy = 1.0; 
      G4int    n_particles = 1; 
      EDirectionMode direction_mode = EDirectionMode::kFixedDir;
      G4ThreeVector direction {0, 0, 1};
    };

    SLArPGunGeneratorAction(const G4String label=""); 
    ~SLArPGunGeneratorAction(); 

    void Configure(const rapidjson::Value& config) override;

    G4String GetGeneratorType() const override {return "particlegun";}
    EGenerator GetGeneratorEnum() const override {return kParticleGun;}
    
    inline void SetParticleMomentumDirection(const G4ThreeVector dir) 
      {fParticleGun->SetParticleMomentumDirection(dir);} 
    inline void SetParticleKineticEnergy(const G4double Ekin)
      {fParticleGun->SetParticleEnergy(Ekin);}
    inline void SetParticlePosition(const G4ThreeVector pos) 
      {fParticleGun->SetParticlePosition(pos);}
    inline void SetParticleTime(const G4double time) 
      {fParticleGun->SetParticleTime(time);}
    void SetParticle(const char* particle_name); 
    void SetParticle(G4ParticleDefinition* particle_def); 
    void GeneratePrimaries(G4Event*) override; 

    G4String WriteConfig() const override; 


  protected: 
    PGunConfig_t fGunConfig; 
    std::unique_ptr<G4ParticleGun> fParticleGun; 
    G4ParticleTable* fParticleTable; 

}; 
}
#endif /* end of include guard SLARPGUNGENERATORACTION_HH */

