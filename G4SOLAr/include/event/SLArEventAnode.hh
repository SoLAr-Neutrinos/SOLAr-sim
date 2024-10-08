/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventAnode.hh
 * @created     : Wed Aug 10, 2022 14:29:23 CEST
 */

#ifndef SLAREVENTANODE_HH

#define SLAREVENTANODE_HH

#include "config/SLArCfgAnode.hh"
#include "config/SLArCfgAssembly.hh"
#include "config/SLArCfgMegaTile.hh"
#include "event/SLArEventMegatile.hh"

class SLArEventAnode : public TNamed {
  public:
    SLArEventAnode(); 
    SLArEventAnode(const SLArCfgAnode& cfg);
    SLArEventAnode(const SLArEventAnode&);
    ~SLArEventAnode(); 

    int ConfigSystem(const SLArCfgAnode& cfg);
    SLArEventMegatile& GetOrCreateEventMegatile(const int mtIdx); 
    inline std::map<int, SLArEventMegatile>& GetMegaTilesMap() {return fMegaTilesMap;}
    inline const std::map<int, SLArEventMegatile>& GetConstMegaTilesMap() const {return fMegaTilesMap;}
    inline int GetNhits() const {return fNhits;}
    inline bool IsActive() const {return fIsActive;}

    SLArEventTile& RegisterHit(const SLArEventPhotonHit& hit, int mt_idx = -999, int t_idx = -999); 
    SLArEventChargePixel& RegisterChargeHit(const SLArCfgAnode::SLArPixIdx& pixId, const SLArEventChargeHit& hit); 
    int ResetHits(); 
    int SoftResetHits();

    void SetActive(bool is_active); 
    inline void SetChargeBacktrackerRecordSize(const UShort_t size) {fChargeBacktrackerRecordSize = size;}
    inline UShort_t GetChargeBacktrackerRecordSize() const {return fChargeBacktrackerRecordSize;}
    inline void SetLightBacktrackerRecordSize(const UShort_t size) {fLightBacktrackerRecordSize = size;}
    inline UShort_t GetLightBacktrackerRecordSize() const {return fLightBacktrackerRecordSize;}
    inline void SetZeroSuppressionThreshold(const UShort_t& threshold) {fZeroSuppressionThreshold = threshold;}
    inline UShort_t GetZeroSuppressionThreshold() const {return fZeroSuppressionThreshold;}

    Int_t ApplyZeroSuppression(); 

    //bool SortHits(); 

    inline void SetID(const int anode_id) {fID = anode_id;}
    inline int  GetID() const {return fID;}

  private:
    int fID; 
    int fNhits; 
    bool fIsActive;
    UShort_t fLightBacktrackerRecordSize;
    UShort_t fChargeBacktrackerRecordSize;
    UShort_t fZeroSuppressionThreshold;
    std::map<int, SLArEventMegatile> fMegaTilesMap;

  public:
    ClassDef(SLArEventAnode, 2)
};

#endif /* end of include guard SLArEventAnode_HH */

