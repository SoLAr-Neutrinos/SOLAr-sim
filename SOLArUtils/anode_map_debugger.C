/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : anode_map_debugger.C
 * @created     : Monday May 06, 2024 10:18:53 CEST
 */

#include <iostream>
#include <TFile.h>

#include <config/SLArCfgAnode.hh>
#include <config/SLArCfgMegaTile.hh>
#include <config/SLArCfgReadoutTile.hh>

int anode_map_debugger(const TString file_path)
{
  TFile* file = new TFile(file_path); 
  SLArCfgAnode* anodeCfg = file->Get<SLArCfgAnode>("AnodeCfg50");

  const auto& mtMap = anodeCfg->GetMap();
  for (const auto& mt : mtMap) {
    printf("MegaTile %i - pos: %.2f, %.2f, %.2f\n", mt.GetIdx(), 
        mt.GetX(), mt.GetY(), mt.GetZ());

    const auto& tMap = mt.GetConstMap(); 
    for (const auto& t : tMap) {
      printf("\tTile %i - pos: %.2f, %.2f, %.2f\n", t.GetIdx(), 
          t.GetX(), t.GetY(), t.GetZ());
    }
  }

  return 0;
}

