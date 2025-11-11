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


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArRunAction::SLArRunAction()
 : G4UserRunAction(), fG4MacroFile(""), fEventAction(nullptr), fElectronDrift(nullptr)
{ 
  SLArAnalysisManager* anamgr = SLArAnalysisManager::Instance();
  fTRandomInterface = new SLArRandom(); 

  const auto detector = (SLArDetectorConstruction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  fElectronDrift = new SLArElectronDrift(detector->GetLArProperties()); 
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

  // dump cross sections
  for (const auto& xsec : SLArAnaMgr->GetXSecDumpVector()) {
    SLArAnaMgr->WriteCrossSection(xsec); 
  }
  /*
  auto volumeStore = G4PhysicalVolumeStore::GetInstance();
  for (auto vol : *volumeStore) {
    G4cout << "Nome: " << vol->GetName()
           << ", Copia: " << vol->GetCopyNo()
           << ", Madre: " << (vol->GetMotherLogical() ? vol->GetMotherLogical()->GetName() : "NULL")
           << G4endl;
}
  G4String vol_name = "LightGuideLV";
  G4cout << "Searching for volume " << vol_name << G4endl;
  G4cout << "Calling the function" << G4endl;
  auto volume_found = geo::SearchLogicalVolumeInParametrisedVolume(vol_name, "pds_30");
  if (volume_found) {
    G4cout << "FOUND volume " << volume_found->logical_volume->GetName() << " !!!" << G4endl;
  }
  G4cout << "Volume dimension: "
         << "X: " << volume_found->dimension->x() 
         << ", Y: " << volume_found->dimension->y() 
         << ", Z: " << volume_found->dimension->z() 
         << G4endl;
  /*G4cout << "Volume position: "
         << "X: " << volume_found->position->x() 
         << ", Y: " << volume_found->position->y() 
         << ", Z: " << volume_found->position->z() 
         << G4endl;*/

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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
