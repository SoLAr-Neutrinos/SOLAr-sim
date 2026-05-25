/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArGenRecord.hpp
 * @created     : Monday Sep 23, 2024 14:17:10 CEST
 */

#ifndef SLARGENRECORDS_HH

#define SLARGENRECORDS_HH

#include <cstdio>
#include <vector>
#include <array>
#include <TObject.h>
#include <TString.h>

class SLArGenRecord : public TObject {
  public : 
    inline SLArGenRecord() : fGenCode(99), fGenLabel(), fStatus{} {} 
    inline SLArGenRecord(const UShort_t code, const TString label) : 
      fGenCode(code), fGenLabel(label), fStatus{} {}
    SLArGenRecord(const SLArGenRecord& other);
    inline ~SLArGenRecord() {Reset();}

    SLArGenRecord& operator=(const SLArGenRecord& other);
    bool operator==(const SLArGenRecord& other) const;

    inline UShort_t& GetGenCode() {return fGenCode;}
    inline TString& GetGenLabel() {return fGenLabel;}
    inline std::vector<Float_t>& GetGenStatus() {return fStatus;}
    inline const UShort_t& GetGenCode() const {return fGenCode;}
    inline const TString& GetGenLabel() const {return fGenLabel;}
    inline const std::vector<Float_t>& GetGenStatus() const {return fStatus;}

    const Float_t GetEnergy() const;
    const std::array<Float_t, 3> GetDirection();
    void Reset();

  private : 
    UShort_t fGenCode; 
    TString  fGenLabel; 
    std::vector<Float_t> fStatus; 

  public: 
    ClassDef(SLArGenRecord, 1)
};

class SLArGenRecordsVector : public TObject {
  public: 
    inline SLArGenRecordsVector() : fEvNumber(-1), fStatusVector{} {}
    inline SLArGenRecordsVector(const Int_t iev) : fEvNumber(iev), fStatusVector{} {};
    SLArGenRecordsVector(const SLArGenRecordsVector& other);
    inline ~SLArGenRecordsVector() { fStatusVector.clear(); }; 

    inline SLArGenRecord& AddRecord(const UShort_t code, const TString label) {
      fStatusVector.push_back( SLArGenRecord(code, label) ); 
      return fStatusVector.back();
    }
    inline void SetEventNumber(const Int_t iev) {fEvNumber = iev;}
    inline Int_t& GetEventNumber() {return fEvNumber;}
    inline const Int_t& GetEventNumber() const {return fEvNumber;}
    inline std::vector<SLArGenRecord>& GetRecordsVector() {return fStatusVector;}
    inline const std::vector<SLArGenRecord>& GetRecordsVector() const {return fStatusVector;}

    inline void Reset() {
      fStatusVector.clear();
    }

  private: 
    Int_t fEvNumber; 
    std::vector<SLArGenRecord> fStatusVector; 

  public: 
    ClassDef(SLArGenRecordsVector, 1)
};


#endif /* end of include guard SLARGENSTATUS_HH */

