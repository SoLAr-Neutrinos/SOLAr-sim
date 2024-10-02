/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArBoxSurfaceVertexGenerator
 * @created     Tue Apr 11, 2023 09:58:41 CEST
 */

#include <regex>

#include <SLArBoxSurfaceVertexGenerator.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4RandomTools.hh>
#include <G4Box.hh>

namespace gen {
SLArBoxSurfaceVertexGenerator::SLArBoxSurfaceVertexGenerator()
{
  fBulkInverseRotation = fBulkRotation.inverse(); 
}

SLArBoxSurfaceVertexGenerator::SLArBoxSurfaceVertexGenerator(const SLArBoxSurfaceVertexGenerator& origin)
{
  fLogVol = origin.fLogVol; 
  fBulkTranslation = origin.fBulkTranslation;
  fBulkRotation = origin.fBulkRotation;
  fTolerance = origin.fTolerance; 
  fRandomSeed = origin.fRandomSeed; 
  fNoDaughters = origin.fNoDaughters; 

  fSolid = origin.fSolid; 
  fBulkInverseRotation = origin.fBulkInverseRotation; 
  fCounter = origin.fCounter; 

}

SLArBoxSurfaceVertexGenerator::~SLArBoxSurfaceVertexGenerator()
{
  std::clog << "[log] SLArBoxSurfaceVertexGenerator::DTOR: counter=" << fCounter << " generated vertexes\n";
}
 
void SLArBoxSurfaceVertexGenerator::SetBoxLogicalVolume(const G4LogicalVolume * logvol_)
{
  fSolid = logvol_->GetSolid();
  fLogVol = logvol_;
  std::clog << "[log] SLArBoxSurfaceVertexGenerator::SetBoxLogicalVolume: solid=" << fSolid << "\n";
}

const G4VSolid * SLArBoxSurfaceVertexGenerator::GetSolid() const
{
  return fSolid;
}

const G4ThreeVector & SLArBoxSurfaceVertexGenerator::GetSolidTranslation() const
{
  return fBulkTranslation;
}

void SLArBoxSurfaceVertexGenerator::SetSolidTranslation(const G4ThreeVector & translation_)
{
  fBulkTranslation = translation_;
  std::clog << "[log] SLArBoxSurfaceVertexGenerator::SetSolidTranslation: translation=" << (fBulkTranslation / CLHEP::mm) << " mm\n";
}

const G4RotationMatrix & SLArBoxSurfaceVertexGenerator::GetSolidRotation() const
{
  return fBulkRotation;
}

const G4RotationMatrix & SLArBoxSurfaceVertexGenerator::GetSolidInverseRotation() const
{
  return fBulkInverseRotation;
}
  
void SLArBoxSurfaceVertexGenerator::SetSolidRotation(G4RotationMatrix* rotation_)
{
  if (!rotation_) fBulkRotation = G4RotationMatrix(); 
  else fBulkRotation = *rotation_;
  fBulkInverseRotation = fBulkRotation.inverse();
  std::clog << "[log] SLArBoxSurfaceVertexGenerator::SetSolidRotation: rotation=" << fBulkRotation << "\n";
}

double SLArBoxSurfaceVertexGenerator::GetTolerance() const
{
  return fTolerance;
}
  
void SLArBoxSurfaceVertexGenerator::SetTolerance(double tolerance_)
{
  if (tolerance_ <= 0.0) {
    throw std::range_error("SLArBoxSurfaceVertexGenerator::SetTolerance: invalid tolerance!");
  }
  fTolerance = tolerance_;  
  std::clog << "[log] SLArBoxSurfaceVertexGenerator::SetBoxRotation: tolerance=" << fTolerance / CLHEP::mm << " mm\n";
}

void SLArBoxSurfaceVertexGenerator::SetRandomSeed(unsigned int seed_)
{
  fRandomSeed = seed_;
  std::clog << "[log] SLArBoxSurfaceVertexGenerator::SetRandomSeed: seed=" << fRandomSeed << " after " << fCounter << " generated vertexes\n";
}

void SLArBoxSurfaceVertexGenerator::SetNoDaughters(bool no_daughters_)
{
  fNoDaughters = no_daughters_;
}

G4double SLArBoxSurfaceVertexGenerator::GetSurfaceGenerator() const {
  if (fFixFace == false) {
    return geo::get_bounding_volume_surface(fSolid); 
    //if (dynamic_cast<const G4Box*>(fSolid)) {
      //const auto box = (G4Box*)fSolid;
      //return box->GetSurfaceArea(); 
    //}
    //else {
      //printf("SLArBoxSurfaceVertexGenerator WARNING: "); 
      //printf("GetSurfaceGenerator() is only implemented for G4Box solids. "); 
      //printf("Feel free to work on your solid's implementation and let me know!\n");
      //printf("Using a box approximation.\n");

      //G4ThreeVector lo; 
      //G4ThreeVector hi;
      //G4ThreeVector dim; 
      //fSolid->BoundingLimits(lo, hi);

      //for (int i=0; i<3; i++) dim[i] = fabs(hi[i] - lo[i]); 

      //G4double half_area = dim[0]*dim[1] + dim[0]*dim[2] + dim[1]*dim[2];

      //return 2*half_area; 
    //}
  }
  else {
    if (dynamic_cast<const G4Box*>(fSolid)) {
      const auto box = (G4Box*)fSolid;
      if (fVtxFace == geo::kXplus || fVtxFace == geo::kXminus) {
        return box->GetYHalfLength()*box->GetZHalfLength()*4;
      }
      else if (fVtxFace == geo::kYplus || fVtxFace == geo::kYminus) {
        return box->GetYHalfLength()*box->GetZHalfLength()*4;
      }
      else if (fVtxFace == geo::kZplus || fVtxFace == geo::kZminus) {
        return box->GetYHalfLength()*box->GetXHalfLength()*4;
      }
      else {
        return 0.;
      }
    }
    else {
      printf("SLArBoxSurfaceVertexGenerator WARNING: "); 
      printf("GetSurfaceGenerator() is only implemented for G4Box solids. "); 
      printf("Feel free to work on your solid's implementation and let me know!\n"); 
      return 0; 
    }

  }
}
 
void SLArBoxSurfaceVertexGenerator::ShootVertex(G4ThreeVector & vertex_)
{
  // sample a random vertex inside the volume given by fVolumeName
  G4ThreeVector lo; 
  G4ThreeVector hi;
  G4ThreeVector dim; 
  std::vector<G4ThreeVector> axis = 
  {G4ThreeVector(1, 0, 0), G4ThreeVector(0, 1, 0), G4ThreeVector(0, 0, 1)}; 
  fSolid->BoundingLimits(lo, hi);

  for (int i=0; i<3; i++) dim[i] = fabs(hi[i] - lo[i]); 

  if (fFixFace == false) {
    G4double total_area = 2*(dim.x()*dim.y() + dim.x()*dim.z() + dim.y()*dim.z()); 
    std::map<geo::EBoxFace, G4double> area_fraction; 

    for (int i=0; i<6; i++) {
      geo::EBoxFace kFace = geo::EBoxFace(i); 
      G4double area = 1; 
      for (int  j=0; j < 3; j++) {
        if (geo::BoxFaceNormal[kFace].dot(axis[j]) == 0) area *= dim[j]; 
      }

      area_fraction.insert( std::make_pair(kFace, area / total_area)); 
    }

    G4double face_sample = G4UniformRand(); 
    G4double face_prob = 0.; 
    G4int iface = -1;
    //printf("face_sample = %g\n", face_sample);
    while (face_sample > face_prob) {
      iface++;
      face_prob += area_fraction[(geo::EBoxFace)iface]; 
    }

    fVtxFace = (geo::EBoxFace)iface;
  }

  //printf("vertex generator face is: %i\n", fVtxFace);
  //G4cout << "face normal is: " << BoxFaceNormal[fVtxFace] << G4endl;
  G4ThreeVector face_axis(0, 0, 0); 
  for (int j=0; j<3; j++) {
    if (axis[j].dot(geo::BoxFaceNormal[fVtxFace]) != 0) {
      face_axis = axis[j]; 
      break;
    }
  }

  G4ThreeVector face_center_local = 
    -0.5*dim.dot(geo::BoxFaceNormal[fVtxFace])*face_axis;
  //G4cout << "face_center_local: " << face_center_local << G4endl; 
  G4ThreeVector local_displacement(0, 0, 0); 
  for (int j=0; j<3; j++) {
    double rnd = G4UniformRand() - 0.5; 
    local_displacement[j] += 
    geo::BoxFaceNormal[fVtxFace].cross( axis[j] ).mag()*rnd*dim[j];
  }
  //G4cout << "local_displacement: " << local_displacement << G4endl;

  G4ThreeVector localVertex = face_center_local + local_displacement;
  G4ThreeVector finalVertex = fBulkInverseRotation(localVertex) + fBulkTranslation;

  vertex_.set(finalVertex.x(), finalVertex.y(), finalVertex.z()); 
  //G4cout << "vertex = " << vertex_ << G4endl;
  //getchar(); 
  axis.clear();
  fCounter++;
}

void SLArBoxSurfaceVertexGenerator::Config( const rapidjson::Value& config) {
  if ( !config.HasMember("volume") ) {
    throw std::invalid_argument("box surface vtx gen missing mandatory \"volume\" field\n");
  }

  G4String volumeName = config["volume"].GetString();
  auto volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(volumeName); 
  if (volume == nullptr) {
    char err_msg[200]; 
    sprintf(err_msg, "SLArBoxSurfaceVertexGenerator::Config ERROR\nUnable to find %s in physical volume store.\n", volumeName.data());
    throw std::runtime_error(err_msg);
  }

  SetBoxLogicalVolume(volume->GetLogicalVolume()); 
  SetSolidTranslation(volume->GetTranslation()); 
  SetSolidRotation(volume->GetRotation()); 

  if (config.HasMember("origin_face")) {
      FixVertexFace(true); 
      SetVertexFace(
          (geo::EBoxFace)config["origin_face"].GetInt()); 
  }

  return;
}

void SLArBoxSurfaceVertexGenerator::Config( const G4String& config ) {
  std::regex pattern("([a-zA-Z0-9_\\-]+)\\.?(\\d+)?$"); 
  std::smatch match; 
  std::regex_search(config, match, pattern); 

  G4String volumeName = match[1].str(); 
  G4String faceName = match[2].str(); 

  auto volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(volumeName); 
  if (volume == nullptr) {
    char err_msg[200];
    sprintf(err_msg, "SLArBoxSurfaceVertexGenerator::Config ERROR\nUnable to find %s in physical volume store.\n", volumeName.data());
    throw std::runtime_error(err_msg); 
  }

  SetBoxLogicalVolume(volume->GetLogicalVolume()); 
  SetSolidTranslation(volume->GetTranslation()); 
  SetSolidRotation(volume->GetRotation()); 

  if (const auto& box = dynamic_cast<G4Box*>(volume->GetLogicalVolume()->GetSolid())) {
    fSurface = box->GetSurfaceArea(); 
  }


  if (faceName.empty() == false) {
      FixVertexFace(true); 
      SetVertexFace(
          (geo::EBoxFace)std::atoi(faceName) ); 
  }

  return;


}

const rapidjson::Document SLArBoxSurfaceVertexGenerator::ExportConfig() const {
  G4String gen_type = GetType();
  G4String solid_name = fSolid->GetName();
  G4String logic_name = fLogVol->GetName();
  rapidjson::Document vtx_info; 
  vtx_info.SetObject(); 

  rapidjson::Value str_gen_type;
  char buffer[50];
  int len = sprintf(buffer, "%s", gen_type.data());
  str_gen_type.SetString(buffer, len, vtx_info.GetAllocator());
  vtx_info.AddMember("type", str_gen_type, vtx_info.GetAllocator()); 
  memset(buffer, 0, sizeof(buffer));

  rapidjson::Value str_solid_vol;
  len = sprintf(buffer, "%s", solid_name.data());
  str_solid_vol.SetString(buffer, len, vtx_info.GetAllocator());
  vtx_info.AddMember("solid_volume", str_solid_vol, vtx_info.GetAllocator()); 
  memset(buffer, 0, sizeof(buffer));

  rapidjson::Value str_logic_vol; 
  len = sprintf(buffer, "%s", logic_name.data());
  str_logic_vol.SetString(buffer, len, vtx_info.GetAllocator());
  memset(buffer, 0, sizeof(buffer));
  vtx_info.AddMember("logical_volume", str_logic_vol, vtx_info.GetAllocator()); 
  
  vtx_info.AddMember("is_fixed_face", fFixFace, vtx_info.GetAllocator()); 
  if (fFixFace) {
    vtx_info.AddMember("fixed_face", fVtxFace, vtx_info.GetAllocator()); 
  }
  rapidjson::Value surface_val( rapidjson::kObjectType ); 
  surface_val.AddMember("val", GetSurfaceGenerator() / CLHEP::cm2, vtx_info.GetAllocator()); 
  surface_val.AddMember("unit", "cm2", vtx_info.GetAllocator()); 
  vtx_info.AddMember("surface", surface_val, vtx_info.GetAllocator()); 
  
  return vtx_info;
}
}
