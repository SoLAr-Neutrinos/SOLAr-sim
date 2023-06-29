/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        build_pix_map.C
 * @created     Thu Jun 29, 2023 09:25:40 CEST
 * @brief       Quickly creates a json configuration file describing the pixel 
 *              readout map for the SoLAr_tile_v2
 */

#include <iostream>
#include <stdio.h>
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TRandom3.h"

void build_pix_map() 
{
  const int ncols = 8; 
  const int nrows = 8; 
  const float spacing = 4; 
  const float dd = spacing*0.5;
  const TString unit = "mm"; 

  const float x_0 = 0.5*nrows*spacing;
  const float z_0 =-0.5*ncols*spacing;

  FILE* json_map; 
  json_map = fopen("tile_v2_pixmap.json", "w"); 

  fprintf(json_map, "\"pixelmap\" : [\n"); 

  int ipix = 1; 
  for (int irow = 0; irow < nrows; irow++) {
    float xc = x_0 - (0.5 + irow)*spacing; 

    for (int icol = 0; icol < ncols; icol++) {
      float zc = z_0 + (0.5 + icol)*spacing;

      fprintf(json_map, "{\"name\": \"%s\", \"edges\" : [\n", Form("qpix%i", ipix)); 
      fprintf(json_map, "\t{\"z\":{\"val\": %g, \"unit\":\"%s\"}, \"x\":{\"val\":%g, \"unit\":\"\%s\"}},\n", zc-dd, unit.Data(), xc-dd, unit.Data());
      fprintf(json_map, "\t{\"z\":{\"val\": %g, \"unit\":\"%s\"}, \"x\":{\"val\":%g, \"unit\":\"\%s\"}},\n", zc+dd, unit.Data(), xc-dd, unit.Data());
      fprintf(json_map, "\t{\"z\":{\"val\": %g, \"unit\":\"%s\"}, \"x\":{\"val\":%g, \"unit\":\"\%s\"}},\n", zc+dd, unit.Data(), xc+dd, unit.Data());
      fprintf(json_map, "\t{\"z\":{\"val\": %g, \"unit\":\"%s\"}, \"x\":{\"val\":%g, \"unit\":\"\%s\"}},\n", zc-dd, unit.Data(), xc+dd, unit.Data());
      fprintf(json_map, "\t{\"z\":{\"val\": %g, \"unit\":\"%s\"}, \"x\":{\"val\":%g, \"unit\":\"\%s\"}}\n", zc-dd, unit.Data(), xc-dd, unit.Data());
      fprintf(json_map, "]},\n"); 
      
      ipix++; 
    }
  }

  fprintf(json_map, "]\n}\n"); 

  fclose(json_map); 
  
  return;
}

