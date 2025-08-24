/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArReadoutTileHit.hh
 * @created     Wed Aug 10, 2022 08:56:55 CEST
 */

#ifndef SLARREADOUTTILEHIT_HH

#define SLARREADOUTTILEHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4Transform3D.hh"
#include "G4RotationMatrix.hh"

class G4AttDef;
class G4AttValue;

/// ReadoutTile hit
///
/// It records:
/// - the ReadoutTile ID
/// - the particle time
/// - the particle local and global positions


class SLArReadoutTileHit : public G4VHit
{
public:
    SLArReadoutTileHit();
    SLArReadoutTileHit(G4double z);
    SLArReadoutTileHit(const SLArReadoutTileHit &right);
    virtual ~SLArReadoutTileHit();

    const SLArReadoutTileHit& operator=(const SLArReadoutTileHit &right);
    int operator==(const SLArReadoutTileHit &right) const;
    
    inline void *operator new(size_t);
    inline void operator delete(void *aHit);
    
    virtual void Draw();
    virtual const std::map<G4String,G4AttDef>* GetAttDefs() const;
    virtual std::vector<G4AttValue>* CreateAttValues() const;
    virtual void Print();

    void SetPhotonWavelength(G4double z) { fWavelength = z; }
    G4double GetPhotonWavelength() const { return fWavelength; }

    void SetTime(G4double t) { fTime = t; }
    G4double GetTime() const { return fTime / CLHEP::ns; }

    void SetLocalPos(G4ThreeVector xyz) { fLocalPos = xyz; }
    G4ThreeVector GetLocalPos() const { return fLocalPos; }

    void SetWorldPos(G4ThreeVector xyz) { fWorldPos = xyz; }
    G4ThreeVector GetWorldPos() const { return fWorldPos; }

    void SetPhotonProcess(G4String prname);
    inline void SetPhotonProcess(const int prid) { fPhType = prid; }
    G4int     GetPhotonProcessId() const;
    G4String  GetPhotonProcessName() const;

    inline void SetAnodeIdx(G4int idx) { fAnodeIdx= idx; }
    inline G4int GetAnodeIdx() const { return fAnodeIdx; }
    inline void SetRowMegaTileReplicaNr(G4int idx) { fRowMegaTileReplicaNr= idx; }
    inline G4int GetRowMegaTileReplicaNr() const { return fRowMegaTileReplicaNr; }
    inline void SetMegaTileReplicaNr(G4int idx) { fMegaTileReplicaNr= idx; }
    inline G4int GetMegaTileReplicaNr() const { return fMegaTileReplicaNr; }
    inline void SetRowTileReplicaNr(G4int idx) { fRowTileReplicaNr= idx; }
    inline G4int GetRowTileReplicaNr() const { return fRowTileReplicaNr; }
    inline void SetTileReplicaNr(G4int idx) { fTileReplicaNr= idx; }
    inline G4int GetTileReplicaNr() const { return fTileReplicaNr; }
    inline void SetCellNr(G4int n) {fCellNr = n;}
    inline G4int GetCellNr() const {return fCellNr;}
    inline void SetProducerID(const int trk_id) {fPhProducerID = trk_id;}
    inline G4int GetProducerID() const {return fPhProducerID;}
    inline void SetOriginVolumeID(const int vol_id) {fPhOriginVolumeID = vol_id;}
    inline G4int GetOriginVolumeID() const {return fPhOriginVolumeID;}
    void SetRowCellNr(G4int n) {fRowCellNr = n;}
    G4int GetRowCellNr() {return fRowCellNr;}

    G4String GetProcessName(int kType) const;

private:
    G4int         fAnodeIdx; 
    G4int         fRowMegaTileReplicaNr;
    G4int         fMegaTileReplicaNr;
    G4int         fRowTileReplicaNr; 
    G4int         fTileReplicaNr;
    G4int         fRowCellNr; 
    G4int         fCellNr; 
    G4double      fWavelength;
    G4float       fTime;
    G4int         fPhType;
    G4int         fPhProducerID;
    G4int         fPhOriginVolumeID;
    G4ThreeVector fLocalPos;
    G4ThreeVector fWorldPos;
};

typedef G4THitsCollection<SLArReadoutTileHit> SLArReadoutTileHitsCollection;

extern G4ThreadLocal G4Allocator<SLArReadoutTileHit>* SLArReadoutTileHitAllocator;

inline void* SLArReadoutTileHit::operator new(size_t)
{
    if (!SLArReadoutTileHitAllocator)
        SLArReadoutTileHitAllocator = new G4Allocator<SLArReadoutTileHit>;
    return (void*)SLArReadoutTileHitAllocator->MallocSingle();
}

inline void SLArReadoutTileHit::operator delete(void* aHit)
{
    SLArReadoutTileHitAllocator->FreeSingle((SLArReadoutTileHit*) aHit);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /* end of include guard SLARREADOUTTILEHIT_HH */
