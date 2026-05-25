/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArRun.hh
 * @created     : giovedì nov 03, 2022 19:05:05 CET
 */

#ifndef SLARRUN_HH

#define SLARRUN_HH

#include "G4Run.hh"
#include "G4Event.hh"
#include "G4THitsMap.hh"


class SLArRun : public G4Run {

  public:
    // constructor and destructor.
    //  vector of multifunctionaldetector name has to given to constructor.
    SLArRun(const std::vector<G4String> mfdName);
    virtual ~SLArRun();

  public:
    // virtual method from G4Run. 
    // The method is overriden in this class for scoring.
    virtual void RecordEvent(const G4Event*);
    virtual void Merge(const G4Run*);

    // Access methods for scoring information.
    // - Number of HitsMap for this RUN. 
    //   This is equal to number of collections.
    G4int GetNumberOfHitsMap() const {return fRunMap.size();}
    // - Get HitsMap of this RUN.
    //   by sequential number, by multifucntional name and collection name,
    //   and by collection name with full path.
    G4THitsMap<G4double>* GetHitsMap(G4int i){return fRunMap[i];}
    G4THitsMap<G4double>* GetHitsMap(const G4String& detName, 
        const G4String& colName);
    G4THitsMap<G4double>* GetHitsMap(const G4String& fullName);
    // - Dump All HitsMap of this RUN.
    //   This method calls G4THisMap::PrintAll() for individual HitsMap.
    void DumpAllScorer();

  private:
    std::vector<G4String> fCollName;
    std::vector<G4int> fCollID;
    std::vector<G4THitsMap<G4double>*> fRunMap;
};




#endif /* end of include guard SLARRUN_HH */

