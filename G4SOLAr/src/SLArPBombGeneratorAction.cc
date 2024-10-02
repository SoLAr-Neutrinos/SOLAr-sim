/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPBombGeneratorAction
 * @created     Friday Sep 22, 2023 09:18:51 CEST
 */

#include "SLArPBombGeneratorAction.hh"
#include "SLArRandomExtra.hh"
#include "G4ParticlePropertyTable.hh"
#include "G4OpticalPhoton.hh"
#include "G4PrimaryVertex.hh"
#include "G4Event.hh"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace gen {
SLArPBombGeneratorAction::SLArPBombGeneratorAction(const G4String label) : 
  SLArBaseGenerator(label), 
  fVertex( G4ThreeVector(0, 0, 0) )
{
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}

SLArPBombGeneratorAction::SLArPBombGeneratorAction(const G4String label, const int n_particle) 
  : SLArPBombGeneratorAction(label)
{
  fBombConfig.n_particles = n_particle;
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}


void SLArPBombGeneratorAction::SetParticle(const char* particle_name) {
  G4ParticleDefinition* particle = fParticleTable->FindParticle(particle_name); 
  SetParticle(particle); 
}

void SLArPBombGeneratorAction::SetParticle(G4ParticleDefinition* particle_def) 
{
  if (particle_def) {
    fBombConfig.particle_name = particle_def->GetParticleName();

    fParticleDefinition = particle_def;
    return;
  } else {
    printf("SLArPBombGeneratorAction::SetParticle "); 
    printf("ERROR: cannot find %s in particle table.\n", 
        particle_def->GetParticleName().c_str()); 
    return;
  }
}

void SLArPBombGeneratorAction::GeneratePrimaries(G4Event* anEvent) 
{
  G4PrimaryVertex* vertex = new G4PrimaryVertex( fVertex, fBombConfig.time ); 
  G4ThreeVector dir(0, 0, 0); 

  for (size_t n=0; n<fBombConfig.n_particles; n++) {
    G4PrimaryParticle* particle = new G4PrimaryParticle(fParticleDefinition);
    if (fBombConfig.direction_mode == EDirectionMode::kRandomDir) {
      dir = SampleRandomDirection();
    }
    else {
      dir.set(fBombConfig.direction.x(), fBombConfig.direction.y(), fBombConfig.direction.z());
    }
    particle->SetMomentumDirection( dir ); 
    particle->SetKineticEnergy( fBombConfig.particle_energy ); 

    if (fParticleDefinition == G4OpticalPhoton::OpticalPhotonDefinition()) {
      G4ThreeVector polarization = SampleRandomDirection();
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

void SLArPBombGeneratorAction::Configure(const rapidjson::Value& config) {
  if (config.HasMember("particle")) {
    fBombConfig.particle_name = config["particle"].GetString();
    SetParticle( fBombConfig.particle_name ); 
  }
  if (config.HasMember("energy")) {
    fBombConfig.particle_energy = unit::ParseJsonVal( config["energy"] ); 
  }
  if (config.HasMember("n_particles")) {
    fBombConfig.n_particles = config["n_particles"].GetInt();
  }
  if (config.HasMember("direction")) {
    if (config["direction"].IsString()) {
      G4String dir_mode = config["direction"].GetString(); 
      if (dir_mode == "isotropic") {
        fBombConfig.direction_mode = EDirectionMode::kRandomDir;
      } else if (dir_mode == "fixed") {
        fBombConfig.direction_mode = EDirectionMode::kFixedDir;
        fBombConfig.direction.set(0, 0, 1); 
      }
    }
    else if (config["direction"].IsArray()) {
      fBombConfig.direction_mode = EDirectionMode::kFixedDir;
      assert( config["direction"].GetArray().Size() == 3 ); 
      G4double dir[3] = {0}; 
      G4int idir = 0; 
      for (const auto& p : config["direction"].GetArray()) {
        dir[idir] = p.GetDouble(); idir++; 
      }
      fBombConfig.direction.set(dir[0], dir[1], dir[2]); 
    }
  }
  if (config.HasMember("vertex_gen")) {
    ConfigureVertexGenerator( config["vertex_gen"] ); 
  }
  else {
    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
  }


}

G4String SLArPBombGeneratorAction::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  G4String particle_name = fParticleDefinition->GetParticleName();
  G4String gen_type = GetGeneratorType(); 

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  d.AddMember("particle", rapidjson::StringRef(particle_name.data()), d.GetAllocator()); 
  d.AddMember("energy", fBombConfig.particle_energy, d.GetAllocator()); 

  if (fBombConfig.direction_mode == EDirectionMode::kFixedDir) {
    d.AddMember("direction_mode", rapidjson::StringRef("fixed"), d.GetAllocator()); 
    rapidjson::Value jdir; 
    jdir.SetArray();
    jdir.PushBack( fBombConfig.direction.x(), d.GetAllocator() ); 
    jdir.PushBack( fBombConfig.direction.y(), d.GetAllocator() ); 
    jdir.PushBack( fBombConfig.direction.z(), d.GetAllocator() ); 
    d.AddMember("direction", jdir, d.GetAllocator()); 
  }
  else if (fBombConfig.direction_mode == EDirectionMode::kRandomDir) {
    d.AddMember("direction_mode", rapidjson::StringRef("isotropic"), d.GetAllocator()); 
  }

  const rapidjson::Document vtx_json = fVtxGen->ExportConfig(); 
  rapidjson::Value vtx_config;
  vtx_config.CopyFrom(vtx_json, d.GetAllocator()); 
  d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 

  d.Accept(writer);
  config_str = buffer.GetString();
  return config_str;

  d.Accept(writer);
  config_str = buffer.GetString();
  return config_str;
}


}
