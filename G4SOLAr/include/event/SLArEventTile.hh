/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArEventTile.hh
 * @created     Wed Aug 10, 2022 12:13:26 CEST
 */

#ifndef SLAREVENTTILE_HH

#define SLAREVENTTILE_HH

#include <map>
#include "event/SLArEventSiPM.hh"
#include "event/SLArEventChargePixel.hh"

class SLArEventTile :  public TNamed 
{
  public: 
    SLArEventTile(); 
    SLArEventTile(const int idx); 
    SLArEventTile(const SLArEventTile& ev); 
    ~SLArEventTile(); 

    double GetTime() const;
    inline std::map<int, SLArEventChargePixel>& GetPixelEvents() {return fPixelHits;}
    inline const std::map<int, SLArEventChargePixel>& GetConstPixelEvents() const {return fPixelHits;}
    inline double GetNPixelHits() const {return fPixelHits.size();}
    inline std::map<int, SLArEventSiPM>& GetSiPMEvents() {return fSiPMHits;}
    inline const std::map<int, SLArEventSiPM>& GetConstSiPMEvents() const {return fSiPMHits;}
    inline double GetNSiPMHits() const {return fSiPMHits.size();}
    double GetPixelHits() const; 
    int GetSiPMHits() const;
    inline void SetChargeBacktrackerRecordSize(const UShort_t size) {fChargeBacktrackerRecordSize = size;}
    inline UShort_t GetChargeBacktrackerRecordSize() const {return fChargeBacktrackerRecordSize;}
    inline void SetSiPMBacktrackerRecordSize(const UShort_t size) {fSiPMBacktrackerRecordSize = size;}
    inline UShort_t GetSiPMBacktrackerRecordSize() const {return fSiPMBacktrackerRecordSize;}   
    void PrintHits() const; 
    SLArEventChargePixel& RegisterChargeHit(const int&, const SLArEventChargeHit& ); 
    SLArEventSiPM& RegisterSiPMHit(const int&, const SLArEventPhotonHit& );
    SLArEventSiPM& RegisterSiPMHit(const SLArEventPhotonHit& );
    int ResetHits(); 
    inline void SetActive(bool is_active) {fIsActive = is_active;}
    //int SoftResetHits();

    //bool SortHits(); 
    //bool SortPixelHits();

  protected:
    int fIdx;
    bool fIsActive;
    UShort_t fChargeBacktrackerRecordSize;
    UShort_t fSiPMBacktrackerRecordSize;
    std::map<int, SLArEventChargePixel> fPixelHits; 
    std::map<int, SLArEventSiPM> fSiPMHits;

  public:
     ClassDef(SLArEventTile, 3)
};

#endif /* end of include guard SLAREVENTTILE_HH */

