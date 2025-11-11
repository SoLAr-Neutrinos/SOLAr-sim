/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPGunGeneratorAction.hh
 * @created     Mon Jan 02, 2023 15:49:57 CET
 */

#ifndef SLARPGUNGENERATORACTION_HH

#define SLARPGUNGENERATORACTION_HH

#include <string>
#include "SLArBaseGenerator.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
class G4ParticleTable; 

namespace gen {
class SLArPGunGeneratorAction : public SLArBaseGenerator
{
  public: 
    struct PGunConfig_t : public GenConfig_t {
      G4String particle_name = "e-"; //<!< Name of the particle to be generated
      G4double spec_activity = 0; //!< Specific activity of the source
    };

    SLArPGunGeneratorAction(const G4String label=""); 
    ~SLArPGunGeneratorAction(); 

    void SourceConfiguration(const rapidjson::Value& config) override;
    void Configure() override;

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
    inline virtual void SetGenRecord( SLArGenRecord& record) const override {
      SLArBaseGenerator::SetGenRecord(record, fConfig);
    } 
    void GeneratePrimaries(G4Event*) override; 

  protected: 
    PGunConfig_t fConfig; 
    std::unique_ptr<G4ParticleGun> fParticleGun; 
    G4ParticleTable* fParticleTable; 
    G4ParticleDefinition* fParticleDefinition;

}; 
}
#endif /* end of include guard SLARPGUNGENERATORACTION_HH */

