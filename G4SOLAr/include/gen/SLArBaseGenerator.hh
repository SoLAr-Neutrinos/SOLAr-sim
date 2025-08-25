/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArBaseGenerator.hh
 * @created     : Saturday Mar 30, 2024 22:02:27 CET
 */

#ifndef SLARBASEGENERATOR_HH

#define SLARBASEGENERATOR_HH

#include "G4String.hh"
#include "G4IonTable.hh"
#include "G4ParticleTable.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "SLArGeneratorConfig.hh"
#include "SLArVertexGenerator.hh"
#include "SLArDirectionGenerator.hh"
#include "event/SLArGenRecords.hh"

#include "TH1D.h"

namespace gen{

  /**
   * @class SLArBaseGenerator
   * @brief Base class for all SoLAr's generators
   *
   * This class is the base class for all generators in SoLAr. It provides
   * methods to configure the generator, including the setup the vertex and direction
   * generators.
   *
   */
  class SLArBaseGenerator : public G4VUserPrimaryGeneratorAction {
    public: 
      inline SLArBaseGenerator(const G4String label="") 
        : G4VUserPrimaryGeneratorAction(), fVerbose(0), fLabel(label) {}
      inline virtual ~SLArBaseGenerator() {};

      inline void SetVerboseLevel(const G4int i) {fVerbose = i;}
      inline G4int GetVerboseLevel() const {return fVerbose;}

      inline void SetLabel(const G4String& label) {fLabel = label;}
      inline G4String GetLabel() const {return fLabel;}
      virtual void SourceConfiguration(const rapidjson::Value& config) = 0; 
      virtual void Configure() { Configure(fConfig); } 
      virtual void Configure(const GenConfig_t& config); 

      void SetupVertexGenerator(const rapidjson::Value& config);
      void SetupDirectionGenerator(const rapidjson::Value& config);

      virtual G4String GetGeneratorType() const = 0; 
      virtual EGenerator GetGeneratorEnum() const = 0; 

      virtual void SetGenRecord( SLArGenRecord& record, const GenConfig_t& config ) const; 
      inline virtual void SetGenRecord( SLArGenRecord& record) const {
        SetGenRecord( record, fConfig ); 
      } 
      virtual G4String WriteConfig() const;

      virtual void GeneratePrimaries(G4Event*) = 0; 
      void RegisterPrimaries(const G4Event*, const G4int); 

      inline static G4bool PDGCodeIsValid(const G4int pdgcode) {
        G4ParticleDefinition* def = nullptr; 
        def = G4ParticleTable::GetParticleTable()->FindParticle(pdgcode);
        if ( def != nullptr ) {return true;}
        def = G4IonTable::GetIonTable()->GetIon(pdgcode);
        if ( def != nullptr ) {return true;}
        return false;
      }

    protected: 
      G4int fVerbose;
      G4String fLabel;
      GenConfig_t fConfig; 
      std::unique_ptr<vertex::SLArVertexGenerator> fVtxGen;
      std::unique_ptr<direction::SLArDirectionGenerator> fDirGen;
      std::unique_ptr<TH1D> fEnergySpectrum; 
      G4String fJSONConfigDump;

      void CopyConfigurationToString(const rapidjson::Value& config); 
      virtual void SourceEnergyConfig(const rapidjson::Value& ene_config); 
      inline void SourceCommonConfig(const rapidjson::Value& config, GenConfig_t& local) {
        if (config.HasMember("n_particles")) {
          local.n_particles = config["n_particles"].GetInt(); 
        }

        if (config.HasMember("vertex_gen")) {
          SetupVertexGenerator( config["vertex_gen"] );
        }

        if (config.HasMember("direction")) {
          SetupDirectionGenerator( config["direction"] );
        }

        if (config.HasMember("energy")) {
          SourceEnergyConfig( config["energy"], local.ene_config ); 
        }

        return;
      };
      virtual void SourceEnergyConfig(const rapidjson::Value& ene_config, EnergyConfig_t& local); 
      virtual const rapidjson::Document ExportEnergyConfig() const; 
      virtual G4double SampleEnergy(); 
      virtual G4double SampleEnergy(EnergyConfig_t& ene_config); 
  };

}

#endif /* end of include guard SLARVERTEXGENERATORINTERFACE_HH */

