/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventSiPM.hh
 * @created     : Thursday Apr 17, 2025 17:00:35 CEST
 */

#ifndef SLAREVENTSIPM_HH

#define SLAREVENTSIPM_HH

#include "event/SLArEventHitsCollection.hh"
#include "event/SLArEventPhotonHit.hh"

class SLArEventSiPM : public SLArEventHitsCollection<SLArEventPhotonHit> {
  public: 
    inline SLArEventSiPM() : SLArEventHitsCollection<SLArEventPhotonHit>() {} 

    inline SLArEventSiPM(const int& idx) 
      : SLArEventHitsCollection<SLArEventPhotonHit>(idx) 
    {
      fName = Form("EvSiPM%i", fIdx); 
    }

    inline SLArEventSiPM(const int& idx, const SLArEventPhotonHit& hit) 
      : SLArEventSiPM(idx) 
    {
      RegisterHit(hit); 
    }

    inline SLArEventSiPM(const SLArEventSiPM& right) 
      : SLArEventHitsCollection<SLArEventPhotonHit>(right) 
    {}

    inline double GetTime() const {
      if (fHits.empty()) return -999.;

      return (fHits.begin()->first * fClockUnit);
    }

  private: 

  public: 
    ClassDef(SLArEventSiPM, 1)
};



#endif /* end of include guard SLAREVENTSIPM_HH */

