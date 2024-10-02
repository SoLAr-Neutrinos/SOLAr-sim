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
// $Id: SLArSuperCellSD.hh 76474 2013-11-11 10:36:34Z gcosmo $
//
/// \file SLArSuperCellSD.hh
/// \brief Definition of the SLArSuperCellSD class

#ifndef SLArSuperCellSD_h
#define SLArSuperCellSD_h 1

#include "G4VSensitiveDetector.hh"

#include "detector/SuperCell/SLArSuperCellHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

/// Drift chamber sensitive detector

class SLArSuperCellSD : public G4VSensitiveDetector
{
public:
    SLArSuperCellSD(G4String name);
    virtual ~SLArSuperCellSD();
    
    virtual void Initialize(G4HCofThisEvent*HCE);
    virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
    G4bool ProcessHits_constStep(const G4Step* ,
                                 G4TouchableHistory* );

    int ph_hits = 0;
    double E_Kin = 0;
    std::vector<double> xVec = {};
    std::vector<double> yVec = {};
    std::vector<double> zVec = {};
    std::vector<double> PosVec = {}; 
    
    std::vector<double> energyVec ={}; 
    std::vector<double> wavelengthVec={}; 
    const double h = 6.626e-34;
    const double c = 2.99e8;
    const double e_charge = 1.6e-19;
    const double hc = (h*c)/e_charge;    
   
private:
    SLArSuperCellHitsCollection* fHitsCollection;
    G4int fHCID;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
