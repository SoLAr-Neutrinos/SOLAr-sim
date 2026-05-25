/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArEventTile.hh
 * @created     Wed Aug 10, 2022 12:13:26 CEST
 */

#ifndef SLAREVENTTILE_HH

#define SLAREVENTTILE_HH

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "event/SLArEventHitsCollection.hh"
#include "event/SLArEventChargePixel.hh"
#include "event/SLArEventPhotonHit.hh"

class SLArEventTile :  public SLArEventHitsCollection<SLArEventPhotonHit> 
{
  public: 
    SLArEventTile(); 
    SLArEventTile(const int idx); 
    SLArEventTile(const SLArEventTile& ev); 
    ~SLArEventTile(); 

    double GetTime() const;
    double GetTime(EPhProcess proc) const;
    inline std::map<int, SLArEventChargePixel>& GetPixelEvents() {return fPixelHits;}
    inline const std::map<int, SLArEventChargePixel>& GetConstPixelEvents() const {return fPixelHits;}
    inline double GetNPixelHits() const {return fPixelHits.size();}
    double GetPixelHits() const; 
    inline void SetChargeBacktrackerRecordSize(const UShort_t size) {fChargeBacktrackerRecordSize = size;}
    inline UShort_t GetChargeBacktrackerRecordSize() const {return fChargeBacktrackerRecordSize;}
    void PrintHits() const; 
    SLArEventChargePixel& RegisterChargeHit(const int&, const SLArEventChargeHit& ); 
    int ResetHits(); 
    int SoftResetHits();

    //bool SortHits(); 
    //bool SortPixelHits();

  protected:
    UShort_t fChargeBacktrackerRecordSize;
    std::map<int, SLArEventChargePixel> fPixelHits; 

  public:
     ClassDef(SLArEventTile, 2)
};

#endif /* end of include guard SLAREVENTTILE_HH */

