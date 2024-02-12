/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArAnalysisManagerMsgr.cc
 * @created     : Tue Mar 03, 2020 17:15:44 CET
 */
#include <sstream>
#include "SLArAnalysisManager.hh"
#include "SLArAnalysisManagerMsgr.hh"
#include "SLArDetectorConstruction.hh"

//#include "G4RunManager.hh"

//#include "G4UImessenger.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4PhysicalVolumeStore.hh"
//#include "G4UIcmdWithADouble.hh"
//#include "G4UIcmdWith3Vector.hh"
//#include "G4UIcmdWithADoubleAndUnit.hh"
//#include "G4UIcmdWith3VectorAndUnit.hh"

#ifdef SLAR_GDML
#include "G4GDMLParser.hh"
#endif

SLArAnalysisManagerMsgr::SLArAnalysisManagerMsgr() :
  fMsgrDir  (nullptr), fConstr_(nullptr),
  fCmdOutputFileName(nullptr),  fCmdOutputPath(nullptr), 
  fCmdWriteCfgFile(nullptr), fCmdPlotXSec(nullptr), 
  fCmdGeoAnodeDepth(nullptr), 
  fCmdEnableBacktracker(nullptr),
  fCmdRegisterBacktracker(nullptr), 
  fCmdSetZeroSuppressionThrs(nullptr)
#ifdef SLAR_GDML
  ,fCmdGDMLFileName(nullptr), fCmdGDMLExport(nullptr), 
  fGDMLFileName("slar_export.gdml")
#endif
{
  TString UIManagerPath = "/SLAr/manager/";
  TString UIGeometryPath = "/SLAr/geometry/";
  TString UIExportPath = "/SLAr/export/"; 

  fMsgrDir = new G4UIdirectory(UIManagerPath);
  fMsgrDir->SetGuidance("SLAr manager instructions");

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  // Create commands
  
  fCmdOutputFileName = 
    new G4UIcmdWithAString(UIManagerPath+"SetOutputName", this);
  fCmdOutputFileName->SetGuidance("Set output file name");
  fCmdOutputFileName->SetParameterName("FileName", "slar_output.root");
  fCmdOutputFileName->SetDefaultValue("slar_output.root");

  fCmdOutputPath = 
    new G4UIcmdWithAString(UIManagerPath+"SetOutputFolder", this);
  fCmdOutputPath->SetGuidance("Set output folder");
  fCmdOutputPath->SetParameterName("Path", "./output/");
  fCmdOutputPath->SetDefaultValue("./output/");

  fCmdWriteCfgFile = 
    new G4UIcmdWithAString(UIManagerPath+"WriteCfgFile", this);
  fCmdOutputPath->SetGuidance("Write cfg file to output");
  fCmdOutputPath->SetParameterName("name path", false); 

  fCmdPlotXSec = 
    new G4UIcmdWithAString(UIManagerPath+"WriteXSection", this);
  fCmdPlotXSec->SetGuidance("Write a cross-section to  output");
  fCmdPlotXSec->SetParameterName("xsec_spec", false);
  fCmdPlotXSec->SetGuidance("Specfiy [particle]:[process]:[material]:[log(0-1)]");

  fCmdStoreFullTrajectory = 
    new G4UIcmdWithABool(UIManagerPath+"storeFullTrajectory", this);
  fCmdStoreFullTrajectory->SetGuidance("Store full track trajectory");

  fCmdEnableBacktracker = 
    new G4UIcmdWithAString(UIManagerPath+"enableBacktracker", this);
  fCmdEnableBacktracker->SetGuidance("Enable backtracker on readout system");
  fCmdEnableBacktracker->SetParameterName("backtraker_system", false);
  fCmdEnableBacktracker->SetGuidance("Specfiy readout system");
  fCmdEnableBacktracker->SetCandidates("charge vuv_sipm supercell");

  fCmdRegisterBacktracker = 
    new G4UIcmdWithAString(UIManagerPath+"registerBacktracker", this);
  fCmdRegisterBacktracker->SetGuidance("Add backtracker on readout system");
  fCmdRegisterBacktracker->SetParameterName("backtraker_system", false);
  fCmdRegisterBacktracker->SetGuidance("Specfiy readout system and backtracker [readout_system]:[backtraker]");

  fCmdSetZeroSuppressionThrs = 
    new G4UIcmdWithAnInteger(UIManagerPath+"setZeroSuppressionThrs", this);
  fCmdSetZeroSuppressionThrs->SetGuidance("Set charge readout zero suppression threshold");
  fCmdSetZeroSuppressionThrs->SetParameterName("threshold", false);
  
  fCmdGeoAnodeDepth = 
    new G4UIcmdWithAnInteger(UIGeometryPath+"setAnodeVisDepth", this);
  fCmdGeoAnodeDepth->SetGuidance("Set visualization depth for SoLAr anode");
  fCmdGeoAnodeDepth->SetParameterName("depth", false);
  
#ifdef SLAR_GDML
  fCmdGDMLFileName = 
    new G4UIcmdWithAString(UIExportPath+"SetGDMLFileName", this); 
  fCmdGDMLFileName->SetGuidance("Set file name for GDML volume export"); 
  fCmdGDMLFileName->SetParameterName("GDMLFileName", true);
  fCmdGDMLFileName->SetDefaultValue(fGDMLFileName.c_str()); 

  fCmdGDMLExport = 
    new G4UIcmdWithAString(UIExportPath+"ExportVolume", this); 
  fCmdGDMLExport->SetGuidance("Export volume to gdml file"); 
  fCmdGDMLExport->SetParameterName("VolumeName", true);
  fCmdGDMLExport->SetDefaultValue("World"); 
#endif
}

SLArAnalysisManagerMsgr::~SLArAnalysisManagerMsgr()
{
  G4cerr << "Deleting SLArAnalysisManagerMsgr..." << G4endl;
  if (fMsgrDir               ) delete fMsgrDir               ;
  if (fCmdOutputPath         ) delete fCmdOutputPath         ;
  if (fCmdOutputFileName     ) delete fCmdOutputFileName     ;
  if (fCmdWriteCfgFile       ) delete fCmdWriteCfgFile       ; 
  if (fCmdPlotXSec           ) delete fCmdPlotXSec           ; 
  if (fCmdGeoAnodeDepth      ) delete fCmdGeoAnodeDepth      ; 
  if (fCmdStoreFullTrajectory) delete fCmdStoreFullTrajectory;
  if (fCmdEnableBacktracker  ) delete fCmdEnableBacktracker  ;
  if (fCmdRegisterBacktracker) delete fCmdRegisterBacktracker;
  if (fCmdSetZeroSuppressionThrs) delete fCmdSetZeroSuppressionThrs;
#ifdef SLAR_DGML
  if (fCmdGDMLFileName  ) delete fCmdGDMLFileName  ;
  if (fCmdGDMLExport    ) delete fCmdGDMLExport    ;
#endif

  G4cerr << "SLArAnalysisManagerMsgr DONE" << G4endl;
}

void SLArAnalysisManagerMsgr::SetNewValue
                            (G4UIcommand* cmd, G4String newVal) 
{
  SLArAnalysisManager* SLArAnaMgr = SLArAnalysisManager::Instance();

  if (cmd == fCmdOutputPath)
    SLArAnaMgr->SetOutputPath(newVal);
  else if (cmd == fCmdOutputFileName)
    SLArAnaMgr->SetOutputName(newVal);
  else if (cmd == fCmdWriteCfgFile) {
    std::stringstream strm;
    strm << newVal.c_str(); 
    std::string name;
    std::string file_path; 
    strm >> name >> file_path; 

    SLArAnaMgr->WriteCfgFile(name, file_path.c_str()); 
  }
  else if (cmd == fCmdPlotXSec) {
    std::stringstream input(newVal); 
    G4String temp;

    G4String _particle; 
    G4String _process; 
    G4String _material;
    G4String _log = "0";

    G4int ifield = 0;
    while ( getline(input, temp, ':') ) {
      if (ifield == 0) _particle = temp;
      else if (ifield == 1) _process = temp;
      else if (ifield == 2) _material = temp;
      else if (ifield == 3) _log = temp;

      ifield++;
    }
    
    SLArAnaMgr->RegisterXSecDump( 
        SLArAnalysisManager::SLArXSecDumpSpec(
          _particle, _process, _material, std::atoi(_log)
        )
    ); 
  }
  else if (cmd == fCmdGeoAnodeDepth) {
    fConstr_->SetAnodeVisAttributes( std::atoi(newVal) ); 
  }
  else if (cmd == fCmdStoreFullTrajectory) {
    SLArAnaMgr->SetStoreTrajectoryFull( G4UIcmdWithABool::GetNewBoolValue(newVal) );
  }
  else if (cmd == fCmdEnableBacktracker) {
    SLArAnaMgr->ConstructBacktracker( newVal );
  }
  else if (cmd == fCmdRegisterBacktracker) {
    std::stringstream input(newVal); 
    G4String temp;

    G4String _system; 
    G4String _backtracker; 
    G4String _name = "";

    G4int ifield = 0;
    while ( getline(input, temp, ':') ) {
      if (ifield == 0) _system = temp;
      else if (ifield == 1) _backtracker = temp;
      else if (ifield == 2) _name = temp;
      ifield++;
    }

    auto bkt_mngr = SLArAnaMgr->GetBacktrackerManager(_system);
    bkt_mngr->RegisterBacktracker(backtracker::GetBacktrackerEnum(_backtracker), _name);
  }
  else if (cmd == fCmdSetZeroSuppressionThrs) {
    int thrs = std::atoi( newVal ); 
    for (auto& anode_itr : SLArAnaMgr->GetEvent()->GetEventAnode()) {
      anode_itr.second.SetZeroSuppressionThreshold( thrs ); 
    }
  }
#ifdef SLAR_GDML
  else if (cmd == fCmdGDMLFileName) {
    fGDMLFileName = newVal; 
  }
  else if (cmd == fCmdGDMLExport) {
    G4GDMLParser parser; 
    auto pvstore = G4PhysicalVolumeStore::GetInstance(); 
    for (const auto &pv : *pvstore) {
      if (pv->GetName() == newVal) {
        parser.Write(fGDMLFileName, pv); 
      }
    }
  }
#endif
}


