/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPGunGeneratorAction.cc
 * @created     Mon Jan 02, 2023 15:52:56 CET
 */

#include "SLArPGunGeneratorAction.hh"
#include "SLArRandomExtra.hh"
#include "G4ParticlePropertyTable.hh"


#include <G4Types.hh>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace gen {
SLArPGunGeneratorAction::SLArPGunGeneratorAction(const G4String label)  
  : SLArBaseGenerator(label), fParticleGun(nullptr)
{
  fParticleGun = std::make_unique<G4ParticleGun>(1); 
  fParticleTable = G4ParticleTable::GetParticleTable(); 
}


void SLArPGunGeneratorAction::SetParticle(const char* particle_name) {
  G4ParticleDefinition* particle = fParticleTable->FindParticle(particle_name); 
  SetParticle(particle); 
}

void SLArPGunGeneratorAction::SetParticle(G4ParticleDefinition* particle_def) 
{
  if (particle_def) {
    fParticleGun->SetParticleDefinition(particle_def); 
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
  for (size_t i = 0; i < fGunConfig.n_particles; i++) {
    G4ThreeVector vtx(0, 0, 0); 
    G4ParticleMomentum p(0, 0, 1); 
    if (fGunConfig.direction_mode == EDirectionMode::kFixedDir) {
      p.set( fGunConfig.direction.x(), fGunConfig.direction.y(), fGunConfig.direction.z()); 
    } 
    else if (fGunConfig.direction_mode == EDirectionMode::kRandomDir) {
      G4ThreeVector random_dir = SampleRandomDirection(); 
      p.set(random_dir.x(), random_dir.y(), random_dir.z()); 
    }
    fVtxGen->ShootVertex( vtx ); 
    fParticleGun->SetParticlePosition( vtx ); 
    fParticleGun->SetParticleMomentumDirection( p );
    fParticleGun->SetParticleEnergy( fGunConfig.particle_energy ); 
    fParticleGun->GeneratePrimaryVertex(anEvent);
  }
}

SLArPGunGeneratorAction::~SLArPGunGeneratorAction()
{
  printf("deleting PGun generator action... ");
  //if (fParticleGun) {delete fParticleGun; fParticleGun = nullptr;} 
  printf("DONE\n");
}

void SLArPGunGeneratorAction::Configure(const rapidjson::Value& config) {
  if (config.HasMember("particle")) {
    fGunConfig.particle_name = config["particle"].GetString();
    SetParticle( fGunConfig.particle_name ); 
  }
  if (config.HasMember("energy")) {
    fGunConfig.particle_energy = unit::ParseJsonVal( config["energy"] ); 
  }
  if (config.HasMember("n_particles")) {
    fGunConfig.n_particles = config["n_particles"].GetInt();
  }
  if (config.HasMember("direction")) {
    if (config["direction"].IsString()) {
      G4String dir_mode = config["direction"].GetString(); 
      if (dir_mode == "isotropic") {
        fGunConfig.direction_mode = EDirectionMode::kRandomDir;
      } else if (dir_mode == "fixed") {
        fGunConfig.direction_mode = EDirectionMode::kFixedDir;
        fGunConfig.direction.set(0, 0, 1); 
      }
    }
    else if (config["direction"].IsArray()) {
      fGunConfig.direction_mode = EDirectionMode::kFixedDir;
      assert( config["direction"].GetArray().Size() == 3 ); 
      G4double dir[3] = {0}; 
      G4int idir = 0; 
      for (const auto& p : config["direction"].GetArray()) {
        dir[idir] = p.GetDouble(); idir++; 
      }
      fGunConfig.direction.set(dir[0], dir[1], dir[2]); 
    }
  }
  if (config.HasMember("vertex_gen")) {
    ConfigureVertexGenerator( config["vertex_gen"] ); 
  }
  else {
    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
  }

  return;
}

G4String SLArPGunGeneratorAction::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

  G4String gen_type = GetGeneratorType(); 

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  d.AddMember("particle", rapidjson::StringRef(fGunConfig.particle_name.data()), d.GetAllocator()); 
  d.AddMember("energy", fGunConfig.particle_energy, d.GetAllocator()); 
  if (fGunConfig.direction_mode == EDirectionMode::kFixedDir) {
    d.AddMember("direction_mode", rapidjson::StringRef("fixed"), d.GetAllocator()); 
    rapidjson::Value jdir; 
    jdir.SetArray();
    jdir.PushBack( fGunConfig.direction.x(), d.GetAllocator() ); 
    jdir.PushBack( fGunConfig.direction.y(), d.GetAllocator() ); 
    jdir.PushBack( fGunConfig.direction.z(), d.GetAllocator() ); 
    d.AddMember("direction", jdir, d.GetAllocator()); 
  }
  else if (fGunConfig.direction_mode == EDirectionMode::kRandomDir) {
    d.AddMember("direction_mode", rapidjson::StringRef("isotropic"), d.GetAllocator()); 
  }

  const rapidjson::Document vtx_json = fVtxGen->ExportConfig(); 
  rapidjson::Value vtx_config;
  vtx_config.CopyFrom(vtx_json, d.GetAllocator()); 
  d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 

  d.Accept(writer);
  config_str = buffer.GetString();
  return config_str;
}

}
