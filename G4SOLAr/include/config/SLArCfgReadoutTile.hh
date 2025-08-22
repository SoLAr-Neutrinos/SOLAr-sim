/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArCfgReadoutTile.hh
 * @created     : martedì lug 19, 2022 13:38:21 CEST
 */

#ifndef SLARCFGREADOUTTILE_HH

#define SLARCFGREADOUTTILE_HH
#include <map>
#include <array>
#include "config/SLArCfgBaseModule.hh"
#include "TGraph.h"
#include "TH2Poly.h"

class SLArCfgReadoutTile : public SLArCfgBaseModule 
{
  public: 
    typedef std::array<double, 2> xypoint;

    SLArCfgReadoutTile(); 
    SLArCfgReadoutTile(int idx); 
    SLArCfgReadoutTile(int idx, float xc, float yc, float zc, float phi, float theta, float psi); 
    SLArCfgReadoutTile(const SLArCfgReadoutTile& ref); 
    ~SLArCfgReadoutTile(); 

    //TH2Poly* GetPixHistMap() {return fH2Pixels;}
    //std::map<int, TGraph*>& GetPixBins() {return fPixelBins;}
    double Get2DSize_X() {return f2DSize_X;}
    double Get2DSize_Y() {return f2DSize_Y;}
    void   Set2DSize_X(float _x) {f2DSize_X = _x;}
    void   Set2DSize_Y(float _y) {f2DSize_Y = _y;}
    void   DumpInfo() const override;
    TGraph BuildGShape() const override;
    void   SetNCellRows(UInt_t nrows) {fNCellRows = nrows;}
    void   SetNCellCols(UInt_t ncols) {fNCellCols = ncols;}
    UInt_t GetNCellRows() const {return fNCellRows;}
    UInt_t GetNCellCols() const {return fNCellCols;}

    // TODO: Move this method in a more appropriate place
    //void   AddPixelToHistMap(TH2Poly* hmap, std::vector<xypoint>);
    // TODO: Move this method in SLArCfgSystemPix
    //int    FindPixel(double, double); 

  protected:
    float    f2DSize_X; 
    float    f2DSize_Y;
    UInt_t   fNCellRows = {};
    UInt_t   fNCellCols = {};
    //TH2Poly* fH2Pixels; 
    //std::map<int, TGraph*> fPixelBins; 

  public:
    ClassDefOverride(SLArCfgReadoutTile, 2);
}; 



#endif /* end of include guard SLARCFGTILE_HH */

