/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventSuperCellArray.hh
 * @created     : Thur Oct 20, 2022 16:08:33 CEST
 */

#ifndef SLAREVENTSUPERCELLARRAY_HH

#define SLAREVENTSUPERCELLARRAY_HH

#include "event/SLArEventSuperCell.hh"
#include "config/SLArCfgSuperCellArray.hh"

class SLArEventSuperCellArray : public TNamed {
  public: 
    SLArEventSuperCellArray(); 
    SLArEventSuperCellArray(const SLArCfgSuperCellArray& cfg); 
    SLArEventSuperCellArray(const SLArEventSuperCellArray&); 
    ~SLArEventSuperCellArray();

    //int ConfigSystem(const SLArCfgSuperCellArray& cfg); 
    inline std::map<int, SLArEventSuperCell>& GetSuperCellMap() {return fSuperCellMap;}
    inline const std::map<int, SLArEventSuperCell>& GetConstSuperCellMap() const {return fSuperCellMap;}
    inline int GetNhits() const {return fNhits;}
    inline bool IsActive() const {return fIsActive;}

    inline void SetLightBacktrackerRecordSize(const UShort_t size) {fLightBacktrackerRecordSize = size;}
    inline UShort_t GetLightBacktrackerRecordSize() const {return fLightBacktrackerRecordSize;}
    SLArEventSuperCell& GetOrCreateEventSuperCell(const int scIdx); 
    SLArEventSuperCell& RegisterHit(const SLArEventPhotonHit& hit, int sc_idx = -999); 
    int ResetHits(); 
    int SoftResetHits();

    void SetActive(bool is_active); 

  private: 
    int fNhits; 
    bool fIsActive; 
    UShort_t fLightBacktrackerRecordSize;
    std::map<int, SLArEventSuperCell> fSuperCellMap;

  public:
    ClassDef(SLArEventSuperCellArray, 2); 
}; 

class SLArListEventPDS: public TObject {
  public: 
    inline SLArListEventPDS() : TObject(), fEvNumber(-1)  {}

    inline SLArListEventPDS(const SLArListEventPDS& ev) 
      : TObject(ev), fEvNumber(ev.fEvNumber)
    {
      for (const auto& p : ev.fOpDetArrayMap) {
        fOpDetArrayMap[p.first] = SLArEventSuperCellArray(p.second);
      }
    }

    inline ~SLArListEventPDS() { fOpDetArrayMap.clear(); }

    inline void SetEventNumber(int ev) {fEvNumber = ev;}
    
    inline int GetEventNumber() const {return fEvNumber;}
    
    inline std::map<int, SLArEventSuperCellArray>& GetOpDetArrayMap() {return fOpDetArrayMap;}
    
    inline const std::map<int, SLArEventSuperCellArray>& GetConstOpDetArrayMap() const {return fOpDetArrayMap;}
    
    inline SLArEventSuperCellArray& GetOpDetArrayByID(int id) {
      auto it = fOpDetArrayMap.find(id);
      if (it != fOpDetArrayMap.end()) {
        return it->second;
      }
      throw std::out_of_range("PDS Wall ID not found");
    }

    inline void Reset() {
      for (auto& p : fOpDetArrayMap) {
        p.second.ResetHits();
      }
      fEvNumber = -1;
    }
  private: 
    Int_t fEvNumber = {};
    std::map<int, SLArEventSuperCellArray> fOpDetArrayMap;

  public: 
    ClassDef(SLArListEventPDS, 2);
};

#endif /* end of include guard SLAREVENTSUPERCELLARRAY */

