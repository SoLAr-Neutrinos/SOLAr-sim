/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArMCPrimaryInfo.cc
 * @created     Fri Feb 14, 2020 16:56:53 CET
 */


#include <iostream>
#include <string.h>
#include <memory>
#include "event/SLArMCPrimaryInfo.hh"

ClassImp(SLArMCPrimaryInfo)

SLArMCPrimaryInfo::SLArMCPrimaryInfo() : 
  TNamed(),
  fPDG(0), fTrkID(0), fGeneratorLabel(), fEnergy(0.),
  fTotalEdep(0.), fTotalLArEdep(0), fTotalScintPhotons(0), fTotalCerenkovPhotons(0),
  fVertex(3, 0.), fMomentum(3, 0.), fTrajectories{}
{
  fTrajectories.reserve(100);
}

SLArMCPrimaryInfo::SLArMCPrimaryInfo(const SLArMCPrimaryInfo& p) 
  : TNamed(p), 
    fPDG(p.fPDG), fTrkID(p.fTrkID), fGeneratorLabel(p.fGeneratorLabel), fEnergy(p.fEnergy), 
    fTime(p.fTime), fTotalEdep(p.fTotalEdep), fTotalLArEdep(p.fTotalLArEdep), 
    fTotalScintPhotons(p.fTotalScintPhotons), fTotalCerenkovPhotons(p.fTotalCerenkovPhotons),
    fVertex(p.fVertex), fMomentum(p.fMomentum) 
{
  for (const auto& t : p.fTrajectories) {
    fTrajectories.emplace_back( new SLArEventTrajectory(*t) ); 
  }
}

SLArMCPrimaryInfo& SLArMCPrimaryInfo::operator=(const SLArMCPrimaryInfo& p) 
{
  if (this != &p) {
    TNamed::operator=(p);
    fPDG = p.fPDG;
    fTrkID = p.fTrkID;
    fGeneratorLabel = p.fGeneratorLabel;
    fEnergy = p.fEnergy;
    fTime = p.fTime;
    fTotalEdep = p.fTotalEdep;
    fTotalLArEdep = p.fTotalLArEdep;
    fTotalScintPhotons = p.fTotalScintPhotons;
    fTotalCerenkovPhotons = p.fTotalCerenkovPhotons;
    fVertex = p.fVertex;
    fMomentum = p.fMomentum;

    ClearTrajectories();
    for (const auto& t : p.fTrajectories) {
      fTrajectories.emplace_back( new SLArEventTrajectory(*t) ); 
    }
  }
  return *this;
}

SLArMCPrimaryInfo::~SLArMCPrimaryInfo() {
  ClearTrajectories();
}

void SLArMCPrimaryInfo::SetPosition(const double& x, const double& y,
                                    const double& z, const double& t)
{
  fVertex[0] = x;
  fVertex[1] = y;
  fVertex[2] = z;
  fTime   = t;
  fTotalCerenkovPhotons = 0; 
  fTotalScintPhotons = 0; 
}

void SLArMCPrimaryInfo::SetMomentum(const double& px, const double& py, const double& pz, const double& ene)
{
  fMomentum[0] = px;
  fMomentum[1] = py;
  fMomentum[2] = pz;
  fEnergy   = ene;
}

void SLArMCPrimaryInfo::ResetParticle()
{
  fPDG           = 0;
  fTrkID        = 0; 
  fGeneratorLabel = "";
  fName         = "";
  fTitle        = "";
  fEnergy       = 0.;
  fTime         = 0.;
  fTotalEdep    = 0.;
  fTotalLArEdep = 0.;
  fTotalScintPhotons = 0; 
  fTotalCerenkovPhotons = 0; 

  ClearTrajectories();

  std::fill(fVertex.begin(), fVertex.end(), 0.); 
  std::fill(fMomentum.begin(), fMomentum.end(), 0.); 
}


void SLArMCPrimaryInfo::PrintParticle() const
{
  std::cout << "SLAr Primary Info: " << std::endl;
  std::cout << "Generator:" << fGeneratorLabel << std::endl;
  std::cout << "Particle:" << fName << ", pdg: " << fPDG <<", trk id: " << fTrkID << std::endl;
  std::cout << "Energy  :" << fEnergy <<std::endl;
  std::cout << "Time    :" << fTime << " (ns)" << std::endl;
  std::cout << "Vertex:" << fVertex[0] << ", " 
                         << fVertex[1]<< ", " 
                         << fVertex[2] << " (mm)" << std::endl;
  std::cout << "Momentum:" << fMomentum[0] << ", " 
                           << fMomentum[1]<< ", " 
                           << fMomentum[2]<< std::endl;
}

int SLArMCPrimaryInfo::RegisterTrajectory(SLArEventTrajectory&& trj)
{
  fTotalEdep += trj.GetTotalEdep(); 
  fTrajectories.emplace_back( new SLArEventTrajectory(std::move(trj)) );
  return (int)fTrajectories.size();
}

int SLArMCPrimaryInfo::RegisterTrajectory(const SLArEventTrajectory& trj)
{
  fTotalEdep += trj.GetTotalEdep(); 
  fTrajectories.emplace_back( new SLArEventTrajectory(trj) );
  return (int)fTrajectories.size();
}


