/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArBulkVertexGenerator
 * @created     : giovedì giu 23, 2022 15:34:13 CEST
 */

#include "SLArBulkVertexGenerator.hh"
#include "G4PhysicalVolumeStore.hh"
#include <G4TransportationManager.hh>
#include <G4Material.hh>
#include "G4RandomTools.hh"

namespace gen {
SLArBulkVertexGenerator::SLArBulkVertexGenerator()
{
  fBulkInverseRotation = fBulkRotation.inverse();
}

SLArBulkVertexGenerator::SLArBulkVertexGenerator(const SLArBulkVertexGenerator& origin)
{
  fLogVol = origin.fLogVol; 
  fBulkTranslation = origin.fBulkTranslation;
  fBulkRotation = origin.fBulkRotation;
  fTolerance = origin.fTolerance; 
  fRandomSeed = origin.fRandomSeed; 
  fNoDaughters = origin.fNoDaughters; 
  fFVFraction = origin.fFVFraction; 

  fSolid = origin.fSolid; 
  fBulkInverseRotation = origin.fBulkInverseRotation; 
  fCounter = origin.fCounter; 
}

SLArBulkVertexGenerator::~SLArBulkVertexGenerator()
{
  std::clog << "[log] SLArBulkVertexGenerator::DTOR: counter=" << fCounter << " generated vertexes\n";
}
 
const G4LogicalVolume * SLArBulkVertexGenerator::GetBulkLogicalVolume() const
{
  return fLogVol;
}
  
void SLArBulkVertexGenerator::SetBulkLogicalVolume(G4LogicalVolume * logvol_)
{
  fSolid = logvol_->GetSolid();
  fLogVol = logvol_;
  std::clog << "[log] SLArBulkVertexGenerator::SetBulkLogicalVolume: solid=" << fSolid << "\n";
}

const G4VSolid * SLArBulkVertexGenerator::GetSolid() const
{
  return fSolid;
}

const G4ThreeVector & SLArBulkVertexGenerator::GetSolidTranslation() const
{
  return fBulkTranslation;
}

void SLArBulkVertexGenerator::SetSolidTranslation(const G4ThreeVector & translation_)
{
  fBulkTranslation = translation_;
  std::clog << "[log] SLArBulkVertexGenerator::SetSolidTranslation: translation=" << (fBulkTranslation / CLHEP::mm) << " mm\n";
}

const G4RotationMatrix & SLArBulkVertexGenerator::GetSolidRotation() const
{
  return fBulkRotation;
}

const G4RotationMatrix & SLArBulkVertexGenerator::GetSolidInverseRotation() const
{
  return fBulkInverseRotation;
}
  
void SLArBulkVertexGenerator::SetSolidRotation(G4RotationMatrix* rotation_)
{
  if (!rotation_) fBulkRotation = G4RotationMatrix(); 
  else fBulkRotation = *rotation_;
  fBulkInverseRotation = fBulkRotation.inverse();
  std::clog << "[log] SLArBulkVertexGenerator::SetSolidRotation: rotation=" << fBulkRotation << "\n";
}

double SLArBulkVertexGenerator::GetTolerance() const
{
  return fTolerance;
}
  
void SLArBulkVertexGenerator::SetTolerance(double tolerance_)
{
  if (tolerance_ <= 0.0) {
    throw std::range_error("SLArBulkVertexGenerator::SetTolerance: invalid tolerance!");
  }
  fTolerance = tolerance_;  
  std::clog << "[log] SLArBulkVertexGenerator::SetBoxRotation: tolerance=" << fTolerance / CLHEP::mm << " mm\n";
}

void SLArBulkVertexGenerator::SetRandomSeed(unsigned int seed_)
{
  fRandomSeed = seed_;
  std::clog << "[log] SLArBulkVertexGenerator::SetRandomSeed: seed=" << fRandomSeed << " after " << fCounter << " generated vertexes\n";
}

void SLArBulkVertexGenerator::SetNoDaughters(bool no_daughters_)
{
  fNoDaughters = no_daughters_;
}
 
void SLArBulkVertexGenerator::ShootVertex(G4ThreeVector & vertex_)
{

  // sample a random vertex inside the volume given by fVolumeName
  G4ThreeVector lo; 
  G4ThreeVector hi;
  fSolid->BoundingLimits(lo, hi);

  //printf("lo: [%.1f, %.1f, %.1f]\nhi: [%.1f, %.1f, %.1f]\n", 
      //lo.x(), lo.y(), lo.z(), hi.x(), hi.y(), hi.z());

  double delta = 0.; 
  if (fFVFraction < 1.0) {
    delta = ComputeDeltaX(lo, hi); 
    //printf("delta = %g\n", delta);
  }

  auto navigator = G4TransportationManager::GetTransportationManager()->GetNavigator("World"); 

  G4ThreeVector localVertex;
  G4String localMaterial; 
  G4int maxtries=100000, itry=1;
  do {
    localVertex.set(
        lo.x() + 0.5*delta + G4UniformRand()*(hi.x()-lo.x()-delta),
        lo.y() + 0.5*delta + G4UniformRand()*(hi.y()-lo.y()-delta),
        lo.z() + 0.5*delta + G4UniformRand()*(hi.z()-lo.z()-delta));

    G4VPhysicalVolume* vol = navigator->LocateGlobalPointAndSetup(localVertex);
    localMaterial = vol->GetLogicalVolume()->GetMaterial()->GetName();
  } while (!fSolid->Inside(localVertex) && ++itry < maxtries && strcmp(fMaterial, localMaterial) != 0) ;

  G4ThreeVector vtx = fBulkInverseRotation(localVertex) + fBulkTranslation;
  vertex_.set(vtx.x(), vtx.y(), vtx.z()); 
  fCounter++;
}

double SLArBulkVertexGenerator::ComputeDeltaX(
    G4ThreeVector& lo, G4ThreeVector& hi, double fiducialf) {
  if (fiducialf == 0) fiducialf = fFVFraction; 

  double deltax = 0.; 
  double A = hi.x() - lo.x(); 
  double B = hi.y() - lo.y(); 
  double C = hi.z() - lo.z(); 

  //printf("A: %g, B: %g, C: %g, f: %g\n", A, B, C, fiducialf);

  double a = 1.0; 
  double b = -1.0*(A+B+C); 
  double c = (A*B + A*C + B*C); 
  double d = -1.0*(A*B*C)*(1-fiducialf); 

  double D0 = b*b - 3*a*c; 
  double D1 = 2*b*b*b -9*a*b*c +27*a*a*d; 
  double DD = std::cbrt(0.5*(D1 + std::sqrt(D1*D1 -4*D0*D0*D0)));

  deltax = - (b + DD + D0/DD) / (3*a); 
  //printf("deltax: %g\n", deltax); 

  return deltax; 
}

void SLArBulkVertexGenerator::Config(const G4String& volumeName) {
  auto volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(volumeName); 
  if (volume == nullptr) {
    char err_msg[200]; 
    sprintf(err_msg, "SLArBulkVertexGenerator::Config Error.\nUnable to find %s in physical volume store.\n", volumeName.c_str());
    throw std::runtime_error(err_msg);
  }

  SetBulkLogicalVolume(volume->GetLogicalVolume()); 
  SetSolidTranslation(volume->GetTranslation()); 
  SetSolidRotation(volume->GetRotation()); 
  return;
}

void SLArBulkVertexGenerator::Config(const rapidjson::Value& cfg) {
  if ( !cfg.HasMember("volume") ) {
    throw std::invalid_argument("Missing mandatory \"volume\" field from bulk vtx generator specs.\n"); 
  }
  G4String volName = cfg["volume"].GetString(); 
  if (cfg.HasMember("fiducial_fraction")) {
    fFVFraction = cfg["fiducial_fraction"].GetDouble(); 
  }
  if (cfg.HasMember("avoid_daughters")) {
    fNoDaughters = cfg["avoid_daughters"].GetBool();
  }
  if (cfg.HasMember("material")) {
    fMaterial = cfg["material"].GetString(); 
    fRequireMaterialMatch = true;
  }
  Config(volName);
}

const rapidjson::Document SLArBulkVertexGenerator::ExportConfig() const {
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
  
  vtx_info.AddMember("fiducial_volume_fraction", fFVFraction, vtx_info.GetAllocator()); 

  rapidjson::Value volume_val( rapidjson::kObjectType ); 
  volume_val.AddMember("val", GetCubicVolumeGenerator()/CLHEP::cm3, vtx_info.GetAllocator()); 
  volume_val.AddMember("unit", "cm3", vtx_info.GetAllocator()); 
  vtx_info.AddMember("cubic_volume", volume_val, vtx_info.GetAllocator()); 
  rapidjson::Value mass_val( rapidjson::kObjectType ); 
  mass_val.AddMember("val", GetMassVolumeGenerator()/CLHEP::kg, vtx_info.GetAllocator()); 
  mass_val.AddMember("unit", "kg", vtx_info.GetAllocator()); 
  vtx_info.AddMember("mass", mass_val, vtx_info.GetAllocator()); 
  return vtx_info;
}
}
