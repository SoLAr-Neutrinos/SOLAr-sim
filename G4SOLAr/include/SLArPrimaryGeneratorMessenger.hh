/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArPrimaryGeneratorMessenger.hh
 * @created     Friday Dec 30, 2022 18:49:15 CET
 */

#ifndef SLArPrimaryGeneratorMessenger_h
#define SLArPrimaryGeneratorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class SLArPrimaryGeneratorAction;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWith3Vector;
class G4UIcmdWithAString;
class G4UIcmdWithADouble; 
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool; 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class SLArPrimaryGeneratorMessenger: public G4UImessenger
{
  public:
    SLArPrimaryGeneratorMessenger(SLArPrimaryGeneratorAction* );
    virtual ~SLArPrimaryGeneratorMessenger();
 
    virtual void SetNewValue(G4UIcommand*, G4String);
 
  private:
    SLArPrimaryGeneratorAction*         fSLArAction;
    G4UIdirectory*                      fCmdGunDir;
    G4UIcmdWith3VectorAndUnit*          fCmdGunPosition;
    G4UIcmdWith3Vector*                 fCmdGunDirection;

    G4UIcmdWithAString*                 fCmdDirectionMode;
    G4UIcmdWithAString*                 fCmdGenerator;
    G4UIcmdWithADoubleAndUnit*          fCmdEnergy; 
    G4UIcmdWithAString*                 fCmdBulkVol;
    G4UIcmdWithAString*                 fCmdParticle;
    G4UIcmdWithAnInteger*               fCmdNumberOfParticles; 
    G4UIcmdWithADouble*                 fCmdBulkVolFraction;
    G4UIcmdWithAString*                 fCmdMarley;
    G4UIcmdWithAString*                 fCmdBackgoundConf; 
    G4UIcmdWithAString*                 fCmdExternalConf; 

    G4UIcmdWithABool*                   fCmdTracePhotons;
    G4UIcmdWithABool*                   fCmdDriftElectrons;

    G4UIcmdWithAnInteger*               fCmdGENIEEvtSeed; //--JM
    G4UIcmdWithAString*                 fCmdGENIEFile; //--JM
                                                       
    G4UIcmdWithAnInteger*               fCmdVerbose;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
