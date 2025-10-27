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

#include "SLArVersion.hh"
#include "SLArUserPath.hh"
#include "SLArDebugManager.hh"
#include "SLArRootUtilities.hh"
#include "SLArPrimaryGeneratorAction.hh"
#include "SLArAnalysisManager.hh"
#include "SLArPhysicsList.hh"
#include "detector/SLArDetectorConstruction.hh"
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
    fprintf(stdout, " solar_sim\t[-m/--macro macro_file]]\n");
    fprintf(stdout, " \t\t[-x/--generator generator configuration file]\n");
    fprintf(stdout, " \t\t[-l/--physics_list set basic physics list (default is FTFP_BERT_HP)]\n");
    fprintf(stdout, " \t\t[-o/--output output_file_name]\n");
    fprintf(stdout, " \t\t[-d/--output_dir output_dir]\n");
    fprintf(stdout, " \t\t[-u/--session session]\n");
    fprintf(stdout, " \t\t[-r/--seed user_seed]\n");
    fprintf(stdout, " \t\t[-g/--geometry geometry_cfg_file]\n");
    fprintf(stdout, " \t\t[-p/--materials material_db_file]\n");
    fprintf(stdout, " \t\t[-b/--bias particle <process_list> bias_factor]\n");
    fprintf(stdout, " \t\t[-c/--cerenkov 0/1 enable/disable Cerenkov process]\n");
    fprintf(stdout, " \t\t[-D/--debug comma separated list of debug modules\n");
    fprintf(stdout, " \t\t            available modules: PRIMARY_EVENT, TRACKING,\n");
    fprintf(stdout, " \t\t            OPTICALPHYSICS, DETECTOR, MATERIALS, ALL]\n");
    fprintf(stdout, " \t\t[-h/--help print usage]\n");
    exit(0);
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
  // export G4CASCADE data path
  setenv("CAPGAM_DATA_DIR", G4CASCADE_DATA_DIR, 1);

  G4String macro;
  G4String session;
  G4String output = ""; 
  G4String output_dir = ""; 
  G4String generator_file = ""; 
  G4String geometry_file = G4String(SLAR_ASSETS_DIR) + "/geometry/geometry.json"; 
  G4String material_file = G4String(SLAR_ASSETS_DIR) + "/materials/materials_db.json"; 
  G4String debug_modules = "";
  G4bool   do_cerenkov = false; 
  G4bool   do_bias = false; 
  G4String bias_particle = ""; 
  G4double bias_factor = 1; 
  G4long myseed = 345354;

  G4String physName = "FTFP_BERT_HP";
  std::vector<G4String> bias_process;

#ifdef G4MULTITHREADED
  G4int nThreads = 0;
#endif

  const char* short_opts = "m:o:d:l:x:u:t:r:g:p:b:c:D:h";
  static struct option long_opts[15] = 
  {
    {"macro", required_argument, 0, 'm'}, 
    {"output", required_argument, 0, 'o'}, 
    {"output_dir", required_argument, 0, 'd'}, 
    {"physics_list", required_argument, 0, 'l'},
    {"session", required_argument, 0, 'u'}, 
    {"threads", required_argument, 0, 't'}, 
    {"seed", required_argument, 0, 'r'}, 
    {"generator", required_argument, 0, 'x'},
    {"geometry", required_argument, 0, 'g'}, 
    {"materials", required_argument, 0, 'p'},
    {"bias", required_argument, 0, 'b'},
    {"cerenkov", required_argument, 0, 'c'},
    {"help", no_argument, 0, 'h'}, 
    {"debug", required_argument, 0, 'D'},
    {nullptr, no_argument, nullptr, 0}
  };

  int c, option_index; 

  printf("************************************************************\n");
  printf("* solar_sim version %s\n", SOLARSIM_VERSION);
  printf("* The SoLAr Collaboration\n");
  printf("************************************************************\n");

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

      case 'x' : {
        generator_file = optarg;
        printf("getting generator configuration from %s\n", generator_file.data()); 
        break;
      }

      case 'l': {
        physName = optarg;
        printf("solar_sim baseline physics list: %s\n", physName.c_str());  
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
        myseed = std::atol(optarg); 
        printf("solar_sim seed: %ld\n", myseed);
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
      };
      case 'D' : 
      {
        debug_modules = optarg; 
        break;
      };
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

  SLArDebugManager::Instance().LoadFromEnvironment();
  SLArDebugManager::Instance().LoadFromString( debug_modules );
  SLArDebugManager::Instance().PrintStatus();

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

  // Set mandatory initialization classes
  //
  // Detector construction
  printf("Creating Detector Construction...\n");
  
  if (file_exists(geometry_file) == false) {
    printf("solar_sim ERROR: geometry file %s does not exist\n", geometry_file.data());
    exit(EXIT_FAILURE);
  }
  
  if (file_exists(material_file) == false) {
    printf("solar_sim ERROR: material file %s does not exist\n", material_file.data());
    exit(EXIT_FAILURE);
  }

  validate_json(geometry_file);
  validate_json(material_file);

  auto detector = new SLArDetectorConstruction(geometry_file, material_file);
  runManager-> SetUserInitialization(detector);

  auto analysisManager = SLArAnalysisManager::Instance(); 
  analysisManager->SetSeed( myseed ); 
  printf("storing seed in analysis manager: %ld - %ld\n", 
      myseed, G4Random::getTheSeed());


  // Physics list
  printf("Creating Phiscs Lists...\n");
  auto physicsList = new SLArPhysicsList(physName, do_cerenkov);
  // External background biasing option
#ifdef SLAR_EXTERNAL
#ifndef SLAR_EXTERNAL_PARTICLE
  printf("solar-sim WARNING: target built with SLAR_EXTERNAL flag but external particle is not specified"); 
#else
  const char* ext_particle = SLAR_EXTERNAL_PARTICLE;
  G4GeometrySampler* mgs = nullptr; 
  G4bool activate_importance_sampling = true;
  mgs = new G4GeometrySampler(detector->GetPhysicalWorld(), ext_particle);
  printf("Built with SLAR_EXTERNAL flag for particle %s\n", ext_particle);
  if (activate_importance_sampling) physicsList->RegisterPhysics(new G4ImportanceBiasing(mgs));
#endif
#endif


  if ( do_bias ) {
    auto biasingPhysics = new G4GenericBiasingPhysics("biasing_"+bias_particle);  
    if (bias_process.size() > 0) biasingPhysics->Bias(bias_particle, bias_process); 
    else biasingPhysics->Bias(bias_particle); 

    analysisManager->RegisterPhyicsBiasing(bias_particle, bias_factor); 

    physicsList->RegisterPhysics( biasingPhysics ); 
  }

  runManager->SetUserInitialization(physicsList);

  // User action initialization
  printf("Creating User Action...\n");
  runManager->SetUserInitialization (new SLArActionInitialization());

  // Initialize G4 kernel
  //
  printf("RunManager initialization...\n");
  runManager->Initialize();

  if (generator_file.empty() == false) {
    gen::SLArPrimaryGeneratorAction* gen = 
      (gen::SLArPrimaryGeneratorAction*)runManager->GetUserPrimaryGeneratorAction(); 
    printf("Configuring generator as per %s", generator_file.data()); 
    gen->SourceConfiguration( generator_file ); 
    printf("Done\n"); 
  }

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
  UImanager->SetMacroSearchPath(G4String(SLAR_BIN_DIR) + "/assets/macros");

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
