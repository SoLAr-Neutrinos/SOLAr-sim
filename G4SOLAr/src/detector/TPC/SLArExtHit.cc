/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArExtHit.hh
 * @created     : Friday Feb 09, 2024 17:03:08 CET
 */

#include "detector/TPC/SLArExtHit.hh"

#include "G4AttDef.hh"
#include "G4VVisManager.hh"
#include "G4ios.hh"


G4ThreadLocal G4Allocator<SLArExtHit>* SLArExtHitAllocator;


SLArExtHit::SLArExtHit() : G4VHit(),
  fEvNumber(0), fPDGCode(0), fTrkID(-1), fParentID(-1), fOriginVol(-1),
  fOriginEnergy(0.0), fEnergy(0.0), fTime(0.0), fWeight(1.0), fVertex{0.0}, 
  fCreator("")
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArExtHit::~SLArExtHit()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArExtHit::SLArExtHit(const SLArExtHit &right) : G4VHit(),
  fEvNumber(right.fEvNumber), fPDGCode(right.fPDGCode), fTrkID(right.fTrkID), 
  fParentID(right.fParentID), fOriginVol(right.fOriginVol), 
  fOriginEnergy(right.fOriginEnergy), fEnergy(right.fEnergy), fTime(right.fTime), 
  fWeight(right.fWeight), fVertex{0}, fCreator(right.fCreator) 
{
  for (size_t i = 0; i < 3; i++) {
    fVertex[i] = right.fVertex[i];
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const SLArExtHit& SLArExtHit::operator=(const SLArExtHit &right)
{
  fEvNumber = right.fEvNumber;
  fPDGCode = right.fPDGCode;
  fTrkID = right.fTrkID;
  fParentID = right.fParentID;
  fOriginVol = right.fOriginVol;
  fOriginEnergy = right.fOriginEnergy;
  fEnergy = right.fEnergy;
  fTime = right.fTime;
  fWeight = right.fWeight;
  fCreator = right.fCreator;
  for (size_t i = 0; i < 3; i++) {
    fVertex[i] = right.fVertex[i];
  }

  return *this;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool SLArExtHit::operator==(const SLArExtHit &right) const
{
  G4bool is_equal = true; 
  is_equal *= (fEvNumber == right.fEvNumber);
  is_equal *= (fPDGCode == right.fPDGCode);
  is_equal *= (fTrkID == right.fTrkID);
  is_equal *= (fParentID == right.fParentID);
  is_equal *= (fOriginVol == right.fOriginVol);
  is_equal *= (fOriginEnergy == right.fOriginEnergy);
  is_equal *= (fEnergy == right.fEnergy);
  is_equal *= (fTime == right.fTime);
  is_equal *= (fWeight == right.fWeight);
  is_equal *= (fCreator == right.fCreator);
  for (size_t i = 0; i < 3; i++) {
    is_equal *= (fVertex[i] == right.fVertex[i]);
  }

  return is_equal;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArExtHit::Print() const
{
  printf("SLArExtHit::Print()\n");
  printf("event %i:\n", fEvNumber); 
  printf("pdg: %i - trkID: %i - parentID: %i\n", fPDGCode, fTrkID, fParentID); 
  printf("Origin: [%g, %g, %g] (copy nr %i) - creator: %s - initial energy: %g\n", 
      fVertex[0], fVertex[1], fVertex[2], fOriginVol, fCreator.data(), fOriginEnergy); 
  printf("time: %g, weight: %g\n", fTime, fWeight); 

  return;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArExtHit::reset() {
  fEvNumber = 0;
  fPDGCode = 0;
  fTrkID = 0;
  fParentID = 0;
  fOriginVol = 0;
  fOriginEnergy = 0;
  fEnergy = 0;
  fTime = 0;
  fWeight = 0;
  for (size_t i = 0; i < 3; i++) {
    fVertex[i] = 0.0;
  }
  fCreator = "";
  
  return;
}

