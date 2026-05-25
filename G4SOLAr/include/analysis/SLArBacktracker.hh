/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArBacktracker.hh
 * @created     Thursday Sep 28, 2023 18:58:08 CEST
 */

#ifndef SLARBACKTRACKER_HH

#define SLARBACKTRACKER_HH

#include <vector>
#include <functional>
#include "G4String.hh"

class SLArEventGenericHit;
class SLArEventPhotonHit;
class SLArEventBacktrackerRecord;

namespace backtracker{

enum EBkTrkReadoutSystem {kNoSystem = -1, kCharge = 0, kVUVSiPM = 1, kSuperCell = 2};
extern const G4String BkTrkReadoutSystemTag[3]; 
EBkTrkReadoutSystem GetBacktrackerReadoutSystem(const G4String sys);

enum EBacktracker {kNoBacktracker = -1, kTrkID = 0, kAncestorID = 1, kOpticalProc = 2, kSiPMNr = 3};
extern const G4String BacktrackerLabel[4];
EBacktracker GetBacktrackerEnum(const G4String bkt);

class SLArBacktracker {
  public: 
    SLArBacktracker(); 
    SLArBacktracker(const G4String name);
    inline ~SLArBacktracker() {}; 
    
    inline virtual void Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) {}; 
    inline G4String GetName() const {return fName;}
    inline void SetName(const G4String name) {fName = name;}

  protected:
    G4String fName;
};

class SLArBacktrackerTrkID : public SLArBacktracker {
  public: 
    inline SLArBacktrackerTrkID() : SLArBacktracker() {}
    inline SLArBacktrackerTrkID(const G4String name) : SLArBacktracker(name) {}
    inline ~SLArBacktrackerTrkID() {}

    void Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) override;
};

class SLArBacktrackerAncestorID : public SLArBacktracker {
  public: 
    inline SLArBacktrackerAncestorID() : SLArBacktracker() {}
    inline SLArBacktrackerAncestorID(const G4String name) : SLArBacktracker(name) {}
    inline ~SLArBacktrackerAncestorID() {}

    void Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) override;
};

class SLArBacktrackerOpticalProcess : public SLArBacktracker {
  public: 
    inline SLArBacktrackerOpticalProcess() : SLArBacktracker() {}
    inline SLArBacktrackerOpticalProcess(const G4String name) : SLArBacktracker(name) {}
    inline ~SLArBacktrackerOpticalProcess() {}

    void Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) override;
};

class SLArBacktrackerSiPMNr : public SLArBacktracker {
  public: 
    inline SLArBacktrackerSiPMNr() : SLArBacktracker() {}
    inline SLArBacktrackerSiPMNr(const G4String name) : SLArBacktracker(name) {}
    inline ~SLArBacktrackerSiPMNr() {}

    void Eval(SLArEventGenericHit* hit, SLArEventBacktrackerRecord* rec) override;
};


}





#endif /* end of include guard SLARBACKTRACKER_HH */

