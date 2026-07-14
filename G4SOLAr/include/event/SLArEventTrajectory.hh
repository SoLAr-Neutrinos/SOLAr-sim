/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArEventTrajectory.hh
 * @created     Mon Aug 31, 2020 17:30:50 CEST
 */

#ifndef SLArEVENTTRAJECTORY_HH

#define SLArEVENTTRAJECTORY_HH

#include <iostream>
#include <cassert>
#include "TObject.h"
#include "TString.h"
#include "TVector3.h"

struct trj_point {
  float fX; 
  float fY; 
  float fZ;
  float fKEnergy;
  float fEdep;
  int   fNph; 
  int   fNel; 
  int   fCopy; 
  bool  fLAr;

  trj_point() : fX(0.), fY(0.), fZ(0.), fKEnergy(0.), fEdep(0.), fNph(0), fNel(0), fCopy(0), fLAr(false) {}

  trj_point(double x, double y, double z, double energy, double edep, int n_ph = 0, int n_el = 0, int copy = 0, bool in_lar = false) {
    fX = x; 
    fY = y; 
    fZ = z; 
    fKEnergy = energy;
    fEdep = edep; 
    fNph = n_ph; 
    fNel = n_el; 
    fCopy = copy;
    fLAr = in_lar;
  }

  trj_point(const trj_point& right) {
    fX = right.fX;
    fY = right.fY;
    fZ = right.fZ;
    fKEnergy = right.fKEnergy;
    fEdep = right.fEdep;
    fNph = right.fNph; 
    fNel = right.fNel;
    fCopy = right.fCopy; 
    fLAr = right.fLAr;
  }

  trj_point(trj_point&& right) noexcept {
    fX = right.fX;
    fY = right.fY;
    fZ = right.fZ;
    fKEnergy = right.fKEnergy;
    fEdep = right.fEdep;
    fNph = right.fNph; 
    fNel = right.fNel;
    fCopy = right.fCopy; 
    fLAr = right.fLAr;
  }

};

class SLArEventTrajectoryLite;

class SLArEventTrajectory : public TObject
{
  public:
    SLArEventTrajectory();
    SLArEventTrajectory(const SLArEventTrajectory& trj);
    SLArEventTrajectory(SLArEventTrajectory&& trj) = default;
    ~SLArEventTrajectory();

    TString GetParticleName() const {return fParticleName;}
    TString GetCreatorProcess() const {return fCreatorProcess ;}
    TString GetEndProcess() const {return fEndProcess ;}
    int     GetPDGID() const {return fPDGID ;}
    int     GetTrackID() const {return fTrackID;}
    int     GetParentID() const {return fParentID;}
    float   GetInitKineticEne() const {return fInitKineticEnergy;}
    float   GetTrakLength() const {return fTrackLength;}
    const TVector3& GetInitMomentum() const {return fInitMomentum;}
    float GetTime()      const {return fTime;}
    float GetWeight() const {return fWeight;}
    int GetOriginVolumeCopyNo() const {return fOriginVolCopyNo;}
    float GetTotalEdep() const {return fTotalEdep;} 
    float GetTotalNph () const {return fTotalNph;} 
    float GetTotalNel () const {return fTotalNel;} 
    Bool_t DoStoreTrajectoryPts() const {return fStoreTrajectoryPts;}

    inline void SetStoreTrajectoryPts(const bool store_pts) {fStoreTrajectoryPts = store_pts;}
    inline void SetParticleName(const TString& name) {fParticleName = name;}
    inline void SetCreatorProcess(const TString& proc) {fCreatorProcess = proc;}
    inline void SetEndProcess(const TString& proc) {fEndProcess = proc;}
    inline void SetPDGID(const int& pdgID) {fPDGID = pdgID;}
    inline void SetTrackID(const int& trkID) {fTrackID = trkID;}
    inline void SetParentID(const int& prtID) {fParentID = prtID;}
    inline void SetInitKineticEne(const float& k) {fInitKineticEnergy=k;}
    inline void SetTrackLength(const float& l) {fTrackLength = l;}
    inline void SetInitMomentum(const TVector3& p) {fInitMomentum = p;}
    inline void SetInitMomentum(const double& px, const double& py, const double& pz) {fInitMomentum.SetXYZ(px, py, pz);}
    inline void SetTime(const float& t) {fTime = t;}
    inline void SetWeight(const float& w) {fWeight = w;}
    inline void SetOriginVolCopyNo(const int& copyno) {fOriginVolCopyNo = copyno;}
    inline void IncrementEdep(const double& edep) {fTotalEdep += edep;}
    inline void IncrementNion(const int& nion) {fTotalNel += nion;}
    inline void IncrementNph(const int& nph) {fTotalNph += nph;}

    std::vector<trj_point>& GetPoints()      {return fTrjPoints;}
    const std::vector<trj_point>& GetConstPoints()  const {return fTrjPoints;}
    void    RegisterPoint(double x, double y, double z, double ene, double edep, int n_ph, int n_el, int copy);
    void    RegisterPoint(const trj_point& point); 

    friend class SLArEventTrajectoryLite;

  private:
    Bool_t                 fStoreTrajectoryPts;
    TString                fParticleName     ; 
    TString                fCreatorProcess   ; 
    TString                fEndProcess       ;
    int                    fPDGID            ; 
    int                    fTrackID          ; 
    int                    fParentID         ; 
    int                    fOriginVolCopyNo  ; 
    float                  fInitKineticEnergy;
    float                  fTrackLength      ; 
    float                  fTime             ; 
    float                  fWeight           ;
    TVector3               fInitMomentum     ;
    std::vector<trj_point> fTrjPoints        ;
    float                  fTotalEdep        ; 
    float                  fTotalNph         ; 
    float                  fTotalNel         ; 

  public:
    ClassDef(SLArEventTrajectory, 5);
};

class SLArEventTrajectoryLite : public TObject {
  public: 
    struct Coordinates_t {
      Float_t x; 
      Float_t y; 
      Float_t z;

      Coordinates_t() : x(0.0), y(0.0), z(0.0) {}
      Coordinates_t(Float_t* xyz) : x(xyz[0]), y(xyz[1]), z(xyz[2]) {}
      Coordinates_t(Float_t _x, Float_t _y, Float_t _z) : x(_x), y(_y), z(_z) {}
      Float_t& operator[](Int_t index) {
        switch (index) {
          case 0:
            return x;
          case 1:
            return y;
          case 2:
            return z;
          default:
            throw std::out_of_range("Index out of range");
        }
      }

      const Float_t& operator[](Int_t index) const {
        switch (index) {
          case 0:
            return x;
          case 1:
            return y;
          case 2:
            return z;
          default:
            throw std::out_of_range("Index out of range");
        }
      }
    };

  public: 
    SLArEventTrajectoryLite(); 
    SLArEventTrajectoryLite(const SLArEventTrajectoryLite&); 
    ~SLArEventTrajectoryLite(); 

    Int_t GetEvNumber() const {return fEvNumber;};
    Int_t GetPDGCode() const {return fPDGCode;};
    Int_t GetTrackID() const {return fTrkID;};
    Int_t GetParentID() const {return fParentID;};
    Float_t GetOriginEnergy() const {return fOriginEnergy;};
    Float_t GetEnergyAtLAr() const {return fEnergy;};
    Float_t GetTime() const {return fTime;};
    Float_t GetWeight() const {return fWeight;};
    TString GetCreator() const {return fCreator;};
    Int_t GetOriginVol() const {return fOriginVol;};
    const Coordinates_t& GetOriginVertex() const {return fOriginVertex;}
    const Coordinates_t& GetScorerVertex() const {return fScorerVertex;}

    void SetValues(const SLArEventTrajectory&); 
    inline void SetEvNumber(const Int_t& iev) {fEvNumber = iev;}
    inline void SetPDGCode(const Int_t& pdg) {fPDGCode = pdg;}
    inline void SetTrackID(const Int_t& tid) {fTrkID = tid;}
    inline void SetParentID(const Int_t& pid) {fParentID = pid;}
    inline void SetOriginEnergy(const Double_t& ene) {fOriginEnergy = ene;}
    inline void SetEnergyAtScorer(const Double_t& ene) {fEnergy = ene;}
    inline void SetTime(const Float_t& time) {fTime = time;}
    inline void SetWeight(const Float_t& w) {fWeight = w;}
    inline void SetCreator(const TString& creator) {fCreator = creator;}
    inline void SetOriginVol(const Int_t& ovol) {fOriginVol = ovol;}
    inline void SetOriginVertex(const Float_t* vtx) {
      for (int i=0; i<3; i++) {
        fOriginVertex[i] = vtx[i]; 
      }
    }
    inline void SetOriginVertex(const Float_t& x, const Float_t& y, const Float_t& z) {
      fOriginVertex.x = x; 
      fOriginVertex.y = y; 
      fOriginVertex.z = z; 
    }
    inline void SetScorerVertex(const Float_t* vtx) {
      for (int i=0; i<3; i++) {
        fScorerVertex[i] = vtx[i]; 
      }
    }
    inline void SetScorerVertex(const Float_t& x, const Float_t& y, const Float_t& z) {
      fScorerVertex.x = x; 
      fScorerVertex.y = y; 
      fScorerVertex.z = z; 
    }


    void Reset(); 

    friend class SLArEventTrajectory;
    friend class SLArAnalysisManager;

  protected: 
    Int_t fEvNumber;
    Int_t fPDGCode;
    Int_t fTrkID;
    Int_t fParentID;
    Int_t fOriginVol;
    Float_t fOriginEnergy;
    Float_t fEnergy;
    Float_t fTime;
    Float_t fWeight;
    Coordinates_t fOriginVertex;
    Coordinates_t fScorerVertex;
    TString fCreator;

  public: 
    ClassDef(SLArEventTrajectoryLite, 1)

};

#endif /* end of include guard SLArEVENTTRAJECTORY_HH */

