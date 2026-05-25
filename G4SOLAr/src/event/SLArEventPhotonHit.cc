/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArEventPhotonHit.cc
 * @created     : mercoledì ago 10, 2022 12:10:50 CEST
 */

#include "event/SLArEventPhotonHit.hh"


ClassImp(SLArEventPhotonHit)

SLArEventPhotonHit::SLArEventPhotonHit() : 
  SLArEventGenericHit(),
  fMegaTileRowNr(0), fMegaTileNr(0), fRowTileNr(0), fTileNr(0), 
  fRowCellNr(0), fCellNr(0), 
  fWavelength(0.), fLocPos{0, 0, 0}, fProcess(kAll)
{}

SLArEventPhotonHit::SLArEventPhotonHit(float time, EPhProcess proc, float wvl)
  : SLArEventGenericHit(), fMegaTileRowNr(0), fMegaTileNr(0), fRowTileNr(0), fTileNr(0),
    fRowCellNr(0), fCellNr(0), fLocPos{0, 0, 0} 
{
  fTime    = time;
  fWavelength  = wvl;
  fProcess = proc;
}

SLArEventPhotonHit::SLArEventPhotonHit(float time, int proc, float wvl)
  : SLArEventGenericHit(), fMegaTileRowNr(0), fMegaTileNr(0), fRowTileNr(0), fTileNr(0),
    fRowCellNr(0), fCellNr(0), fLocPos{0, 0, 0} 
{
  fTime    = time;
  fWavelength  = wvl;
  if      (proc == 1) fProcess = kCher;
  else if (proc == 2) fProcess = kScnt;
  else if (proc == 3) fProcess = kWLS ; 
  else                fProcess = kAll ;
}

SLArEventPhotonHit::SLArEventPhotonHit(const SLArEventPhotonHit &pmtHit) 
  : SLArEventGenericHit(pmtHit)
{
  fProcess       = pmtHit.fProcess;
  fWavelength    = pmtHit.fWavelength;
  fMegaTileRowNr = pmtHit.fMegaTileRowNr;
  fMegaTileNr    = pmtHit.fMegaTileNr;
  fRowTileNr     = pmtHit.fRowTileNr;
  fTileNr        = pmtHit.fTileNr;
  fRowCellNr     = pmtHit.fRowCellNr;
  fCellNr        = pmtHit.fCellNr;
}


void SLArEventPhotonHit::DumpInfo() const
{
  printf("SLArEventPhotonHit: ");
  printf("time = %g ns - loc pos = [%.1f, %.1f, %.1f] mm - proc = %s\n",
      fTime, fLocPos[0], fLocPos[1], fLocPos[2], 
      EPhProcTitle[fProcess].Data());
  printf("copyNo hierarchy:\n"); 
  printf("MT Row Nr : %i\n", fMegaTileRowNr); 
  printf("MT Nr : %i\n", fMegaTileNr   ); 
  printf("Tile Row Nr: %i\n", fRowTileNr    ); 
  printf("Tile Nr : %i\n", fTileNr       ); 
  printf("Cell Row Nr : %i\n", fRowCellNr    ); 
  printf("Cell Nr : %i\n", fCellNr       ); 
  printf("\n");

}

void SLArEventPhotonHit::SetLocalPos(float x, float y, float z) {
  fLocPos[0] = x;
  fLocPos[1] = y;
  fLocPos[2] = z;
}

void SLArEventPhotonHit::SetTileInfo(int mtrow, int mg, int row, int tile) {
  fMegaTileRowNr = mtrow;
  fMegaTileNr = mg; 
  fRowTileNr = row; 
  fTileNr = tile; 
  return;
}
