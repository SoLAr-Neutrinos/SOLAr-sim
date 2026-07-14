/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArExternalGeneratorAction.hh
 * @created     Tue Apr 11, 2023 09:43:54 CEST
 */

#ifndef SLAREXTERNALGENERATORACTION_HH

#define SLAREXTERNALGENERATORACTION_HH

#include <SLArVertexGenerator.hh>
#include <SLArBaseGenerator.hh>
#include <SLArPGunGeneratorAction.hh>

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>

#include <TH1D.h>

class G4ParticleTable;

namespace gen {

class SLArExternalGeneratorAction : public SLArBaseGenerator 
{
  public: 
    struct ExtGenConfig_t : public GenConfig_t {
      G4String ext_primary_particle {};
      G4double flux {};
    };
    SLArExternalGeneratorAction(const G4String label = "");
    virtual ~SLArExternalGeneratorAction(); 

    G4String GetGeneratorType() const override {return "external";}
    EGenerator GetGeneratorEnum() const override {return kExternalGen;}
    
    void Configure() override;
    void SourceConfiguration(const rapidjson::Value& config) override;
    virtual void GeneratePrimaries(G4Event* ev) override; 

    inline virtual void SetGenRecord( SLArGenRecord& record) const override {
      SLArBaseGenerator::SetGenRecord(record, fConfig);
    } 
    //G4String WriteConfig() const override;

  protected:
    ExtGenConfig_t fConfig;
    std::unique_ptr<G4ParticleGun> fParticleGun; 
    G4ParticleDefinition* fParticleDef; 
}; 

}
#endif /* end of include guard SLAREXTERNALGENERATORACTION_HH */

