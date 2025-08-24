/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArEventReadoutLinkDef.h
 * @created     : Wed Aug 10, 2022 15:24:11 CEST
 */

#ifdef __MAKECINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class SLArEventGenericHit+; 
#pragma link C++ class SLArEventPhotonHit+; 
#pragma link C++ class SLArEventChargeHit+; 
#pragma link C++ class std::vector<SLArEventChargeHit>+; 
#pragma link C++ class std::vector<SLArEventPhotonHit>+;
#pragma link C++ class std::map<Int_t, UShort_t>+;
#pragma link C++ typedef BacktrackerCounter_t+;
#pragma link C++ class SLArEventBacktrackerRecord+;
#pragma link C++ class std::vector<SLArEventBacktrackerRecord>+;
#pragma link C++ class SLArEventBacktrackerVector+;
#pragma link C++ class std::map<UShort_t, SLArEventBacktrackerVector>+;
#pragma link C++ typedef BacktrackerVectorCollection_t+;
#pragma link C++ class std::map<UShort_t, UShort_t>+;
#pragma link C++ typedef HitsCollection_t+;
#pragma link C++ class SLArEventHitsCollection<SLArEventPhotonHit>+;
#pragma link C++ class SLArEventHitsCollection<SLArEventChargeHit>+;
#pragma link C++ class SLArEventChargePixel+; 
#pragma link C++ class SLArEventSiPM+; 
#pragma link C++ class std::map<int, SLArEventChargePixel>+; 
#pragma link C++ class std::map<int, SLArEventSiPM>+; 
#pragma link C++ class SLArEventTile+;
#pragma link C++ class std::map<int, SLArEventTile>+;
#pragma link C++ class SLArEventMegatile+;
#pragma link C++ class std::map<int, SLArEventMegatile>+;
#pragma link C++ class SLArEventAnode+;
#pragma link C++ class std::map<int, SLArEventAnode>+;
#pragma link C++ class SLArListEventAnode+;
#pragma link C++ class SLArEventSuperCell+;
#pragma link C++ class std::map<int, SLArEventSuperCell>+;
#pragma link C++ class SLArEventSuperCellArray+;
#pragma link C++ class std::map<int, SLArEventSuperCellArray>+;
#pragma link C++ class SLArListEventPDS+;

#endif


