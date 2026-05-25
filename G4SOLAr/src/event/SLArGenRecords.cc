/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArGenRecords.cc
 * @created     : Friday Nov 22, 2024 15:53:04 CET
 */

#include "event/SLArGenRecords.hh"

ClassImp(SLArGenRecord)
ClassImp(SLArGenRecordsVector)

SLArGenRecord::SLArGenRecord(const SLArGenRecord& other) 
  : TObject(other), fGenCode(other.fGenCode), fGenLabel(other.fGenLabel), fStatus(other.fStatus) 
{}

SLArGenRecord& SLArGenRecord::operator=(const SLArGenRecord& other) {
  if (this != &other) { // Avoid self-assignment
    fGenCode = other.fGenCode;
    fGenLabel = other.fGenLabel;
    fStatus = other.fStatus;
  }
  return *this;
}

bool SLArGenRecord::operator==(const SLArGenRecord& other) const {
  return fGenCode == other.fGenCode &&
    fGenLabel == other.fGenLabel &&
    fStatus == other.fStatus;
}

const Float_t SLArGenRecord::GetEnergy() const 
{
    if (fStatus.size() == 0) {
      fprintf(stderr, "SLArGenRecord::GetEnergy() WARNING. Energy not set.\n"); 
      return 0.0;
    }
    return fStatus.at(0);
  }

const std::array<Float_t, 3> SLArGenRecord::GetDirection() 
{
  std::array<Float_t, 3> dir{0, 0, 0}; 
  if (fStatus.size() < 4) {
    fprintf(stderr, "SLArGenRecord::GetDirection() WARNING. Direction not set.\n"); 
  }
  else {
    std::copy( fStatus.begin()+1, fStatus.begin()+4, dir.begin() ); 
  }
  return dir;
}

void SLArGenRecord::Reset() 
{
  fGenCode = 99;
  fGenLabel.Clear(); 
  fStatus.clear(); 
}

// ------------------------------------------------------------------------


SLArGenRecordsVector::SLArGenRecordsVector(const SLArGenRecordsVector& other) :
  TObject(other),
  fEvNumber(other.fEvNumber),
  fStatusVector(other.fStatusVector) {}


