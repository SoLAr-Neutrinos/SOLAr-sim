/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventGenericHit
 * @created     : venerdì nov 11, 2022 13:58:33 CET
 */

#ifndef SLAREVENTGENERICHIT_HH

#define SLAREVENTGENERICHIT_HH

#include "TObject.h"
#include "TString.h"
#include <iostream>
#include <algorithm>

class SLArEventGenericHit : public TObject {
  public: 
    SLArEventGenericHit(); 
    SLArEventGenericHit(float t, int prodTrkID=-1, int primaryTrkID=-1);
    SLArEventGenericHit(const SLArEventGenericHit& h); 
    virtual ~SLArEventGenericHit() {}

    virtual void DumpInfo() {} 

    inline virtual float GetTime() const {return fTime;}
    inline virtual int   GetProducerTrkID() const {return fProducerTrkID;}
    inline virtual int   GetPrimaryProducerTrkID() const {return fPrimaryProducerTrkID;}
    inline virtual void  SetProducerTrkID(const int id) {fProducerTrkID = id;}
    inline virtual void  SetPrimaryProducerTrkID(const int id) {fPrimaryProducerTrkID = id;}
    inline virtual void  SetTime(float t) {fTime = t;}

    virtual bool operator< (const SLArEventGenericHit& other) const; 
    static  bool CompareHitPtrs(const SLArEventGenericHit* left, const SLArEventGenericHit* right); 

  protected:
    float fTime; 
    int   fProducerTrkID;
    int   fPrimaryProducerTrkID;
  
  public: 
    ClassDef(SLArEventGenericHit, 2)
}; 


#endif /* end of include guard SLAREVENTGENERICHIT_HH */

