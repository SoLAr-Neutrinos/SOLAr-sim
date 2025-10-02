/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArCRYGeneratorAction.cc
 * @created     : Wednesday Mar 13, 2024 17:49:49 CET
 */

#include "gen/SLArCRYGeneratorAction.hh"
#include "gen/SLArIsotropicDirectionGenerator.hh"
#include "gen/SLArPointVertexGenerator.hh"
#include "analysis/SLArAnalysisManager.hh"
#include "geo/SLArGeoUtils.hh"
#include "G4RandomTools.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4RandomTools.hh"

#include <cstdio>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

//----------------------------------------------------------------------------//
namespace gen {
namespace cry {
SLArCRYGeneratorAction::SLArCRYGeneratorAction(const G4String label) 
  : SLArBaseGenerator(label), fInputState(0)
{
  // define a particle gun
  particleGun = std::make_unique<G4ParticleGun>();

  // create a vector to store the CRY particle properties
  vect=new std::vector<CRYParticle*>;
}

//----------------------------------------------------------------------------//
void SLArCRYGeneratorAction::InputCRY()
{
  fInputState=1;
}

//----------------------------------------------------------------------------//
void SLArCRYGeneratorAction::UpdateCRY()
{
  G4cout << "Updating cry..." << G4endl;
  G4cout << fConfig.cry_mess_input << G4endl;
  G4cout << "CRYDATAPATH: " << std::getenv("CRYDATAPATH") << G4endl; 
  CRYSetup *setup=new CRYSetup(fConfig.cry_mess_input, std::getenv("CRYDATAPATH"));

  gen = std::make_unique<CRYGenerator>( setup );

  // set random number generator
  auto rndEngine = G4Random::getTheEngine(); 
  RNGWrapper<CLHEP::HepRandomEngine>::set(rndEngine,&CLHEP::HepRandomEngine::flat);
  setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);
  fInputState=0;

  fConfig.cry_mess_input = ""; 
}

//----------------------------------------------------------------------------//
//void SLArCRYGeneratorAction::CRYFromFile(G4String newValue)
//{
  //printf("SLArCRYGeneratorAction::CRYFromFile(%s)\n", newValue.data());
  //printf("Loading CRY configuration file..."); 
  //// Read the cry input file
  //std::ifstream inputFile;
  //inputFile.open(newValue,std::ios::in);
  //char buffer[1000];

  //if (inputFile.fail()) {
    //G4cout << "\nFailed to open input file " << newValue << G4endl;
    //G4cout << "Make sure to define the cry library on the command line" << G4endl;
    //fInputState=-1;
  //}else{
    //std::string setupString("");
    //while ( !inputFile.getline(buffer,1000).eof()) {
      //setupString.append(buffer);
      //setupString.append(" ");
    //}

    //CRYSetup *setup=new CRYSetup(setupString, std::getenv("CRYDATAPATH"));

    //// set random number generator
    //auto rndEngine = G4Random::getTheEngine();
    //RNGWrapper<CLHEP::HepRandomEngine>::set(rndEngine,&CLHEP::HepRandomEngine::flat);
    //setup->setRandomFunction(RNGWrapper<CLHEP::HepRandomEngine>::rng);

    //gen = std::make_unique<CRYGenerator>(setup);

    //fInputState=0;
  //}

  //printf(" DONE\n"); 
//}

//----------------------------------------------------------------------------//
void SLArCRYGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{ 
  if (fInputState != 0) {
    G4String* str = new G4String("CRY library was not successfully initialized");
    //G4Exception(*str);
    G4Exception("SLArCRYGeneratorAction", "1",
                RunMustBeAborted, *str);
  }
  G4String particleName;

  auto particleTable = G4ParticleTable::GetParticleTable();
  G4ThreeVector vertex(0, 0, 0);
  G4ThreeVector direction(0, 0, 0);

  G4EventManager* evManager = G4EventManager::GetEventManager(); 
  int verbose = evManager->GetVerboseLevel(); 
  if (verbose) {
    G4cout << "\nEvent=" << anEvent->GetEventID() << " "
      << "CRY generated nparticles=" << vect->size()
      << G4endl;
  }

  bool target_hit = false;
  bool track_hit = false;

  int cosmics_counter = 0; 

  SLArAnalysisManager* slar_ana_mgr = SLArAnalysisManager::Instance();
  auto& gen_status_vec = slar_ana_mgr->GetGenRecords();

  while (cosmics_counter < fConfig.n_particles) {
    vect->clear();
    gen->genEvent(vect);
    for ( unsigned j=0; j<vect->size(); j++) {
      particleName=CRYUtils::partName((*vect)[j]->id());

      if (verbose) {
        G4cout << "  "          << particleName << " "
          << "charge="      << (*vect)[j]->charge() << " "
          << std::setprecision(4)
          << "energy (MeV)=" << (*vect)[j]->ke()*CLHEP::MeV << " "
          << "pos (m)"
          << G4ThreeVector((*vect)[j]->x(), fConfig.cry_gen_y, (*vect)[j]->y())
          << " " << "direction cosines "
          << G4ThreeVector((*vect)[j]->u(), (*vect)[j]->w(), (*vect)[j]->v())
          << " " << G4endl;
      }

      fVtxGen->ShootVertex(vertex);
      direction.set((*vect)[j]->u(), (*vect)[j]->w(), (*vect)[j]->v());

      if (fConfig.volume_crossing.empty() == false) {
        track_hit = geo::track_crosses_volume(vertex, direction, fConfig.volume_crossing);
      }
      else {
        track_hit = true;
      }
      
      if (track_hit == false) {
        delete (*vect)[j];
        continue;
      }
      else {
        target_hit = true;
        particleGun->SetParticleDefinition(particleTable->FindParticle((*vect)[j]->PDGid()));
        particleGun->SetParticleEnergy((*vect)[j]->ke()*CLHEP::MeV);
        particleGun->SetParticlePosition( vertex );
        particleGun->SetParticleMomentumDirection( direction );
        particleGun->SetParticleTime( fVtxGen->GetTimeGenerator().SampleTime() );
        particleGun->GeneratePrimaryVertex(anEvent);
        delete (*vect)[j];

        // Store CRY direction in direction generator variable
        fDirGen->SetTmpDirection(direction);
        fConfig.ene_config.energy_tmp = particleGun->GetParticleEnergy();

        // Export energy and direction in gen record
        gen_status_vec.AddRecord(GetGeneratorEnum(), fLabel);

        cosmics_counter++;
        printf("[gen] %s primary generator action produced %i vertex(ices)\n", 
            fLabel.data(), cosmics_counter);
      }
    }
  }
  return;
}

void SLArCRYGeneratorAction::CRYConfig_t::activate_particle(const G4String particle_name) {
  if (activeParticles.find( particle_name ) == activeParticles.end()) {
    char err_msg[100]; 
    snprintf(err_msg, sizeof(err_msg), "WARNING: %s is not among cry available primaries\n", 
        particle_name.data());
    std::cerr << err_msg << std::endl;
    return;
  }
  activeParticles[particle_name] = true;

  return;
}
void SLArCRYGeneratorAction::CRYConfig_t::to_input() {
  cry_mess_input.clear(); 

  for (const auto& p : activeParticles) {
    G4String particle = p.first; 
    particle[0] = std::toupper(particle[0]); 
    char line[50]; 
    snprintf(line, sizeof(line), "return%s %i ", particle.data(), p.second);
    cry_mess_input.append(line);
  }
  cry_mess_input.append(" "); 

  G4String line_min = "nParticlesMin " + std::to_string(n_particles_min) + " "; 
  cry_mess_input.append( line_min ); 

  G4String line_max = "nParticlesMax " + std::to_string(n_particles_max) + " "; 
  cry_mess_input.append( line_max ); 

  G4String line_gen_size = "subboxLength " 
    + std::to_string( (box_lenght / CLHEP::m) ) + " "; 
  cry_mess_input.append( line_gen_size ); 

  G4String line_date = "date " + date + " "; 
  cry_mess_input.append( line_date ); 

  G4String line_latitude = "latitude " + std::to_string(latitude) + " "; 
  cry_mess_input.append( line_latitude );

  G4String line_altitude = "altitude " + std::to_string(altitude) + " "; 
  cry_mess_input.append( line_altitude );
}

void SLArCRYGeneratorAction::SourceConfiguration(const rapidjson::Value& config) {
  CopyConfigurationToString(config);
  SourceCommonConfig(config, fConfig);

  if ( !config.HasMember("particles")) {
    throw std::invalid_argument("cry configuration missing mandatory \"particles\" field\n");
  } else {
    const auto& particles = config["particles"]; 
    if (particles.IsArray()) {
      for (const auto& p : particles.GetArray()) {
        const G4String particle_name = p.GetString(); 
        fConfig.activate_particle( particle_name ); 
      }
    }
    else if (particles.IsString()) {
      const G4String particle_name = particles.GetString(); 
      fConfig.activate_particle( particle_name ); 
    }
    else {
      throw std::invalid_argument("ERROR: unsupported \"particles\" format\n"); 
      return;
    }
  }

  if (config.HasMember("date")) {
    fConfig.date = config["date"].GetString();
  }

  if (config.HasMember("latitude")) {
    fConfig.latitude = config["latitude"].GetDouble(); 
  }

  if (config.HasMember("altitude")) {
    G4int altitude = config["altitude"].GetInt();
    if (altitude != 0 && altitude != 2100 && altitude != 11300) {
      printf("WARNING: cry altitude value not allowed. Please choose between 0, 2100, 11300\n"); 
      altitude = 0;
    }
    fConfig.altitude = altitude;
  }

  if (config.HasMember("n_particles")) {
    fConfig.n_particles = config["n_particles"].GetInt();
  }
  if (config.HasMember("n_particles_min")) {
    fConfig.n_particles_min = config["n_particles_min"].GetInt(); 
  }
  if (config.HasMember("n_particles_max")) {
    fConfig.n_particles_max = config["n_particles_max"].GetInt(); 
  }

  if (config.HasMember("box_length")) {
    fConfig.box_lenght = unit::ParseJsonVal( config["box_length"] ); 
  }

  if (config.HasMember("generator_y")) {
    fConfig.cry_gen_y = unit::ParseJsonVal( config["generator_y"] ); 
  }

  if (config.HasMember("force_volume_crossing")) {
    fConfig.volume_crossing = config["force_volume_crossing"].GetString(); 
  }

  if (fVtxGen == nullptr) {
    fVtxGen = std::make_unique<vertex::SLArPointVertexGenerator>();
  }

  fDirGen = std::make_unique<direction::SLArIsotropicDirectionGenerator>();

  return;
}

void SLArCRYGeneratorAction::Configure() {
  fConfig.to_input(); 
  UpdateCRY(); 
}

/*
 *G4String SLArCRYGeneratorAction::WriteConfig() const {
 *  G4String config_str = "";
 *
 *  rapidjson::Document d; 
 *  d.SetObject(); 
 *  rapidjson::StringBuffer buffer;
 *  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
 *
 *  G4String gen_type = GetGeneratorType(); 
 *
 *  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
 *  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
 *  rapidjson::Value jparticles(rapidjson::kArrayType); 
 *  for (const auto& p : fConfig.activeParticles) {
 *    if (p.second) {
 *      jparticles.PushBack( rapidjson::StringRef(p.first.data()), d.GetAllocator()); 
 *    }
 *  }
 *  d.AddMember("particles", jparticles, d.GetAllocator()); 
 *  d.AddMember("latitude", fConfig.latitude, d.GetAllocator()); 
 *  d.AddMember("n_particles_min", fConfig.n_particles_min, d.GetAllocator()); 
 *  d.AddMember("n_particles_max", fConfig.n_particles_max, d.GetAllocator()); 
 *  d.AddMember("box_size", fConfig.box_lenght, d.GetAllocator()); 
 *  d.AddMember("date", rapidjson::StringRef( fConfig.date.data()), d.GetAllocator()); 
 *  d.AddMember("altitude", fConfig.altitude, d.GetAllocator()); 
 *  d.AddMember("vertex_generator_y", fConfig.cry_gen_y, d.GetAllocator()); 
 *  d.AddMember("force_volume_crossing", rapidjson::StringRef(fConfig.volume_crossing.data()), d.GetAllocator());
 *
 *  d.Accept(writer);
 *  config_str = buffer.GetString();
 *  return config_str;
 *}
 *
 */
}  
}
