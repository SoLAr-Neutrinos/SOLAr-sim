/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArAnalysisManagerMsgr.cc
 * @created     : Tue Mar 03, 2020 17:15:44 CET
 */
#include <sstream>
#include "SLArAnalysisManager.hh"
#include "SLArAnalysisManagerMsgr.hh"
#include "detector/SLArDetectorConstruction.hh"
#include "SLArRunAction.hh"

#include "G4RunManager.hh"

#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4SDManager.hh"

#ifdef SLAR_GDML
#include "G4GDMLParser.hh"
#endif

SLArAnalysisManagerMsgr::SLArAnalysisManagerMsgr() :
  fMsgrDir  (nullptr), fConstr_(nullptr),
  fCmdOutputFileName(nullptr),  fCmdOutputPath(nullptr), 
  fCmdWriteCfgFile(nullptr), fCmdPlotXSec(nullptr), 
  fCmdGeoAnodeDepth(nullptr), 
  fCmdGeoFieldCageVis(nullptr),
  fCmdGeoCryoSupportVis(nullptr),
  fCmdEnableMCTruthOutput(nullptr),
  fCmdEnableAnodeOutput(nullptr),
  fCmdEnablePDSOutput(nullptr),
  fCmdDisableSD(nullptr),
  fCmdEnableBacktracker(nullptr),
  fCmdRegisterBacktracker(nullptr), 
  fCmdSetZeroSuppressionThrs(nullptr), 
  fCmdElectronLifetime(nullptr),
  fCmdXSecEMin(nullptr),
  fCmdXSecEMax(nullptr),
  fCmdXSecNPoints(nullptr),
#ifdef SLAR_GDML
  fCmdGDMLFileName(nullptr), fCmdGDMLExport(nullptr),
#endif
  fCmdAddExtScorer(nullptr),
  fGDMLFileName("slar_export.gdml")
{
  TString UIManagerPath = "/SLAr/manager/";
  TString UIGeometryPath = "/SLAr/geometry/";
  TString UIExportPath = "/SLAr/export/"; 
  TString UIPhysPath = "/SLAr/phys/"; 

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

  fCmdEnableMCTruthOutput = 
    new G4UIcmdWithABool(UIManagerPath+"enableMCTruthOutput", this);
  fCmdEnableMCTruthOutput->SetGuidance("Enable MC truth output");

  fCmdEnableAnodeOutput = 
    new G4UIcmdWithABool(UIManagerPath+"enableAnodeOutput", this);
  fCmdEnableAnodeOutput->SetGuidance("Enable anode output");

  fCmdEnablePDSOutput = 
    new G4UIcmdWithABool(UIManagerPath+"enablePDSOutput", this);
  fCmdEnablePDSOutput->SetGuidance("Enable PDS output");

  fCmdDisableSD = 
    new G4UIcmdWithAString(UIManagerPath+"disableSD", this);
  fCmdDisableSD->SetGuidance("Disable sensitive detector");
  fCmdDisableSD->SetParameterName("sensitive_detector", false);

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

  fCmdElectronLifetime = 
    new G4UIcmdWithADoubleAndUnit(UIPhysPath+"setElectronLifetime", this); 
  fCmdElectronLifetime->SetGuidance("Set electrons lifetime in LAr"); 
  fCmdElectronLifetime->SetParameterName("electron_lifetime", false); 
  
  fCmdGeoAnodeDepth = 
    new G4UIcmdWithAnInteger(UIGeometryPath+"setAnodeVisDepth", this);
  fCmdGeoAnodeDepth->SetGuidance("Set visualization depth for SoLAr anode");
  fCmdGeoAnodeDepth->SetParameterName("depth", false);

  fCmdGeoCryoSupportVis = 
    new G4UIcmdWithABool(UIGeometryPath+"setCryoSupportVisibility", this);
  fCmdGeoCryoSupportVis->SetGuidance("Set cryostat support structure visibility");
  fCmdGeoCryoSupportVis->SetParameterName("vis", false, true);

  fCmdGeoFieldCageVis =
    new G4UIcmdWithABool(UIGeometryPath+"setFieldCageVisibility", this);
  fCmdGeoFieldCageVis->SetGuidance("Set field cage visibility");
  fCmdGeoFieldCageVis->SetParameterName("vis", false, true);

  fCmdAddExtScorer = 
    new G4UIcmdWithAString(UIManagerPath+"addExtScorer", this);
  fCmdAddExtScorer->SetGuidance("Add external scorer volume (only for EXT mode)");
  fCmdAddExtScorer->SetParameterName("scorer_pv:alias", false);
  fCmdAddExtScorer->SetGuidance("Specfiy physical volume to be used as ext scorer [pv_name]:[alias]");

  fCmdXSecEMin = new G4UIcmdWithADoubleAndUnit(UIManagerPath+"setXSEmin", this);
  fCmdXSecEMin->SetGuidance("Set Cross Section Output Minimum Energy");
  fCmdXSecEMin->SetParameterName("Emin", false);
  fCmdXSecEMin->SetDefaultUnit("MeV");

  fCmdXSecEMax = new G4UIcmdWithADoubleAndUnit(UIManagerPath+"setXSEmax", this);
  fCmdXSecEMax->SetGuidance("Set Cross Section Output Maximum Energy");
  fCmdXSecEMax->SetParameterName("Emax", false);
  fCmdXSecEMax->SetDefaultUnit("MeV");

  fCmdXSecNPoints = new G4UIcmdWithAnInteger(UIManagerPath+"setXSNPoints", this);
  fCmdXSecNPoints->SetGuidance("Set Cross Section Output Number of points");
  fCmdXSecNPoints->SetParameterName("NPoints", false);
  
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
  if (fCmdGeoFieldCageVis    ) delete fCmdGeoFieldCageVis    ; 
  if (fCmdGeoCryoSupportVis  ) delete fCmdGeoCryoSupportVis  ;
  if (fCmdStoreFullTrajectory) delete fCmdStoreFullTrajectory;
  if (fCmdEnableMCTruthOutput) delete fCmdEnableMCTruthOutput;
  if (fCmdEnableAnodeOutput  ) delete fCmdEnableAnodeOutput  ;
  if (fCmdEnablePDSOutput    ) delete fCmdEnablePDSOutput    ;
  if (fCmdDisableSD          ) delete fCmdDisableSD          ;
  if (fCmdEnableBacktracker  ) delete fCmdEnableBacktracker  ;
  if (fCmdRegisterBacktracker) delete fCmdRegisterBacktracker;
  if (fCmdSetZeroSuppressionThrs) delete fCmdSetZeroSuppressionThrs;
  if (fCmdElectronLifetime)    delete fCmdElectronLifetime   ; 
  if (fCmdAddExtScorer       ) delete fCmdAddExtScorer       ; 
  if (fCmdXSecEMin           ) delete fCmdXSecEMin           ;
  if (fCmdXSecEMax           ) delete fCmdXSecEMax           ;
  if (fCmdXSecNPoints        ) delete fCmdXSecNPoints        ;
  
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
  else if (cmd == fCmdGeoFieldCageVis) {
    for (auto& tpc_ : fConstr_->GetDetTPCs()) {
      auto tpc = tpc_.second;
      tpc->SetFieldCageVisibility( G4UIcmdWithABool::GetNewBoolValue(newVal) );
    }
  }
  else if (cmd == fCmdGeoCryoSupportVis) {
    fConstr_->GetCryostat()->SetSupportStructureVisibility( G4UIcmdWithABool::GetNewBoolValue(newVal) );
    fConstr_->GetCryostat()->SetVisAttributes();
  }
  else if (cmd == fCmdStoreFullTrajectory) {
    SLArAnaMgr->SetStoreTrajectoryFull( G4UIcmdWithABool::GetNewBoolValue(newVal) );
  }
  else if (cmd == fCmdEnableMCTruthOutput) {
    SLArAnaMgr->EnableMCTruthOutput( G4UIcmdWithABool::GetNewBoolValue(newVal) );
  }
  else if (cmd == fCmdEnableAnodeOutput) {
    SLArAnaMgr->EnableEventAnodeOutput( G4UIcmdWithABool::GetNewBoolValue(newVal) );
  }
  else if (cmd == fCmdEnablePDSOutput) {
    SLArAnaMgr->EnableEventPDSOutput( G4UIcmdWithABool::GetNewBoolValue(newVal) );
  }
  else if (cmd == fCmdDisableSD) {
    auto SDman = G4SDManager::GetSDMpointer();
    auto sd = SDman->FindSensitiveDetector(newVal, true);

    if (sd) {
      sd->Activate( false );
    }
    else {
      G4cerr << "SLArAnalysisManagerMsgr::SetNewValue: "
             << "Cannot find sensitive detector with name: " 
             << newVal << G4endl;
    }
    
    return;
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
  else if (cmd == fCmdAddExtScorer) {
    std::stringstream input(newVal); 
    G4String temp;

    G4String _phys_vol_name; 
    G4String _alias; 
    G4String _name = "";

    G4int ifield = 0;
    while ( getline(input, temp, ':') ) {
      if (ifield == 0) _phys_vol_name = temp;
      else if (ifield == 1) _alias = temp;
      else if (ifield == 2) _name = temp;
      ifield++;
    }

    auto construction = 
      (SLArDetectorConstruction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction();
    try {
      construction->AddExternalScorer(_phys_vol_name, _alias);
    }
    catch (const std::exception& e) {
      G4cerr << e.what() << G4endl; 
    }
    return;
  }
  else if ( cmd == fCmdElectronLifetime ) {
    auto detector = 
      (SLArDetectorConstruction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction(); 
    auto& lar_properties = detector->GetLArProperties(); 
    lar_properties.SetElectronLifetime( fCmdElectronLifetime->GetNewDoubleValue( newVal ) ); 
  }

  else if (cmd == fCmdSetZeroSuppressionThrs) {
    int thrs = std::atoi( newVal ); 
    for (auto& anode_itr : SLArAnaMgr->GetEventAnode().GetAnodeMap()) {
      anode_itr.second.SetZeroSuppressionThreshold( thrs ); 
    }
  }
  else if (cmd == fCmdXSecEMin) {
    SLArAnaMgr->SetXSecEmin(fCmdXSecEMin->GetNewDoubleValue(newVal));
  }
  else if (cmd == fCmdXSecEMax) {
    SLArAnaMgr->SetXSecEmax(fCmdXSecEMax->GetNewDoubleValue(newVal));
  }
  else if (cmd == fCmdXSecNPoints) {
    SLArAnaMgr->SetXSecNPoints(fCmdXSecNPoints->GetNewIntValue(newVal));
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


