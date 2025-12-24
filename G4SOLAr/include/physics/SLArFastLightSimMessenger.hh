/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArFastLightSimMessenger
 * @created     : Thursday Dec 18, 2025 12:31:32 CET
 */

#ifndef SLAR_FAST_LIGHT_SIM_MESSENGER_H
#define SLAR_FAST_LIGHT_SIM_MESSENGER_H

#include "G4UImessenger.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"

class SLArRunAction;

class SLArFastLightSimMessenger : public G4UImessenger {
  private:
    SLArRunAction* fRunAction;

    G4UIdirectory* fFastSimDir;
    G4UIcmdWithAString* LoadConfigCmd;
    G4UIcmdWithABool* EnableFastSimCmd;

  public:
    SLArFastLightSimMessenger(SLArRunAction* runAction);
    ~SLArFastLightSimMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;
    G4String GetCurrentValue(G4UIcommand* command) override;
};

#endif // SLAR_FAST_LIGHT_SIM_MESSENGER_H
