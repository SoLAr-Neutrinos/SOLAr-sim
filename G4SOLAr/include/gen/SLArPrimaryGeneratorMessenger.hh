/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArPrimaryGeneratorMessenger.hh
 * @created     Friday Dec 30, 2022 18:49:15 CET
 */

#ifndef SLArPrimaryGeneratorMessenger_h
#define SLArPrimaryGeneratorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"


class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWith3Vector;
class G4UIcmdWithAString;
class G4UIcmdWithADouble; 
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool; 
class G4UIcmdWithoutParameter;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace gen {
class SLArPrimaryGeneratorAction;


class SLArPrimaryGeneratorMessenger: public G4UImessenger
{
  public:
    SLArPrimaryGeneratorMessenger(gen::SLArPrimaryGeneratorAction* );
    virtual ~SLArPrimaryGeneratorMessenger();

    virtual void SetNewValue(G4UIcommand*, G4String);

  private:
    SLArPrimaryGeneratorAction*         fSLArGenAction;
    G4UIcmdWithAString*                 fCmdGenConfig;
    G4UIcmdWithABool*                   fCmdGenRegisterPrimaries;
    G4UIcmdWithAnInteger*               fCmdVerbose;
};
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
#endif
