/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArExternalGeneratorAction.cc
 * @created     Tua Apr 11, 2023 09:44:12 CEST
 */

#include <SLArExternalGeneratorAction.hh>
#include <SLArBoxSurfaceVertexGenerator.hh>
#include <SLArBulkVertexGenerator.hh>
#include <SLArRandomExtra.hh>

#include <stdio.h>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/allocators.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <TFile.h>
#include <TRandom3.h>

#include "G4RandomTools.hh"
#include "G4Poisson.hh"
#include <G4Event.hh>
#include <G4RunManager.hh>

namespace gen {

SLArExternalGeneratorAction::SLArExternalGeneratorAction(const G4String label)
  : SLArBaseGenerator(label)
{
  fParticleGun = std::make_unique<G4ParticleGun>(); 
  fRandomEngine = std::make_unique<TRandom3>( G4Random::getTheSeed() ); 
}

SLArExternalGeneratorAction::~SLArExternalGeneratorAction()
{
}

//G4double SLArExternalGeneratorAction::SourceExternalConfig(const G4String ext_cfg_path) {
  //FILE* ext_cfg_file = std::fopen(, "r");
  //char readBuffer[65536];
  //rapidjson::FileReadStream is(ext_cfg_file, readBuffer, sizeof(readBuffer));

  //rapidjson::Document d;
  //d.ParseStream<rapidjson::kParseCommentsFlag>(is);

  //assert(d.HasMember("externals")); 
  //assert(d["externals"].IsObject());

  //auto external_cfg = d["externals"].GetObject(); 
  

  //const auto& vtx_gen_settings = external_cfg["vertex_gen"];
  //assert( vtx_gen_settings.HasMember("type")); 
  //assert( vtx_gen_settings.HasMember("config")); 
  //G4String vtx_gen_label = vtx_gen_settings["type"].GetString(); 
  //const auto& vtx_gen_config = vtx_gen_settings["config"];
  //if (vtx_gen_label == "bulk" ) {
    //fVtxGen = std::unique_ptr<SLArBoxSurfaceVertexGenerator>(); 
  //}
  //else if (vtx_gen_label == "box_surf") {
    //fVtxGen = std::unique_ptr<SLArBoxSurfaceVertexGenerator>();
  //}
  //else {
    //fVtxGen = std::unique_ptr<SLArBulkVertexGenerator>();
  //}

  //fVtxGen->Config(vtx_gen_config);

  //TFile input_file(external_cfg["file"].GetString()); 
  //if (input_file.IsOpen() == false) {
    //printf("SLArExternalGeneratorAction::SourceExternalConfig ERROR\n");
    //printf("Cannot open external background file %s.\n", external_cfg["file"].GetString()); 
    //exit(2); 
  //}
  //TH1D* h = input_file.Get<TH1D>(external_cfg["key"].GetString()); 
  //h->SetDirectory( nullptr ); 
  //input_file.Close(); 

  //fEnergySpectrum = std::make_unique<TH1D>( *h ); 
  //if (!fEnergySpectrum) {
    //printf("SLArExternalGeneratorAction::SourceExternalConfig ERROR\n");
    //printf("Cannot read key %s from external background file %s.\n", 
        //external_cfg["key"].GetString(), external_cfg["file"].GetString()); 
    //exit(2); 
  //}

  //fclose(ext_cfg_file); 
  //delete h; 

  //return 0.0; 
//}

void SLArExternalGeneratorAction::GeneratePrimaries(G4Event* ev) 
{
#ifdef SLAR_DEBUG
  printf("SLArExternalGeneratorAction::GeneratePrimaries\n");
#endif
  
  for (size_t iev = 0; iev < fConfig.n_particles; iev++) {
    G4ThreeVector vtx_pos(0, 0, 0); 
    fVtxGen->ShootVertex(vtx_pos);

    //printf("Energy spectrum pointer: %p\n", fEnergySpectrum.get());
    //printf("Energy spectrum from %s\n", fEnergySpectrum->GetName());
    G4double energy = fEnergySpectrum->GetRandom( fRandomEngine.get() ); 


    G4ThreeVector dir = SampleRandomDirection(); 

    if (dynamic_cast<SLArBoxSurfaceVertexGenerator*>(fVtxGen.get())) {
      auto face = static_cast<SLArBoxSurfaceVertexGenerator*>(fVtxGen.get())->GetVertexFace(); 
      const auto& face_normal = geo::BoxFaceNormal[face]; 
      //printf("SLArExternalGeneratorAction: vtx face is %i\n", face);
      //printf("SLArExternalGeneratorAction: face normal is [%.1f, %.1f, %.1f]\n", 
      //face_normal.x(), face_normal.y(), face_normal.z()); 

      while ( dir.dot(face_normal) < 0 ) {
        dir = SampleRandomDirection(); 
      }
    }

    //G4cout << "Momentum direction is: " << dir << G4endl; 
    fParticleGun->SetParticleDefinition(fParticleDef); 
    fParticleGun->SetParticleMomentumDirection(dir); 
    fParticleGun->SetParticlePosition(vtx_pos); 
    fParticleGun->SetParticleEnergy(energy); 
    fParticleGun->SetParticleTime(0); 

    fParticleGun->GeneratePrimaryVertex(ev); 

    //getchar(); 
  }

  return;
}

void SLArExternalGeneratorAction::Configure(const rapidjson::Value& config) {
  if (config.HasMember("particle")) {
    fConfig.ext_primary_particle = config["particle"].GetString(); 
    fParticleDef = G4ParticleTable::GetParticleTable()->FindParticle( fConfig.ext_primary_particle ); 
  } else {
    throw std::invalid_argument("ext gen missing mandatory \"particle\" field.\n"); 
  }

  if (config.HasMember("n_particles")) {
    fConfig.n_particles = config["n_particles"].GetInt(); 
  }

  if (config.HasMember("energy_spectrum_file")) {
    fConfig.ext_spectrum_path = config["energy_spectrum_file"].GetString(); 
    if (config.HasMember("energy_spectrum_key")) {
      fConfig.ext_spectrum_key = config["energy_spectrum_key"].GetString(); 
    }
  } else if (config.HasMember("energy")) {
    fConfig.ext_particle_energy = unit::ParseJsonVal( config["energy"] ); 
  } else {
    fConfig.ext_particle_energy = 1.0; 
  }

  if (config.HasMember("vertex_gen")) {
    ConfigureVertexGenerator( config["vertex_gen"] ); 
  }

  if (fConfig.ext_spectrum_path) {
    TFile input_file(fConfig.ext_spectrum_path); 
    if (input_file.IsOpen() == false) {
      char err_msg[200]; 
      sprintf(err_msg, "SLArExternalGeneratorAction::Configure ERROR\nCannot open external background file %s.\n", fConfig.ext_spectrum_path.data()); 
      throw std::runtime_error(err_msg);
    }
    TH1D* h = input_file.Get<TH1D>(fConfig.ext_spectrum_key); 
    h->SetDirectory( nullptr ); 
    input_file.Close(); 

    fEnergySpectrum = std::make_unique<TH1D>( *h ); 
    if (!fEnergySpectrum) {
      char err_msg[200]; 
      sprintf(err_msg,"SLArExternalGeneratorAction::SourceExternalConfig ERROR\nCannot read key %s from external background file %s.\n", 
          fConfig.ext_spectrum_key.data(), fConfig.ext_spectrum_path.data() ); 
      throw std::runtime_error( err_msg ); 
    }
  }

  return;
}

G4String SLArExternalGeneratorAction::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  G4String gen_type = GetGeneratorType(); 

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  d.AddMember("particle", rapidjson::StringRef(fConfig.ext_primary_particle.data()), d.GetAllocator()); 
  d.AddMember("n_particles", fConfig.n_particles, d.GetAllocator()); 
  if (fConfig.ext_spectrum_path.empty()) {
    d.AddMember("energy", rapidjson::StringRef(fEnergySpectrum->GetName()), d.GetAllocator()); 
  } else {
    d.AddMember("energy_spectrum_key", rapidjson::StringRef(fConfig.ext_spectrum_key.data()), d.GetAllocator());
    d.AddMember("energy_spectrum_file", rapidjson::StringRef(fConfig.ext_spectrum_path.data()), d.GetAllocator());
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
