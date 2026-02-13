/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArAnalysisManagerMsgr.hh
 * @created     : Tue Mar 03, 2020 17:15:22 CET
 */

#ifndef SLArANALYSISMANAGERMSGR_HH

#define SLArANALYSISMANAGERMSGR_HH

class G4UIcommand;
class G4UIcmdWithABool;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithADouble;
class G4UIcmdWith3Vector;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;
class SLArDetectorConstruction;


#include "G4UImessenger.hh"
#include "globals.hh"

class SLArAnalysisManagerMsgr : public G4UImessenger 
{
  public:
    explicit SLArAnalysisManagerMsgr();
    ~SLArAnalysisManagerMsgr();
  public:
    void SetNewValue(G4UIcommand* cmd, G4String newVal);
    void AssignDetectorConstruction(SLArDetectorConstruction* detConstr)
                    {fConstr_ = detConstr;}
    bool IsPathValid(G4String path);

  private:
    G4UIdirectory*          fMsgrDir;
    SLArDetectorConstruction* fConstr_;

    void                    UpdatePMTs(); 

    // define command
    G4UIcmdWithAString*         fCmdOutputFileName;
    G4UIcmdWithAString*         fCmdOutputPath    ;
    G4UIcmdWithAString*         fCmdWriteCfgFile  ; 
    G4UIcmdWithAString*         fCmdPlotXSec      ; 
    G4UIcmdWithAnInteger*       fCmdGeoAnodeDepth ; 
    G4UIcmdWithABool*           fCmdGeoFieldCageVis;
    G4UIcmdWithABool*           fCmdGeoCryoSupportVis;
    G4UIcmdWithABool*           fCmdEnableMCTruthOutput;
    G4UIcmdWithABool*           fCmdEnableAnodeOutput; 
    G4UIcmdWithABool*           fCmdEnablePDSOutput;
    G4UIcmdWithAString*         fCmdDisableSD;
    G4UIcmdWithABool*           fCmdStoreFullTrajectory;
    G4UIcmdWithAString*         fCmdEnableBacktracker;
    G4UIcmdWithAString*         fCmdRegisterBacktracker;
    G4UIcmdWithAnInteger*       fCmdSetZeroSuppressionThrs;
    G4UIcmdWithADoubleAndUnit*  fCmdElectronLifetime; 
    G4UIcmdWithADoubleAndUnit*  fCmdXSecEMin;
    G4UIcmdWithADoubleAndUnit*  fCmdXSecEMax;
    G4UIcmdWithAnInteger*       fCmdXSecNPoints;
#ifdef SLAR_GDML
    G4UIcmdWithAString*         fCmdGDMLFileName  ; 
    G4UIcmdWithAString*         fCmdGDMLExport    ;
#endif
    G4UIcmdWithAString*         fCmdAddExtScorer; 
    G4String                    fGDMLFileName     ; 
};

#endif /* end of include guard SLArANALYSISMANAGERMSGR_HH */

