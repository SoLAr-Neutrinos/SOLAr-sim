/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArAnalysisManager.cc
 * @created     : Wed Feb 12, 2020 18:26:02 CET
 */

#include <G4Exception.hh>
#include <cstdio>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

#include "G4ParticleTable.hh"
#include "G4Material.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4HadronicProcessStore.hh"

#include "SLArAnalysisManager.hh"
#include "SLArBacktrackerManager.hh"

#include "SLArEventAnode.hh"
#include "TObjString.h"
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
  fRootFile(nullptr), 
  fEventTree(nullptr), 
  fGenTree(nullptr),
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
    fAnaMsgr = new SLArAnalysisManagerMsgr();
  }
  fgInstance = this;
}

//______________________________________________________________
SLArAnalysisManager::~SLArAnalysisManager()
{
  G4cout << "Deleting SLArAnalysisManager" << G4endl;
  if (fRootFile) {
    if (fRootFile->IsOpen()) {
      fRootFile->cd();
      if (fEventTree) fEventTree->Write();
      if (fGenTree) fGenTree->Write();
#ifdef SLAR_EXTERNAL
      if (fExternalsTree) fExternalsTree->Write(); 
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
  G4cout << "SLArAnalysisManager DONE" << G4endl;
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
  fEventTree = new TTree("EventTree", "SoLAr-sim Event Tree");
  fEventTree->SetDirectory(fRootFile);
  printf("EventTree created with AutoFlush set to %lld\n", fEventTree->GetAutoFlush());

  fEventTree->Branch("EvNumber", &fEventNumber, "EvNumber/I");

  if (fEnableMCTruthOutput) {
    fEventTree->Branch("MCTruth", &fListMCPrimary);
  }

  if (fEnableEventAnodeOutput) {
    fEventTree->Branch("EventAnode", &fListEventAnode);
  }

  if (fEnableEventPDSOutput) {
    fEventTree->Branch("EventPDS", &fListEventPDS);
  }

  if (fEnableGenTreeOutput) {
    fGenTree = new TTree("GenTree", "SoLAr-sim Gen Tree");
    fGenTree->SetDirectory(fRootFile);
    fGenTree->Branch("GenRecords", &fListGenRecords); 
    printf("GenRecords tree created with AutoFlush set to %lld\n", fGenTree->GetAutoFlush());
  }

  // setup backtracker size
  SetupBacktrackerRecords(); 

#ifdef SLAR_EXTERNAL
  SetupExternalsTree(); 
#endif // SLAR_EXTERNAL

  return true;
}

G4bool SLArAnalysisManager::CreateEventStructure() 
{
  printf("configuring anode...\n");
  auto& event_anode_map = fListEventAnode.GetAnodeMap();
  for (const auto& anode_cfg : fAnodeCfg) {
    event_anode_map.insert(std::make_pair(anode_cfg.first, SLArEventAnode(anode_cfg.second)));
    event_anode_map[anode_cfg.first].SetID(anode_cfg.second.GetIdx());
  }

  printf("configuring PDS...\n");
  auto& event_pds_map = fListEventPDS.GetOpDetArrayMap();
  for (const auto& opdetarray_cfg : fPDSysCfg.GetConstMap()) {
    if (event_pds_map.count(opdetarray_cfg.first)) {
      printf("SLArAnalysisManager::CreateEventStructure() WARNING: ");
      printf("SuperCelll array with index %i is aleady stored in the fListEventPDS. Skipping.\n", opdetarray_cfg.first);
      continue;
    }

    event_pds_map.insert(
        std::make_pair(opdetarray_cfg.first, SLArEventSuperCellArray(opdetarray_cfg.second)));
  }

  return true;
}

G4bool SLArAnalysisManager::Save()
{
  if (!fRootFile) return false;

  auto write_tree = [&](TTree* t) {
    if (t) {
      if (t->IsZombie()) {
        printf("%s is a zombie, RUN!\n", t->GetName());
      }
      else {
        t->Write();
      }
    }
  };

  write_tree(fEventTree);
  write_tree(fGenTree);
#ifdef SLAR_EXTERNAL
  write_tree(fExternalsTree);
#endif // SLAR_EXTERNAL

  WriteSysCfg(); 

  fRootFile->Close();

  return true;
}


G4bool SLArAnalysisManager::LoadPDSCfg(SLArCfgSystemSuperCell& pdsCfg)
{
  fPDSysCfg = SLArCfgSystemSuperCell(pdsCfg);
  return true;
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

void SLArAnalysisManager::WriteSysCfg() 
{
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

G4bool SLArAnalysisManager::FillTree() {
#ifdef SLAR_DEBUG
  printf("SLArAnalysisManager::FillTree...");
#endif

  if (fEventTree) {
    if (fEventTree->IsZombie()) {
      printf("%s is a zombie, RUN!\n", fEventTree->GetName());
      return false;
    }
    else {
      fEventTree->Fill();
    }
  }

  return true;
}

G4bool SLArAnalysisManager::FillGenTree() 
{
#ifdef SLAR_DEBUG
  printf("SLArAnalysisManager::FillGenTree...");
#endif
  if (!fGenTree) {
#ifdef SLAR_DEBUG
    printf(" GenTree is NULL!\n");
#endif
    return false;
  }

  fGenTree->Fill();
#ifdef SLAR_DEBUG
  printf(" OK\n");
#endif
  return true;
}

int SLArAnalysisManager::WriteArray(G4String name, G4int size, G4double* val) 
{
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

int SLArAnalysisManager::WriteCfg(G4String name, const char* cfg) 
{
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

int SLArAnalysisManager::WriteCfgFile(G4String name, const char* path) 
{
  std::ifstream ifile; 
  ifile.open(path); 
  if (!ifile.is_open()) {
    G4ExceptionDescription ed; 
      ed << "SLArAnalysisManager::WriteCfgFile WARNING\n"; 
      ed << "Unable to open file " << path << "\n";
    G4Exception("SLArAnalysisManager::WriteCfgFile()", "CfgFileNotFound", FatalException, ed);
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


  const G4double logMin = log10(fXSecEmin); 
  const G4double logMax = log10(fXSecEmax);
  const G4double logdE = (logMax - logMin) / static_cast<G4double> (fXSecNPoints-1);
  const G4double dE = (fXSecEmax-fXSecEmin) / static_cast<G4double> (fXSecNPoints -1);
  G4double energy[fXSecNPoints];
  G4double xsec[fXSecNPoints]; 



  for (int n=0; n<fXSecNPoints; n++) {
    if (xsec_dump.log_span) {
      energy[n] = pow(10, logMin + n*logdE); 
    }
    else {
      energy[n] = fXSecEmin + n*dE;
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

  TGraph* gxsec = new TGraph(fXSecNPoints, energy, xsec); 
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

backtracker::SLArBacktrackerManager* SLArAnalysisManager::GetBacktrackerManager(const G4String sys) 
{
  backtracker::EBkTrkReadoutSystem isys = backtracker::GetBacktrackerReadoutSystem( sys );

  auto bktMngr = GetBacktrackerManager( isys );

  return bktMngr;
}

void SLArAnalysisManager::SetupBacktrackerRecords() 
{
  // charge backtrackers 
  if (fChargeBacktrackerManager) {
    if (fChargeBacktrackerManager->IsNull() == false) {
      for (auto& evAnode : fListEventAnode.GetAnodeMap()) {
        evAnode.second.SetChargeBacktrackerRecordSize( fChargeBacktrackerManager->GetConstBacktrackers().size() ); 
      }
    }
  }

  // vuv sipm backtrackers
  if (fVUVSiPMBacktrackerManager) {
    if (fVUVSiPMBacktrackerManager->IsNull() == false) {
      for (auto& evAnode : fListEventAnode.GetAnodeMap()) {
        evAnode.second.SetLightBacktrackerRecordSize( fVUVSiPMBacktrackerManager->GetConstBacktrackers().size() );
      }
    }
  }

  // supercell backtrakers
  if (fSuperCellBacktrackerManager) {
    if (fSuperCellBacktrackerManager->IsNull() == false) {
      for (auto& evSCA : fListEventPDS.GetOpDetArrayMap()) { {
        evSCA.second.SetLightBacktrackerRecordSize( fSuperCellBacktrackerManager->GetConstBacktrackers().size() ); 
      }
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
