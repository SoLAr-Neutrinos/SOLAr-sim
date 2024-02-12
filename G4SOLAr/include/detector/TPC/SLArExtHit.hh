/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArExtHit.hh
 * @created     : Friday Feb 09, 2024 16:27:51 CET
 */

#ifndef SLAREXTHIT_HH

#define SLAREXTHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"


/**
 * @class SLArExtHit
 * @brief Hit class for external background scorers 
 *
 */
class SLArExtHit : public G4VHit
{
  public:
    SLArExtHit();
    SLArExtHit(const SLArExtHit &right);
    virtual ~SLArExtHit();

    const SLArExtHit& operator=(const SLArExtHit &right);
    G4bool operator==(const SLArExtHit &right) const;

    inline void *operator new(size_t);
    inline void operator delete(void *aHit);

    virtual void Print() const;

    void reset(); 

    G4int fEvNumber;
    G4int fPDGCode;
    G4int fTrkID;
    G4int fParentID;
    G4int fOriginVol;
    G4float fOriginEnergy;
    G4float fEnergy;
    G4float fTime;
    G4float fWeight;
    G4float fVertex[3];
    G4String fCreator;
};

typedef G4THitsCollection<SLArExtHit> SLArExtHitsCollection;

extern G4ThreadLocal G4Allocator<SLArExtHit>* SLArExtHitAllocator;

inline void* SLArExtHit::operator new(size_t)
{
  if (!SLArExtHitAllocator)
    SLArExtHitAllocator = new G4Allocator<SLArExtHit>;
  return (void*)SLArExtHitAllocator->MallocSingle();
}

inline void SLArExtHit::operator delete(void* aHit)
{
  SLArExtHitAllocator->FreeSingle((SLArExtHit*) aHit);
}

#endif /* end of include guard SLAREXTHIT_HH */

