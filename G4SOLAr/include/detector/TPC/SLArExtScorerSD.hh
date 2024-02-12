/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArExtScorerSD.hh
 * @created     : Friday Feb 09, 2024 12:17:33 CET
 */

#ifndef SLAREXTSCORERSD_HH

#define SLAREXTSCORERSD_HH

#include "G4VSensitiveDetector.hh"
#include "detector/TPC/SLArExtHit.hh"

class G4Step;  
class G4HCofThisEvent;
class G4TouchableHistory;

class SLArExtScorerSD : public G4VSensitiveDetector
{
  public: 
    SLArExtScorerSD(G4String name); 
    virtual ~SLArExtScorerSD(); 

    void Initialize(G4HCofThisEvent*HCE) override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* touchHistory) override; 
    G4int GetHitsCollectionID() const {return fHCID;}

  private: 
    SLArExtHitsCollection* fHitsCollection;
    G4int    fHCID;
};

#endif /* end of include guard SLAREXTSCORERSD_HH */

