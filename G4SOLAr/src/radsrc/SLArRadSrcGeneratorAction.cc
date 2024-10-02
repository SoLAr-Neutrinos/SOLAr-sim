/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRadSrcGeneratorAction.cc
 * @created     : Friday May 03, 2024 10:35:08 CEST
 */


#include <radsrc/SLArRadSrcGeneratorAction.hh>
#include <G4RandomTools.hh>
#include <G4EventManager.hh>
#include <G4Event.hh>
#include <G4ParticleTable.hh>
#include <G4Geantino.hh>
#include <G4RandomTools.hh>

#include <cstdio>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

//----------------------------------------------------------------------------//
namespace gen {
namespace radsrc {
SLArRadSrcGeneratorAction::SLArRadSrcGeneratorAction(const G4String label) 
  : SLArBaseGenerator(label), fInputState(0)
{
  // define a particle gun
  particleGun = std::make_unique<G4ParticleGun>();

  // load it with gammas
  G4ParticleDefinition* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  particleGun->SetParticleDefinition( gamma ); 

}

//----------------------------------------------------------------------------//
void SLArRadSrcGeneratorAction::InputRadSrc()
{
  fInputState=1;
}

//----------------------------------------------------------------------------//
void SLArRadSrcGeneratorAction::UpdateRadSrc()
{
  // instance the radsrc generator
  gen = std::unique_ptr<::radsrc::CRadSource>(::radsrc::CApi::newSource());

  G4cout << "Updating RadSrc..." << G4endl;
  G4cout << fConfig.radsrc_mess_input << G4endl;
  ::radsrc::CApi::addConfig(gen.get(), fConfig.radsrc_mess_input);
  G4bool status = ::radsrc::CApi::sourceConfig(gen.get());

  if (status == false) {
    throw std::runtime_error("SLArRadSrcGeneratorAction ERROR in RadSrc configuration\n");
  }

  fInputState=0;

  fConfig.radsrc_mess_input = ""; 
}

//----------------------------------------------------------------------------//
G4double SLArRadSrcGeneratorAction::local_rand(void)  {
  return G4UniformRand();
}

//----------------------------------------------------------------------------//
void SLArRadSrcGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{ 
  if (fInputState != 0) {
    G4String* str = new G4String("RadSrc library was not successfully initialized");
    //G4Exception(*str);
    G4Exception("SLArRadSrcGeneratorAction", "1",
                RunMustBeAborted, *str);
  }

  G4EventManager* evManager = G4EventManager::GetEventManager(); 
  int verbose = evManager->GetVerboseLevel(); 
  if (verbose) {
    G4cout << "\nEvent=" << anEvent->GetEventID() << " "
      << G4endl;
  }

  if(gen == NULL){
    G4Exception("ExN01PrimaryGeneratorAction::GeneratePrimaries",
		"RADSRC has not been defined",RunMustBeAborted,
		" see the interface manual to insure that your input is formatted correctly");
  }

  G4double energy = ::radsrc::CApi::getPhoton(gen.get(), local_rand)  * CLHEP::keV;
  particleGun->SetParticleEnergy(energy);
  //
  G4double phi = 2.0*CLHEP::pi*G4UniformRand();
  G4double costheta = 2.0*G4UniformRand()-1.0;
  G4double sintheta = pow(1.0-pow(costheta,2.0),0.5);
  G4ThreeVector vtx;
  fVtxGen->ShootVertex( vtx );
  particleGun->SetParticlePosition( vtx );
  
  phi = 2.0*CLHEP::pi*G4UniformRand();
  costheta = 2.0*G4UniformRand()-1.0;
  sintheta = sqrt(1.0-costheta*costheta);
  particleGun->SetParticleMomentumDirection(G4ThreeVector(costheta,sintheta*cos(phi),sintheta*sin(phi)));
  particleGun->GeneratePrimaryVertex(anEvent);
}

void SLArRadSrcGeneratorAction::RadSrcConfig_t::to_input() {
  radsrc_mess_input.clear(); 

  for (const auto& p : isotopes) {
    char line[50]; 
    sprintf(line, "%s %g\n", p.first.data(), p.second);
    radsrc_mess_input.append(line);
  }
  radsrc_mess_input.append("\n"); 

  G4String line_age = "AGE " + std::to_string(age) + " " + "\n\n"; 
  radsrc_mess_input.append( line_age ); 

  G4String line_brem = "brem ";
  (add_brem) ? line_brem += "on" : line_brem += "off";
  line_brem += "\n\n";
  radsrc_mess_input.append( line_brem ); 

  if (min_energy != 0 && max_energy != 0) {
    G4String line_range = "range " 
      + std::to_string( min_energy ) + " " 
      + std::to_string( max_energy ) + "\n\n"; 
    radsrc_mess_input.append( line_range ); 
  }
  
  return;
 }

void SLArRadSrcGeneratorAction::Configure(const rapidjson::Value& config) {
  if ( !config.HasMember("isotopes")) {
    throw std::invalid_argument("radsrc configuration missing mandatory \"isotopes\" field\n");
  } else {
    const auto& isotopes = config["isotopes"]; 
    if (isotopes.IsArray()) {
      for (const auto& p : isotopes.GetArray()) {
        if (p.HasMember("name") == false || 
            p.HasMember("fraction") == false) {
          throw std::invalid_argument("SLArRadSrcGeneratorAction ERROR: isotope entries must have \"name\" and \"fraction\" fields\n"); 
        }
        else {
          const G4String name = p["name"].GetString();
          const G4double frac = p["fraction"].GetDouble();
          fConfig.isotopes.insert( {name, frac} ); 
        }
      }
    }
    else {
      throw std::invalid_argument("ERROR: \"isotopes\" field must be an array\n"); 
      return;
    }
  }

  if (config.HasMember("age")) {
    fConfig.age = config["age"].GetDouble();
  }

  if (config.HasMember("brem")) {
    fConfig.add_brem = config["brem"].GetBool(); 
  }

  if (config.HasMember("range")) {
    auto& jrange = config["range"];
    if ( jrange.IsArray() == false ) {
      throw std::invalid_argument("ERROR: radsrc \"range\" field must be a size=2 array\n");
    }
    else {
      if (jrange.GetArray().Size() != 2) {
        throw std::invalid_argument("ERROR: radsrc \"range\" field must be a size=2 array\n");
      }
      fConfig.min_energy = jrange.GetArray()[0].GetDouble(); 
      fConfig.max_energy = jrange.GetArray()[1].GetDouble();
    }
  }

  if (config.HasMember("vertex_gen")) {
    ConfigureVertexGenerator( config["vertex_gen"] ); 
  }
  else {
    fVtxGen = std::make_unique<SLArPointVertexGenerator>();
  }
  
  fConfig.to_input(); 

  UpdateRadSrc(); 
  
  return;
}

G4String SLArRadSrcGeneratorAction::WriteConfig() const {
  G4String config_str = "";

  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

  G4String gen_type = GetGeneratorType(); 

  d.AddMember("type" , rapidjson::StringRef(gen_type.data()), d.GetAllocator()); 
  d.AddMember("label", rapidjson::StringRef(fLabel.data()), d.GetAllocator()); 
  rapidjson::Value jisotopes(rapidjson::kArrayType); 
  for (const auto& p : fConfig.isotopes) {
    if (p.second) {
      rapidjson::Value jisotope(rapidjson::kObjectType); 
      jisotope.AddMember("name", rapidjson::StringRef(p.first.data()), d.GetAllocator()); 
      jisotope.AddMember("fraction", p.second, d.GetAllocator());
      jisotopes.PushBack( jisotope, d.GetAllocator()); 
    }
  }
  d.AddMember("isotopes", jisotopes, d.GetAllocator()); 
  d.AddMember("age", fConfig.age, d.GetAllocator()); 
  d.AddMember("add_brem", fConfig.add_brem, d.GetAllocator()); 
  if (fConfig.min_energy != 0 && fConfig.max_energy != 0) {
    rapidjson::Value jrange(rapidjson::kArrayType); 
    jrange.PushBack(fConfig.min_energy, d.GetAllocator()); 
    jrange.PushBack(fConfig.max_energy, d.GetAllocator()); 
    d.AddMember("range", jrange, d.GetAllocator());
  }

  const rapidjson::Document vtx_json = fVtxGen->ExportConfig(); 
  rapidjson::Value vtx_config;
  vtx_config.CopyFrom(vtx_json, d.GetAllocator(), true); 
  d.AddMember("vertex_generator", vtx_config, d.GetAllocator()); 

  d.Accept(writer);
  config_str = buffer.GetString();
  return config_str;
}

}  
}
