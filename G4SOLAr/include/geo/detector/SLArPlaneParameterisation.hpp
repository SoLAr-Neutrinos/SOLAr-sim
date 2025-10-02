/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArPlaneParameterisation
 * @created     : mercoledì mar 22, 2023 11:20:47 CET
 */

#ifndef SLARPLANEPARAMETERISATION_HPP

#define SLARPLANEPARAMETERISATION_HPP

#include <SLArUnit.hpp>

#include <G4VPVParameterisation.hh>
#include <G4PVParameterised.hh>
#include <G4VPhysicalVolume.hh>

class SLArPlaneParameterisation : public G4VPVParameterisation {
  public: 
    struct PlaneReplicationData_t {
      EAxis fReplicaAxis; 
      G4int fNreplica; 
      G4double fWidth;
      G4double fOffset; 
      G4bool fConsuming; 
      G4ThreeVector fReplicaAxisVec; 
      G4ThreeVector fStartingPos; 

      PlaneReplicationData_t(); 
    }; 

    SLArPlaneParameterisation(EAxis, G4ThreeVector, G4double);
    SLArPlaneParameterisation(const rapidjson::Value&); 
    G4ThreeVector ComputeTranslation(const int& copyNo) const;
    void ComputeTransformation(G4int copyNo, G4VPhysicalVolume* physVol) const; 

    EAxis GetReplicationAxis() {return fReplicaAxis;}
    void SetReplicationAxis(EAxis ax); 
    G4ThreeVector GetReplicationAxisVector() {return fAxisVector;}
    G4double GetSpacing() {return fSpacing;}
    void SetSpacing(G4double spacing) {fSpacing = spacing;}
    G4ThreeVector GetStartPos() {return fStartPos;}
    void SetStartPos(G4ThreeVector pos) {fStartPos = pos;}

  private: 
    EAxis fReplicaAxis; 
    G4ThreeVector fAxisVector; 
    G4ThreeVector fStartPos; 
    G4double  fSpacing; 
};

inline SLArPlaneParameterisation::SLArPlaneParameterisation(
    EAxis replica_axis, G4ThreeVector start_pos, G4double spacing) 
  : fReplicaAxis(replica_axis), fStartPos(start_pos), fSpacing(spacing)
{
  SetReplicationAxis(replica_axis); 
  return; 
}

inline SLArPlaneParameterisation::SLArPlaneParameterisation(
    const rapidjson::Value& jconf) 
{
  assert(jconf.IsObject()); 
  auto jreplica = jconf.GetObject(); 
  
  assert(jreplica.HasMember("replica_axis")); 
  assert(jreplica.HasMember("spacing")); 

  G4String axis_str = jreplica["replica_axis"].GetString(); 
  if (axis_str == "x") SetReplicationAxis(kXAxis);
  else if (axis_str == "y") SetReplicationAxis(kYAxis); 
  else if (axis_str == "z") SetReplicationAxis(kZAxis); 

  fSpacing = unit::ParseJsonVal(jreplica["spacing"]); 
  return; 
}

inline void SLArPlaneParameterisation::SetReplicationAxis(EAxis ax) {
  fReplicaAxis = ax; 

  if      (fReplicaAxis == kXAxis) {fAxisVector = G4ThreeVector(1, 0, 0);} 
  else if (fReplicaAxis == kYAxis) {fAxisVector = G4ThreeVector(0, 1, 0);} 
  else                             {fAxisVector = G4ThreeVector(0, 0, 1);} 
}

inline G4ThreeVector SLArPlaneParameterisation::ComputeTranslation(const int& copyNo) const
{
  G4ThreeVector origin = fStartPos; 
  origin += fAxisVector*(copyNo)*fSpacing;
  return origin;
}

inline void SLArPlaneParameterisation::ComputeTransformation(
    G4int copyNo, G4VPhysicalVolume* physVol) const {
  G4ThreeVector translation = ComputeTranslation(copyNo);

  physVol->SetTranslation(translation); 
  physVol->SetRotation(0); 
  return; 
}

inline SLArPlaneParameterisation::PlaneReplicationData_t::PlaneReplicationData_t() : 
  fReplicaAxis(kZAxis), fNreplica(0), fWidth(0.), fOffset(0.), fConsuming(false), 
  fReplicaAxisVec(0, 0, 1), fStartingPos(0, 0, 0) 
{}

inline static SLArPlaneParameterisation::PlaneReplicationData_t  
get_plane_replication_data(const G4PVParameterised* pv) {
  SLArPlaneParameterisation::PlaneReplicationData_t data; 
  pv->GetReplicationData(data.fReplicaAxis, data.fNreplica, 
      data.fWidth, data.fOffset, data.fConsuming); 
  auto parameterisation = (SLArPlaneParameterisation*)pv->GetParameterisation(); 
  data.fReplicaAxisVec = parameterisation->GetReplicationAxisVector(); 
  data.fStartingPos = parameterisation->GetStartPos(); 
  data.fWidth = parameterisation->GetSpacing(); 
  return data;
};



#endif /* end of include guard SLARPLANEPARAMETERISATION_HPP */

