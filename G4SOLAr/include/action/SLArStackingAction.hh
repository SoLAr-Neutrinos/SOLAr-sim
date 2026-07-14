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
/// 
//  @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
//  @file        : SLArStackingAction.hh
//  @created     : Tuesday Aug 05, 2025 15:38:10 CEST
//  Definition of the SLArStackingAction class
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include <G4ClassificationOfNewTrack.hh>
#ifndef SLArStackingAction_H
#define SLArStackingAction_H 1

#include "globals.hh"
#include "G4UserStackingAction.hh"

class SLArEventAction;
class G4Track;
class SLArMCPrimaryInfo;
class SLArAnalysisManager;

class SLArStackingAction : public G4UserStackingAction
{
  public:
    SLArStackingAction(SLArEventAction*);
    virtual ~SLArStackingAction();

  public:
    virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* aTrack);
    virtual void NewStage();
    virtual void PrepareNewEvent();

  private:
    SLArEventAction* fEventAction;
    G4ClassificationOfNewTrack ClassifyNewTrackOpticalPhoton(const G4Track* aTrack) const;
    G4ClassificationOfNewTrack ClassifyNewGeneralTrack(const G4Track* aTrack) const;
    bool PositivePrimaryIdentification(const G4Track*, SLArMCPrimaryInfo&) const;
    void UpdatePrimaryTrackID(const G4Track* aTrack, SLArAnalysisManager* creatorProc) const;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
