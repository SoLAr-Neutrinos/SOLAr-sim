/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArSuperCellHit
 * @created     : venerdì ott 21, 2022 10:27:34 CEST
 */

#ifndef SLArSuperCellHit_h
#define SLArSuperCellHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4Transform3D.hh"
#include "G4RotationMatrix.hh"

class G4AttDef;
class G4AttValue;

/// SuperCell hit
///
/// It records:
/// - the SuperCell ID
/// - the particle time
/// - the particle local and global positions


class SLArSuperCellHit : public G4VHit
{
public:
    SLArSuperCellHit();
    SLArSuperCellHit(G4double z);
    SLArSuperCellHit(const SLArSuperCellHit &right);
    virtual ~SLArSuperCellHit();

    const SLArSuperCellHit& operator=(const SLArSuperCellHit &right);
    int operator==(const SLArSuperCellHit &right) const;
    
    inline void *operator new(size_t);
    inline void operator delete(void *aHit);
    
    virtual void Draw();
    virtual const std::map<G4String,G4AttDef>* GetAttDefs() const;
    virtual std::vector<G4AttValue>* CreateAttValues() const;
    virtual void Print();

    void SetPhotonEnergy(G4double z) { fEnergy = z; }
    G4double GetPhotonEnergy() const { return fEnergy; }

    void SetPhotonWavelength(G4double z) { fWavelength = z; }
    G4double GetPhotonWavelength() const { return fWavelength; }

    void SetTime(G4double t) { fTime = t; }
    G4double GetTime() const { return fTime / CLHEP::ns; }

    inline void SetProducerID(const int trk_id) {fPhProducerID = trk_id;}
    inline G4int GetProducerID() const {return fPhProducerID;}

    void SetLocalPos(G4ThreeVector xyz) { fLocalPos = xyz; }
    G4ThreeVector GetLocalPos() const { return fLocalPos; }

    void SetWorldPos(G4ThreeVector xyz) { fWorldPos = xyz; }
    G4ThreeVector GetWorldPos() const { return fWorldPos; }

    void      SetPhotonProcess(G4String prname);
    void      SetPhotonProcess(const int prid) { fPhType = prid; }
    G4int     GetPhotonProcessId() const;
    G4String  GetPhotonProcessName() const;

    inline void SetOriginVolumeID(const int vol_id) {fPhOriginVolumeID = vol_id;}
    inline G4int GetOriginVolumeID() const {return fPhOriginVolumeID;}

    inline void SetSuperCellNo(G4int idx) { fSuperCellNo= idx; }
    inline G4int GetSuperCellNo() const { return fSuperCellNo; }
    inline void SetSuperCellRowNo(G4int n) {fSuperCellRowNo = n; }
    inline G4int GetSuperCellRowNo() const {return fSuperCellRowNo;}
    inline void SetSuperCellArrayNo(G4int n) {fSuperCellArrayNo = n; }
    inline G4int GetSuperCellArrayNo() const {return fSuperCellArrayNo;}
    G4String GetProcessName(int kType);

private:
    G4int         fSuperCellNo;
    G4int         fSuperCellRowNo;
    G4int         fSuperCellArrayNo;
    G4double      fEnergy;
    G4double      fWavelength; 
    G4double      fTime;
    G4int         fPhType;
    G4int         fPhProducerID;
    G4int         fPhOriginVolumeID;
    G4ThreeVector fLocalPos;
    G4ThreeVector fWorldPos;
};

typedef G4THitsCollection<SLArSuperCellHit> SLArSuperCellHitsCollection;

extern G4ThreadLocal G4Allocator<SLArSuperCellHit>* SLArSuperCellHitAllocator;

inline void* SLArSuperCellHit::operator new(size_t)
{
    if (!SLArSuperCellHitAllocator)
        SLArSuperCellHitAllocator = new G4Allocator<SLArSuperCellHit>;
    return (void*)SLArSuperCellHitAllocator->MallocSingle();
}

inline void SLArSuperCellHit::operator delete(void* aHit)
{
    SLArSuperCellHitAllocator->FreeSingle((SLArSuperCellHit*) aHit);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
