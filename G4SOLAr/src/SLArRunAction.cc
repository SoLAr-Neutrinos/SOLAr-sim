/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArRunAction
 * @created     : venerdì nov 04, 2022 09:28:13 CET
 */

#include <SLArAnalysisManager.hh>
#include <SLArDetectorConstruction.hh>
#include <SLArPrimaryGeneratorAction.hh>
//#include "SLArExternalGeneratorAction.hh>
//#include "SLArBoxSurfaceVertexGenerator.hh>
//#include "SLArBulkVertexGenerator.hh>
#include <SLArRunAction.hh>
#include <SLArRun.hh>

#include <G4Run.hh>
#include <G4RunManager.hh>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArRunAction::SLArRunAction()
 : G4UserRunAction(), fG4MacroFile(""), fEventAction(nullptr), fElectronDrift(nullptr)
{ 
  // Create custom SLAr Analysis Manager
  SLArAnalysisManager* anamgr = SLArAnalysisManager::Instance();

  const auto detector = (SLArDetectorConstruction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  fElectronDrift = new SLArElectronDrift(detector->GetLArProperties()); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArRunAction::~SLArRunAction()
{
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
  SLArLArProperties& lar_properties = detector->GetLArProperties(); 
  lar_properties.ComputeProperties(); 
  lar_properties.PrintProperties(); 
  G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;

  for (const auto& xsec : SLArAnaMgr->GetXSecDumpVector()) {
    SLArAnaMgr->WriteCrossSection(xsec); 
  }
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



  //if (SLArGen->GetGeneratorIndex() == kMarley) {
    //SLArAnaMgr->WriteCfgFile("marley", SLArGen->GetMarleyConf().c_str()); 
  //}
  //if (SLArGen->GetGeneratorIndex() == kExternalGen) {
    //auto gen = (SLArExternalGeneratorAction*)SLArGen->GetGenerator(); 

    //if (dynamic_cast<SLArBoxSurfaceVertexGenerator*>( gen->GetVertexGenerator() )) {
      //auto vtxGen = (SLArBoxSurfaceVertexGenerator*)gen->GetVertexGenerator();
      //G4double surface = vtxGen->GetSurfaceGenerator();

      //printf("surface box area is %g mm2\n", surface);
      //SLArAnaMgr->WriteVariable("surface_generator", surface); 
    //}
    //else if ( dynamic_cast<SLArBulkVertexGenerator*>(gen->GetVertexGenerator()) ) {
      //auto vtxGen = (SLArBulkVertexGenerator*)gen->GetVertexGenerator();
      //G4double volume = vtxGen->GetCubicVolumeGenerator();
      //G4double mass   = vtxGen->GetMassVolumeGenerator();

      //printf("volume of the generator is %g mm2\n", volume);
      //SLArAnaMgr->WriteVariable("volume_generator", volume); 
      //SLArAnaMgr->WriteVariable("mass_generator", mass);

    //} 

    //for (const auto& scorer : fExtScorerLV) {
      //auto scorer_solid = scorer->GetSolid(); 
      //printf("scorer solid volume is %s\n", scorer_solid->GetName().data()); 
      //SLArAnaMgr->WriteVariable("surface_scorer_"+scorer->GetName(), 
          //slargeo::get_bounding_volume_surface(scorer_solid)); 
    //}
  //}

  SLArAnaMgr->WriteCfg("git_hash", GIT_COMMIT_HASH); 

  SLArAnaMgr->WriteVariable("rndm_seed", SLArAnaMgr->GetSeed()); 

  SLArAnaMgr->Save();

  delete fElectronDrift;  fElectronDrift = nullptr;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
