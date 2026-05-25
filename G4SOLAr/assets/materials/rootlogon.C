/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : rootlogon.C
 * @created     : mercoledì lug 20, 2022 16:33:35 CEST
 */

#include <iostream>
#include "TSystem.h"

void rootlogon()
{
  gSystem->AddIncludePath("/home/guff/Dune/SOLAr/G4SoLAr/G4SOLAr/include"); 
  gSystem->AddIncludePath("/home/guff/Software/geant4/geant4-11.0.1-install/include/Geant4");
  gSystem->AddDynamicPath("/home/guff/Dune/SOLAr/G4SoLAr/install/lib");
  gSystem->Load("/home/guff/Dune/SOLAr/G4SoLAr/install/lib/libSLArMCPrimaryInfo.so"); 
  gSystem->Load("/home/guff/Dune/SOLAr/G4SoLAr/install/lib/libSLArMCEventReadoutTile.so"); 
  gSystem->Load("/home/guff/Dune/SOLAr/G4SoLAr/install/lib/libSLArMCEvent.so"); 
  gSystem->Load("/home/guff/Dune/SOLAr/G4SoLAr/install/lib/libSLArReadoutSystemConfig.so"); 
}

