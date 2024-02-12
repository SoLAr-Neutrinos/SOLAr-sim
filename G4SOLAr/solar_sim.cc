/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        solar_sim.cc
 * @created     Mon Jun 19, 2023 18:11:25 CEST
 */

#include <sstream>
#include <regex>
#include <getopt.h>
#include "G4Types.hh"

#ifdef G4MULTITHREADED
#include "G4MTRunManager.hh"
#else
#include "G4RunManager.hh"
#endif

#include "G4UImanager.hh"

#include "SLArUserPath.hh"
#include "SLArAnalysisManager.hh"
#include "SLArPhysicsList.hh"
#include "SLArDetectorConstruction.hh"
#include "SLArActionInitialization.hh"
#include "SLArRunAction.hh"

#include "G4GenericBiasingPhysics.hh"
#include "G4ImportanceBiasing.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
namespace {
  void PrintUsage() {
    G4cerr << " Usage: " << G4endl;
    fprintf(stderr, " solar_sim\t[-m/--macro macro_file]]\n");
    fprintf(stderr, " \t\t[-o/--output output_file_name]\n");
    fprintf(stderr, " \t\t[-d/--output_dir output_dir]\n");
    fprintf(stderr, " \t\t[-u/--session session]\n");
    fprintf(stderr, " \t\t[-r/--seed user_seed]\n");
    fprintf(stderr, " \t\t[-g/--geometry geometry_cfg_file]\n");
    fprintf(stderr, " \t\t[-p/--materials material_db_file]\n");
    fprintf(stderr, " \t\t[-b/--bias particle <process_list> bias_factor]\n");
    fprintf(stderr, " \t\t[-h/--help print usage]\n");
  }
  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  void setup_bias(const std::string val, G4String& particle_name, G4double& bias_factor, std::vector<G4String>& process_name) {
    printf("solar_sim::setup_bias(%s)\n", val.c_str());
    std::stringstream sstream; 
    sstream << val; 
    G4String process_tmp; 
    std::regex pattern("\\d+(\\.\\d+)?([eE]\\d+)?"); 

    sstream >> particle_name;

    auto is_numeric = [pattern](G4String str) {
      return std::regex_match(str, pattern);  
    }; 

    while (sstream >> process_tmp) {
      if (is_numeric(process_tmp)) {
        bias_factor = std::atof(process_tmp.data()); 
      } 
      else process_name.push_back(process_tmp); 
    }

    return;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
int main(int argc,char** argv)
{

  G4String macro;
  G4String session;
  G4String output = ""; 
  G4String output_dir = ""; 
  G4String geometry_file = "./assets/geometry/geometry.json"; 
  G4String material_file = "./assets/materials/materials_db.json"; 
  G4bool   do_cerenkov = true; 
  G4bool   do_bias = false; 
  G4String bias_particle = ""; 
  G4double bias_factor = 1; 
  std::vector<G4String> bias_process;

#ifdef G4MULTITHREADED
  G4int nThreads = 0;
#endif

  G4long myseed = 345354;
  const char* short_opts = "m:o:d:u:t:r:g:p:b:c:h";
  static struct option long_opts[12] = 
  {
    {"macro", required_argument, 0, 'm'}, 
    {"output", required_argument, 0, 'o'}, 
    {"output_dir", required_argument, 0, 'd'}, 
    {"session", required_argument, 0, 'u'}, 
    {"threads", required_argument, 0, 't'}, 
    {"seed", required_argument, 0, 'r'}, 
    {"geometry", required_argument, 0, 'g'}, 
    {"materials", required_argument, 0, 'p'},
    {"bias", required_argument, 0, 'b'},
    {"cerenkov", required_argument, 0, 'c'},
    {"help", no_argument, 0, 'h'}, 
    {nullptr, no_argument, nullptr, 0}
  };

  int c, option_index; 

  while ( (c = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1) {
    switch(c) {
      case 'm' : 
      {
        macro = optarg;
        printf("solar_sim config macro: %s\n", macro.c_str());
        break;
      };
      case 'o' : {
        output = optarg;
        printf("solar_sim output file: %s\n", output.c_str());
        break;
      };
      case 'd': {
        output_dir = optarg;
        printf("solar_sim output directory: %s\n", output_dir.c_str());  
        break;
      }
      case 'u' : 
      {
        session = optarg; 
        printf("solar_sim session: %s\n", session.c_str());
        break;
      };
      case 'r':
      {
        myseed = std::atoi(optarg); 
        printf("solar_sim seed: %lu\n", myseed);
        break;
      }; 
      case 'g':
      {
        geometry_file = optarg; 
        printf("solar_sim geometry configuration file: %s\n", geometry_file.c_str());
        break;
      };
      case 'p' : 
      {
        material_file = optarg; 
        printf("solar_sim material database: %s\n", material_file.c_str());
        break;
      };
      case 'b' : 
      {
        do_bias = true; 
        setup_bias(optarg, bias_particle, bias_factor, bias_process); 
        printf("solar_sim cross-section biasing\n");
        printf("%s: biasing process(es) x%g\n", bias_particle.data(), bias_factor); 
        for (const auto &proc : bias_process) {
          printf("\t- %s\n", proc.data()); 
        }
        break;
      };
      case 'c' : 
      {
        do_cerenkov = std::atoi( optarg ); 
        break;
      }

      case 'h' : 
      {
        PrintUsage(); 
        return 4;
        break;
      };
      case '?' : 
      {
        printf("solar_sim error: unknown flag %c\n", optopt);
        PrintUsage(); 
        return 4;
        break;
      }; 
#ifdef G4MULTITHREADED
      case 't':
      {
        nThreads = std::atoi(optarg); 
        printf("solar_sim running on %i threads\n", nThreads);
        break;
      };
#endif
    }
  }

  // Instantiate G4UIExecutive if interactive mode
  G4UIExecutive* ui = nullptr;
  if ( macro.size() == 0 ) {
    ui = new G4UIExecutive(argc, argv);
  }

  // Choose the Random engine
  //
  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  // Construct the default run manager
  //
#ifdef G4MULTITHREADED
  // force just 1 thread
  nThreads = 1;
  G4MTRunManager * runManager = new G4MTRunManager;
  if ( nThreads > 0 ) runManager->SetNumberOfThreads(nThreads);
#else
  G4RunManager * runManager = new G4RunManager;
#endif

  // Seed the random number generator manually
  G4Random::setTheSeed(myseed);

  //G4String physName = "FTFP_BERT";
  G4String physName = "QGSP_BIC_AllHP";

  // Set mandatory initialization classes
  //
  // Detector construction
  printf("Creating Detector Construction...\n");
  auto detector = new SLArDetectorConstruction(geometry_file, material_file);
  runManager-> SetUserInitialization(detector);

  auto analysisManager = SLArAnalysisManager::Instance(); 
  analysisManager->SetSeed( myseed ); 
  printf("storing seed in analysis manager: %ld - %ld\n", 
      myseed, G4Random::getTheSeed());

  // External background biasing option
#ifdef SLAR_EXTERNAL
#ifndef SLAR_EXTERNAL_PARTICLE
  printf("solar-sim WARNING: target built with SLAR_EXTERNAL flag but external particle is not specified"); 
#else
  const char* ext_particle = SLAR_EXTERNAL_PARTICLE;
  G4GeometrySampler* mgs = nullptr; 
  G4bool activate_importance_sampling = true;
  if (G4ParticleTable::GetParticleTable()->FindParticle(ext_particle) == nullptr) {
    activate_importance_sampling = false;   
  }
  else {
    mgs = new G4GeometrySampler(detector->GetPhysicalWorld(), ext_particle);
  }
  printf("Built with SLAR_EXTERNAL flag for particle %s\n", ext_particle);
#endif
#endif



  // Physics list
  printf("Creating Phiscs Lists...\n");
  auto physicsList = new SLArPhysicsList(physName, do_cerenkov);
#if (defined SLAR_EXTERNAL &&  defined SLAR_EXTERNAL_PARTICLE)
  if (activate_importance_sampling) physicsList->RegisterPhysics(new G4ImportanceBiasing(mgs));
#endif
  if ( do_bias ) {
    auto biasingPhysics = new G4GenericBiasingPhysics("biasing_"+bias_particle);  
    if (bias_process.size() > 0) biasingPhysics->Bias(bias_particle, bias_process); 
    else biasingPhysics->Bias(bias_particle); 

    analysisManager->RegisterPhyicsBiasing(bias_particle, bias_factor); 

    physicsList->RegisterPhysics( biasingPhysics ); 
  }

  runManager-> SetUserInitialization(physicsList);

  // User action initialization
  printf("Creating User Action...\n");
  runManager->SetUserInitialization (new SLArActionInitialization());

  // Initialize G4 kernel
  //
  printf("RunManager initialization...\n");
  runManager->Initialize();

  #ifdef SLAR_EXTERNAL
  if (activate_importance_sampling) detector->CreateImportanceStore(); 
  #endif

  // Initialize visualization
  //
  G4VisManager* visManager = new G4VisExecutive;
  // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  // G4VisManager* visManager = new G4VisExecutive("Quiet");
  printf("visManager initialization...\n");
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  //
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  //

  if ( macro.size() ) {
    // Batch mode
    if (!output.empty()) {
      G4String command = "/SLAr/manager/SetOutputName "; 
      UImanager->ApplyCommand(command+output); 
    }
    if (!output_dir.empty()) {
      G4String command = "/SLAr/manager/SetOutputFolder "; 
      UImanager->ApplyCommand(command+output_dir); 
    }
 
    SLArRunAction* runAction = (SLArRunAction*)runManager->GetUserRunAction(); 
    runAction->SetG4MacroFile(macro); 

    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command+macro);
  }
  else // Define UI session for interactive mode
  {
    UImanager->ApplyCommand("/control/execute vis.mac");
    if (ui->IsGUI())
      UImanager->ApplyCommand("/control/execute gui.mac");
    ui->SessionStart();
    delete ui;
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  //                 owned and deleted by the run manager, so they should not
  //                 be deleted in the main() program !

  delete visManager;
  delete runManager;

  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
