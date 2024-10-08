/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventPhotonHit.hh
 * @created     : Wed Aug 10, 2022 12:08:32 CEST
 */

#ifndef SLAREVENTPHOTONHIT_HH

#define SLAREVENTPHOTONHIT_HH

#include "event/SLArEventGenericHit.hh"

enum  EPhProcess {kAll = 0, kCher = 1, kScnt = 2, kWLS = 3};
extern TString EPhProcName[4];
extern TString EPhProcTitle[4];

class SLArEventPhotonHit : public SLArEventGenericHit  
{
  public:
    SLArEventPhotonHit();
    SLArEventPhotonHit(float time, EPhProcess proc, float wvl = 0);
    SLArEventPhotonHit(float time, int proc, float wvl = 0);
    SLArEventPhotonHit(const SLArEventPhotonHit &pmtHit);
    ~SLArEventPhotonHit() {}

    void  SetProcess (EPhProcess p) {fProcess = p;}
    void  SetLocalPos(float x, float y, float z);
    void  SetWavelength(float w) {fWavelength = w;}
    void  SetMegaTileRowNr(int id) {fMegaTileRowNr = id;}
    void  SetMegaTileNr(int id) {fMegaTileNr = id;}
    void  SetRowTileNr(int id) {fRowTileNr = id;}
    void  SetTileNr(int id) {fTileNr = id;}
    void  SetCellNr(int n) {fCellNr = n;}
    void  SetRowCellNr(int n) {fRowCellNr = n; }
    void  SetTileInfo(int mtrow, int mg, int row, int tile); 

    int   GetMegaTileNr() const {return fMegaTileNr;}
    int   GetRowTileNr() const {return fRowTileNr;}
    int   GetTileNr() const {return fTileNr;}
    inline int GetTileID() const {return (fRowTileNr+1)*100 + fTileNr;}
    inline int GetMegaTileID() const {return (fMegaTileRowNr+1)*1000 + fMegaTileNr;}
    int   GetCellNr() const {return fCellNr;}
    int   GetRowCellNr() const {return fRowCellNr;}
    float GetWavelength() const {return fWavelength;}
    int   GetProcess() const {return fProcess;}
    float* GetLocalPos() {return fLocPos ;}

    void DumpInfo() const;

  private:
    int          fMegaTileRowNr; 
    int          fMegaTileNr; 
    int          fRowTileNr; 
    int          fTileNr; 
    int          fRowCellNr; 
    int          fCellNr;
    float        fWavelength;
    float        fLocPos[3];
    EPhProcess   fProcess;

    ClassDef(SLArEventPhotonHit, 3);
};

#endif /* end of include guard SLAREVENTPHOTONHIT_HH */

