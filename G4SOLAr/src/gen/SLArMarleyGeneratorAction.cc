/// @file SLArMarleyGeneratorAction.cc
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
#include <fstream>
#include <cstdio>

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalConstants.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4RunManager.hh"
#include "G4RandomTools.hh"

#include "marley/Event.hh"
#include "marley/Particle.hh"
#include "marley/RootJSONConfig.hh"
#include "marley/marley_root.hh"

#include "SLArRootUtilities.hh"
#include "SLArAnalysisManager.hh"
#include "SLArMarleyGeneratorAction.hh"
#include "SLArRunAction.hh"
#include "SLArRandomExtra.hh"
#include "SLArPointVertexGenerator.hh"
#include "SLArFixedDirectionGenerator.hh"
#include "SLArSunDirectionGenerator.hh"

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
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

void SLArMarleyGeneratorAction::SetupMarleyGen() 
{
  const auto run_seed = SLArAnalysisManager::Instance()->GetSeed();
  const auto& econfig = fConfig.ene_config; 

  rapidjson::Document mdoc; 
  mdoc.SetObject();
  mdoc.AddMember("seed", static_cast<uint64_t>(run_seed), mdoc.GetAllocator()); 

  rapidjson::Value jtarget; jtarget.SetObject(); 
  rapidjson::Value jnucl; jnucl.SetArray(); 
  rapidjson::Value jfrac; jfrac.SetArray(); 
  const size_t n_targets = fConfig.target.nuclides.size(); 
  for (size_t i=0; i < n_targets; i++) {
    jnucl.PushBack( fConfig.target.nuclides.at(i), mdoc.GetAllocator()); 
  }
  jtarget.AddMember("nuclides", jnucl, mdoc.GetAllocator()); 
  for (size_t i=0; i < n_targets; i++) {
    jfrac.PushBack( fConfig.target.fraction.at(i), mdoc.GetAllocator()); 
  }
  jtarget.AddMember("atom_fractions", jfrac, mdoc.GetAllocator()); 
  mdoc.AddMember("target", jtarget, mdoc.GetAllocator()); 

  rapidjson::Value jreact; jreact.SetArray(); 
  for (const auto& rr : fConfig.reactions) {
    jreact.PushBack(rapidjson::StringRef(rr.data()), mdoc.GetAllocator()); 
  }
  mdoc.AddMember("reactions", jreact, mdoc.GetAllocator()); 

  rapidjson::Value source; source.SetObject(); 
  source.AddMember("neutrino", rapidjson::StringRef(fConfig.neutrino_label.data()), mdoc.GetAllocator()); 
  const G4String& distr_label = econfig.energy_distribution_label;
  if (econfig.mode == EEnergyMode::kCustom) {
    source.AddMember("type", rapidjson::StringRef(distr_label.data()), mdoc.GetAllocator()); 
    if (distr_label == "fermi-dirac") {
      source.AddMember("Emin", econfig.energy_min, mdoc.GetAllocator()); 
      source.AddMember("Emax", econfig.energy_max, mdoc.GetAllocator()); 
      source.AddMember("temperature", econfig.temperature, mdoc.GetAllocator()); 
      source.AddMember("eta", econfig.eta, mdoc.GetAllocator());
    }
    else if (distr_label == "beta-fit") {
      source.AddMember("Emin", econfig.energy_min, mdoc.GetAllocator()); 
      source.AddMember("Emax", econfig.energy_max, mdoc.GetAllocator()); 
      source.AddMember("Emean", econfig.energy_mean, mdoc.GetAllocator()); 
      source.AddMember("beta", econfig.beta, mdoc.GetAllocator()); 
    }
    else if (distr_label == "histogram") {
      rapidjson::Value jbins; jbins.SetArray(); 
      for (const auto& b : econfig.energy_bin_left) {
        jbins.PushBack(b, mdoc.GetAllocator()); 
      }
      source.AddMember("E_bin_lefts", jbins, mdoc.GetAllocator());
      rapidjson::Value jweights; jweights.SetArray();
      for (const auto& b : econfig.weights) jweights.PushBack(b, mdoc.GetAllocator());
      source.AddMember("weights", jweights, mdoc.GetAllocator());
      source.AddMember("Emax", econfig.energy_max, mdoc.GetAllocator());
    }
    else if (distr_label == "grid") {
      rapidjson::Value jene; jene.SetArray(); 
      for (const auto& b : econfig.energies) {
        jene.PushBack(b, mdoc.GetAllocator()); 
      }
      source.AddMember("energies", jene, mdoc.GetAllocator());
      rapidjson::Value jweights; jweights.SetArray();
      for (const auto& b : econfig.weights) jweights.PushBack(b, mdoc.GetAllocator());
      source.AddMember("prob_densities", jweights, mdoc.GetAllocator());
    }
  } 
  else if (econfig.mode == EEnergyMode::kFixed) {
    printf("Setting energy mode as %s - %g MeV\n", econfig.energy_distribution_label.data(), econfig.energy_value); 
    source.AddMember("type", rapidjson::StringRef(econfig.energy_distribution_label.data()), mdoc.GetAllocator()); 
    source.AddMember("energy", econfig.energy_value, mdoc.GetAllocator()); 
  }
  else if (econfig.mode == EEnergyMode::kExtSpectrum) {
    source.AddMember("type", rapidjson::StringRef(econfig.spectrum_hist.type.data()), mdoc.GetAllocator()); 
    source.AddMember("tfile", rapidjson::StringRef(econfig.spectrum_hist.filename.data()), mdoc.GetAllocator()); 
    source.AddMember("namecycle", rapidjson::StringRef(econfig.spectrum_hist.objname.data()), mdoc.GetAllocator()); 
  }
  source.AddMember("weight_flux", fConfig.weight_flux, mdoc.GetAllocator());
  mdoc.AddMember("source", source, mdoc.GetAllocator()); 

  FILE* marley_cfg_tmp; 
  char writeBuffer[65536];
  char filePathBuffer[200]; 
  std::snprintf(filePathBuffer, 200, "/tmp/solarsim_mconfig_%ld.json", run_seed);
  std::string marley_cfg_path = filePathBuffer;
  marley_cfg_tmp = std::fopen(marley_cfg_path.data(), "w"); 
  rapidjson::FileWriteStream buffer(marley_cfg_tmp, writeBuffer, sizeof(writeBuffer)); 
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(buffer); 
  mdoc.Accept(writer);
  fclose(marley_cfg_tmp); 

  G4cout << "Setting up Marley generator..." << G4endl;
  std::ifstream tmp_file(marley_cfg_path.data());  // Apri il file

  if (!tmp_file.is_open()) {
    std::cerr << "Unable to open temporary marley configuration file" << std::endl;
    return;
  }
  std::string line;
  while (std::getline(tmp_file, line)) {
    std::cout << line << std::endl;
  }
  tmp_file.close();

  ::marley::RootJSONConfig config( marley_cfg_path.data() ); 

  fMarleyGenerator = config.create_generator();
  //fMarleyGenerator.reseed( run_seed ); 
}

double SLArMarleyGeneratorAction::SampleDecayTime(const double half_life) const  {
  return CLHEP::RandExponential::shoot( half_life / log(2) ); 
}

void SLArMarleyGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction();
  SLArRandom* slar_random = run_action->GetTRandomInterface(); 
  // Create a new primary vertex at the spacetime origin.
  G4ThreeVector vtx(0., 0., 0); 
  G4double time_reference = 0.0; 
  if (fVtxGen) {
    fVtxGen->ShootVertex(vtx); 
    time_reference = fVtxGen->GetTimeGenerator().SampleTime();
  }

  auto& gen_records = SLArAnalysisManager::Instance()->GetGenRecords();

  G4ThreeVector ddir(0., 0., 0.);
  fDirGen->ShootDirection(ddir);

  std::array<double, 3> dir = {
    fDirGen->GetTmpDirection().x(), 
    fDirGen->GetTmpDirection().y(), 
    fDirGen->GetTmpDirection().z()};

  if (fOscillogram) {
    // build neutrino energy pdf given cos(nadir)
    const int ibin_nadir = fOscillogram->GetYaxis()->FindBin( fDirGen->GetTmpCosNadir() ); 
    std::unique_ptr<TH1D> energy_hist = std::unique_ptr<TH1D>(
        fOscillogram->ProjectionX("energy_hist", ibin_nadir, ibin_nadir) );
    // setup generator
    auto marley_source = 
      ::marley_root::make_root_neutrino_source( fMarleyGenerator.get_source().get_pid(), 
          energy_hist.get() );
    fMarleyGenerator.set_source( std::move(marley_source) );
  }

  fMarleyGenerator.set_neutrino_direction(dir); 
  // Generate a new MARLEY event using the owned marley::Generator object
  ::marley::Event ev = fMarleyGenerator.create_event();

  for (const auto &ip : ev.get_initial_particles()) {
    const int fabs_pdg = fabs( ip->pdg_code() );
    if ( fabs_pdg == 12 || fabs_pdg == 14 || fabs_pdg == 16 ) {
      double total_energy = ip->total_energy();
      fConfig.ene_config.energy_tmp = ip->kinetic_energy();
    }
  }

  auto& record = gen_records.AddRecord( GetGeneratorEnum(), GetLabel() ); 
  SetGenRecord( record ); 

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
    auto vertex = new G4PrimaryVertex(vtx, time_reference); 

    // Also set the charge of the G4PrimaryParticle appropriately
    particle->SetCharge( fp->charge() );

    if (particle_idx > 1 && 
        marley_residue_pdg == 1000190400 && 
        cascade_idx < ev.get_cascade_levels().size()) {
      const auto& excited_state = marley_cascades[cascade_idx]->energy();

      auto iter = fHalfLifeTable.find(excited_state);
      if (iter != fHalfLifeTable.end()) {
        double decay_time = SampleDecayTime(iter->second);
        vertex->SetT0(time_reference + decay_time);
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

void SLArMarleyGeneratorAction::Configure() {

  if (fConfig.oscillogram_info.is_empty() == false) {
    fOscillogram = std::unique_ptr<TH2F>(
        get_from_rootfile<TH2F>( 
          fConfig.oscillogram_info.filename,
          fConfig.oscillogram_info.objname)
        );
  }

  if ( fDirGen->GetDirectionGeneratorEnum() == direction::kSunDir) {
    direction::SLArSunDirectionGenerator* sun_dir_gen = 
      dynamic_cast<direction::SLArSunDirectionGenerator*>(fDirGen.get());
    if (sun_dir_gen->GetNadirDistribution() == nullptr) {
      if (fOscillogram) {
        TH1D* hnadir = fOscillogram->ProjectionY("nadir_hist", 1, fOscillogram->GetNbinsX() );
        sun_dir_gen->SetNadirDistribution( hnadir ); 
      }
      else {
        fprintf(stderr, "SLArMarleyGenerator::Configure() ERROR: nadir histogram not set. Cannot be recovered from oscillogram\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  if (fConfig.ene_config.mode == EEnergyMode::kExtSpectrum) {
    TH1D* hist_spectrum = get_from_rootfile<TH1D>( 
        fConfig.ene_config.spectrum_hist.filename , 
        fConfig.ene_config.spectrum_hist.objname );

    fEnergySpectrum = std::unique_ptr<TH1D>( std::move(hist_spectrum) ); 
    printf("SLArMarleyGenerator::Configure() Sourcing external energy spectrum\n"); 
    printf("fEnergySpectrum ptr: %p\n", fEnergySpectrum.get());
  }

  SetupMarleyGen(); 
}

void SLArMarleyGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {

  CopyConfigurationToString(config);

  SourceCommonConfig(config, fConfig);

  if (config.HasMember("neutrino")) {
    fConfig.neutrino_label = config["neutrino"].GetString();
  }

  if (config.HasMember("reactions")) {
    if (config["reactions"].IsArray()) {
      for (const auto& rr : config["reactions"].GetArray()) {
        fConfig.reactions.push_back( rr.GetString() ); 
      }
    }
    else if (config["reactions"].IsString()) {
      fConfig.reactions.push_back( config["reactions"].GetString() ); 
    }
    else {
      fprintf(stderr, "SLArMarleyGeneratorAction:SourceConfiguration ERROR: \"reactions\" must be either a string or an array of strings\n");
      exit(EXIT_FAILURE);
    }
  }

  if (config.HasMember("target")) {
    const auto& jtarget = config["target"].GetObject(); 
    for (const auto& nn : jtarget["nuclides"].GetArray()) {
      fConfig.target.nuclides.push_back( nn.GetInt() ); 
    }

    for (const auto& af : jtarget["atom_fractions"].GetArray()) {
      fConfig.target.fraction.push_back( af.GetDouble() ); 
    }
  }

  if (config.HasMember("weight_flux")) {
    fConfig.weight_flux = config["weight_flux"].GetBool();
  }

  if (config.HasMember("oscillogram")) {
    fConfig.oscillogram_info.Configure( config["oscillogram"] ); 
  }

  if (fVtxGen == nullptr) {
    fVtxGen = std::make_unique<vertex::SLArPointVertexGenerator>();
  }

  if (fDirGen == nullptr) {
    fDirGen = std::make_unique<direction::SLArFixedDirectionGenerator>();
  }

  return;
}

//G4String SLArMarleyGeneratorAction::WriteConfig() const 
//{
  //G4String config_str = "";

  //rapidjson::Document config; 
  //FILE* config_file = std::fopen(fConfig.marley_config_path, "r"); 
  //char readBuffer[65536];
  //rapidjson::FileReadStream is(config_file, readBuffer, sizeof(readBuffer));

  //config.ParseStream<rapidjson::kParseCommentsFlag>(is);

  //rapidjson::Document vtx_json = fVtxGen->ExportConfig();

  //rapidjson::Document d; 
  //d.SetObject(); 
  //rapidjson::StringBuffer buffer;
  //rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  //G4String gen_type = GetGeneratorType(); 

  //d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  //d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  //d.AddMember("marley_config", config.GetObj(), d.GetAllocator()); 
  //rapidjson::Value vtx_config; 
  //vtx_config.CopyFrom(vtx_json, config.GetAllocator()); 
  //d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 
  //d.Accept(writer);
  //config_str = buffer.GetString();

  //fclose(config_file); 
  //return config_str;
//}

} // close namespace marley

} // close namespace gen
