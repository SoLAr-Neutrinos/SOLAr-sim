/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFastLightSimMessenger
 * @created     : Thursday Dec 18, 2025 12:31:13 CET
 */

#include "SLArFastLightSimMessenger.hh"
#include "SLArRunAction.hh"

SLArFastLightSimMessenger::SLArFastLightSimMessenger(
    SLArRunAction* runAction)
  : G4UImessenger(),
  fRunAction(runAction),
  fFastSimDir(nullptr),
  LoadConfigCmd(nullptr),
  EnableFastSimCmd(nullptr) 
{

    // Create directory for fast light simulation commands
    fFastSimDir = new G4UIdirectory("/SLAr/phys/fastLightSim/");
    fFastSimDir->SetGuidance("Fast light simulation configuration commands");

    // Command to load JSON configuration file
    LoadConfigCmd = new G4UIcmdWithAString("/SLAr/phys/fastLightSim/loadConfig", this);
    LoadConfigCmd->SetGuidance("Load JSON configuration file for fast light simulation");
    LoadConfigCmd->SetGuidance("This file should contain photon library settings,");
    LoadConfigCmd->SetGuidance("channel mapping, and simulator parameters");
    LoadConfigCmd->SetParameterName("configFile", false);
    LoadConfigCmd->SetDefaultValue("fast_light_config.json");
    LoadConfigCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // Command to enable/disable fast simulation
    EnableFastSimCmd = new G4UIcmdWithABool("/SLAr/phys/fastLightSim/enable", this);
    EnableFastSimCmd->SetGuidance("Enable or disable fast light simulation");
    EnableFastSimCmd->SetGuidance("When disabled, full optical photon tracking is used");
    EnableFastSimCmd->SetParameterName("enable", false);
    EnableFastSimCmd->SetDefaultValue(true);
    EnableFastSimCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  }

SLArFastLightSimMessenger::~SLArFastLightSimMessenger() {
  delete LoadConfigCmd;
  delete EnableFastSimCmd;
  delete fFastSimDir;
}

void SLArFastLightSimMessenger::SetNewValue(
    G4UIcommand* command, G4String newValue) {

  if (command == LoadConfigCmd) {
    fRunAction->SetFastLightSimConfig(newValue);
  }
  else if (command == EnableFastSimCmd) {
    G4bool enable = EnableFastSimCmd->GetNewBoolValue(newValue);
    fRunAction->EnableFastLightSim(enable);
  }
}

G4String SLArFastLightSimMessenger::GetCurrentValue(G4UIcommand* command) {
  G4String cv;

  if (command == LoadConfigCmd) {
    cv = fRunAction->GetFastLightSimConfigPath();
  }
  else if (command == EnableFastSimCmd) {
    cv = fRunAction->IsFastLightSimEnabled() ? "true" : "false";
  }

  return cv;
}
