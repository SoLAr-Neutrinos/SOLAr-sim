/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArLArSD.hh
 * @created     : giovedì nov 03, 2022 12:26:11 CET
 */

#ifndef SLARLARSD_HH

#define SLARLARSD_HH

#include "G4VSensitiveDetector.hh"

#include "SensitiveDetectors/SLArLArHit.hh"

class G4Step;  
class G4HCofThisEvent;
class G4TouchableHistory;

/// LAr-volume sensitive detector

class SLArLArSD : public G4VSensitiveDetector
{
public:
    SLArLArSD(G4String name, G4int tpcID);
    virtual ~SLArLArSD();
    
    virtual void Initialize(G4HCofThisEvent*HCE);
    virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);
   
private:
    SLArLArHitsCollection* fHitsCollection;
    G4int    fHCID;
    G4int    fTPCID;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /* end of include guard SLARLARSD_HH */
