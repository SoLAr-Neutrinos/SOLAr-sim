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
  SetSimulatorTypeCmd(nullptr),
  EnableFastSimCmd(nullptr) {

    // Create directory for fast light simulation commands
    fFastSimDir = new G4UIdirectory("/slar/fastLightSim/");
    fFastSimDir->SetGuidance("Fast light simulation configuration commands");

    // Command to load JSON configuration file
    LoadConfigCmd = new G4UIcmdWithAString("/slar/fastLightSim/loadConfig", this);
    LoadConfigCmd->SetGuidance("Load JSON configuration file for fast light simulation");
    LoadConfigCmd->SetGuidance("This file should contain photon library settings,");
    LoadConfigCmd->SetGuidance("channel mapping, and simulator parameters");
    LoadConfigCmd->SetParameterName("configFile", false);
    LoadConfigCmd->SetDefaultValue("fast_light_config.json");
    LoadConfigCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // Command to set simulator type
    SetSimulatorTypeCmd = new G4UIcmdWithAString("/slar/fastLightSim/setType", this);
    SetSimulatorTypeCmd->SetGuidance("Set the type of fast light simulator to use");
    SetSimulatorTypeCmd->SetGuidance("Options: SemiAnalytical, LookupTable");
    SetSimulatorTypeCmd->SetParameterName("simulatorType", false);
    SetSimulatorTypeCmd->SetDefaultValue("LookupTable");
    SetSimulatorTypeCmd->SetCandidates("SemiAnalytical LookupTable");
    SetSimulatorTypeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // Command to enable/disable fast simulation
    EnableFastSimCmd = new G4UIcmdWithABool("/slar/fastLightSim/enable", this);
    EnableFastSimCmd->SetGuidance("Enable or disable fast light simulation");
    EnableFastSimCmd->SetGuidance("When disabled, full optical photon tracking is used");
    EnableFastSimCmd->SetParameterName("enable", false);
    EnableFastSimCmd->SetDefaultValue(true);
    EnableFastSimCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  }

SLArFastLightSimMessenger::~SLArFastLightSimMessenger() {
  delete LoadConfigCmd;
  delete SetSimulatorTypeCmd;
  delete EnableFastSimCmd;
  delete fFastSimDir;
}

void SLArFastLightSimMessenger::SetNewValue(
    G4UIcommand* command, G4String newValue) {

  if (command == LoadConfigCmd) {
    fRunAction->SetFastLightSimConfig(newValue);
  }
  else if (command == SetSimulatorTypeCmd) {
    fRunAction->SetFastLightSimulatorType(newValue);
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
  else if (command == SetSimulatorTypeCmd) {
    cv = fRunAction->GetFastLightSimulatorType();
  }
  else if (command == EnableFastSimCmd) {
    cv = fRunAction->IsFastLightSimEnabled() ? "true" : "false";
  }

  return cv;
}
