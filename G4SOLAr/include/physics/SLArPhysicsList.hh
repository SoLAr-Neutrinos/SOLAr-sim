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
/// \file include/SLArPhysicsList.hh
/// \brief Definition of the WLSPhysicsList class
///   
/// reimplemented from wls G4 example

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#ifndef SLArPhysicsList_h
#define SLArPhysicsList_h 1

#include "globals.hh"
#include "G4VModularPhysicsList.hh"

class G4VPhysicsConstructor;
class SLArPhysicsListMessenger;

class SLArStepMax;
class SLArOpticalPhysics;

class SLArPhysicsList: public G4VModularPhysicsList
{
  public:

    SLArPhysicsList(G4String, G4bool);
    virtual ~SLArPhysicsList();

    inline bool DoTraceOptPhotons() const {return fDoTraceOptPhotons;}
    inline bool DoDriftElectrons() const {return fDoDriftElectrons;}
    inline void SetTraceOptPhotons(bool do_trace) {fDoTraceOptPhotons = do_trace;}
    inline void SetDriftElectrons(bool do_drift) {fDoDriftElectrons = do_drift;}

    void SetCuts();
    void SetCutForGamma(G4double);
    void SetCutForElectron(G4double);
    void SetCutForPositron(G4double);
    void EnforceEMCuts(G4bool);

    void SetStepMax(G4double);
    SLArStepMax* GetStepMaxProcess();
    void AddStepMax();

    //! Remove specific physics from physics list.
    void RemoveFromPhysicsList(const G4String&);

    //! Make sure that the physics list is empty.
    void ClearPhysics();

    virtual void ConstructParticle();
    virtual void ConstructProcess();

    //! Turn on or off the absorption process
    void SetAbsorption(G4bool);
    void SetNbOfPhotonsCerenkov(G4int);

    void SetVerbose(G4int);

  private:
    G4bool fDoDriftElectrons = true;
    G4bool fDoTraceOptPhotons = true;

    G4double fCutForGamma;
    G4double fCutForElectron;
    G4double fCutForPositron;

    SLArStepMax* fStepMaxProcess;

    SLArOpticalPhysics* fOpticalPhysics;

    SLArPhysicsListMessenger* fMessenger;

    G4bool fAbsorptionOn;
    G4bool fCerenkovOn;

    G4VMPLData::G4PhysConstVectorData* fPhysicsVector;
};

#endif
