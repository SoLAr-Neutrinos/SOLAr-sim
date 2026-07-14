/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArPBombGeneratorAction
 * @created     Friday Sep 22, 2023 09:17:19 CEST
 */

#ifndef SLARPBOMBGENERATORACTION_HH

#define SLARPBOMBGENERATORACTION_HH

#include <string>
#include <SLArVertexGenerator.hh>
#include <SLArBaseGenerator.hh>

#include <G4ParticleDefinition.hh>
#include <G4ThreeVector.hh>
class G4ParticleTable; 

namespace gen {
class SLArPBombGeneratorAction : public SLArBaseGenerator
{
  public: 
    struct PBombConfig_t : public GenConfig_t {
      G4String particle_name = "opticalphoton"; 
    };

    SLArPBombGeneratorAction(const G4String label = ""); 
    SLArPBombGeneratorAction(const G4String label, const int n_particle); 
    ~SLArPBombGeneratorAction(); 

    void SourceConfiguration( const rapidjson::Value& config) override;

    G4String GetGeneratorType() const override {return "particlebomb";}
    EGenerator GetGeneratorEnum() const override {return kParticleBomb;}

    //inline void SetParticleMomentumDirection(const G4ThreeVector dir) 
      //{fConfig.direction.set(dir.x(), dir.y(), dir.z());} 
    //inline void SetParticleKineticEnergy(const G4double Ekin)
      //{fConfig.particle_energy = Ekin;}
    //inline void SetParticlePosition(const G4ThreeVector pos) 
      //{fVertex = pos;}
    inline void SetNumberOfParticles(const G4int n_particles) 
      {fConfig.n_particles = n_particles;} 
    void SetParticle(const char* particle_name); 
    void SetParticle(G4ParticleDefinition* particle_def); 
    inline virtual void SetGenRecord( SLArGenRecord& record) const override {
      SLArBaseGenerator::SetGenRecord(record, fConfig);
    } 
    void Configure() override;
    void GeneratePrimaries(G4Event*) override; 

    //G4String WriteConfig() const override;

  protected: 
    PBombConfig_t fConfig;
    G4ParticleDefinition* fParticleDefinition; 
    G4ParticleTable* fParticleTable; 

}; 
}
#endif /* end of include guard SLARPBOMBGENERATORACTION_HH */

