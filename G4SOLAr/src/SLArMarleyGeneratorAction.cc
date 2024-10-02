/// @file
/// @copyright Copyright (C) 2016-2021 Steven Gardiner
/// @license GNU General Public License, version 3
//
// This file is part of MARLEY (Model of Argon Reaction Low Energy Yields)
//
// MARLEY is free software: you can redistribute it and/or modify it under the
// terms of version 3 of the GNU General Public License as published by the
// Free Software Foundation.
//
// For the full text of the license please see COPYING or
// visit http://opensource.org/licenses/GPL-3.0
//
// Please respect the MCnet academic usage guidelines. See GUIDELINES
// or visit https://www.montecarlonet.org/GUIDELINES for details.
// 
// Reimplemented in SoLAr-sim - SoLAr collaboration
// @author      Daniele Guffanti (University & INFN Milano-Bicocca), Nicholas Lane (University of Manchester)

#include <iostream>

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalConstants.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4RandomTools.hh"

#include "marley/Event.hh"
#include "marley/Particle.hh"
#include "marley/RootJSONConfig.hh"

#include "SLArAnalysisManager.hh"
#include "SLArMarleyGeneratorAction.hh"
#include <SLArRandomExtra.hh>

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"



namespace gen {
namespace marley {
SLArMarleyGeneratorAction::SLArMarleyGeneratorAction(const G4String label) 
  : SLArBaseGenerator(label)
{
  fHalfLifeTable = {
        //{ 0.0298299*CLHEP::MeV, 4.25*CLHEP::ns },
        //{ 0.800143*CLHEP::MeV, 0.26*CLHEP::ps },
        { 1.64364*CLHEP::MeV, 0.336*CLHEP::us },
        //{ 1.95907*CLHEP::MeV, 0.54*CLHEP::ps },
        //{ 2.010368*CLHEP::MeV, 0.32*CLHEP::ps },
    };
}

void SLArMarleyGeneratorAction::SetupMarleyGen(const std::string& config_file_name) 
{
  fMarleyConfig.marley_config_path = config_file_name;
  SetupMarleyGen();
}

void SLArMarleyGeneratorAction::SetupMarleyGen() 
{
  ::marley::RootJSONConfig config( fMarleyConfig.marley_config_path );
  fMarleyGenerator = config.create_generator();
  const auto run_seed = SLArAnalysisManager::Instance()->GetSeed();
  fMarleyGenerator.reseed( run_seed ); 
}


double SLArMarleyGeneratorAction::SampleDecayTime(const double half_life) const  {
  return CLHEP::RandExponential::shoot( half_life / log(2) ); 
}

void SLArMarleyGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // Create a new primary vertex at the spacetime origin.
  G4ThreeVector vtx(0., 0., 0); 
  if (fVtxGen) {
    fVtxGen->ShootVertex(vtx); 
  }

  G4double marley_time = 0.; 

  std::array<double, 3> dir = 
  {fMarleyConfig.direction.x(), fMarleyConfig.direction.y(), fMarleyConfig.direction.z()};
  if (fMarleyConfig.direction_mode == EDirectionMode::kRandomDir) {
    G4ThreeVector dir_tmp = SampleRandomDirection();
    dir.at(0) = dir_tmp.x(); 
    dir.at(1) = dir_tmp.y(); 
    dir.at(2) = dir_tmp.z();
  }
  fMarleyGenerator.set_neutrino_direction(dir); 

  // Generate a new MARLEY event using the owned marley::Generator object
  ::marley::Event ev = fMarleyGenerator.create_event();

  // Get nuclear cascade info
  const auto& marley_cascades = ev.get_cascade_levels(); 
  const int marley_residue_pdg = ev.residue().pdg_code(); 

  // Loop over each of the final particles in the MARLEY event
  size_t particle_idx = 0; 
  size_t cascade_idx = 0; 
  std::vector<G4PrimaryVertex*> primary_vertices;

  for ( const auto& fp : ev.get_final_particles() ) {

    // Convert each one from a marley::Particle into a G4PrimaryParticle.
    // Do this by first setting the PDG code and the 4-momentum components.
    G4PrimaryParticle* particle = new G4PrimaryParticle( fp->pdg_code(),
      fp->px(), fp->py(), fp->pz(), fp->total_energy() );
    auto vertex = new G4PrimaryVertex(vtx, 0.); 

    // Also set the charge of the G4PrimaryParticle appropriately
    particle->SetCharge( fp->charge() );


    if (particle_idx > 1 && 
        marley_residue_pdg == 1000190400 && 
        cascade_idx < ev.get_cascade_levels().size()) {
      const auto& excited_state = marley_cascades[cascade_idx]->energy();

      auto iter = fHalfLifeTable.find(excited_state);
      if (iter != fHalfLifeTable.end()) {
        double decay_time = SampleDecayTime(iter->second);
        vertex->SetT0(marley_time + decay_time);
#ifdef SLAR_DEBUG
        std::cout << "Decay time: " << decay_time << std::endl;
        std::cout << "Energy: " << excited_state << std::endl;
        getchar(); 
#endif
      }
      cascade_idx++;
    }
    
    vertex->SetPrimary(particle); 
    primary_vertices.push_back(vertex); 

    // Add the fully-initialized G4PrimaryParticle to the primary vertex
    particle_idx++; 
  }


  // The primary vertex has been fully populated with all final-state particles
  // from the MARLEY event. Add it to the G4Event object so that Geant4 can
  // begin tracking the particles through the simulated geometry.
  for (const auto& vertex : primary_vertices) {
    anEvent->AddPrimaryVertex( vertex );
  }
}

void SLArMarleyGeneratorAction::Configure(const rapidjson::Value& config) {
  if (config.HasMember("marley_config_path")) {
    fMarleyConfig.marley_config_path = config["marley_config_path"].GetString(); 
  } else {
    throw std::invalid_argument("Marley gen missing mandatory field \"marley_config_path\"\n");
  }

  if (config.HasMember("direction")) {
    if (config["direction"].IsString()) {
      G4String dir_mode = config["direction"].GetString(); 
      if (dir_mode == "isotropic") {
        fMarleyConfig.direction_mode = EDirectionMode::kRandomDir;
      } else if (dir_mode == "fixed") {
        fMarleyConfig.direction_mode = EDirectionMode::kFixedDir;
        fMarleyConfig.direction.set(0, 0, 1); 
      }
    }
    else if (config["direction"].IsArray()) {
      fMarleyConfig.direction_mode = EDirectionMode::kFixedDir;
      assert( config["direction"].GetArray().Size() == 3 ); 
      G4double dir[3] = {0}; 
      G4int idir = 0; 
      for (const auto& p : config["direction"].GetArray()) {
        dir[idir] = p.GetDouble(); idir++; 
      }
      fMarleyConfig.direction.set(dir[0], dir[1], dir[2]); 
    }
  }
  if (config.HasMember("vertex_gen")) {
    ConfigureVertexGenerator( config["vertex_gen"] ); 
  }
  else {
    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
  }

  SetupMarleyGen(); 
  return;
}

G4String SLArMarleyGeneratorAction::WriteConfig() const 
{
  G4String config_str = "";

  rapidjson::Document config; 
  FILE* config_file = std::fopen(fMarleyConfig.marley_config_path, "r"); 
  char readBuffer[65536];
  rapidjson::FileReadStream is(config_file, readBuffer, sizeof(readBuffer));

  config.ParseStream<rapidjson::kParseCommentsFlag>(is);

  rapidjson::Document vtx_json = fVtxGen->ExportConfig();

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  G4String gen_type = GetGeneratorType(); 

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  d.AddMember("marley_config", config.GetObj(), d.GetAllocator()); 
  rapidjson::Value vtx_config; 
  vtx_config.CopyFrom(vtx_json, config.GetAllocator()); 
  d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 
  d.Accept(writer);
  config_str = buffer.GetString();

  fclose(config_file); 
  return config_str;
}

} // close namespace marley

} // close namespace gen
