/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArLArHit.hh
 * @created     : giovedì nov 03, 2022 12:27:32 CET
 */

#ifndef SLARLARHIT_HH

#define SLARLARHIT_HH


#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4Transform3D.hh"
#include "G4RotationMatrix.hh"

class G4AttDef;
class G4AttValue;

/// Particle interaction inside the LAr volume
///
/// It records:
///
/// - the energy deposit
/// - the particle time (at entrance)
/// - the particle global position (at entrance)


class SLArLArHit : public G4VHit
{
  public:
    SLArLArHit();
    SLArLArHit(const SLArLArHit &right);
    virtual ~SLArLArHit();

    const SLArLArHit& operator=(const SLArLArHit &right);
    int operator==(const SLArLArHit &right) const;

    inline void *operator new(size_t);
    inline void operator delete(void *aHit);

    virtual const std::map<G4String,G4AttDef>* GetAttDefs() const;
    virtual std::vector<G4AttValue>* CreateAttValues() const;
    virtual void Print();

    void SetDepositedEnergy(G4double z) { fEneDep = z; }
    G4double GetDepositedEnergy() const { return fEneDep; }

    void Add(double ed);

    void      SetParticleType(G4String prname);
    G4String  GetParticleType();

  private:
    G4double      fEneDep;
    G4int         fNsecondaries;
    //G4int         fCounterE; // electron-positron counter
    //G4int         fCounterP; // proton counter
    //G4int         fCounterG; // gamma counter
    //G4int         fCounterPI;// pion counter
    G4String      fParticleType;
};

typedef G4THitsCollection<SLArLArHit> SLArLArHitsCollection;

extern G4ThreadLocal G4Allocator<SLArLArHit>* SLArLArHitAllocator;

inline void* SLArLArHit::operator new(size_t)
{
  if (!SLArLArHitAllocator)
    SLArLArHitAllocator = new G4Allocator<SLArLArHit>;
  return (void*)SLArLArHitAllocator->MallocSingle();
}

inline void SLArLArHit::operator delete(void* aHit)
{
  SLArLArHitAllocator->FreeSingle((SLArLArHit*) aHit);
}

#endif /* end of include guard SLARLARHIT_HH */



