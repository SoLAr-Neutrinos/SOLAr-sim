/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventMegatile.hh
 * @created     : mercoledì ago 10, 2022 13:42:54 CEST
 */

#ifndef SLAREVENTMEGATILE_HH

#define SLAREVENTMEGATILE_HH
#include <map>
#include <memory>

#include "event/SLArEventTile.hh"
#include "config/SLArCfgMegaTile.hh"
#include "config/SLArCfgReadoutTile.hh"

class SLArEventMegatile : public TNamed {
  public: 
    SLArEventMegatile(); 
    SLArEventMegatile(const SLArCfgMegaTile* cfg); 
    SLArEventMegatile(const SLArEventMegatile& right);
    ~SLArEventMegatile(); 

    SLArEventTile& GetOrCreateEventTile(const int& tileIdx); 
    int ConfigModule(const SLArCfgMegaTile* cfg);

    inline const std::map<int, SLArEventTile>& GetConstTileMap() const {return fTilesMap;}
    inline std::map<int, SLArEventTile>& GetTileMap() {return fTilesMap;}
    int GetNPhotonHits() const;
    int GetNChargeHits() const; 
    inline int GetIdx() const {return fIdx;}

    SLArEventTile& RegisterHit(const SLArEventPhotonHit& hit, const int idx = -999); 
    int ResetHits(); 
    int SoftResetHits();

    void SetActive(bool is_active); 
    void SetIdx(int idx) {fIdx = idx;}
    inline void SetChargeBacktrackerRecordSize(const UShort_t size) {fChargeBacktrackerRecordSize = size;}
    inline UShort_t GetChargeBacktrackerRecordSize() const {return fChargeBacktrackerRecordSize;}
    inline void SetLightBacktrackerRecordSize(const UShort_t size) {fLightBacktrackerRecordSize = size;}
    inline UShort_t GetLightBacktrackerRecordSize() const {return fLightBacktrackerRecordSize;}
    //bool SortHits(); 

  private:
    int fIdx; 
    bool fIsActive; 
    int fNhits; 
    UShort_t fLightBacktrackerRecordSize;
    UShort_t fChargeBacktrackerRecordSize;
    std::map<int, SLArEventTile> fTilesMap; 

  public:
    ClassDef(SLArEventMegatile, 2)
};

#endif /* end of include guard SLAREVENTMEGATILE_HH */

