/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPBombGeneratorAction
 * @created     Friday Sep 22, 2023 09:18:51 CEST
 */

#include "SLArPBombGeneratorAction.hh"
#include "SLArPointVertexGenerator.hh"
#include "SLArIsotropicDirectionGenerator.hh"
#include "SLArAnalysisManager.hh"
#include "SLArRandomExtra.hh"
#include "SLArRootUtilities.hh"
#include "G4ParticlePropertyTable.hh"
#include "G4OpticalPhoton.hh"
#include "G4PrimaryVertex.hh"
#include "G4Event.hh"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace gen {
SLArPBombGeneratorAction::SLArPBombGeneratorAction(const G4String label) : 
  SLArBaseGenerator(label)
{
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}

SLArPBombGeneratorAction::SLArPBombGeneratorAction(const G4String label, const int n_particle) 
  : SLArPBombGeneratorAction(label)
{
  fConfig.n_particles = n_particle;
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}

void SLArPBombGeneratorAction::SetParticle(const char* particle_name) {
  G4ParticleDefinition* particle = fParticleTable->FindParticle(particle_name); 
  SetParticle(particle); 
}

void SLArPBombGeneratorAction::SetParticle(G4ParticleDefinition* particle_def) 
{
  if (particle_def) {
    fConfig.particle_name = particle_def->GetParticleName();
    fParticleDefinition = particle_def;
    return;
  } else {
    fprintf(stderr, "SLArPBombGeneratorAction::SetParticle "); 
    fprintf(stderr, "ERROR: cannot find %s in particle table.\n", 
        particle_def->GetParticleName().c_str()); 
    exit(EXIT_FAILURE);
    return;
  }
}

void SLArPBombGeneratorAction::Configure() {
  if (fConfig.ene_config.mode == EEnergyMode::kExtSpectrum) {
    TH1D* hist_spectrum = get_from_rootfile<TH1D>( 
        fConfig.ene_config.spectrum_hist.filename , 
        fConfig.ene_config.spectrum_hist.objname );

    fEnergySpectrum = std::unique_ptr<TH1D>( std::move(hist_spectrum) ); 
    printf("SLArPBombGenerator::Configure() Sourcing external energy spectrum\n"); 
    printf("fEnergySpectrum ptr: %p\n", fEnergySpectrum.get());
  }

  SetParticle( fConfig.particle_name.data() );
}

void SLArPBombGeneratorAction::GeneratePrimaries(G4Event* anEvent) 
{
  G4ThreeVector vtx; fVtxGen->ShootVertex( vtx ); 
  G4double vtx_time = fVtxGen->GetTimeGenerator().SampleTime();
  G4ThreeVector dir; 

  G4PrimaryVertex* vertex = new G4PrimaryVertex( vtx, vtx_time ); 
  auto& gen_records = SLArAnalysisManager::Instance()->GetGenRecords();


  auto& record = gen_records.AddRecord( GetGeneratorEnum(), GetLabel() ); 
  auto& status = record.GetGenStatus();
  status.resize(3); 

  const G4ThreeVector vtx_lar_frame = geo::transform_frame_world_to_det( vtx ); 

  status[0] = vtx_lar_frame.x();
  status[1] = vtx_lar_frame.y();
  status[2] = vtx_lar_frame.z();

#ifdef SLAR_DEBUG
  printf("SLArPBombGeneratorAction::GeneratePrimaries(): Generating %i %ss\n", 
      fConfig.n_particles, fConfig.particle_name.data()); 
  printf("Vertex position: %.2f, %.2f, %.2f\n", vtx.x(), vtx.y(), vtx.z());
  printf("Vertex position in LAr frame: %.2f, %.2f, %.2f\n", 
      vtx_lar_frame.x(), vtx_lar_frame.y(), vtx_lar_frame.z()); 
#endif // DEBUG


  for (size_t n=0; n<fConfig.n_particles; n++) {
    G4PrimaryParticle* particle = new G4PrimaryParticle(fParticleDefinition);

    fDirGen->ShootDirection( dir );
    fConfig.ene_config.energy_tmp = SampleEnergy(fConfig.ene_config);

    particle->SetMomentumDirection( dir ); 
    particle->SetKineticEnergy( fConfig.ene_config.energy_tmp ); 

    if (fParticleDefinition == G4OpticalPhoton::OpticalPhotonDefinition()) {
      G4ThreeVector polarization = SLArRandom::SampleLinearPolarization( dir ); 
      particle->SetPolarization(polarization.x(), polarization.y(), polarization.z()); 
    }


    vertex->SetPrimary( particle ) ; 
  }

  anEvent->AddPrimaryVertex(vertex); 
  return;
}

SLArPBombGeneratorAction::~SLArPBombGeneratorAction()
{
  printf("deleting PBomb generator action... DONE\n");
}

void SLArPBombGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {

  CopyConfigurationToString(config);

  if (config.HasMember("particle")) {
    fConfig.particle_name = config["particle"].GetString();
    SetParticle( fConfig.particle_name ); 
  }

  SourceCommonConfig(config, fConfig);

  if (fVtxGen == nullptr) {
    fVtxGen = std::make_unique<vertex::SLArPointVertexGenerator>();
  }

  if (fDirGen == nullptr) {
    fDirGen = std::make_unique<direction::SLArIsotropicDirectionGenerator>();
  }

  if (fDirGen->GetType() == "fixed_dir_generator" ) {
    fprintf(stderr, "SLArPBombGeneratorAction::SourceConfiguration() WARNING. ");
    fprintf(stderr, "Fixed direction with more than 1 particle is not allowed. Reverting to isotropic sampling\n");
    fDirGen = std::make_unique<direction::SLArIsotropicDirectionGenerator>();
  }


  return;
}

//G4String SLArPBombGeneratorAction::WriteConfig() const {
  //G4String config_str = "";

  //rapidjson::Document d; 
  //d.SetObject(); 
  //rapidjson::StringBuffer buffer;
  //rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  //G4String particle_name = fParticleDefinition->GetParticleName();
  //G4String gen_type = GetGeneratorType(); 

  //d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  //d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  //d.AddMember("particle", rapidjson::StringRef(particle_name.data()), d.GetAllocator()); 

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
