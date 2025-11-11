/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArBulkVertexGenerator
 * @created     : giovedì giu 23, 2022 15:34:13 CEST
 */

#include <SLArBulkVertexGenerator.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4TransportationManager.hh>
#include <G4Material.hh>
#include <G4RandomTools.hh>
#include "geo/SLArGeoUtils.hh"
#include "geo/VolumeStruct.hh"

#include <TMath.h>

namespace gen {
namespace vertex {
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
  
void SLArBulkVertexGenerator::SetBulkLogicalVolume(G4LogicalVolume * logvol_, int counter)
{
  fSolid = logvol_->GetSolid();
  fLogVol = logvol_;
  fMass = GetMassVolumeGenerator() * counter;
  std::clog << "[log] SLArBulkVertexGenerator::SetBulkLogicalVolume: solid volume =" << fSolid << "\n";
  std::clog << "[log] SLArBulkVertexGenerator::SetBulkLogicalVolume: logic volume =" << fLogVol << "\n";
  std::clog << "[log] SLArBulkVertexGenerator::SetBulkLogicalVolume: mass [kg] =" << fMass / CLHEP::kg << "\n";
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

  //G4ThreeVector localVertex;
  G4Point3D localVertex;
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

  HepGeom::Point3D<G4double> vtx(localVertex.x(), localVertex.y(), localVertex.z());
  //Randomizer
  int index = fBulkTransformVec.size();

  if (index == 1) {
    vtx = fBulkTransformVec.at(0) * vtx; 
  }
  else {
    int rnd_seed = static_cast<int>(G4UniformRand() * (index + 1));
    vtx = fBulkTransformVec.at(rnd_seed) * vtx; 
  }

  //G4ThreeVector vtx = fBulkInverseRotation(localVertex) + fBulkTranslation;
  vertex_.set(vtx.x(), vtx.y(), vtx.z()); 
  //G4cout << "Vertex coordinates: " << vtx.x() << ", " << vtx.y() << ", " << vtx.z() << " m\n";
  fCounter++;
}

double SLArBulkVertexGenerator::ComputeDeltaX(
    const G4ThreeVector& lo, const G4ThreeVector& hi) const {
  return ComputeDeltaX(lo, hi, fFVFraction);
}

double SLArBulkVertexGenerator::ComputeDeltaX(
    const G4ThreeVector& lo, const G4ThreeVector& hi, const double fiducialf) const {
  // box dimensions in m reduce rounding errors
  const std::array<double,3> dx = {
    (hi.x() - lo.x())/CLHEP::m,
    (hi.y() - lo.y())/CLHEP::m,
    (hi.z() - lo.z())/CLHEP::m}; 
  const double short_side = *( std::min_element(dx.begin(), dx.end()) );

  const double a = 1.0; 
  const double b = -dx[0]-dx[1]-dx[2]; 
  const double c = dx[0]*dx[1] + dx[0]*dx[2] + dx[1]*dx[2]; 
  const double d = -dx[0]*dx[1]*dx[2]*(1-fiducialf);

  double r0 = 0, r1 = 0, r2 = 0; 
  const double coeff[4] = {d, c, b, a};
  bool has_one_rroot = TMath::RootsCubic(coeff, r0, r1, r2); 

  auto is_a_good_root = [short_side] (const double rroot) {
    if (rroot < 0) {return false;}
    if (rroot > short_side) {return false;}
    return true;
  };

  if (has_one_rroot) {
    r0 = 0.5*r0*CLHEP::m;
    return r0;
  }
  else {
    if      ( is_a_good_root(r0) ) {r0 = 0.5*r0*CLHEP::m; return r0;}
    else if ( is_a_good_root(r1) ) {r1 = 0.5*r1*CLHEP::m; return r1;}
    else if ( is_a_good_root(r2) ) {r2 = 0.5*r2*CLHEP::m; return r2;}
    else {
      printf("SLArBulkVertexGenerator::ComputeDeltaX() WARNING: no root matches the required conditions\n"); 
      return 0.0;
    }
  }
}


void SLArBulkVertexGenerator::Config(const G4String& target_volume_name) {
  auto volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(target_volume_name); 
  if (volume == nullptr) {
    char err_msg[200]; 
    snprintf(err_msg, sizeof(err_msg),
        "SLArBulkVertexGenerator::Config Error.\nUnable to find %s in physical volume store.\n", target_volume_name.data());
    throw std::runtime_error(err_msg);
  }

  SetBulkLogicalVolume(volume->GetLogicalVolume(), 1); 
  //SetSolidTranslation(volume->GetTranslation()); 
  //SetSolidRotation(volume->GetRotation()); 

  fBulkTransformVec.push_back(geo::GetTransformToGlobal(volume));
  return;  
}

void SLArBulkVertexGenerator::Config(const G4String& target_volume_name, const G4String& mother_volume_name) {
  auto volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(target_volume_name); 
  if (volume == nullptr) {
    char err_msg[200]; 
    snprintf(err_msg, sizeof(err_msg),
	    "SLArBulkVertexGenerator::Config Error.\nUnable to find %s in physical volume store.\n", target_volume_name.data());
    throw std::runtime_error(err_msg);
  }

  //VolumeStruct* result = geo::SearchLogicalVolumeInParametrisedVolume(target_volume_name, mother_volume_name);
  //if (!result) {
    //throw std::runtime_error("SLArBulkVertexGenerator::Config Error. Unable to find target logical volume.");
  //}
  //G4LogicalVolume* target_lv = result->logical_volume;
  //G4VPhysicalVolume* target_pv = result->physical_volume;

  //SetSolidTranslation(volume->GetTranslation()); 
  //SetSolidRotation(volume->GetRotation()); 
  
  fBulkTransformVec = geo::get_volume_transforms(target_volume_name, mother_volume_name);
  SetBulkLogicalVolume(volume->GetLogicalVolume(), fBulkTransformVec.size()); 

  return;
}

void SLArBulkVertexGenerator::Config(const rapidjson::Value& cfg) {
  if ( cfg.HasMember("volume")==false ) {
    throw std::invalid_argument("Missing mandatory \"volume\" field from bulk vtx generator specs.\n"); 
  }
  fTargetVolumeName = cfg["volume"].GetString(); 
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
  if ( cfg.HasMember("time") ) {
    fTimeGen.SourceConfiguration( cfg["time"] );
  }
  if (cfg.HasMember("mother")) {
    fMotherVolumeName = cfg["mother"].GetString();
    Config(fTargetVolumeName, fMotherVolumeName);
  }
  else if (cfg.HasMember("mother") == false) {
    Config(fTargetVolumeName);
  }
}

const rapidjson::Document SLArBulkVertexGenerator::ExportConfig() const {
  G4String gen_type = GetType();
  G4String solid_name = fSolid->GetName();
  G4String logic_name = fLogVol->GetName();

  rapidjson::Document vtx_info; 
  vtx_info.SetObject(); 

  rapidjson::Value str_gen_type;
  char buffer[50];
  int len = snprintf(buffer, sizeof(buffer), "%s", gen_type.data());
  str_gen_type.SetString(buffer, len, vtx_info.GetAllocator());
  vtx_info.AddMember("type", str_gen_type, vtx_info.GetAllocator()); 
  memset(buffer, 0, sizeof(buffer));

  rapidjson::Value str_solid_vol;
  len = snprintf(buffer, sizeof(buffer), "%s", solid_name.data());
  str_solid_vol.SetString(buffer, len, vtx_info.GetAllocator());
  vtx_info.AddMember("solid_volume", str_solid_vol, vtx_info.GetAllocator()); 
  memset(buffer, 0, sizeof(buffer));

  rapidjson::Value str_logic_vol; 
  len = snprintf(buffer, sizeof(buffer), "%s", logic_name.data());
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

  auto dtime = fTimeGen.ExportConfig();
  rapidjson::Value jtime; jtime.SetObject();
  jtime.CopyFrom( dtime, vtx_info.GetAllocator() );
  vtx_info.AddMember("time", jtime, vtx_info.GetAllocator());


  return vtx_info;
}
}
}
