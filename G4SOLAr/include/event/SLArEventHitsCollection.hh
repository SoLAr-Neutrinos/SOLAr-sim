/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArEventHitsCollection.hh
 * @created     Fri Nov 11, 2022 14:21:29 CET
 */

#ifndef SLAREVENTHITSCOLLECTION_HH

#define SLAREVENTHITSCOLLECTION_HH

#include <RtypesCore.h>
#include <iostream>
#include <map>

#include "TNamed.h"
#include "event/SLArEventGenericHit.hh"
#include "event/SLArEventBacktrackerRecord.hh"

typedef std::map<Int_t, UShort_t> HitsCollection_t; 
typedef std::map<Int_t, SLArEventBacktrackerVector> BacktrackerVectorCollection_t;

template<class T>
class SLArEventHitsCollection : public TNamed {
  public: 
    SLArEventHitsCollection(); 
    SLArEventHitsCollection(const int); 
    SLArEventHitsCollection(const int, const UShort_t);
    SLArEventHitsCollection(const SLArEventHitsCollection&); 
    virtual ~SLArEventHitsCollection(); 

    void Copy(SLArEventHitsCollection& record) const;

    template<typename TT>
    Int_t ConvertToClock(const TT& val) {return static_cast<Int_t>(val / fClockUnit);}
    inline UShort_t GetClockUnit() const {return fClockUnit;}
    inline int GetIdx() const {return fIdx;}
    inline int GetNhits() const {return fNhits;}
    inline virtual double GetTime() const {return -1.;} 
    inline HitsCollection_t& GetHits() {return fHits;}
    inline const HitsCollection_t& GetConstHits() const {return fHits;}
    inline BacktrackerVectorCollection_t& GetBacktrackerRecordCollection() {return fBacktrackerCollections;}
    inline const BacktrackerVectorCollection_t& GetBacktrackerRecordCollection() const {return fBacktrackerCollections;}

    inline UShort_t GetBacktrackerRecordSize() const {return fBacktrackerRecordSize;}
    SLArEventBacktrackerVector& GetBacktrackerVector(UShort_t key); 
    inline bool IsActive() const {return fIsActive;} 

    virtual void PrintHits() const; 

    virtual int RegisterHit(const T hit); 
    virtual int ResetHits(); 

    //virtual bool SortHits(); 
    inline void SetActive(bool is_active) {fIsActive = is_active;}
    inline void SetIdx(int idx) {fIdx = idx;}
    inline void SetClockUnit(const UShort_t unit) {fClockUnit = unit;}
    inline void SetBacktrackerRecordSize(const UShort_t size) {fBacktrackerRecordSize = size;}
    inline void SetNhits(const int& n) {fNhits = n;}

    int ZeroSuppression(const UShort_t threshold);  

  protected:
    int fIdx; 
    bool fIsActive; 
    UShort_t fNhits; 
    UShort_t fBacktrackerRecordSize;
    HitsCollection_t fHits; 
    BacktrackerVectorCollection_t fBacktrackerCollections;
    UShort_t fClockUnit; 

  public: 
    ClassDefOverride(SLArEventHitsCollection, 2);
}; 



#endif /* end of include guard SLAREVENTHITSCOLLECTION_HH */

