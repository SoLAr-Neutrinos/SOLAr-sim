/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : pixel_map_designer
 * @created     : Thursday May 02, 2024 10:57:39 CEST
 */

#include <iostream>
#include <fstream>
#include <TH2Poly.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/filewritestream.h>

const char* unit = "mm";

rapidjson::Value add_point(const double z, const double x, rapidjson::Document& d) {
  rapidjson::Value jj(rapidjson::kObjectType); 
  rapidjson::Value jz(rapidjson::kObjectType); 
  jz.AddMember("val", z, d.GetAllocator()); 
  jz.AddMember("unit", rapidjson::StringRef(unit), d.GetAllocator()); 
  jj.AddMember("z", jz, d.GetAllocator()); 
  rapidjson::Value jx(rapidjson::kObjectType);       
  jx.AddMember("val", x, d.GetAllocator()); 
  jx.AddMember("unit", rapidjson::StringRef(unit), d.GetAllocator()); 
  jj.AddMember("x", jx, d.GetAllocator()); 

  return jj;
}

int unitcell_designer(const double cell_zdim, const double cell_xdim, const double pix_pitch)
{
  const double pix_z_dim = pix_pitch;
  const double pix_x_dim = pix_pitch;

  const int n_z = std::floor( cell_zdim / pix_z_dim );
  const int n_x = std::floor( cell_xdim / pix_x_dim );

  TString fname = TString::Format("unitcell_z%.1f_x%.1f_pix%.1f.json", cell_zdim, cell_xdim, pix_pitch);
  FILE* fp = fopen(fname.Data(), "w");
  char writeBuffer[65536];
  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::StringBuffer buffer;
 
  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);

  rapidjson::Value junitcell(rapidjson::kObjectType);
  auto& allocator = d.GetAllocator();

  typedef std::pair<UInt_t, UInt_t> pixidx_t ; // (z, x) pixel index
  std::vector<pixidx_t> skip_list = {
    pixidx_t(3, 3),
    pixidx_t(3, 4),
    pixidx_t(4, 3),
    pixidx_t(4, 4) 
  }; 

  rapidjson::Value jucDimensionsArray(rapidjson::kArrayType);
  rapidjson::Value jucCellSizeX(rapidjson::kObjectType);
  rapidjson::Value jucCellSizeZ(rapidjson::kObjectType);
  jucCellSizeX.AddMember("name", rapidjson::StringRef("cell_size_x"), allocator);
  jucCellSizeX.AddMember("val", cell_xdim, allocator);
  jucCellSizeX.AddMember("unit", rapidjson::StringRef("mm"), allocator);
  jucCellSizeZ.AddMember("name", rapidjson::StringRef("cell_size_z"), allocator);
  jucCellSizeZ.AddMember("val", cell_zdim, allocator);
  jucCellSizeZ.AddMember("unit", rapidjson::StringRef("mm"), allocator);
  jucDimensionsArray.PushBack(jucCellSizeX, allocator);
  jucDimensionsArray.PushBack(jucCellSizeZ, allocator);
  junitcell.AddMember("dimensions", jucDimensionsArray, allocator);

  rapidjson::Value jucLineup(rapidjson::kArrayType);
  rapidjson::Value jucSiPM(rapidjson::kObjectType);
  jucSiPM.AddMember("component", rapidjson::StringRef("sipm"), allocator);
  jucSiPM.AddMember("name", rapidjson::StringRef("sipm"), allocator);
  jucSiPM.AddMember("copy", 0, allocator);
  rapidjson::Value jucSiPMPosX(rapidjson::kObjectType);
  rapidjson::Value jucSiPMPosZ(rapidjson::kObjectType);
  jucSiPMPosX.AddMember("val", 0.0, allocator);
  jucSiPMPosX.AddMember("unit", rapidjson::StringRef("mm"), allocator);
  jucSiPMPosZ.AddMember("val", 0.0, allocator);
  jucSiPMPosZ.AddMember("unit", rapidjson::StringRef("mm"), allocator);
  jucSiPM.AddMember("pos_x", jucSiPMPosX, allocator);
  jucSiPM.AddMember("pos_z", jucSiPMPosZ, allocator);
  jucLineup.PushBack(jucSiPM, allocator);

  UInt_t pix_nr = 1;
  for (UInt_t iz = 0; iz < n_z; iz++) {
    for (UInt_t ix = 0; ix < n_x; ix++) {
      // skip pixels in the skip list
      if (std::find(skip_list.begin(), skip_list.end(), pixidx_t(iz, ix)) != skip_list.end()) {
        continue;
      }

      const double _z = (-0.5*(n_z-1) + iz)*pix_z_dim;
      const double _x = (-0.5*(n_x-1) + ix)*pix_x_dim;

      rapidjson::Value jpix(rapidjson::kObjectType); 
      Int_t copy_nr = pix_nr;
      char name_c[10]; std::sprintf(name_c, "pix%u", copy_nr); 
      printf("pixel name: %s\n", name_c); 
      jpix.AddMember("component", rapidjson::StringRef("pixel"), allocator);
      jpix.AddMember("name", rapidjson::StringRef(name_c), allocator); 
      jpix.AddMember("copy", copy_nr, allocator);

      rapidjson::Value jpixPosX(rapidjson::kObjectType);
      rapidjson::Value jpixPosZ(rapidjson::kObjectType);
      jpixPosX.AddMember("val", _x, allocator);
      jpixPosX.AddMember("unit", rapidjson::StringRef("mm"), allocator);
      jpixPosZ.AddMember("val", _z, allocator);
      jpixPosZ.AddMember("unit", rapidjson::StringRef("mm"), allocator);
      jpix.AddMember("pos_x", jpixPosX, allocator);
      jpix.AddMember("pos_z", jpixPosZ, allocator);

      jucLineup.PushBack(jpix, allocator);
      pix_nr++;
    }
  }

  junitcell.AddMember("lineup", jucLineup, allocator);
  d.AddMember("unit_cell", junitcell, d.GetAllocator()); 
  
  d.Accept( writer ); 
  fclose( fp ); 
  return 0; 
}

int pixel_map_designer(const double cell_z_dim, const double cell_x_dim, const double pix_pitch)
{
  const double pix_z_dim = pix_pitch;
  const double pix_x_dim = pix_pitch;

  const int n_z = std::floor( cell_z_dim / pix_z_dim );
  const int n_x = std::floor( cell_x_dim / pix_x_dim );

  TString fname = TString::Format("pixelmap%.1f_x%.1f_pix%.1f.json", cell_z_dim, cell_x_dim, pix_pitch);
  FILE* fp = fopen(fname.Data(), "w");
  char writeBuffer[65536];
  rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::StringBuffer buffer;
 
  rapidjson::Document d; 
  d.SetObject(); 
  rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);

  rapidjson::Value jpixelmap(rapidjson::kArrayType);
  
  UInt_t pix_nr = 1; 
  for (int iz = 0; iz < n_z; iz++) {
    for (int ix = 0; ix < n_x; ix++) {

      const double _z = (-0.5*(n_z-1) + iz)*pix_z_dim;
      const double _x = (-0.5*(n_x-1) + ix)*pix_x_dim;

      rapidjson::Value jpix(rapidjson::kObjectType); 
      char name_c[10]; std::sprintf(name_c, "qpix%u", pix_nr); 
      printf("pixel name: %s\n", name_c); 
      jpix.AddMember("name", rapidjson::StringRef(name_c), d.GetAllocator()); 

      rapidjson::Value jedges(rapidjson::kArrayType); 
      rapidjson::Value j0 = add_point(_z-0.5*pix_z_dim, _x-0.5*pix_x_dim, d); jedges.PushBack(j0, d.GetAllocator()); 
      rapidjson::Value j1 = add_point(_z+0.5*pix_z_dim, _x-0.5*pix_x_dim, d); jedges.PushBack(j1, d.GetAllocator()); 
      rapidjson::Value j2 = add_point(_z+0.5*pix_z_dim, _x+0.5*pix_x_dim, d); jedges.PushBack(j2, d.GetAllocator()); 
      rapidjson::Value j3 = add_point(_z-0.5*pix_z_dim, _x+0.5*pix_x_dim, d); jedges.PushBack(j3, d.GetAllocator()); 
      rapidjson::Value j4 = add_point(_z-0.5*pix_z_dim, _x-0.5*pix_x_dim, d); jedges.PushBack(j4, d.GetAllocator()); 

      jpix.AddMember("edges", jedges, d.GetAllocator()); 

      jpixelmap.PushBack( jpix, d.GetAllocator() ); 
      pix_nr++;
    }
  }

  d.AddMember("pixelmap", jpixelmap, d.GetAllocator()); 
  
  d.Accept( writer ); 
  fclose( fp ); 


  return 0;
}

