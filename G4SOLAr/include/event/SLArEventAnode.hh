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

    SLArEventSiPM& RegisterHit(const SLArEventPhotonHit& hit, int mt_idx = -999, int t_idx = -999); 
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
    ClassDef(SLArEventAnode, 3)
};

class SLArListEventAnode: public TObject {
  public:
    inline SLArListEventAnode() : TObject() {}

    inline SLArListEventAnode(const SLArListEventAnode& ev) : 
      fEvNumber(ev.fEvNumber), TObject(ev) {
        for (const auto& anode : ev.fAnodeMap) {
          fAnodeMap[anode.first] = SLArEventAnode(anode.second);
        }
      }

    inline ~SLArListEventAnode() { fAnodeMap.clear(); }
    
    void SetEventNumber(const int& evNumber) {fEvNumber = evNumber;}
    
    inline int GetEventNumber() const {return fEvNumber;}
    
    inline std::map<int, SLArEventAnode>& GetAnodeMap() {return fAnodeMap;}
    
    inline const std::map<int, SLArEventAnode>& GetConstAnodeMap() const {return fAnodeMap;}

    inline SLArEventAnode& GetEventAnodeByTPCID(const int& tpc_id) {
      auto it = fAnodeMap.find(tpc_id);
      if (it != fAnodeMap.end()) {
        return it->second;
      }

      throw std::out_of_range(Form("Anode for TPC %i not found", tpc_id));
    }

    inline SLArEventAnode& GetEventAnodeByID(const int& id) {
      for (auto &anode : fAnodeMap) {
        if (anode.second.GetID() == id) {return anode.second;}
      }

      throw std::out_of_range(Form("Anode with ID %i not found", id));
    }

    inline void Reset() {
      for (auto& anode : fAnodeMap) {
        anode.second.ResetHits();
      }
      fEvNumber = -1;
    }

  private:
    Int_t fEvNumber = -1;
    std::map<int, SLArEventAnode> fAnodeMap;

  public: 
    ClassDef(SLArListEventAnode, 1)
};

#endif /* end of include guard SLArEventAnode_HH */

