/**
 * @author      : guff (guff@guff-gssi)
 * @file        : SLArEventTrajectory
 * @created     : lunedì ago 31, 2020 17:30:50 CEST
 */

#ifndef SLArEVENTTRAJECTORY_HH

#define SLArEVENTTRAJECTORY_HH

#include <iostream>
#include "TObject.h"
#include "TString.h"
#include "TVector3.h"

struct trj_point {
  float fX; 
  float fY; 
  float fZ;
  float fEdep; 

  trj_point() : fX(0.), fY(0.), fZ(0.), fEdep(0.) {}
  trj_point(double x, double y, double z, double edep) {
    fX = x; 
    fY = y; 
    fZ = z; 
    fEdep = edep;
  }
};



class SLArEventTrajectory : public TObject
{
  public:
    SLArEventTrajectory();
    SLArEventTrajectory(SLArEventTrajectory* trj);
    ~SLArEventTrajectory();

    TString GetParticleName()   {return fParticleName   ;}
    TString GetCreatorProcess() {return fCreatorProcess ;}
    int     GetPDGID()          {return fPDGID          ;}
    int     GetTrackID()        {return fTrackID        ;}
    int     GetParentID()       {return fParentID       ;}
    float   GetInitKineticEne() {return fInitKineticEnergy;}
    float   GetTrakLength()     {return fTrackLength    ;}
    TVector3& GetInitMomentum() {return fInitMomentum   ;}
    float   GetTime()           {return fTime           ;}

    void    SetParticleName(TString name)   {fParticleName = name;}
    void    SetCreatorProcess(TString proc) {fCreatorProcess = proc;}
    void    SetPDGID       (int pdgID)      {fPDGID    = pdgID   ;}
    void    SetTrackID     (int trkID)      {fTrackID  = trkID   ;}
    void    SetParentID    (int prtID)      {fParentID = prtID   ;}
    void    SetInitKineticEne(float k)      {fInitKineticEnergy=k;}
    void    SetTrackLength (float l)        {fTrackLength = l    ;}
    void    SetInitMomentum(TVector3 p)     {fInitMomentum = p   ;}
    void    SetTime(float t)                {fTime = t           ;}

    std::vector<trj_point>& GetPoints()      {return fTrjPoints   ;}
    void    RegisterPoint(double x, double y, double z, double edep);


  private:
    TString               fParticleName      ;
    TString               fCreatorProcess    ; 
    int                   fPDGID             ; 
    int                   fTrackID           ; 
    int                   fParentID          ; 
    float                 fInitKineticEnergy ;
    float                 fTrackLength       ; 
    float                 fTime              ; 
    TVector3              fInitMomentum      ;
    std::vector<trj_point> fTrjPoints        ;

  public:
    ClassDef(SLArEventTrajectory, 1);
};


#endif /* end of include guard SLArEVENTTRAJECTORY_HH */

