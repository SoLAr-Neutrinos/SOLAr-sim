//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file include/WLSPhysicsListMessenger.hh
/// \brief Definition of the WLSPhysicsListMessenger class
///
/// Reimplemented from include/WLSPhysicsListMessenger.hh

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#ifndef SLArPhysicsListMessenger_h
#define SLArPhysicsListMessenger_h 1

#include "globals.hh"
#include "G4UImessenger.hh"

#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

#include "G4DecayTable.hh"
#include "G4VDecayChannel.hh"

class SLArPhysicsList;

class G4UIdirectory;
class G4UIcmdWithABool;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithoutParameter;
class G4UIcmdWithADoubleAndUnit;

/// Provide control of the physics list and cut parameters

class SLArPhysicsListMessenger : public G4UImessenger
{
  public:

    SLArPhysicsListMessenger(SLArPhysicsList* );
    virtual ~SLArPhysicsListMessenger();

    virtual void SetNewValue(G4UIcommand*, G4String);

  private:

    SLArPhysicsList* fPhysicsList;

    G4UIdirectory* fDirectory;

    G4UIcmdWithABool* fCmdTracePhotons;
    G4UIcmdWithABool* fCmdDriftElectrons;

    G4UIdirectory* fDecayDirectory;
    G4UIcmdWithABool* fSetAbsorptionCMD;

    G4UIcmdWithAnInteger* fVerboseCmd;
    G4UIcmdWithAnInteger* fCerenkovCmd;

    G4UIcmdWithADoubleAndUnit* fGammaCutCMD;
    G4UIcmdWithADoubleAndUnit* fElectCutCMD;
    G4UIcmdWithADoubleAndUnit* fPosCutCMD;
    G4UIcmdWithADoubleAndUnit* fAllCutCMD;
    G4UIcmdWithADoubleAndUnit* fStepMaxCMD;
    G4UIcmdWithABool*          fEnforceEMCutsCMD;

    G4UIcmdWithAString*        fRemovePhysicsCMD;
    G4UIcmdWithoutParameter*   fClearPhysicsCMD;

    G4UIcmdWithoutParameter*   fListCMD;

    G4UIcmdWithoutParameter* fPienuCMD;
    G4UIcmdWithoutParameter* fPimunuCMD;

};

#endif
