/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArBaseGenerator.cc
 * @created     : Saturday Mar 30, 2024 22:08:11 CET
 */

#include <cstdio>
#include <memory>
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"

#include "SLArRootUtilities.hh"
#include "SLArBaseGenerator.hh"
#include "SLArPointVertexGenerator.hh"
#include "SLArBulkVertexGenerator.hh"
#include "SLArBoxSurfaceVertexGenerator.hh"
#include "SLArGPSVertexGenerator.hh"
#include "SLArFixedDirectionGenerator.hh"
#include "SLArIsotropicDirectionGenerator.hh"
#include "SLArSunDirectionGenerator.hh"
#include "SLArGPSDirectionGenerator.hh"
#include "SLArAnalysisManager.hh"
#include "SLArRunAction.hh"
#include "SLArRandomExtra.hh"
#include "SLArUnit.hpp"

#include "rapidjson/prettywriter.h"


namespace gen {

/**
 * @brief Setup the vertex generator
 *
 * @param config The JSON configuration object for the vertex generator
 *
 * This method sets up the vertex generator based on the provided configuration.
 * The "type" of the chosen generator is mandatory. Available types are:
 * - "point" for `SLArPointVertexGenerator`
 * - "gps_pos" for `SLArGPSVertexGenerator`
 * - "bulk" for `SLArBulkVertexGenerator`
 * - "boxsurface" for `SLArBoxSurfaceVertexGenerator`
 *
 * Each one of these generators has its own configuration options, which are
 * passed in the "config" field of the JSON object. See the respective
 * generator classes for details.
 */
void SLArBaseGenerator::SetupVertexGenerator(const rapidjson::Value& config) {
  if (!config.HasMember("type")) {
    throw std::invalid_argument("vertex generator missing mandatory \"type\" field\n");
  }
  
  G4String type = config["type"].GetString(); 
  vertex::EVertexGenerator kGen = vertex::getVtxGenIndex( type ); 
  printf("[gen] Building %s vertex generator (type %i)\n", type.data(), kGen);

  switch (kGen) {
    case (vertex::EVertexGenerator::kPoint) : 
      {
        fVtxGen = std::make_unique<vertex::SLArPointVertexGenerator>();
        try { fVtxGen->Config( config["config"] ); }        
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArPointVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }
    
    case (vertex::EVertexGenerator::kGPSPos) : 
      {
        printf("Building GPS vertex generator\n");
        fVtxGen = std::make_unique<vertex::SLArGPSVertexGenerator>();
        try { fVtxGen->Config( config["config"] ); }
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArGPSPosVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    case (vertex::EVertexGenerator::kBulk) : 
      {
        fVtxGen = std::make_unique<vertex::SLArBulkVertexGenerator>(); 
        try {fVtxGen->Config( config["config"] );}
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArBulkVertexGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    case (vertex::EVertexGenerator::kBoxVolSurface) : 
      {
        fVtxGen = std::make_unique<vertex::SLArBoxSurfaceVertexGenerator>();
        try {fVtxGen->Config( config["config"] );} 
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArBoxSurfaceVertexGenerator" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    default:
      {
        char err_msg[100];
        gen::vertex::printVtxGeneratorType();
        snprintf(err_msg, sizeof(err_msg), "Unable to find %s vertex generator among the available options\n", 
            type.data()); 
        std::cerr << err_msg << std::endl;
        exit( EXIT_FAILURE ); 
        break;      
      }
  }

  printf("[gen] %s vtx gen for generator %s: %p\n", 
      fVtxGen->GetType().data(), fLabel.data(), static_cast<void*>(fVtxGen.get())); 
  fVtxGen->Print();
  
  return;
}

void SLArBaseGenerator::SetupDirectionGenerator(const rapidjson::Value& config) {
  if (!config.HasMember("type")) {
    throw std::invalid_argument("direction generator missing mandatory \"type\" field\n");
  }
  
  G4String type = config["type"].GetString(); 
  direction::EDirectionGenerator kGen = direction::getDirGenIndex( type ); 
  printf("[gen] Building %s direction generator (type %i)\n", type.data(), kGen);

  if (kGen != direction::kIsotropicDir && config.HasMember("config") == false) {
    throw std::invalid_argument("direction generator missing mandatory \"config\" field\n");
  }

  switch (kGen) {
    case (direction::EDirectionGenerator::kFixedDir) : 
      {
        fDirGen = std::make_unique<direction::SLArFixedDirectionGenerator>();
        try { fDirGen->Config( config["config"] ); }        
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArFixedDirectionGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    case (direction::kIsotropicDir) : 
      {
        fDirGen = std::make_unique<direction::SLArIsotropicDirectionGenerator>();
        break;
      }

    case (direction::kSunDir) : 
      {
        fDirGen = std::make_unique<direction::SLArSunDirectionGenerator>();
        try { fDirGen->Config( config["config"] ); }        
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArSunDirectionGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }

    case (direction::kGPSDir) : 
      {
        fDirGen = std::make_unique<direction::SLArGPSDirectionGenerator>();
        try { fDirGen->Config( config["config"] ); }        
        catch (const std::exception& e) {
          std::cerr << "ERROR configuring SLArGPSDirectionGenerator()" << std::endl;
          std::cerr << e.what() << std::endl;
          exit( EXIT_FAILURE );
        }
        break;
      }
    default:
      {
        char err_msg[100];
        direction::printDirGeneratorType();
        snprintf(err_msg, sizeof(err_msg), "Unable to find %s direction generator among the available options\n", 
            type.data()); 
        std::cerr << err_msg << std::endl;
        exit( EXIT_FAILURE ); 
        break;      
      }
      break;
  }

}

void SLArBaseGenerator::CopyConfigurationToString(const rapidjson::Value& config) {
  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

  rapidjson::Value config_copy; config_copy.SetObject(); 
  config_copy.CopyFrom(config, d.GetAllocator()); 

  d.AddMember("config", config_copy, d.GetAllocator());

  d.Accept(writer); 
  fJSONConfigDump = buffer.GetString(); 
}

/*
 *void SLArBaseGenerator::SourceConfiguration(const rapidjson::Value& config, GenConfig_t& local) {
 *  CopyConfigurationToString(config);
 *
 *  if (config.HasMember("n_particles")) {
 *    local.n_particles = config["n_particles"].GetInt();
 *  }
 *
 *  if (config.HasMember("direction")) {
 *    SourceDirectionConfig( config["direction"], local.dir_config );
 *  }
 *
 *  if (config.HasMember("energy")) {
 *    SourceEnergyConfig( config["energy"], local.ene_config );
 *  }
 *
 *  if (config.HasMember("vertex_gen")) {
 *    SetupVertexGenerator( config["vertex_gen"] ); 
 *  }
 *  else {
 *    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
 *  }
 *}
 */

/*
 *void SLArBaseGenerator::SourceConfiguration(const rapidjson::Value& config) {
 *
 *  CopyConfigurationToString(config);
 *
 *  if (config.HasMember("n_particles")) {
 *    fConfig.n_particles = config["n_particles"].GetInt();
 *  }
 *
 *  if (config.HasMember("direction")) {
 *    SourceDirectionConfig( config["direction"] );
 *  }
 *
 *  if (config.HasMember("energy")) {
 *    SourceEnergyConfig( config["energy"] );
 *  }
 *
 *  if (config.HasMember("vertex_gen")) {
 *    SetupVertexGenerator( config["vertex_gen"] ); 
 *  }
 *  else {
 *    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
 *  }
 *}
 */

void SLArBaseGenerator::Configure(const GenConfig_t& config) {


  printf("SLArBaseGeneratorAction::Configure(): Energy mode: %i\n", fConfig.ene_config.mode);
  if (fConfig.ene_config.mode == EEnergyMode::kExtSpectrum) {
    TH1D* hist_spectrum = get_from_rootfile<TH1D>(
        fConfig.ene_config.spectrum_hist.filename , 
        fConfig.ene_config.spectrum_hist.objname );

    fEnergySpectrum = std::unique_ptr<TH1D>( std::move(hist_spectrum) ); 
    printf("SLArBaseGenerator::Configure() Sourcing external energy spectrum\n"); 
  }

  return;
}

G4double SLArBaseGenerator::SampleEnergy(EnergyConfig_t& ene_config) {
  G4double ene = 1.0*CLHEP::MeV;

  SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction(); 
  SLArRandom* slar_random = run_action->GetTRandomInterface(); 

  if (ene_config.mode == EEnergyMode::kFixed) {
    ene_config.energy_tmp = ene_config.energy_value; 
  }
  else if (ene_config.mode == EEnergyMode::kExtSpectrum) {
    if (fEnergySpectrum == nullptr) {
      G4Exception("SLArBaseGenerator::SampleEnergy", "NoEnergySpectrum", FatalException, 
          "Attempting to sample from an external energy spectrum but no spectrum has been configured.");
    }
    ene_config.energy_tmp = fEnergySpectrum->GetRandom( slar_random->GetEngine().get() ); 
  }

  return ene_config.energy_tmp;
}

G4double SLArBaseGenerator::SampleEnergy() {
  return SampleEnergy(fConfig.ene_config); 
}

void SLArBaseGenerator::SourceEnergyConfig(const rapidjson::Value& ene_config) {
  SourceEnergyConfig( ene_config, fConfig.ene_config ); 
  return;
}

void SLArBaseGenerator::SourceEnergyConfig(const rapidjson::Value& ene_config, EnergyConfig_t& local) {
  if (ene_config.IsObject() == false) {
    fprintf(stderr, "ConfigureEnergy ERROR: Energy configuration must be an object\n"); 
    exit(EXIT_FAILURE); 
  }

  if (ene_config.HasMember("mode") == false) {
    fprintf(stderr, "ConfigureEnergy ERROR: User must specify direction \"mode\" (monoenergetic, custom, spectrum)\n");
    exit(EXIT_FAILURE);
  }

  G4String ene_mode = ene_config["mode"].GetString(); 

  if (ene_mode == "monoenergetic" || ene_mode == "mono") {
    local.mode = EEnergyMode::kFixed;
    local.energy_distribution_label = "monoenergetic"; 

    if ( ene_config.HasMember("value") == false ) {
      fprintf(stderr, "ConfigureEnergy ERROR: monoenergetic particle set but no energy value provided\n"); 
      exit(EXIT_FAILURE); 
    }
    local.energy_value = unit::ParseJsonVal( ene_config["value"] ); 
    printf("set energy to %g MeV\n", local.energy_value); 
  }
  else if (ene_mode == "custom") {
    local.mode = EEnergyMode::kCustom; 
    if ( ene_config.HasMember("label") == false ) {
      fprintf(stderr, "ConfigureEnergy ERROR: custom energy spectrum set but no label given\n");
      exit(EXIT_FAILURE); 
    }
    local.energy_distribution_label = ene_config["label"].GetString(); 

    if (ene_config.HasMember("Emin")) {
      local.energy_min = ene_config["Emin"].GetDouble();
    }
    if (ene_config.HasMember("Emax")) {
      local.energy_max = ene_config["Emax"].GetDouble();
    }
    if (ene_config.HasMember("temperature")) {
      local.temperature = ene_config["temperature"].GetDouble();
    }
    if (ene_config.HasMember("eta")) {
      local.eta = ene_config["eta"].GetDouble();
    }
    if (ene_config.HasMember("Emean")) {
      local.energy_mean = ene_config["Emean"].GetDouble();
    }
    if (ene_config.HasMember("beta")) {
      local.beta = ene_config["beta"].GetDouble();
    }
    if (ene_config.HasMember("E_bin_lefts")) {
      if (ene_config["E_bin_lefts"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"E_bin_lefts\" MUST be an array\n"); 
        exit(EXIT_FAILURE);
      }

      for (const auto& jb : ene_config["E_bin_lefts"].GetArray()) {
        local.energy_bin_left.push_back( jb.GetDouble() ); 
      }
    }
    if (ene_config.HasMember("energies")) {
      if (ene_config["energies"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"energies\" MUST be an array\n"); 
        exit(EXIT_FAILURE);
      }

      for (const auto& jb : ene_config["energies"].GetArray()) {
        local.energies.push_back( jb.GetDouble() ); 
      }
    }
    if (ene_config.HasMember("weights")) {
      if (ene_config["weights"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"weights\" MUST be an array\n"); 
        exit(EXIT_FAILURE);
      }

      for (const auto& jb : ene_config["weights"].GetArray()) {
        local.weights.push_back( jb.GetDouble() ); 
      }
    }
    if (ene_config.HasMember("prob_densities")) {
      if (ene_config["prob_densities"].IsArray() == false) {
        fprintf(stderr, "SourceEnergyConfiguration ERROR: \"prob_densities\" MUST be an array\n");
        exit(EXIT_FAILURE);
      }

      for (const auto& p : ene_config["prob_densities"].GetArray()) {
        local.prob_densities.push_back( p.GetDouble() );
      }
    }
    if (ene_config.HasMember("interpolation_rule")) {
      local.interpolation_rule = ene_config["interpolation_rule"].GetString(); 
    }
  }
  else if (ene_mode == "ext_spectrum") {
    local.mode = EEnergyMode::kExtSpectrum; 
    local.energy_distribution_label = "th1"; 
    if (ene_config.HasMember("energy_distribution") == false) {
      fprintf(stderr, "ConfigureEnergy ERROR: energy spectrum set but no file or object info given\n");
      exit(EXIT_FAILURE); 
    }
    local.spectrum_hist.Configure( ene_config["energy_distribution"] );
  }

  return;
}

const rapidjson::Document SLArBaseGenerator::ExportEnergyConfig() const {
  rapidjson::Document doc; 
  doc.SetObject(); 

  const auto& econfig = fConfig.ene_config; 
  char buffer[100]; 
  int len = 1; 
  rapidjson::Value jmode; 

  if (econfig.mode == EEnergyMode::kFixed) {
    len = snprintf(buffer, sizeof(buffer), "fixed"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
    doc.AddMember("energy_MeV", econfig.energy_value, doc.GetAllocator()); 
  }
  else if (econfig.mode == EEnergyMode::kCustom) {
    len = snprintf(buffer, sizeof(buffer), "custom"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
    doc.AddMember("label", rapidjson::StringRef(econfig.energy_distribution_label.data()), doc.GetAllocator()); 
  }
  else if (econfig.mode == EEnergyMode::kExtSpectrum) {
    len = snprintf(buffer, sizeof(buffer), "spectrum"); 
    jmode.SetString(buffer, len, doc.GetAllocator()); 
    doc.AddMember("mode", jmode, doc.GetAllocator()); 
    const rapidjson::Document spectrum_doc = ExportEnergyConfig(); 
    rapidjson::Value spectrum_info; 
    spectrum_info.CopyFrom(spectrum_doc, doc.GetAllocator()); 
    doc.AddMember("spectrum_info", spectrum_info, doc.GetAllocator()); 
  }

  return doc; 
}

void SLArBaseGenerator::RegisterPrimaries(const G4Event* anEvent, const G4int firstVertex) {

  SLArRunAction* run_action = (SLArRunAction*)G4RunManager::GetRunManager()->GetUserRunAction();
  const G4Transform3D& world2LArVolume = run_action->GetTransformWorld2Det();

  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();
  G4IonTable* ionTable = G4IonTable::GetIonTable(); 

  G4int total_vertices = anEvent->GetNumberOfPrimaryVertex(); 



  if (fVerbose) {
    printf("[gen] %s primary generator action produced %i vertex(ices)\n", 
        fLabel.data(), total_vertices - firstVertex); 
  }
  for (int i=firstVertex; i<total_vertices; i++) {
    const G4PrimaryVertex* primary_vertex = anEvent->GetPrimaryVertex(i); 
    const G4int np = primary_vertex->GetNumberOfParticle(); 
    if (fVerbose) {
      printf("vertex %i has %i particles at t = %g\n", i, np, 
          primary_vertex->GetT0()); 
    }

    for (int ip = 0; ip<np; ip++) {
      auto particle = primary_vertex->GetPrimary(ip); 
       
      G4String name = ""; 
      SLArMCPrimaryInfo tc_primary;

      if (!particle->GetParticleDefinition()) {
        tc_primary.SetPDG  (particle->GetPDGcode()); 
        name = ionTable->GetIon( particle->GetPDGcode() )->GetParticleName(); 
        tc_primary.SetName(name);
        tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      } else {
        tc_primary.SetPDG  (particle->GetPDGcode());
        name = particle->GetParticleDefinition()->GetParticleName(); 
        tc_primary.SetName(name);
        tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      }

      tc_primary.SetTrackID(particle->GetTrackID());

      const HepGeom::Point3D<double> pos = anEvent->GetPrimaryVertex(i)->GetPosition();
      const HepGeom::Point3D<double> posLAr = world2LArVolume * pos;

      tc_primary.SetPosition(posLAr.x(), posLAr.y(), posLAr.z());

      tc_primary.SetMomentum(
          particle->GetPx(), particle->GetPy(), particle->GetPz(), 
          particle->GetKineticEnergy());
      tc_primary.SetTime(primary_vertex->GetT0()); 
      tc_primary.SetGeneratorLabel( fLabel.data() ); 

#ifdef SLAR_DEBUG
      //printf("Adding particle to primary output list\n"); 
      //tc_primary.PrintParticle(); 
      //getchar();
#endif
      SLArAnaMgr->GetMCTruth().RegisterPrimary( tc_primary );
    }
  }

}

void SLArBaseGenerator::SetGenRecord(SLArGenRecord& record, const GenConfig_t& config) const {
  record.GetGenCode() = this->GetGeneratorEnum(); 
  record.GetGenLabel() = this->GetLabel(); 

  auto& status = record.GetGenStatus(); 
  status.resize(4, 0.0); 
  status.at(0) = config.ene_config.energy_tmp;
  status.at(1) = fDirGen->GetTmpDirection().x(); 
  status.at(2) = fDirGen->GetTmpDirection().y(); 
  status.at(3) = fDirGen->GetTmpDirection().z(); 

  return; 
}

G4String SLArBaseGenerator::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document vtx_json = fVtxGen->ExportConfig();
  rapidjson::Document doc_gen; 
  doc_gen.Parse( fJSONConfigDump.data() );

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  G4String gen_type = GetGeneratorType(); 

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  rapidjson::Value gen_config;
  gen_config.CopyFrom(doc_gen, d.GetAllocator()); 
  d.AddMember("config", gen_config.GetObj(), d.GetAllocator()); 
  rapidjson::Value vtx_config;
  vtx_config.CopyFrom(vtx_json, d.GetAllocator());
  d.AddMember("vertex_generator", vtx_config, d.GetAllocator());
  d.Accept(writer);
  config_str = buffer.GetString();

  return config_str;
}

//template TH1D*   SLArBaseGenerator::GetFromRootfile<TH1D>  (const G4String& filename, const G4String& key); 
//template TH1F*   SLArBaseGenerator::GetFromRootfile<TH1F>  (const G4String& filename, const G4String& key); 
//template TH2D*   SLArBaseGenerator::GetFromRootfile<TH2D>  (const G4String& filename, const G4String& key); 
//template TH2F*   SLArBaseGenerator::GetFromRootfile<TH2F>  (const G4String& filename, const G4String& key); 
}

