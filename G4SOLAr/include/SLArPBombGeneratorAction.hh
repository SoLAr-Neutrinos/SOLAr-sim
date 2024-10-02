/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPBombGeneratorAction
 * @created     Friday Sep 22, 2023 09:17:19 CEST
 */

#ifndef SLARPBOMBGENERATORACTION_HH

#define SLARPBOMBGENERATORACTION_HH

#include <string>
#include <SLArVertextGenerator.hh>
#include <SLArBaseGenerator.hh>

#include <G4ParticleDefinition.hh>
#include <G4ThreeVector.hh>
class G4ParticleTable; 

namespace gen {
class SLArPBombGeneratorAction : public SLArBaseGenerator
{
  public: 
    struct PBombConfig_t {
      G4String particle_name = "opticalphoton"; 
      G4double particle_energy = 1.0e-6; 
      G4double time = 0.0; 
      G4int    n_particles = 1; 
      EDirectionMode direction_mode = EDirectionMode::kRandomDir;
      G4ThreeVector direction {0, 0, 0};
    };

    SLArPBombGeneratorAction(const G4String label = ""); 
    SLArPBombGeneratorAction(const G4String label, const int n_particle); 
    ~SLArPBombGeneratorAction(); 

    void Configure( const rapidjson::Value& config) override;

    G4String GetGeneratorType() const override {return "particlebomb";}
    EGenerator GetGeneratorEnum() const override {return kParticleBomb;}

    inline void SetParticleMomentumDirection(const G4ThreeVector dir) 
      {fBombConfig.direction.set(dir.x(), dir.y(), dir.z());} 
    inline void SetParticleKineticEnergy(const G4double Ekin)
      {fBombConfig.particle_energy = Ekin;}
    inline void SetParticlePosition(const G4ThreeVector pos) 
      {fVertex = pos;}
    inline void SetParticleTime(const G4double time) 
      {fBombConfig.time = time;}
    inline void SetNumberOfParticles(const G4int n_particles) 
      {fBombConfig.n_particles = n_particles;} 
    void SetParticle(const char* particle_name); 
    void SetParticle(G4ParticleDefinition* particle_def); 
    void GeneratePrimaries(G4Event*) override; 

    G4String WriteConfig() const override;

  protected: 
    PBombConfig_t fBombConfig;
    G4ThreeVector fVertex; 
    G4ParticleDefinition* fParticleDefinition; 
    G4ParticleTable* fParticleTable; 

}; 
}
#endif /* end of include guard SLARPBOMBGENERATORACTION_HH */

