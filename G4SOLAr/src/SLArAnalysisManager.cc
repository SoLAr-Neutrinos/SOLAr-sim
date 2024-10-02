/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArAnalysisManager.cc
 * @created     : Wed Feb 12, 2020 18:26:02 CET
 */

#include "G4ParticleTable.hh"
#include "G4Material.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4HadronicProcessStore.hh"

#include "SLArAnalysisManager.hh"
#include "SLArBacktrackerManager.hh"
#include <cstdio>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

#include "SLArEventAnode.hh"
#include "TObjString.h"
#include "TParameter.h"
#include "TVectorD.h"

SLArAnalysisManager* SLArAnalysisManager::fgMasterInstance = nullptr;
G4ThreadLocal SLArAnalysisManager* SLArAnalysisManager::fgInstance = nullptr;

SLArAnalysisManager::SLArXSecDumpSpec::SLArXSecDumpSpec() 
  : particle_name(""), process_name(""), material_name(""), log_span(false)
{}

SLArAnalysisManager::SLArXSecDumpSpec::SLArXSecDumpSpec(const G4String& par, const G4String& proc, const G4String& mat, const G4bool& do_log) 
  : particle_name(par), process_name(proc), material_name(mat), log_span(do_log)
{}

//__________________________________________________________________
SLArAnalysisManager* SLArAnalysisManager::Instance()
{
  if ( fgInstance == nullptr ) {
    G4bool isMaster = ! G4Threading::IsWorkerThread();
    fgInstance = new SLArAnalysisManager(isMaster);
  }
  
  return fgInstance;
}    

//__________________________________________________________________
G4bool SLArAnalysisManager::IsInstance()
{
  return ( fgInstance != 0 );
}    

SLArAnalysisManager::SLArAnalysisManager(G4bool isMaster)
  : fAnaMsgr  (nullptr),
    fIsMaster(isMaster), fSeed( time(NULL) ), fOutputPath(""),
    fOutputFileName("solarsim_output.root"), 
    fTrajectoryFull( true ),
    fSuperCellBacktrackerManager(nullptr), 
    fVUVSiPMBacktrackerManager(nullptr), 
    fChargeBacktrackerManager(nullptr), 
    fPDSysCfg(nullptr)
{
  if ( ( isMaster && fgMasterInstance ) || ( fgInstance ) ) {
    G4ExceptionDescription description;
    description 
      << "      " 
      << "SLArAnalysisManager already exists." 
      << "Cannot create another instance.";
    G4Exception("SLArAnalysisManager::SLArAnalysisManager()",
        "Analysis_F001", FatalException, description);
  }
  if ( isMaster ) {
    fgMasterInstance = this;
    //fMCEvent = std::make_unique<SLArMCEvent>();
#ifdef SLAR_EXTERNAL
    //fExternalRecord = std::make_unique<SLArEventTrajectoryLite>(); 
#endif // DEBUG
    fAnaMsgr = new SLArAnalysisManagerMsgr();
  }
  fgInstance = this;
}

//______________________________________________________________
SLArAnalysisManager::~SLArAnalysisManager()
{
  G4cerr << "Deleting SLArAnalysisManager" << G4endl;
  if (fRootFile) {
    if (fRootFile->IsOpen()) {
      fRootFile->cd();
      if (fEventTree) fEventTree->Write();
#ifdef SLAR_EXTERNAL
      fExternalsTree->Write(); 
#endif // SLAR_EXTERNAL
      fRootFile->Close(); 
    }
  }

  if (fChargeBacktrackerManager) delete fChargeBacktrackerManager;
  if (fVUVSiPMBacktrackerManager) delete fVUVSiPMBacktrackerManager;
  if (fSuperCellBacktrackerManager) delete fSuperCellBacktrackerManager;
  if (this->fIsMaster) fgMasterInstance = nullptr;
  if (fAnaMsgr) delete  fAnaMsgr; 
  fgInstance = nullptr;
  G4cerr << "SLArAnalysisManager DONE" << G4endl;
}

G4bool SLArAnalysisManager::CreateFileStructure()
{
  G4String filepath = fOutputPath;
  filepath.append(fOutputFileName);
  fRootFile = new TFile(filepath, "recreate");

  if (!fRootFile)
  {
    G4cout << "SLArAnalysisManager::CreateFileStructure\n" << G4endl;
    G4cout << "rootfile not created! Quit."              << G4endl;
    return false;
  }
  fEventTree = new TTree("EventTree", "SoLAr-sim Simulated Events");
 
  // setup backtracker size
  SetupBacktrackerRecords(); 

  printf("setting up ROOT TTree Branch...\n");
  fEventTree->Branch("MCEvent", &fMCEvent);

  printf("MCEvent tree created with AutoFlush set to %lld\n", fEventTree->GetAutoFlush());

#ifdef SLAR_EXTERNAL
  SetupExternalsTree(); 
#endif // SLAR_EXTERNAL

  return true;
}

G4bool SLArAnalysisManager::CreateEventStructure() {
  //printf("fMCEvent pointer: %p\n", fMCEvent.get());

  printf("configuring anode...\n");
  fMCEvent.ConfigAnode( fAnodeCfg ); 

  printf("configuring PDS...\n");
  if (!fPDSysCfg.GetMap().empty()) fMCEvent.ConfigSuperCellSystem(fPDSysCfg);

  return true;
}

G4bool SLArAnalysisManager::Save()
{
  if (!fRootFile) return false;

  if (fEventTree) {
    fEventTree->Write();
  }

  WriteSysCfg(); 

#ifdef SLAR_EXTERNAL
  fExternalsTree->Write();
#endif // SLAR_EXTERNAL

  fRootFile->Close();

  return true;
}


G4bool SLArAnalysisManager::LoadPDSCfg(SLArCfgSystemSuperCell& pdsCfg)
{
  fPDSysCfg = SLArCfgSystemSuperCell(pdsCfg);
  return true;
  //if (!fPDSysCfg) return false;
  //else return true ; 
}

G4bool SLArAnalysisManager::LoadAnodeCfg(SLArCfgAnode& anodeCfg)
{
  if (fAnodeCfg.count(anodeCfg.GetTPCID())) {
    printf("SLArAnalysisManager::LoadAnodeCfg WARNING "); 
    printf("an anode configuration with index %i is already registered. skip.\n",
        anodeCfg.GetID());
    return false;
  }

  fAnodeCfg.insert(std::make_pair(anodeCfg.GetTPCID(), std::move(anodeCfg)));

  return true;
}

void SLArAnalysisManager::WriteSysCfg() {
  if (!fRootFile) {
    G4cout << "SLArAnalysisManager::WriteSysCfg" << G4endl;
    G4cout << "rootfile has null ptr! Quit."   << G4endl;
    return;
  }

  if (fPDSysCfg.GetMap().empty() == false) {
    fRootFile->cd();
    fPDSysCfg.Write("PDSSysConfig");
  } else {
    G4cout << "SLArAnalysisManager::WritePDSSysConfig" << G4endl;
    G4cout << "fPDSysCfg is nullptr! Quit."      << G4endl;
  }

  for (auto &anodeCfg : fAnodeCfg) {
    fRootFile->cd();
    anodeCfg.second.Write(Form("AnodeCfg%i", anodeCfg.second.GetIdx()));
  } 
  return;
}

G4bool SLArAnalysisManager::FillEvTree() {
#ifdef SLAR_DEBUG
  printf("SLArAnalysisManager::FillEvTree...");
#endif
  if (!fEventTree) {
#ifdef SLAR_DEBUG
    printf(" EventTree is NULL!\n");
#endif
    return false;
  }
  
  fEventTree->Fill();
#ifdef SLAR_DEBUG
  printf(" OK\n");
#endif
  return true;
}

//template<typename T> 
//int SLArAnalysisManager::WriteVariable (G4String name, T val) {
  //if (!fRootFile) {
    //printf("SLArAnalysisManager::WriteVariable WARNING ");
    //printf("rootfile not present yet. Cannot write %s variable.\n", 
        //name.c_str());
    //return 666    
  //} 

  //TParameter<T> var(name, val); 
  //fRootFile->cd(); 
  //int status = var.Write(); 
  //return status; 
//}

int SLArAnalysisManager::WriteArray(G4String name, G4int size, G4double* val) {
  if (!fRootFile) {
    printf("SLArAnalysisManager::WriteArray WARNING ");
    printf("rootfile not present yet. Cannot write %s variable.\n", 
        name.c_str());
    return 666;
  } 

  TVectorD var(size, val); 
  fRootFile->cd(); 
  int status = var.Write(name); 
  return status; 
}

int SLArAnalysisManager::WriteCfg(G4String name, const char* cfg) {
  if (!fRootFile) {
    printf("SLArAnalysisManager::WriteCfg WARNING ");
    printf("rootfile not present yet. Cannot write %s variable.\n", 
        name.c_str());
    return 666;
  } 

  TObjString cfg_str(cfg); 
  fRootFile->cd(); 
  int status = cfg_str.Write(name); 
  return status; 
}

int SLArAnalysisManager::WriteCfgFile(G4String name, const char* path) {
  std::ifstream ifile; 
  ifile.open(path); 
  if (!ifile.is_open()) {
    printf("SLArAnalysisManager::WriteCfgFile WARNING ");
    printf("Unable to open file %s\n", path);
    return 4; 
  }

  std::stringstream strm; 
  strm << ifile.rdbuf(); 
  const std::string buff(strm.str()); 

  WriteCfg(name, buff.c_str()); 

  ifile.close(); 

  return 0;
}

G4bool SLArAnalysisManager::FakeAccess()
{
  G4cout << "SLArAnalysisManager::FakeAccess() - still alive" 
         << G4endl;

  return true;
}

bool SLArAnalysisManager::IsPathValid(G4String path)
{
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

void SLArAnalysisManager::SetOutputPath(G4String path)
{
  G4String spath = path;
  if ( IsPathValid(spath) ) 
  {
    // check is path is file 
    struct stat s;
    stat(path, &s);
    if (s.st_mode & S_IFDIR)
    { // append '/' if necessary 
      G4String last = &spath.operator[](spath.length()-1);
      if ( G4StrUtil::icompare(last, "/") ) spath.append("/");
      fOutputPath = spath;
      return;
    }
    else 
    {
      std::cerr << "Output file path is not a directory! "
                << "(maybe an existing file?)"
                << std::endl;
      fOutputPath = "./";
      return;
    }
  }
  else 
  {
    // Creating a new directory 
    if (mkdir(path, 0777) == -1) 
      std::cerr << "Error :  " << strerror(errno) << std::endl; 

    else
    {
      G4String last = &spath.operator[](spath.length()-1);
      if ( G4StrUtil::icompare(last, "/") ) spath.append("/");
      fOutputPath = spath;
      std::cout << "Directory created" << std::endl; 
    }
  }
}

void SLArAnalysisManager::SetOutputName(G4String filename)
{
  if (! G4StrUtil::contains(filename, ".root") ) filename.append(".root");
  fOutputFileName = filename;
}

void SLArAnalysisManager::RegisterPhyicsBiasing(G4String particle_name, G4double biasing_factor) {
  if (fBiasing.count(particle_name) > 0) {
    printf("SLArAnalysisManager::RegisterPhyicsBiasing WARNING\n");
    printf("%s is already biased, updating biasing factor\n", particle_name.data());
    fBiasing[particle_name] = biasing_factor;
    return;
  }
  else {
    fBiasing.insert( std::make_pair(particle_name, biasing_factor) ); 
    return;
  }
}

void SLArAnalysisManager::RegisterXSecDump(const SLArXSecDumpSpec xsec_dump) {
  fXSecDump.push_back(xsec_dump); 
  return;
}

int SLArAnalysisManager::WriteCrossSection(const SLArXSecDumpSpec xsec_dump) {

  auto particle = G4ParticleTable::GetParticleTable()->FindParticle(xsec_dump.particle_name);
  
  if (particle == nullptr) {
    printf("SLArAnalysisManager::WriteCrossSection ERROR: No particle named %s found in particle table\n", xsec_dump.particle_name.data());
    return 4;
  }

  auto material = G4Material::GetMaterial(xsec_dump.material_name);
  if (material == nullptr) {
    printf("SLArAnalysisManager::WriteCrossSection ERROR: No material named %s found in particle table\n", xsec_dump.material_name.data());
    return 5;
  }

  auto process = particle->GetProcessManager()->GetProcess(xsec_dump.process_name);
  if (process == nullptr) {
    printf("SLArAnalysisManager::WriteCrossSection ERROR: particle %s has no process %s registered\n", 
        xsec_dump.particle_name.data(), xsec_dump.process_name.data());
    printf("Here is the process table:\n");
    for (int j=0; j<particle->GetProcessManager()->GetProcessListLength(); j++) {
      printf("\t- %s\n", particle->GetProcessManager()->GetProcessList()->operator[](j)->GetProcessName().data());
    }
    return 6;
  }

  auto hpStore = G4HadronicProcessStore::Instance(); 

  const G4double energy_min = 0.01*CLHEP::MeV;
  const G4double energy_max = 50.0*CLHEP::MeV;
  const G4int n_points = 500; 
  const G4double logMin = log10(energy_min); 
  const G4double logMax = log10(energy_max);
  const G4double logdE = (logMax - logMin) / (n_points-1);
  const G4double dE = (energy_max-energy_min) / (n_points -1);
  G4double energy[n_points];
  G4double xsec[n_points]; 

  

  for (int n=0; n<n_points; n++) {
    if (xsec_dump.log_span) {
      energy[n] = pow(10, logMin + n*logdE); 
    }
    else {
      energy[n] = energy_min + n*dE;
    }

    if (G4StrUtil::contains(xsec_dump.process_name, "Inelastic")){
      xsec[n] = 
        hpStore->GetInelasticCrossSectionPerVolume(particle, energy[n], material);
    }
    else if (G4StrUtil::contains(xsec_dump.process_name, "Elastic")) {
      xsec[n] = 
        hpStore->GetElasticCrossSectionPerVolume(particle, energy[n], material);
    }
    else if (G4StrUtil::contains(xsec_dump.process_name, "Capture")) {
      xsec[n] = 
        hpStore->GetCaptureCrossSectionPerVolume(particle, energy[n], material);
    }
    else {
      printf("SLArAnalysisManager::WriteCrossSection WARNING: cross section dump for process %s not implemented yet.\n", 
          xsec_dump.process_name.data());
    }
  }

  TGraph* gxsec = new TGraph(n_points, energy, xsec); 
  gxsec->SetNameTitle(
      Form("g_xsec_%s_%s_%s", 
        xsec_dump.particle_name.data(), 
        xsec_dump.process_name.data(), 
        xsec_dump.material_name.data()), 
      Form("%s %s cross-section on %s", 
        xsec_dump.particle_name.data(), 
        xsec_dump.process_name.data(), 
        xsec_dump.material_name.data()) 
      );

  if (fRootFile->IsOpen()) {
    gxsec->Write(); 
  }

  delete gxsec;

  return 0; 
}

void SLArAnalysisManager::ConstructBacktracker(const backtracker::EBkTrkReadoutSystem isys) {

  switch (isys) {
    case backtracker::kSuperCell:
      {
        fSuperCellBacktrackerManager = new backtracker::SLArBacktrackerManager(); 
        break;
      }
    case backtracker::kVUVSiPM:
      {
        fVUVSiPMBacktrackerManager = new backtracker::SLArBacktrackerManager();
        break;
      }
    case backtracker::kCharge:
      {
        fChargeBacktrackerManager = new backtracker::SLArBacktrackerManager();
        break;
      }
    default :
      {
        printf("SLArAnalysisManager::ConstructBacktracker() WARNING case %i is not implemented\n", isys);
        break;
      }
  }
  return;
}

void SLArAnalysisManager::ConstructBacktracker(const G4String sys) {

  backtracker::EBkTrkReadoutSystem isys = backtracker::GetBacktrackerReadoutSystem( sys );
  ConstructBacktracker( isys );

  return;
}

backtracker::SLArBacktrackerManager* SLArAnalysisManager::GetBacktrackerManager(const backtracker::EBkTrkReadoutSystem isys) 
{
  backtracker::SLArBacktrackerManager* bktMngr = nullptr;

  switch (isys) {
    case backtracker::kCharge:
      bktMngr = fChargeBacktrackerManager;
      break;

    case backtracker::kVUVSiPM:
      bktMngr = fVUVSiPMBacktrackerManager;
      break;

    case backtracker::kSuperCell:
       bktMngr = fSuperCellBacktrackerManager; 
       break;

    default:
       break;
  }
  return bktMngr;
}

backtracker::SLArBacktrackerManager* SLArAnalysisManager::GetBacktrackerManager(const G4String sys) {

  backtracker::EBkTrkReadoutSystem isys = backtracker::GetBacktrackerReadoutSystem( sys );

  auto bktMngr = GetBacktrackerManager( isys );

  return bktMngr;
}

void SLArAnalysisManager::SetupBacktrackerRecords() {

  // charge backtrackers 
  if (fChargeBacktrackerManager) {
    if (fChargeBacktrackerManager->IsNull() == false) {
      for (auto& evAnode : fMCEvent.GetEventAnode()) {
        evAnode.second.SetChargeBacktrackerRecordSize( fChargeBacktrackerManager->GetConstBacktrackers().size() ); 
      }
    }
  }

  // vuv sipm backtrackers
  if (fVUVSiPMBacktrackerManager) {
    if (fVUVSiPMBacktrackerManager->IsNull() == false) {
      for (auto& evAnode : fMCEvent.GetEventAnode()) {
        evAnode.second.SetLightBacktrackerRecordSize( fVUVSiPMBacktrackerManager->GetConstBacktrackers().size() );
      }
    }
  }

  // supercell backtrakers
  if (fSuperCellBacktrackerManager) {
    if (fSuperCellBacktrackerManager->IsNull() == false) {
      for (auto& evSCA : fMCEvent.GetEventSuperCellArray() ) {
        evSCA.second.SetLightBacktrackerRecordSize( fSuperCellBacktrackerManager->GetConstBacktrackers().size() ); 
      }
    }
  }

}

#ifdef SLAR_EXTERNAL
void SLArAnalysisManager::SetupExternalsTree() {
  fExternalsTree = new TTree("ExternalTree", "Externals reaching LAr interface");

  fExternalsTree->Branch("iEv", &fExternalRecord.fEvNumber); 
  fExternalsTree->Branch("pdgID", &fExternalRecord.fPDGCode); 
  fExternalsTree->Branch("trkID", &fExternalRecord.fTrkID); 
  fExternalsTree->Branch("parentID", &fExternalRecord.fParentID); 
  fExternalsTree->Branch("origin_vol", &fExternalRecord.fOriginVol);
  fExternalsTree->Branch("origin_energy", &fExternalRecord.fOriginEnergy);
  fExternalsTree->Branch("weight", &fExternalRecord.fWeight);
  fExternalsTree->Branch("time", &fExternalRecord.fTime); 
  fExternalsTree->Branch("scorer_energy", &fExternalRecord.fEnergy); 
  fExternalsTree->Branch("origin_vertex", &fExternalRecord.fOriginVertex);
  fExternalsTree->Branch("scorer_vertex", &fExternalRecord.fScorerVertex);
  fExternalsTree->Branch("creator", &fExternalRecord.fCreator); 

  printf("ExternalsTree created with AutoFlush set to %lld\n", fExternalsTree->GetAutoFlush()); 
}
#endif // SLAR_EXTERNAL

