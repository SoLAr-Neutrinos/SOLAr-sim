/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArLArHit.cc
 * @created     : giovedì nov 03, 2022 12:29:04 CET
 */


#include "SensitiveDetectors/SLArLArHit.hh"

#include "G4VVisManager.hh"
#include "G4VisAttributes.hh"
#include "G4AttDefStore.hh"
#include "G4AttDef.hh"
#include "G4AttValue.hh"
#include "G4UnitsTable.hh"
#include "G4ios.hh"


G4ThreadLocal G4Allocator<SLArLArHit>* SLArLArHitAllocator;


SLArLArHit::SLArLArHit()
: G4VHit(), fEneDep(0.), fNsecondaries(0), fParticleType()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArLArHit::~SLArLArHit()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SLArLArHit::SLArLArHit(const SLArLArHit &right)
: G4VHit() {
    fEneDep       = right.fEneDep;
    fNsecondaries = right.fNsecondaries;
    fParticleType = right.fParticleType;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const SLArLArHit& SLArLArHit::operator=(const SLArLArHit &right)
{
    fEneDep       = right.fEneDep;
    fNsecondaries = right.fNsecondaries;
    fParticleType = right.fParticleType;
    return *this;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int SLArLArHit::operator==(const SLArLArHit &/*right*/) const
{
    return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const std::map<G4String,G4AttDef>* SLArLArHit::GetAttDefs() const
{
    G4bool isNew;
    std::map<G4String,G4AttDef>* store
      = G4AttDefStore::GetInstance("SLArLArHit",isNew);

    if (isNew) {
        (*store)["Particle"] 
          = G4AttDef("Particle","Particle","Physics","","G4String");
        
        (*store)["EneDep"] 
          = G4AttDef("EneDep","Energy Deposit","Physics","G4BestUnit","G4double");
    }
    return store;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

std::vector<G4AttValue>* SLArLArHit::CreateAttValues() const
{
    std::vector<G4AttValue>* values = new std::vector<G4AttValue>;
    
    values
      ->push_back(G4AttValue("EneDep",G4BestUnit(fEneDep,"Energy"), ""));
    values
      ->push_back(G4AttValue("Particle", fParticleType, ""));
    
    return values;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArLArHit::Print()
{
  printf("LAr hit\n");
  G4cout << G4BestUnit(fEneDep, "Energy") << " energy deposit" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SLArLArHit::SetParticleType(G4String prname)
{
  fParticleType = prname;
}

void SLArLArHit::Add(double ed)
{
  fEneDep += ed;
}

G4String SLArLArHit::GetParticleType()
{
  return fParticleType;
}
