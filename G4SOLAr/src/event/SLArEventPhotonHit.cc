/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventPhotonHit.cc
 * @created     : mercoledì ago 10, 2022 12:10:50 CEST
 */

#include "event/SLArEventPhotonHit.hh"


ClassImp(SLArEventPhotonHit)

TString EPhProcName[4] = {"All", "Cher"     , "Scint"        , "WLS"};
TString EPhProcTitle[4]= {"All", "Cherenkov", "Scintillation", "WLS"};

SLArEventPhotonHit::SLArEventPhotonHit() : 
  fMegaTileIdx(0), fRowTileNr(0), fTileNr(0), 
  fTime(0.), fWavelength(0.), fLocPos{0, 0, 0}, fProcess(kAll)
{}

SLArEventPhotonHit::SLArEventPhotonHit(float time, EPhProcess proc, float wvl)
  : fMegaTileIdx(0), fRowTileNr(0), fTileNr(0), fLocPos{0, 0, 0} 
{
  fTime    = time;
  fWavelength  = wvl;
  fProcess = proc;
}

SLArEventPhotonHit::SLArEventPhotonHit(float time, int proc, float wvl)
  : fMegaTileIdx(0), fRowTileNr(0), fTileNr(0), fLocPos{0, 0, 0} 
{
  fTime    = time;
  fWavelength  = wvl;
  if      (proc == 1) fProcess = kCher;
  else if (proc == 2) fProcess = kScnt;
  else if (proc == 3) fProcess = kWLS ; 
  else                fProcess = kAll ;
}

SLArEventPhotonHit::SLArEventPhotonHit(const SLArEventPhotonHit &pmtHit) :
  TObject(pmtHit)
{
  fTime    = pmtHit.fTime;
  fProcess = pmtHit.fProcess;
  fWavelength  = pmtHit.fWavelength;
  fMegaTileIdx = pmtHit.fMegaTileIdx; 
  fRowTileNr = pmtHit.fRowTileNr; 
  fTileNr = pmtHit.fTileNr;
}

SLArEventPhotonHit::~SLArEventPhotonHit() {}

void SLArEventPhotonHit::DumpInfo()
{
  printf("SLArEventPhotonHit info:\n");
  printf("time = %g ns\nloc pos = [%.1f, %.1f, %.1f] mm\nproc = %s\n\n",
      fTime, fLocPos[0], fLocPos[1], fLocPos[2], 
      EPhProcTitle[fProcess].Data());
}

void SLArEventPhotonHit::SetLocalPos(float x, float y, float z) {
  fLocPos[0] = x;
  fLocPos[1] = y;
  fLocPos[2] = z;
}

void SLArEventPhotonHit::SetTileInfo(int mg, int row, int tile) {
  fMegaTileIdx = mg; 
  fRowTileNr = row; 
  fTileNr = tile; 
  return;
}
