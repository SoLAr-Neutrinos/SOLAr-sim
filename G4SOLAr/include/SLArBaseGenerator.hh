/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArBaseGenerator
 * @created     : Saturday Mar 30, 2024 22:02:27 CET
 */

#ifndef SLARBASEGENERATOR_HH

#define SLARBASEGENERATOR_HH

#include <SLArVertextGenerator.hh>
#include <rapidjson/document.h>

#include <G4VUserPrimaryGeneratorAction.hh>

namespace gen{

  enum  EDirectionMode {kFixedDir = 0, kRandomDir = 1};

  enum  EGenerator {
    kUndefinedGen = -1 
      ,kParticleGun=0
      ,kParticleBomb=1 
      ,kDecay0=2 
      ,kMarley=3 
      ,kBackground=4 
      ,kExternalGen=5
      ,kGENIE=6
#ifdef SLAR_CRY
      ,kCRY=7
#endif 
#ifdef SLAR_RADSRC
      ,kRadSrc=8
#endif // DEBUG
  };

  static const std::map<G4String, EGenerator> genMap = {
    {"particlegun", EGenerator::kParticleGun}
    ,{"particlebomb", EGenerator::kParticleBomb} 
    ,{"decay0", EGenerator::kDecay0}
    ,{"marley", EGenerator::kMarley}
    ,{"external", EGenerator::kExternalGen}
    ,{"genie", EGenerator::kGENIE}
#ifdef SLAR_CRY
    ,{"cry", EGenerator::kCRY}
#endif
#ifdef SLAR_RADSRC
    ,{"radsrc", EGenerator::kRadSrc}
#endif
  };

  static inline EGenerator GetGeneratorIndex(const G4String gen_type) {
    EGenerator kGen = EGenerator::kUndefinedGen;
    if (genMap.find(gen_type) != genMap.end()) {
      kGen = genMap.find(gen_type)->second;
    }
    return kGen;
  }

  static inline void printGeneratorType() {
    printf("Available primary generators:\n");
    for (const auto& gen : genMap) {
      printf("\t- %s\n", gen.first.data());
    }
    return;
  }

  class SLArBaseGenerator : public G4VUserPrimaryGeneratorAction {
    public: 
      inline SLArBaseGenerator(const G4String label="") 
        : G4VUserPrimaryGeneratorAction(), fVerbose(0), fLabel(label) {}
      inline virtual ~SLArBaseGenerator() {};

      inline void SetVerboseLevel(const G4int i) {fVerbose = i;}
      inline G4int GetVerboseLevel() const {return fVerbose;}

      inline void SetLabel(const G4String& label) {fLabel = label;}
      inline G4String GetLabel() const {return fLabel;}
      virtual void Configure(const rapidjson::Value& config)=0; 
      void ConfigureVertexGenerator(const rapidjson::Value& config);

      virtual G4String GetGeneratorType() const = 0; 
      virtual EGenerator GetGeneratorEnum() const = 0; 

      virtual G4String WriteConfig() const = 0;

      virtual void GeneratePrimaries(G4Event*) = 0; 
      void RegisterPrimaries(const G4Event*, const G4int); 

    protected: 
      G4int fVerbose;
      G4String fLabel;
      std::unique_ptr<SLArVertexGenerator> fVtxGen;
  };

}

#endif /* end of include guard SLARVERTEXGENERATORINTERFACE_HH */

