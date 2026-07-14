/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArMCTruthLinkDef.h
 * @created     : Fri Feb 14, 2020 17:28:47 CET
 */


#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ struct trj_point+;
#pragma link C++ class std::vector<trj_point>+;
#pragma link C++ class SLArEventTrajectory+;
#pragma link C++ struct SLArEventTrajectoryLite::Coordinates_t+; 
#pragma link C++ class SLArEventTrajectoryLite+;
#pragma link C++ class std::vector<SLArEventTrajectory*>+;
#pragma link C++ class SLArMCPrimaryInfo+;
#pragma link C++ class std::vector<SLArMCPrimaryInfo>+;
#pragma link C++ class SLArMCTruth+;
#endif

