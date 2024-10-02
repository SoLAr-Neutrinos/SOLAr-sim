/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArPrimaryGeneratorAction
 * @created     : Sunday Mar 31, 2024 20:22:05 CEST
 */


#include <SLArAnalysisManager.hh>

#include <SLArPrimaryGeneratorAction.hh>
#include <SLArPrimaryGeneratorMessenger.hh>
#include <SLArBulkVertexGenerator.hh>
#include <SLArBoxSurfaceVertexGenerator.hh>
#include <SLArPGunGeneratorAction.hh>
#include <SLArPBombGeneratorAction.hh>
#include <SLArMarleyGeneratorAction.hh>
#include <SLArDecay0GeneratorAction.hh>
#include <SLArExternalGeneratorAction.hh>
//#include <SLArBackgroundGeneratorAction.hh>
#include <SLArGENIEGeneratorAction.hh>
#ifdef SLAR_CRY
#include <cry/SLArCRYGeneratorAction.hh>
#endif
#ifdef SLAR_RADSRC
#include <radsrc/SLArRadSrcGeneratorAction.hh>
#endif

#include <Randomize.hh>
#include <SLArRandomExtra.hh>

#include <G4VSolid.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4Event.hh>
#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4ParticleDefinition.hh>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace gen {

SLArPrimaryGeneratorAction::SLArPrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(), 
   //fVolumeName(""), 
   //fGeneratorEnum(kParticleGun), 
   //fGenPosition(0, 0, 0),
   //fGenDirection(0, 0, 1), 
   fDoTraceOptPhotons(true), 
   fDoDriftElectrons(true), 
   fVerbose(0)
{
  //create a messenger for this class
  fGunMessenger = new SLArPrimaryGeneratorMessenger(this);
}

SLArPrimaryGeneratorAction::SLArPrimaryGeneratorAction(const rapidjson::Value& config) 
: SLArPrimaryGeneratorAction()
{
  Configure( config ); 

  return;
}

SLArPrimaryGeneratorAction::SLArPrimaryGeneratorAction(const G4String config_file_path) 
: SLArPrimaryGeneratorAction() 
{
  FILE* config_file = std::fopen(config_file_path, "r");
  char readBuffer[65536];
  rapidjson::FileReadStream is(config_file, readBuffer, sizeof(readBuffer));

  rapidjson::Document d;
  d.ParseStream<rapidjson::kParseCommentsFlag>(is);
  assert(d.IsObject());

  Configure( d ); 

  std::fclose( config_file ); 
}

void SLArPrimaryGeneratorAction::Configure(const G4String config_file_path) {
  FILE* gen_cfg_file = std::fopen(config_file_path, "r");
  if (gen_cfg_file == nullptr) {
    fprintf(stderr, "SLArPrimaryGeneratorAction::Configure ERROR: Cannot open configuration file %s",
        config_file_path.data());
    exit( EXIT_FAILURE ); 
  }
  char readBuffer[65536];
  rapidjson::FileReadStream is(gen_cfg_file, readBuffer, sizeof(readBuffer));

  rapidjson::Document d;
  d.ParseStream<rapidjson::kParseCommentsFlag>(is);
  assert(d.IsObject());

  Configure( d );

  fclose( gen_cfg_file ); 
  return;
}  

void SLArPrimaryGeneratorAction::Configure(const rapidjson::Value& configuration) {
  if (fGeneratorActions.empty() == false) {
    Reset();
  }
  assert( configuration.HasMember("generator") );

  const auto& gen_list = configuration["generator"];

  if (gen_list.IsArray()) {
    for (const auto& gen_config : gen_list.GetArray()) {
      try {
        AddGenerator(gen_config); 
      }
      catch (const std::invalid_argument& e) {
        std::cerr << "SLArPrimaryGeneratorAction::Configure ERROR:" << std::endl;
        std::cerr << e.what() <<std::endl;
        exit( EXIT_FAILURE ); 
      }
    }
  } 
  else if (gen_list.IsObject()) {
    try {AddGenerator( gen_list );}
    catch (const std::invalid_argument& e) {
      std::cerr << "SLArPrimaryGeneratorAction::Configure ERROR:" << std::endl;
      std::cerr << e.what() <<std::endl;
      exit( EXIT_FAILURE ); 
    }
  }

  return;
}

void SLArPrimaryGeneratorAction::Reset() {
  printf("SLArPrimaryGeneratorAction::Reset() Clear current generator settings...\n");
  for (auto &gen : fGeneratorActions) {
    printf("[gen] Removing %s...\n", gen.first.data());
    if (gen.second) delete gen.second;
  }

  fGeneratorActions.clear(); 
}

void SLArPrimaryGeneratorAction::AddGenerator(const rapidjson::Value& jgen) {
  if ( !jgen.HasMember("type"  ) ) {
    throw std::invalid_argument("Generator spec does not include mandatory \"type\" field");
  }
  if ( !jgen.HasMember("label" ) ) {
    throw std::invalid_argument("Generator spec does not include mandatory \"label\" field");
  }
  if( !jgen.HasMember("config") ) {
    throw std::invalid_argument("Generator spec does not include mandatory \"config\" field");
  };

  G4String label = jgen["label"].GetString(); 
  G4String type  = jgen["type"].GetString(); 

  EGenerator kGenIdx = gen::GetGeneratorIndex( type ); 
  SLArBaseGenerator* this_gen = nullptr;

  printf("[gen] Adding %s generator with label %s...\n", type.data(), label.data());

  switch (kGenIdx) {
    case (kParticleGun) : 
      {
        auto gen = new SLArPGunGeneratorAction(label); 
        gen->Configure( jgen["config"] ); 
        this_gen = gen; 
        break;
      }

    case (kParticleBomb) : 
      {
        auto gen = new SLArPGunGeneratorAction(label); 
        gen->Configure( jgen["config"] ); 
        this_gen = gen; 
        break;
      }

    case (kDecay0) : 
      {
        auto gen = new bxdecay0_g4::SLArDecay0GeneratorAction(label); 
        gen->Configure( jgen["config"] ); 
        this_gen = gen; 
        break;
      }

    case (kMarley) : 
      {
        auto gen = new marley::SLArMarleyGeneratorAction(label); 
        gen->Configure( jgen["config"] ); 
        this_gen = gen;
        break;
      }

    case (kExternalGen) : 
      {
        auto gen = new SLArExternalGeneratorAction(label); 
        gen->Configure( jgen["config"] ); 
        this_gen = gen; 
        break;
      }

    case (kGENIE) : 
      {
        auto gen = new SLArGENIEGeneratorAction(label); 
        gen->Configure( jgen["config"] ); 
        this_gen = gen; 
        break;
      }

#ifdef SLAR_CRY
    case (kCRY) : 
      {
        auto gen = new cry::SLArCRYGeneratorAction(label); 
        gen->Configure( jgen["config"] ); 
        this_gen = gen; 
        break;
      }
#endif 

#ifdef SLAR_RADSRC
    case (kRadSrc) : 
      {
        auto gen = new radsrc::SLArRadSrcGeneratorAction(label);
        gen->Configure( jgen["config"] ); 
        this_gen = gen;
        break;
      }
#endif

    default:
      {
        gen::printGeneratorType(); 
        char err_msg[200]; 
        sprintf(err_msg, "Unable to find %s primary gen among available options.\n", 
            type.data()); 
        throw std::invalid_argument(err_msg); 
      }
  }

  fGeneratorActions.insert(std::make_pair(label, this_gen)); 
  return;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPrimaryGeneratorAction::~SLArPrimaryGeneratorAction()
{
  printf("Deleting SLArPrimaryGeneratorAction...\n");
  int igen = -1; 
  for (auto &gen : fGeneratorActions) {
    igen = gen.second->GetGeneratorEnum();
    printf("igen = %i\n", igen);
    if (gen.second) {
      printf("Deleting gen %i\n", igen);
      if (igen == kParticleGun) {
        SLArPGunGeneratorAction* local = (SLArPGunGeneratorAction*)gen.second;
        delete local;
      }
      else if (igen == kParticleBomb) {
        SLArPBombGeneratorAction* local = (SLArPBombGeneratorAction*)gen.second;
        delete local;
      }
      else if (igen == kDecay0) {
        auto local = (bxdecay0_g4::SLArDecay0GeneratorAction*)gen.second;
        delete local;
      }
      else if (igen == kMarley) {
        auto local = (marley::SLArMarleyGeneratorAction*)gen.second;
        delete local;
      }
      //else if (igen == kBackground) {
        //auto local = (SLArBackgroundGeneratorAction*)gen.second;
        //delete local;
      //}
      else if (igen == kExternalGen) {
        auto local = (SLArExternalGeneratorAction*)gen.second;
        delete local;
      }
      else if (igen == kGENIE) {
        auto local = (SLArGENIEGeneratorAction*)gen.second;
        delete local; 
      }
#ifdef SLAR_CRY
      else if (igen == kCRY) {
        auto local = (cry::SLArCRYGeneratorAction*)gen.second; 
        delete local; 
      }
#endif
#ifdef SLAR_RADSRC
      else if (igen == kRadSrc) {
        auto local = (radsrc::SLArRadSrcGeneratorAction*)gen.second;
        delete local;
      }
#endif 
      //gen = nullptr;
    }
  }
  //if (fBulkGenerator) delete fBulkGenerator;
  //if (fBoxGenerator) delete fBoxGenerator;
  if (fGunMessenger) delete fGunMessenger;
  printf("DONE\n");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//void SLArPrimaryGeneratorAction::ConfigBulkVertexGen(G4String vol) {
  //fBulkGenerator->Config( vol ); 
  //return;
//}

//void SLArPrimaryGeneratorAction::ConfigBoxSurfaceVertexGen(G4String vol) {
  //fBoxGenerator->Config( vol ); 
//}

//void SLArPrimaryGeneratorAction::SetGunParticle(const G4String particle_name) 
//{
  //SLArPGunGeneratorAction* particle_gun = 
    //(SLArPGunGeneratorAction*)fGeneratorActions[kParticleGun]; 
  //particle_gun->SetParticle(particle_name); 

  //SLArPBombGeneratorAction* particle_bomb = 
    //(SLArPBombGeneratorAction*)fGeneratorActions[kParticleBomb]; 
  //particle_bomb->SetParticle(particle_name); 
//}

//void SLArPrimaryGeneratorAction::SetGunEnergy(const G4double ekin) 
//{
  //SLArPGunGeneratorAction* particle_gun = 
    //(SLArPGunGeneratorAction*)fGeneratorActions[kParticleGun]; 
  //particle_gun->SetParticleKineticEnergy(ekin); 

  //SLArPBombGeneratorAction* particle_bomb = 
    //(SLArPBombGeneratorAction*)fGeneratorActions[kParticleBomb]; 
  //particle_bomb->SetParticleKineticEnergy(ekin); 
//}

//void SLArPrimaryGeneratorAction::SetGunNumberOfParticles(const G4int n) 
//{
  ////SLArPGunGeneratorAction* particle_gun = 
    ////(SLArPGunGeneratorAction*)fGeneratorActions[kParticleGun]; 
  ////particle_gun->SetNumberOfParticles(n); 

  //SLArPBombGeneratorAction* particle_bomb = 
    //(SLArPBombGeneratorAction*)fGeneratorActions[kParticleBomb]; 
  //particle_bomb->SetNumberOfParticles(n); 
//}


void SLArPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // Store Primary information id dst
  //SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();

  //if (fDirectionMode == kRandom) {
    //fGenDirection = SampleRandomDirection();
  //}

  //if (fBulkGenerator->GetBulkLogicalVolume()) {
      //fBulkGenerator->ShootVertex( fGunPosition ); 
      //if (fVerbose) {
        //printf("Gun position: %.2f, %.2f, %.2f\n", 
            //fGunPosition.x(), fGunPosition.y(), fGunPosition.z()); 
      //}
  //}  
 
  //G4IonTable* ionTable = G4IonTable::GetIonTable(); 

  for (const auto& gen : fGeneratorActions) {
    G4int previousNrOfVertices = anEvent->GetNumberOfPrimaryVertex();
    gen.second->GeneratePrimaries( anEvent ); 
    gen.second->RegisterPrimaries( anEvent, previousNrOfVertices ); 
  }

  //G4int n = anEvent->GetNumberOfPrimaryVertex(); 

  //if (fVerbose) {
    //printf("Primary Generator Action produced %i vertex(ices)\n", n); 
  //}
  //for (int i=0; i<n; i++) {
    ////std::unique_ptr<SLArMCPrimaryInfoUniquePtr> tc_primary = std::make_unique<SLArMCPrimaryInfoUniquePtr>();
    //SLArMCPrimaryInfo tc_primary;
    //G4int np = anEvent->GetPrimaryVertex(i)->GetNumberOfParticle(); 
    //if (fVerbose) {
      //printf("vertex %i has %i particles at t = %g\n", n, np, 
          //anEvent->GetPrimaryVertex(i)->GetT0()); 
    //}
    //for (int ip = 0; ip<np; ip++) {
      ////printf("getting particle %i...\n", ip); 
      //auto particle = anEvent->GetPrimaryVertex(i)->GetPrimary(ip); 
      //G4String name = ""; 

      //if (!particle->GetParticleDefinition()) {
        //tc_primary.SetID  (particle->GetPDGcode()); 
        //name = ionTable->GetIon( particle->GetPDGcode() )->GetParticleName(); 
        //tc_primary.SetName(name);
        //tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      //} else {
        //tc_primary.SetID  (particle->GetPDGcode());
        //name = particle->GetParticleDefinition()->GetParticleName(); 
        //tc_primary.SetName(name);
        //tc_primary.SetTitle(name + " [" + particle->GetTrackID() +"]"); 
      //}

      //tc_primary.SetTrackID(particle->GetTrackID());
      //tc_primary.SetPosition(anEvent->GetPrimaryVertex(i)->GetX0(),
          //anEvent->GetPrimaryVertex(i)->GetY0(), 
          //anEvent->GetPrimaryVertex(i)->GetZ0());
      //tc_primary.SetMomentum(
          //particle->GetPx(), particle->GetPy(), particle->GetPz(), 
          //particle->GetKineticEnergy());
      //tc_primary.SetTime(anEvent->GetPrimaryVertex(i)->GetT0()); 
      

//#ifdef SLAR_DEBUG
      //printf("Adding particle to primary output list\n"); 
      //tc_primary.PrintParticle(); 
      //getchar();
//#endif
      //SLArAnaMgr->GetEvent().RegisterPrimary( tc_primary );
    //}
  //}

  return;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//void SLArPrimaryGeneratorAction::SetMarleyConf(G4String marley_conf) {
  //fMarleyCfg = marley_conf; 
  //delete fGeneratorActions[kMarley]; 
  //fGeneratorActions[kMarley] = new marley::SLArMarleyGeneratorAction(fMarleyCfg); 

  //marley::SLArMarleyGeneratorAction* marley_gen = 
      //(marley::SLArMarleyGeneratorAction*)fGeneratorActions[kMarley]; 
    ////marley_gen->SetVertexGenerator(fBulkGenerator); 
  //return; 
//}

//void SLArPrimaryGeneratorAction::SetExternalConf(G4String external_cfg) {
  //SLArExternalGeneratorAction* gen = 
    //(SLArExternalGeneratorAction*)fGeneratorActions[kExternalGen]; 
  ////gen->SetVertexGenerator(fBulkGenerator);
  //gen->SourceExternalConfig(external_cfg); 
  //return; 
//}

//void SLArPrimaryGeneratorAction::SetBackgroundConf(G4String background_conf)
//{
  //fIncludeBackground = true; 
  //fBackgoundModelCfg = background_conf; 
  //SLArBackgroundGeneratorAction* bkgGen = 
    //(SLArBackgroundGeneratorAction*)fGeneratorActions[kBackground];
   //bkgGen->BuildBackgroundTable(fBackgoundModelCfg);
   //return;
//}

//void SLArPrimaryGeneratorAction::SetGENIEEvntExt(G4int evntID) { // --JM
  //// Set event ID to start simulation at
  //printf("Setting GENIE starting event ID to %i.",evntID);
  //fGENIEEvntNum = evntID;
//}

//void SLArPrimaryGeneratorAction::SetGENIEFile(G4String filename) { // --JM
  //printf("Setting GENIE file as:\n\t %s.",filename.data());
  //fGENIEFile = filename;
//}

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
