/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPGunGeneratorAction.cc
 * @created     Mon Jan 02, 2023 15:52:56 CET
 */

#include "SLArPGunGeneratorAction.hh"
#include "SLArAnalysisManager.hh"
#include "SLArRandomExtra.hh"
#include "SLArRootUtilities.hh"
#include "G4ParticlePropertyTable.hh"
#include "SLArPointVertexGenerator.hh"
#include "SLArFixedDirectionGenerator.hh"


#include "G4Types.hh"
#include "G4OpticalPhoton.hh"
#include "rapidjson/document.h"

namespace gen {
SLArPGunGeneratorAction::SLArPGunGeneratorAction(const G4String label)  
  : SLArBaseGenerator(label), fParticleGun(nullptr), fParticleTable(nullptr)
{
  fParticleGun = std::make_unique<G4ParticleGun>(1); 
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}

void SLArPGunGeneratorAction::Configure() {
  if (fConfig.ene_config.mode == EEnergyMode::kExtSpectrum) {
    TH1D* hist_spectrum = get_from_rootfile<TH1D>( 
        fConfig.ene_config.spectrum_hist.filename , 
        fConfig.ene_config.spectrum_hist.objname );

    fEnergySpectrum = std::unique_ptr<TH1D>( std::move(hist_spectrum) ); 
    printf("fEnergySpectrum ptr: %p\n", fEnergySpectrum.get());
  }
  SetParticle( fConfig.particle_name.data() ); 
}

void SLArPGunGeneratorAction::SetParticle(const char* particle_name) {
  G4ParticleDefinition* particle = fParticleTable->FindParticle(particle_name); 
  SetParticle(particle); 
}

void SLArPGunGeneratorAction::SetParticle(G4ParticleDefinition* particle_def) 
{
  if (particle_def) {
    fParticleGun->SetParticleDefinition(particle_def); 
    fParticleDefinition = particle_def;
    return;
  } else {
    printf("SLArPGunGeneratorAction::SetParticle "); 
    printf("ERROR: cannot find %s in particle table.\n", 
        particle_def->GetParticleName().c_str()); 
    return;
  }
}

void SLArPGunGeneratorAction::GeneratePrimaries(G4Event* anEvent) 
{
  SLArAnalysisManager* mngr = SLArAnalysisManager::Instance();
  auto& gen_status_vec = mngr->GetGenRecords();

  for (size_t i = 0; i < fConfig.n_particles; i++) {
    G4ThreeVector vtx(0, 0, 0); 
    G4ThreeVector dir(0, 0, 0);
    
    fVtxGen->ShootVertex( vtx );
    fDirGen->ShootDirection( dir );
    const G4double vtx_time = fVtxGen->GetTimeGenerator().SampleTime();
    fParticleGun->SetParticlePosition( vtx ); 
    fParticleGun->SetParticleMomentumDirection( dir );
    fParticleGun->SetParticleEnergy( SampleEnergy(fConfig.ene_config) ); 
    fParticleGun->SetParticleTime( vtx_time ); 
    if (fParticleDefinition == G4OpticalPhoton::OpticalPhotonDefinition()) {
      G4ThreeVector polarization = SLArRandom::SampleLinearPolarization( dir ); 
      fParticleGun->SetParticlePolarization( polarization );
    }
    fParticleGun->GeneratePrimaryVertex(anEvent);

    fConfig.ene_config.energy_tmp = fParticleGun->GetParticleEnergy();
    
    auto& record = gen_status_vec.AddRecord(GetGeneratorEnum(), GetLabel());
    auto& status = record.GetGenStatus();
    status.resize(3); 
    
    const G4ThreeVector vtx_lar_frame = geo::transform_frame_world_to_det( vtx ); 
    
    status[0] = vtx_lar_frame.x()*10.0;
    status[1] = vtx_lar_frame.y()*10.0;
    status[2] = vtx_lar_frame.z()*10.0;
  }
}

SLArPGunGeneratorAction::~SLArPGunGeneratorAction()
{
  printf("deleting PGun generator action...\n");
}

void SLArPGunGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {

  CopyConfigurationToString(config);

  if (config.HasMember("particle")) {
    fConfig.particle_name = config["particle"].GetString();
    SetParticle( fConfig.particle_name ); 
  }

  SourceCommonConfig(config, fConfig);

  if (config.HasMember("specific_activity")) {
    fConfig.spec_activity = unit::ParseJsonVal(config["specific_activity"]);
  }

  if (fVtxGen == nullptr) {
    fVtxGen = std::make_unique<vertex::SLArPointVertexGenerator>();
  }
  
  if (fDirGen == nullptr) {
    fDirGen = std::make_unique<direction::SLArFixedDirectionGenerator>();
  }

  return;
}

//G4String SLArPGunGeneratorAction::WriteConfig() const {
  //G4String config_str = "";

  //rapidjson::Document d; 
  //d.SetObject(); 
  //rapidjson::StringBuffer buffer;
  //rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

  //G4String gen_type = GetGeneratorType(); 

  //d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  //d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  //d.AddMember("particle", rapidjson::StringRef(fConfig.particle_name.data()), d.GetAllocator()); 

  //const rapidjson::Document ene_doc = ExportEnergyConfig(); 
  //rapidjson::Value ene_val; 
  //ene_val.CopyFrom(ene_doc, d.GetAllocator()); 
  //d.AddMember("energy", ene_val, d.GetAllocator()); 

  //const rapidjson::Document dir_doc = ExportDirectionConfig(); 
  //rapidjson::Value dir_val; 
  //dir_val.CopyFrom(dir_doc, d.GetAllocator()); 
  //d.AddMember("direction", dir_val, d.GetAllocator()); 

  //const rapidjson::Document vtx_json = fVtxGen->ExportConfig(); 
  //rapidjson::Value vtx_config;
  //vtx_config.CopyFrom(vtx_json, d.GetAllocator()); 
  //d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 

  //d.Accept(writer);
  //config_str = buffer.GetString();
  //return config_str;
//}

}
