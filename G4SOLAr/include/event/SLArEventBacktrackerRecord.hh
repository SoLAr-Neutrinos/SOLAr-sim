/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArEventBacktrackerRecord.hh
 * @created     Thursday Sep 28, 2023 11:41:15 CEST
 */

#ifndef SLAREVENTBACKTRACKERRECORD_HH

#define SLAREVENTBACKTRACKERRECORD_HH

#include <vector>
#include <map>
#include "TObject.h"

typedef std::map<Int_t, UShort_t> BacktrackerCounter_t;

class SLArEventBacktrackerRecord : public TObject {
  public: 
    SLArEventBacktrackerRecord(); 
    void Reset();
    inline BacktrackerCounter_t& GetCounter() {return fCounter;}
    inline const BacktrackerCounter_t& GetConstCounter() const {return fCounter;}
    UShort_t  UpdateCounter(const int key, const UShort_t val = 1); 

  protected:
    BacktrackerCounter_t fCounter; 

  public: 
    ClassDef(SLArEventBacktrackerRecord, 1)
};


class SLArEventBacktrackerVector : public TObject {
  public: 
    SLArEventBacktrackerVector();
    SLArEventBacktrackerVector(const UShort_t size);
    ~SLArEventBacktrackerVector(); 

    inline std::vector<SLArEventBacktrackerRecord>& GetRecords() {return fRecords;}
    inline const std::vector<SLArEventBacktrackerRecord>& GetConstRecords() const {return fRecords;}

    void InitRecords(const UShort_t size);
    bool IsEmpty() const;

    void Reset(); 

  protected: 
    std::vector<SLArEventBacktrackerRecord> fRecords;

  public: 
    ClassDef(SLArEventBacktrackerVector, 1)
};



#endif /* end of include guard SLAREVENTBACKTRACKERINFO_HH */

