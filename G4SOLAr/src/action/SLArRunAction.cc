/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRunAction
 * @created     : venerdì nov 04, 2022 09:28:13 CET
 */

#include "SLArAnalysisManager.hh"
#include "SLArSteppingAction.hh"
#include "detector/SLArDetectorConstruction.hh"
#include "SLArPrimaryGeneratorAction.hh"
#include "SLArRunAction.hh"
#include "SLArRun.hh"
#include "geo/SLArGeoUtils.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4ProductionCutsTable.hh"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArRunAction::SLArRunAction()
 :  G4UserRunAction(), 
    fG4MacroFile(""), 
    fEventAction(nullptr), 
    fElectronDrift(nullptr), 
    fFastLightSimDispatcher(nullptr),
    fFastLightSimMessenger(nullptr)
{ 
  SLArAnalysisManager* anamgr = SLArAnalysisManager::Instance();
  fTRandomInterface = new SLArRandom(); 

  const auto detector = (SLArDetectorConstruction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  fElectronDrift = new SLArElectronDrift(detector->GetLArProperties()); 

  fFastLightSimMessenger = std::make_unique<SLArFastLightSimMessenger>(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArRunAction::~SLArRunAction()
{
  delete fTRandomInterface;
  delete SLArAnalysisManager::Instance();
  fSDName.clear(); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* SLArRunAction::GenerateRun() {
  return (new SLArRun(fSDName)); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArRunAction::BeginOfRunAction(const G4Run* aRun)
{ 
  //inform the runManager to save random number seed
  //G4RunManager::GetRunManager()->SetRandomNumberStore(true);
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance(); 

  SLArAnaMgr->CreateFileStructure();

  const auto detector = (SLArDetectorConstruction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  const auto stepping = (SLArSteppingAction*)G4RunManager::GetRunManager()->GetUserSteppingAction(); 

  SLArLArProperties& lar_properties = detector->GetLArProperties(); 
  lar_properties.ComputeProperties(); 
  lar_properties.PrintProperties(); 

  // set tranformation for step points output
  const auto target_lar_pv = detector->GetLArTargetVolume()->GetModPV();
  const G4ThreeVector t = target_lar_pv->GetObjectTranslation(); 
  const G4RotationMatrix* r = target_lar_pv->GetObjectRotation();

  const G4Transform3D transform(*r, t);
  fTransformWorld2Det = transform.inverse();
  stepping->SetPointTransformation(fTransformWorld2Det);

  // Initialize fast light simulator if enabled and configured
  if (fFastLightSimEnabled && !fFLSConfigPath.empty()) {
    G4cout << "SLArRunAction: Initializing fast light simulator..." << G4endl;

    try {
      // Create dispatcher
      fFastLightSimDispatcher = std::make_unique<SLArFastLightSimDispatcher>();

      // Load JSON configuration
      std::ifstream configFile(fFLSConfigPath);
      if (!configFile.good()) {
        throw std::runtime_error("Cannot open config file: " + fFLSConfigPath);
      }

      rapidjson::IStreamWrapper isw(configFile);
      rapidjson::Document doc;
      doc.ParseStream(isw);
      if (doc.HasParseError()) {
        throw std::runtime_error("Error parsing JSON config file: " + fFLSConfigPath);
      }

      // Loop over volumes and create fast light sim engines

    }
    catch (const std::exception& e) {
      G4Exception("SLArRunAction::BeginOfRunAction()",
          "FastLightSimInitFailed",
          FatalException,
          ("Failed to initialize fast light simulator: " + 
           std::string(e.what())).c_str());
    }
    
  }
  else if (!fFastLightSimEnabled) {
    G4cout << "SLArRunAction: Fast light simulation disabled, using full optical tracking" << G4endl;
  }

  // dump cross sections
  for (const auto& xsec : SLArAnaMgr->GetXSecDumpVector()) {
    SLArAnaMgr->WriteCrossSection(xsec); 
  }

  G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArRunAction::EndOfRunAction(const G4Run* aRun)
{
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();

  //- SLArRun object.
  SLArRun* solarRun = (SLArRun*)aRun;
  //--- Dump all socred quantities involved in SLArRun.
  solarRun->DumpAllScorer();
  //---

  //auto hmap_capture1 = solarRun->GetHitsMap("BPolyethilene_1", "captureCnts1"); 
  //auto hmap_capture2 = solarRun->GetHitsMap("BPolyethilene_2", "captureCnts2"); 
  //G4double ncapt_1 = 0.; 
  //G4double ncapt_2 = 0.; 
  //for (const auto& v : *hmap_capture1) {
    //G4double val = *v.second; 
    //G4int    idx =  v.first;
    //printf("hmap_capture1[%i]: %g\n", idx, val);
    //ncapt_1 += val;
  //}

  //for (const auto& v : *hmap_capture2) {
    //G4double val = *v.second; 
    //G4int    idx =  v.first;
    //printf("hmap_capture2[%i]: %g\n", idx, val);
    //ncapt_2 += val;
  //}

  //printf("%g neutrons capured in layer 1\n", ncapt_1);
  //printf("%g neutrons capured in layer 2\n", ncapt_2);


  //auto hmap_current_w0 = solarRun->GetHitsMap("CryostatWall0", "nCurrent0"); 
  //auto hmap_current_w1 = solarRun->GetHitsMap("CryostatWall1", "nCurrent1"); 
  //G4double ncurr_0 = 0; 
  //G4double ncurr_1 = 0; 
  //for (const auto& v : *hmap_current_w0) {
    //G4double val = *v.second; 
    //G4int    idx =  v.first;
    //printf("hmap_current_w0[%i]: %g\n", idx, val);
    //ncurr_0 += val; 
  //}

  //for (const auto& v : *hmap_current_w1) {
    //G4double val = *v.second; 
    //G4int    idx =  v.first;
    //printf("hmap_current_w1[%i]: %g\n", idx, val);
    //ncurr_1 += val; 
  //}


  //// save histograms & ntuple
  ////
  //SLArAnaMgr->WriteVariable("nEvents", static_cast<G4double>(aRun->GetNumberOfEvent())); 
  //SLArAnaMgr->WriteVariable("nCapture_BPolyethilene_1", ncapt_1); 
  //SLArAnaMgr->WriteVariable("nCapture_BPolyethilene_2", ncapt_2); 
  //SLArAnaMgr->WriteVariable("nCurrent_outerWall", ncurr_0);
  //SLArAnaMgr->WriteVariable("nCurrent_innerWall", ncurr_1);

  if (!fG4MacroFile.empty()) {
    SLArAnaMgr->WriteCfgFile("g4macro", fG4MacroFile.c_str()); 
  }

  auto RunMngr = G4RunManager::GetRunManager(); 
  auto SLArDetConstr = 
    (SLArDetectorConstruction*)RunMngr->GetUserDetectorConstruction(); 
  SLArAnaMgr->WriteCfgFile("geometry", SLArDetConstr->GetGeometryCfgFile().c_str());
  SLArAnaMgr->WriteCfgFile("materials", SLArDetConstr->GetMaterialCfgFile().c_str());

  auto SLArGen = (gen::SLArPrimaryGeneratorAction*)RunMngr->GetUserPrimaryGeneratorAction(); 
  const auto& generators = SLArGen->GetGenerators(); 

  for (const auto& gen : generators) {
    gen::EGenerator kGen = gen.second->GetGeneratorEnum(); 

    G4String gen_config = gen.second->WriteConfig(); 

    SLArAnaMgr->WriteCfg(gen.first.data(), gen_config.data()); 
  }

  for (const auto& scorer : fExtScorerLV) {
    auto scorer_solid = scorer->GetSolid(); 
    printf("scorer solid volume is %s\n", scorer_solid->GetName().data()); 
    SLArAnaMgr->WriteVariable("surface_scorer_"+scorer->GetName(), 
        geo::get_bounding_volume_surface(scorer_solid)); 
  }

  SLArAnaMgr->WriteCfg("git_hash", GIT_COMMIT_HASH); 

  SLArAnaMgr->WriteVariable("rndm_seed", SLArAnaMgr->GetSeed()); 

  SLArAnaMgr->Save();

  delete fElectronDrift;  fElectronDrift = nullptr;
}

void SLArRunAction::SetFastLightSimConfig(const G4String& configPath) {
    // Verify file exists
    std::ifstream file(configPath);
    if (!file.good()) {
        G4Exception("SLArRunAction::LoadFastLightSimConfig()",
                   "ConfigFileNotFound",
                   FatalException,
                   ("Configuration file not found: " + configPath).c_str());
        return;
    }
    
    fFLSConfigPath = configPath;
    G4cout << "SLArRunAction: Fast light sim config will be loaded from: " 
           << fFLSConfigPath << G4endl;
}

void SLArRunAction::SetFastLightSimulatorType(const G4String& type) {
    if (type != "LookupTable" && type != "SemiAnalytical") {
        G4Exception("SLArRunAction::SetFastLightSimulatorType()",
                   "InvalidSimulatorType",
                   JustWarning,
                   ("Invalid simulator type: " + type).c_str());
        return;
    }
    
    fFastLightSimType = type;
    G4cout << "SLArRunAction: Fast light simulator type set to: " 
           << fFastLightSimType << G4endl;
}

void SLArRunAction::EnableFastLightSim(G4bool enable) {
    fFastLightSimEnabled = enable;
    G4cout << "SLArRunAction: Fast light simulation " 
           << (enable ? "enabled" : "disabled") << G4endl;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
