/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArReadoutTileSD
 * @created     : mercoledì ago 10, 2022 08:54:48 CEST
 */

#ifndef SLARREADOUTTILESD_HH

#define SLARREADOUTTILESD_HH

#include "G4VSensitiveDetector.hh"

#include "SensitiveDetectors/SLArReadoutTileHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class SLArReadoutTileSiPMSD : public G4VSensitiveDetector
{
public:
    SLArReadoutTileSiPMSD(G4String name);
    virtual ~SLArReadoutTileSiPMSD();
    
    virtual void Initialize(G4HCofThisEvent*HCE);
    virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
    G4bool ProcessHits_constStep(const G4Step* ,
                                 G4TouchableHistory* );
   
private:
    SLArReadoutTileHitsCollection* fHitsCollection;
    G4int fHCID;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /* end of include guard SLARREADOUTTILESD_HH */
