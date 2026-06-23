/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArCfgBaseModule.hh
 * @created     : martedì lug 19, 2022 10:40:14 CEST
 */

#ifndef SLARCFGBASEMODULE_HH

#define SLARCFGBASEMODULE_HH

#include <iostream>
#include <array>

#include "TNamed.h"
#include "TVector3.h"
#include "TRotation.h"
#include "TGraph.h"

class SLArCfgBaseModule : public TNamed {
  typedef std::array<float, 3> xyzpoint;

  public: 
    SLArCfgBaseModule(); 
    SLArCfgBaseModule(int id); 
    SLArCfgBaseModule(int id, float xc, float yc, float zc, float phi, float theta, float psi); 
    SLArCfgBaseModule(const SLArCfgBaseModule& base); 
    virtual ~SLArCfgBaseModule(); 

    inline virtual void DumpInfo() const = 0; 
    inline int GetID() const {return fID;}
    inline void SetID(int id) {fID = id;}
    inline int GetIdx() const {return fIdx;}
    inline void SetIdx(int idx) {fIdx = idx;}
    inline float GetX() const {return fX;}
    inline void SetX(float x) {fX = x;}
    inline float GetY() const {return fY;}
    inline void SetY(float y) {fY = y;}
    inline float GetZ() const {return fZ;}
    inline void SetZ(float z) {fZ =  z;}
    inline float GetPhi() const {return fPhi;}
    inline void SetPhi(float phi) {fPhi = phi;}
    inline float GetTheta() const {return fTheta;}
    inline void SetTheta(float theta) {fTheta = theta;}
    inline float GetPsi() const {return fPsi;}
    inline void SetPsi(float psi) {fPsi = psi;}
    inline float GetPhysX() const {return fPhysX;}
    inline void SetPhysX(float x) {fPhysX = x;}
    inline float GetPhysY() const {return fPhysY;}
    inline void SetPhysY(float y) {fPhysY = y;}
    inline float GetPhysZ() const {return fPhysZ;}
    inline void SetPhysZ(float z) {fPhysZ =  z;}
    inline int GetBinIdx() const {return  fBin;}
    inline void SetBinIdx(int bin) {fBin =  bin;}
    //inline TGraph* GetGraphShape() {return fGShape;}
    inline void SetNormal(double x, double y, double z) {fNormal.SetXYZ(x, y, z);};
    inline void SetNormal(TVector3 vN) {fNormal = vN;}
    inline double GetSizeX() const {return fSize.x();}
    inline double GetSizeY() const {return fSize.y();}
    inline double GetSizeZ() const {return fSize.z();}
    inline TVector3 GetSize() const {return fSize;}
    inline void   SetSizeX(const float _x) {fSize.SetX(_x);}
    inline void   SetSizeY(const float _y) {fSize.SetY(_y);}
    inline void   SetSizeZ(const float _z) {fSize.SetZ(_z);}
    inline void   SetSize (const float x, const float y, const float z) { fSize.SetXYZ(x, y, z); }

    inline TVector3 GetNormal() const {return fNormal;}
    virtual TGraph BuildGShape() const =0; 
    inline void SetupAxis0( const TVector3 v) {fAxis0 = v;} 
    inline void SetupAxis1( const TVector3 v) {fAxis1 = v;}
    inline void SetupAxes ( const TVector3& v0, const TVector3& v1 ) {
      Double_t xx[3] = {0, 0, 0}; 
      v0.GetXYZ( xx ); fAxis0.SetXYZ(xx[0], xx[1], xx[2]); 
      v1.GetXYZ( xx ); fAxis1.SetXYZ(xx[0], xx[1], xx[2]); 
    }
    virtual void SetupAxes(); 
    inline const TVector3& GetAxis0() const {return fAxis0;} 
    inline const TVector3& GetAxis1() const {return fAxis1;} 
    inline const xyzpoint& GetVecAxis0() const {return fVecAxis0;} 
    inline const xyzpoint& GetVecAxis1() const {return fVecAxis1;} 
    inline const xyzpoint& GetVecNormal() const {return fVecNormal;}

  protected: 
    int      fID    ; //!< Module ID
    int      fIdx   ; //!< Module index
    int      fBin   ; //!< Module bin id in TH2Poly projection
    float    fX     ; //!< Module x coordinate in the parent volume frame
    float    fY     ; //!< Module y coordinate in the parent volume frame
    float    fZ     ; //!< Module z coordinate in the parent volume frame 
    float    fPhi   ; //!< Module rotation phi respect to parent volume
    float    fTheta ; //!< Module rotation theta respect to parent volume
    float    fPsi   ; //!< Module rotation psi respect to parent volume
    float    fPhysX ; //!< Module x coordinate in the World frame
    float    fPhysY ; //!< Module y coordinate in the World frame
    float    fPhysZ ; //!< Module z coordinate in the World frame 
    TVector3 fNormal; //!< Module normal vector
    TVector3 fAxis0 ; //!< Module frame x-axis 
    TVector3 fAxis1 ; //!< Module frame y-axis
    TVector3 fSize  ; //!< Module size  
    xyzpoint fVecAxis0; 
    xyzpoint fVecAxis1;
    xyzpoint fVecNormal;
    //TGraph*             fGShape; //!< Module shape in the TH2Poly projection


  public:
    ClassDef(SLArCfgBaseModule, 1); 
};



#endif /* end of include guard SLARCFGBASEMODULE_HH */

