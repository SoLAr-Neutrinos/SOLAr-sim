/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArAnalysisLibSetup.hpp
 * @created     : Fri Jun 16, 2022 10:13:21 CEST
 */

#include <iostream>
#include "TSystem.h"

void setup_slar_lib()
{
  printf("--------------------------------------------\n");
  printf("SOLArAnalysis: Loading SOLAr Event Libraries\n\n");
  gSystem->AddDynamicPath("");

  gSystem->Load("/home/guff/Dune/SOLAr/SOLAr-sim/install-v0.3/lib/libSLArReadoutSystemConfig.so"); 
  gSystem->Load("/home/guff/Dune/SOLAr/SOLAr-sim/install-v0.3/lib/libSLArMCEventReadout.so"); 
  gSystem->Load("/home/guff/Dune/SOLAr/SOLAr-sim/install-v0.3/lib/libSLArMCPrimaryInfo.so"); 
  gSystem->Load("/home/guff/Dune/SOLAr/SOLAr-sim/install-v0.3/lib/libSLArMCEvent.so"); 
}


