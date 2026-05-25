/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArPrimaryGeneratorMessenger.cc
 * @created     Friday Dec 30, 2022 18:48:41 CET
 */


#include <cstdlib>
#include "SLArPrimaryGeneratorMessenger.hh"

#include "SLArPrimaryGeneratorAction.hh"
#ifdef SLAR_CRY
#include "SLArCRYGeneratorAction.hh"
#endif
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

namespace gen {
SLArPrimaryGeneratorMessenger::SLArPrimaryGeneratorMessenger(SLArPrimaryGeneratorAction* SLArGun)
  : G4UImessenger(),
    fSLArGenAction(SLArGun)
{
  fCmdGenConfig = 
    new G4UIcmdWithAString("/SLAr/gen/configure", this);
  fCmdGenConfig->SetGuidance("Set generator configuration");
  fCmdGenConfig->SetParameterName("ConfigFile", false);

  fCmdGenRegisterPrimaries = 
    new G4UIcmdWithABool("/SLAr/gen/registerPrimaries", this);
  fCmdGenRegisterPrimaries->SetGuidance("Register primaries in the analysis manager");
  fCmdGenRegisterPrimaries->SetParameterName("registerPrimaries", true, false);

  fCmdVerbose = 
    new G4UIcmdWithAnInteger("/SLAr/gen/verbose", this); 
  fCmdVerbose->SetGuidance("verbose level"); 
  fCmdVerbose->SetParameterName("verbose", true, false); 
  fCmdVerbose->SetDefaultValue( 1 ); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArPrimaryGeneratorMessenger::~SLArPrimaryGeneratorMessenger()
{
  delete fCmdGenConfig;
  delete fCmdVerbose;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArPrimaryGeneratorMessenger::SetNewValue(
              G4UIcommand* command, G4String newValue)
{
  if (command == fCmdGenConfig) {
    G4String config_file = newValue;
    fSLArGenAction->SourceConfiguration( config_file ); 
  }
  else if (command == fCmdVerbose) {
    G4String verbose_str = newValue; 
    fSLArGenAction->SetVerboseLevel( std::stoi( verbose_str ) ); 
  }
  else if (command == fCmdGenRegisterPrimaries) {
    G4bool register_primaries = fCmdGenRegisterPrimaries->GetNewBoolValue(newValue);
    fSLArGenAction->SetRegisterPrimaries(register_primaries);
  }
  else {
    G4Exception("SLArPrimaryGeneratorMessenger::SetNewValue",
                "InvalidCommand", FatalException, "Unknown command");
  }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
}
