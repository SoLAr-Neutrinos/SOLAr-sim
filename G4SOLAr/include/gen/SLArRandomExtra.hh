/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArRandomExtra.hh
 * @created     Thursday Sep 21, 2023 18:15:05 CEST
 */

#ifndef SLARRANDOMEXTRA_HH

#define SLARRANDOMEXTRA_HH

#include <memory>

#include "G4ThreeVector.hh"
#include "G4RandomTools.hh"

#include "TRandom3.h"
#include "TH1.h"

class SLArRandom {
  public:
    inline SLArRandom() {
      fRandomEngine = std::make_unique<TRandom3>( G4Random::getTheSeed() ); 
    }
    inline ~SLArRandom() {}; 

    std::unique_ptr<TRandom3>& GetEngine() {return fRandomEngine;}
    inline G4double SampleFromHist(const TH1& h) {return h.GetRandom(fRandomEngine.get());}
    inline G4double SampleFromHist(const TH1* h) {return h->GetRandom(fRandomEngine.get());}
    static G4ThreeVector SampleRandomDirection();
    static G4ThreeVector SampleLinearPolarization(const G4ThreeVector& momentum); 
    static std::array<G4ThreeVector,2> SampleRandomDirectionAndPolarization();

  protected: 
    std::unique_ptr<TRandom3> fRandomEngine; 
};

#endif /* end of include guard SLARRANDOMEXTRA_HH */

