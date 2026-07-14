/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArRun.cc
 * @created     : Thur Nov 03, 2022 19:09:24 CET
 */

#include "SLArRun.hh"

#include "G4SDManager.hh"

#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
//  Constructor. 
//  (The vector of MultiFunctionalDetector name has to given.)
SLArRun::SLArRun(const std::vector<G4String> mfdName) : G4Run()
{
  printf("SLArRun constructor\n");
  G4SDManager* pSDman = G4SDManager::GetSDMpointer();
  pSDman->ListTree(); 
  //=================================================
  //  Initalize RunMaps for accumulation.
  //  Get CollectionIDs for HitCollections.
  //=================================================
  G4int nMfd = mfdName.size();
  for ( G4int idet = 0; idet < nMfd ; idet++){  // Loop for all MFD.
    G4String detName = mfdName[idet];
    //--- Seek and Obtain MFD objects from SDmanager.
    G4MultiFunctionalDetector* mfd =
      (G4MultiFunctionalDetector*)(pSDman->FindSensitiveDetector(detName));
    //
    if ( mfd ){
      printf("%s found in SDmanager\n", detName.c_str());
      //--- Loop over the registered primitive scorers.
      for (G4int icol = 0; icol < mfd->GetNumberOfPrimitives(); icol++){
        // Get Primitive Scorer object.
        G4VPrimitiveScorer* scorer=mfd->GetPrimitive(icol);
        // collection name and collectionID for HitsCollection,
        // where type of HitsCollection is G4THitsMap in case of primitive 
        // scorer.
        // The collection name is given by <MFD name>/<Primitive Scorer name>.
        G4String collectionName = scorer->GetName();
        G4String fullCollectionName = detName+"/"+collectionName;
        G4int    collectionID = pSDman->GetCollectionID(fullCollectionName);
        //
        /*
         *for (int k=0; k<pSDman->GetHCtable()->entries(); k++) {
         *  printf("%i: %s\n", k, 
         *      pSDman->GetHCtable()->GetHCname(k).c_str()); 
         *}
         */
        if ( collectionID >= 0 ){
          G4cout << "++ "<<fullCollectionName<< " id " << collectionID
                 << G4endl;
          // Store obtained HitsCollection information into data members.
          // And, creates new G4THitsMap for accumulating quantities during RUN.
          fCollName.push_back(fullCollectionName);
          fCollID.push_back(collectionID);
          fRunMap.push_back(new G4THitsMap<G4double>(detName,collectionName));
        }else{
          G4cout << "** collection " << fullCollectionName << " not found. "
                 << G4endl;
        }
      }
    }
    else {
      printf("SLArRUN::SLArRun WARNING: %s NOT found in SDmanager\n", detName.c_str());
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
// Destructor
//    clear all data members.
SLArRun::~SLArRun()
{
  //--- Clear HitsMap for RUN
  G4int nMap = fRunMap.size();
  for ( G4int i = 0; i < nMap; i++){
    if(fRunMap[i] ) fRunMap[i]->clear();
  }
  fCollName.clear();
  fCollID.clear();
  fRunMap.clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
//  RecordEvent is called at end of event.
//  For scoring purpose, the resultant quantity in a event,
//  is accumulated during a Run.
void SLArRun::RecordEvent(const G4Event* aEvent)
{
  numberOfEvent++;  // This is an original line.

  //=============================
  // HitsCollection of This Event
  //============================
  G4HCofThisEvent* pHCE = aEvent->GetHCofThisEvent();
  if (!pHCE) return;

  //=======================================================
  // Sum up HitsMap of this Event  into HitsMap of this RUN
  //=======================================================
  G4int nCol = fCollID.size();
  for ( G4int i = 0; i < nCol ; i++ ){  // Loop over HitsCollection
    G4THitsMap<G4double>* evtMap=0;
    if ( fCollID[i] >= 0 ){           // Collection is attached to pHCE
      evtMap = (G4THitsMap<G4double>*)(pHCE->GetHC(fCollID[i]));
      //printf("[%i] evtMap size: %lu\n", fCollID[i], evtMap->size() );
    }else{
      G4cout <<" Error evtMap Not Found "<< i << G4endl;
    }
    if ( evtMap )  {
      //=== Sum up HitsMap of this event to HitsMap of RUN.===
      *fRunMap[i] += *evtMap;
      //======================================================
    }
  }

  G4Run::RecordEvent(aEvent); 
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void SLArRun::Merge(const G4Run * aRun) {
  const SLArRun * localRun = static_cast<const SLArRun *>(aRun);
  
  //=======================================================
  // Merge HitsMap of working threads
  //=======================================================
  G4int nCol = localRun->fCollID.size();
  for ( G4int i = 0; i < nCol ; i++ ){  // Loop over HitsCollection
    if ( localRun->fCollID[i] >= 0 ){
      *fRunMap[i] += *localRun->fRunMap[i];
    }
  }
  
  G4Run::Merge(aRun);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
//  Access method for HitsMap of the RUN
//
//-----
// Access HitsMap.
//  By  MultiFunctionalDetector name and Collection Name.
G4THitsMap<G4double>* SLArRun::GetHitsMap(const G4String& detName,
                                         const G4String& colName){
    G4String fullName = detName+"/"+colName;
    return GetHitsMap(fullName);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
//-----
// Access HitsMap.
//  By full description of collection name, that is
//    <MultiFunctional Detector Name>/<Primitive Scorer Name>
G4THitsMap<G4double>* SLArRun::GetHitsMap(const G4String& fullName){
    G4int nCol = fCollName.size();
    for ( G4int i = 0; i < nCol; i++){
        if ( fCollName[i] == fullName ){
            return fRunMap[i];
        }
    }
    return NULL;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//
//-----
// - Dump All HitsMap of this RUN. (for debuging and monitoring of quantity).
//   This method calls G4THisMap::PrintAll() for individual HitsMap.
void SLArRun::DumpAllScorer(){

  // - Number of HitsMap in this RUN.
  G4int n = GetNumberOfHitsMap();
  // - GetHitsMap and dump values.
  for ( G4int i = 0; i < n ; i++ ){
    G4THitsMap<G4double>* runMap =GetHitsMap(i);
    if ( runMap ) {
      G4cout << " PrimitiveScorer RUN " 
             << runMap->GetSDname() <<","<< runMap->GetName() << G4endl;
      G4cout << " Number of entries " << runMap->entries() << G4endl;
      std::map<G4int,G4double*>::iterator itr = runMap->GetMap()->begin();
      for(; itr != runMap->GetMap()->end(); itr++) {
        G4cout << "  copy no.: " << itr->first
               << "  Run Value : " << *(itr->second) 
               << G4endl;
      }
    }
  }
}

