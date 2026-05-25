/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArEventBacktrackerRecord.cc
 * @created     Thursday Sep 28, 2023 17:37:02 CEST
 */

#include "event/SLArEventBacktrackerRecord.hh"
#include <cstdio>

ClassImp(SLArEventBacktrackerRecord) 

SLArEventBacktrackerRecord::SLArEventBacktrackerRecord() : TObject() {}

void SLArEventBacktrackerRecord::Reset() {
  fCounter.clear(); 
}

UShort_t SLArEventBacktrackerRecord::UpdateCounter(const int key, const UShort_t val)
{
  fCounter[key] += val;
  return fCounter[key];
}


ClassImp(SLArEventBacktrackerVector)

SLArEventBacktrackerVector::SLArEventBacktrackerVector()
  : TObject()
{}

SLArEventBacktrackerVector::SLArEventBacktrackerVector(const UShort_t size)
  : TObject()
{
  InitRecords(size);
}

void SLArEventBacktrackerVector::InitRecords(const UShort_t size) {
  fRecords.resize(size);
}

bool SLArEventBacktrackerVector::IsEmpty() const {
  return fRecords.empty();
}

void SLArEventBacktrackerVector::Reset() {
  for (auto &record : fRecords) {
    record.Reset();
  }
  return;
}

SLArEventBacktrackerVector::~SLArEventBacktrackerVector()
{
  fRecords.clear();
}

