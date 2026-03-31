/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGPSVertexGenerator
 * @created     : Wednesday Dec 04, 2024 10:58:43 CET
 */

#include "gen/SLArGPSVertexGenerator.hh"
#include "SLArGeoUtils.hh"

namespace gen {
namespace vertex {

SLArGPSVertexGenerator::SLArGPSVertexGenerator() 
{
  fPSPosGen = std::make_unique<G4SPSPosDistribution>();
  fPSRandGen = std::make_unique<G4SPSRandomGenerator>();
  fPSPosGen->SetBiasRndm( fPSRandGen.get() );
}

SLArGPSVertexGenerator::~SLArGPSVertexGenerator() {}

double SLArGPSVertexGenerator::GetSurfaceGenerator() const 
{
  if ( fPSPosGen->GetPosDisType() != "Surface"  && 
       fPSPosGen->GetPosDisType() != "Plane" ) {
    G4Exception("SLArGPSVertexGenerator::GetSurfaceArea", "VtxGen001", FatalException,
        "Surface area is only defined for 'Surface' or 'Plane' position distribution types.");
  }

  G4double surface = 0.0;

  if (fPSPosGen->GetPosDisShape() == "Circle") {
    surface = 2.0 * CLHEP::pi * 
      (fPSPosGen->GetRadius()*fPSPosGen->GetRadius());
  }
  else if (fPSPosGen->GetPosDisShape() == "Annulus") {
    surface = 2.0 * CLHEP::pi * 
      (fPSPosGen->GetRadius()*fPSPosGen->GetRadius() - 
       fPSPosGen->GetRadius0()*fPSPosGen->GetRadius0());
  }
  else if (fPSPosGen->GetPosDisShape() == "Ellipse") {
    surface = 2.0 * CLHEP::pi * 
      (fPSPosGen->GetHalfX() * fPSPosGen->GetHalfY());
  }
  else if (fPSPosGen->GetPosDisShape() == "Square") {
    surface = 4.0 * (fPSPosGen->GetHalfX() * fPSPosGen->GetHalfY());
  }
  else if (fPSPosGen->GetPosDisShape() == "Rectangle") {
    surface = 4.0 * (fPSPosGen->GetHalfX() * fPSPosGen->GetHalfY());
  }
  else {
    G4Exception("SLArGPSVertexGenerator::GetSurfaceArea", "VtxGen002", FatalException,
        ("Surface area calculation not implemented for shape: " + fPSPosGen->GetPosDisShape()).data());
  }

  return surface;
}

G4double SLArGPSVertexGenerator::GetVolumeGenerator() const 
{
  if ( fPSPosGen->GetPosDisType() != "Volume" ) {
    G4Exception("SLArGPSVertexGenerator::GetVolumeGenerator", "VtxGen003", FatalException,
        "Volume is only defined for 'Volume' position distribution type.");
  }

  G4double volume = 0.0;

  if (fPSPosGen->GetPosDisShape() == "Sphere") {
    volume = (4.0/3.0) * CLHEP::pi * 
      (fPSPosGen->GetRadius()*fPSPosGen->GetRadius()*fPSPosGen->GetRadius() - 
       fPSPosGen->GetRadius0()*fPSPosGen->GetRadius0()*fPSPosGen->GetRadius0());
  }
  else if (fPSPosGen->GetPosDisShape() == "Ellipsoid") {
    volume = (4.0/3.0) * CLHEP::pi * 
      (fPSPosGen->GetHalfX() * fPSPosGen->GetHalfY() * fPSPosGen->GetHalfZ());
  }
  else if (fPSPosGen->GetPosDisShape() == "Cylinder") {
    volume = CLHEP::pi * 
      (fPSPosGen->GetRadius()*fPSPosGen->GetRadius() - 
       fPSPosGen->GetRadius0()*fPSPosGen->GetRadius0()) * 
      (2.0 * fPSPosGen->GetHalfZ());
  }
  else if (fPSPosGen->GetPosDisShape() == "Para") {
    volume = 8.0 * fPSPosGen->GetHalfX() * fPSPosGen->GetHalfY() * fPSPosGen->GetHalfZ();
  }
  else {
    G4Exception("SLArGPSVertexGenerator::GetVolumeGenerator", "VtxGen004", FatalException,
        ("Volume calculation not implemented for shape: " + fPSPosGen->GetPosDisShape()).data());
  }

  return volume;
}

void SLArGPSVertexGenerator::Config(const rapidjson::Value& config) 
{
  if (config.HasMember("time")) {
    fTimeGen.SourceConfiguration( config["time"] );
  }
  if ( config.HasMember("volume") ) {
    fReferenceVolumeName = config["volume"].GetString(); 
    G4PhysicalVolumeStore* pvstore = G4PhysicalVolumeStore::GetInstance();
    auto ref = pvstore->GetVolume(fReferenceVolumeName);
    const auto to_global = geo::GetTransformToGlobal(ref);
    fTransform = to_global;
  }
  config.HasMember("type") ?
    fPSPosGen->SetPosDisType( config["type"].GetString() ) :
    fPSPosGen->SetPosDisType( "Point" );
  config.HasMember("shape") ?
    fPSPosGen->SetPosDisShape( config["shape"].GetString() ) :
    fPSPosGen->SetPosDisShape( "Square" );
  if (config.HasMember("center")){
    G4ThreeVector center;
    G4double vunit = 1.0; 
    const auto& jcenter = config["center"];
    if (jcenter.HasMember("unit")) {
      vunit = unit::GetJSONunit(config["center"]);
    }
    if (jcenter.HasMember("val") == false) {
      G4String msg = "SLArGPSVertexGenerator::Config ERROR: ";
      msg += "field \"center\" must have a field \"val\"\n";
      throw std::invalid_argument(msg);
    }

    const auto& jcoords = jcenter["val"];
    if (jcoords.IsArray() == false) {
      G4String msg = "SLArGPSVertexGenerator::Config ERROR: ";
      msg += "field \"val\" of \"center\" must be a rapidjson::Array\n";
      throw std::invalid_argument(msg); 
    }
    double xx[3] = {0.0, 0.0, 0.0}; 
    size_t i = 0; 
    for (const auto& jval : jcoords.GetArray()) {
      if (jval.IsNumber() == false) {
        G4String msg = "SLArGPSVertexGenerator::Config ERROR: ";
        msg += "field \"center\" must be a rapidjson::Array of numbers\n";
        throw std::invalid_argument(msg); 
      }
      xx[i] = jval.GetDouble();
      i++;
    }
    center.set( xx[0]*vunit, xx[1]*vunit, xx[2]*vunit );
    fPSPosGen->SetCentreCoords( center );
  }
  if (config.HasMember("halfx"))
    fPSPosGen->SetHalfX( unit::ParseJsonVal(config["halfx"]) );
  if (config.HasMember("halfy"))
    fPSPosGen->SetHalfY( unit::ParseJsonVal(config["halfy"]) );
  if (config.HasMember("halfz"))
    fPSPosGen->SetHalfZ( unit::ParseJsonVal(config["halfz"]) );
  if (config.HasMember("radius"))
    fPSPosGen->SetRadius( unit::ParseJsonVal(config["radius"]) );
  if (config.HasMember("inner_radius"))
    fPSPosGen->SetRadius0( unit::ParseJsonVal(config["inner_radius"]) );
  if (config.HasMember("radius0"))
    fPSPosGen->SetRadius0( unit::ParseJsonVal(config["radius0"]) );
  if (config.HasMember("rot1")) {
    G4ThreeVector rot1;
    const auto& jrot1 = config["rot1"];
    if (jrot1.IsArray() == false) {
      G4String msg = "SLArGPSVertexGenerator::Config ERROR: ";
      msg += "field \"rot1\" must be a rapidjson::Array\n";
      throw std::invalid_argument(msg); 
    }
    double xx[3] = {0.0, 0.0, 0.0}; 
    size_t i = 0; 
    for (const auto& jval : jrot1.GetArray()) {
      if (jval.IsNumber() == false) {
        G4String msg = "SLArGPSVertexGenerator::Config ERROR: ";
        msg += "field \"rot1\" must be a rapidjson::Array of numbers\n";
        throw std::invalid_argument(msg); 
      }
      xx[i] = jval.GetDouble();
      i++;
    }
    rot1.set( xx[0], xx[1], xx[2] );
    fPSPosGen->SetPosRot1( rot1 );
  }
  if (config.HasMember("rot2")) {
    G4ThreeVector rot2;
    const auto& jrot2 = config["rot2"];
    if (jrot2.IsArray() == false) {
      G4String msg = "SLArGPSVertexGenerator::Config ERROR: ";
      msg += "field \"rot2\" must be a rapidjson::Array\n";
      throw std::invalid_argument(msg); 
    }
    double xx[3] = {0.0, 0.0, 0.0}; 
    size_t i = 0; 
    for (const auto& jval : jrot2.GetArray()) {
      if (jval.IsNumber() == false) {
        G4String msg = "SLArGPSVertexGenerator::Config ERROR: ";
        msg += "field \"rot2\" must be a rapidjson::Array of numbers\n";
        throw std::invalid_argument(msg); 
      }
      xx[i] = jval.GetDouble();
      i++;
    }
    rot2.set( xx[0], xx[1], xx[2] );
    fPSPosGen->SetPosRot2( rot2 );
  }
  if (config.HasMember("beam_sigma_r")) {
    _beam_sigma_r = unit::ParseJsonVal(config["beam_sigma_r"]);
    fPSPosGen->SetBeamSigmaInR( _beam_sigma_r );
  }
  if (config.HasMember("beam_sigma_x")) {
    _beam_sigma_x = unit::ParseJsonVal(config["beam_sigma_x"]);
    fPSPosGen->SetBeamSigmaInX( _beam_sigma_x );
  }
  if (config.HasMember("beam_sigma_y")) {
    _beam_sigma_y = unit::ParseJsonVal(config["beam_sigma_y"]);
    fPSPosGen->SetBeamSigmaInY( _beam_sigma_y );
  }
  if (config.HasMember("alpha"))
    fPSPosGen->SetParAlpha( unit::ParseJsonVal(config["alpha"]) );
  if (config.HasMember("theta"))
    fPSPosGen->SetParTheta( unit::ParseJsonVal(config["theta"]) );
  if (config.HasMember("phi"))
    fPSPosGen->SetParPhi( unit::ParseJsonVal(config["phi"]) );
  if (config.HasMember("confine_to_volume"))
    fPSPosGen->ConfineSourceToVolume( config["confine_to_volume"].GetString() );

  return;
}

void SLArGPSVertexGenerator::ShootVertex(G4ThreeVector& vertex) 
{
  G4ThreeVector x = fPSPosGen->GenerateOne();
  HepGeom::Point3D<G4double> vtx(x.x(), x.y(), x.z());
  vtx = fTransform * vtx;
  vertex.set( vtx.x(), vtx.y(), vtx.z() );
  return;
}

rapidjson::Document SLArGPSVertexGenerator::ExportConfigPointSource() const
{
  rapidjson::Document src_info; 
  src_info.SetObject();
  auto &dalloc = src_info.GetAllocator();

  rapidjson::Value gen_center;
  FillConfigVector( fPSPosGen->GetCentreCoords(), gen_center, dalloc );
  src_info.AddMember("center", gen_center, dalloc); 
  if (fReferenceVolumeName.empty() == false) {
    rapidjson::Value jvol; 
    jvol.SetString(fReferenceVolumeName.data(), dalloc);
    src_info.AddMember("reference_volume", jvol, dalloc);
  }

  return src_info;
}

rapidjson::Document SLArGPSVertexGenerator::ExportConfigBeamSource() const 
{
  rapidjson::Document src_info;
  src_info.SetObject();
  auto &dalloc = src_info.GetAllocator();

  rapidjson::Value gen_center;
  FillConfigVector( fPSPosGen->GetCentreCoords(), gen_center, dalloc );
  src_info.AddMember("center", gen_center, dalloc); 
  src_info.AddMember("sigma_r", _beam_sigma_r, dalloc);
  src_info.AddMember("sigma_x", _beam_sigma_x, dalloc);
  src_info.AddMember("sigma_y", _beam_sigma_y, dalloc);
  rapidjson::Value gen_rotx;
  FillConfigVector( fPSPosGen->GetRotx(), gen_rotx, dalloc );
  src_info.AddMember("rotx", gen_rotx, dalloc);
  rapidjson::Value gen_roty;
  FillConfigVector( fPSPosGen->GetRoty(), gen_roty, dalloc );
  src_info.AddMember("roty", gen_roty, dalloc);
  rapidjson::Value gen_rotz;
  FillConfigVector( fPSPosGen->GetRotz(), gen_rotz, dalloc );
  src_info.AddMember("rotz", gen_rotz, dalloc);

  if (fReferenceVolumeName.empty() == false) {
    rapidjson::Value jvol; 
    jvol.SetString(fReferenceVolumeName.data(), dalloc);
    src_info.AddMember("reference_volume", jvol, dalloc);
  }


  return src_info;
}

rapidjson::Document SLArGPSVertexGenerator::ExportConfigPlaneSource() const 
{
  rapidjson::Document src_info;
  src_info.SetObject();
  auto &dalloc = src_info.GetAllocator();

  G4String gen_shape = fPSPosGen->GetPosDisShape();
  rapidjson::Value jshape(rapidjson::kStringType);
  jshape.SetString(gen_shape.data(), gen_shape.size(), dalloc);
  src_info.AddMember("shape", jshape, dalloc);

  rapidjson::Value gen_center; 
  FillConfigVector( fPSPosGen->GetCentreCoords(), gen_center, dalloc );
  src_info.AddMember("center", gen_center, dalloc); 

  if (gen_shape == "Square" || gen_shape == "Rectangle" || gen_shape == "Ellipse") {
    src_info.AddMember("halfx", fPSPosGen->GetHalfX(), dalloc);
    src_info.AddMember("halfy", fPSPosGen->GetHalfY(), dalloc);
  }
  else if (gen_shape == "Circle") {
    src_info.AddMember("radius", fPSPosGen->GetRadius(), dalloc);
  }
  else if (gen_shape == "Annulus") {
    src_info.AddMember("radius", fPSPosGen->GetRadius(), dalloc);
    src_info.AddMember("inner_radius", fPSPosGen->GetRadius0(), dalloc);
  }
  else {
    G4String msg = "SLArGPSVertexGenerator::ExportConfigPlaneSource: ";
    msg += ("unknown shape " + gen_shape + "\n");
    throw std::invalid_argument(msg);
  }
  rapidjson::Value gen_rotx;
  FillConfigVector( fPSPosGen->GetRotx(), gen_rotx, dalloc );
  src_info.AddMember("rotx", gen_rotx, dalloc);
  rapidjson::Value gen_roty;
  FillConfigVector( fPSPosGen->GetRoty(), gen_roty, dalloc );
  src_info.AddMember("roty", gen_roty, dalloc);
  rapidjson::Value gen_rotz;
  FillConfigVector( fPSPosGen->GetRotz(), gen_rotz, dalloc );
  src_info.AddMember("rotz", gen_rotz, dalloc);

  if (fReferenceVolumeName.empty() == false) {
    rapidjson::Value jvol; 
    jvol.SetString(fReferenceVolumeName.data(), dalloc);
    src_info.AddMember("reference_volume", jvol, dalloc);
  }


  return src_info;
}

rapidjson::Document SLArGPSVertexGenerator::ExportConfigVolumeSource() const {
  rapidjson::Document src_info;
  src_info.SetObject();
  auto &dalloc = src_info.GetAllocator();

  G4String gen_shape = fPSPosGen->GetPosDisShape();
  rapidjson::Value jshape(rapidjson::kStringType);
  jshape.SetString(gen_shape.data(), gen_shape.size(), dalloc);
  src_info.AddMember("shape", jshape, dalloc);

  rapidjson::Value gen_center; 
  FillConfigVector( fPSPosGen->GetCentreCoords(), gen_center, dalloc );
  src_info.AddMember("center", gen_center, dalloc); 

  if (gen_shape == "Sphere") {
    src_info.AddMember("radius", fPSPosGen->GetRadius(), dalloc);
  }
  else if (gen_shape == "Cylinder") {
    src_info.AddMember("radius", fPSPosGen->GetRadius(), dalloc);
    src_info.AddMember("halfz", fPSPosGen->GetHalfZ(), dalloc);
  }
  else if (gen_shape == "Ellipsoid" || gen_shape == "Right") {
    src_info.AddMember("halfx", fPSPosGen->GetHalfX(), dalloc);
    src_info.AddMember("halfy", fPSPosGen->GetHalfY(), dalloc);
    src_info.AddMember("halfz", fPSPosGen->GetHalfZ(), dalloc);
  }
  else {
    G4String msg = "SLArGPSVertexGenerator::ExportConfigVolumeSource: ";
    msg += ("unknown shape " + gen_shape + "\n");
    throw std::invalid_argument(msg);
  }
  rapidjson::Value gen_rotx;
  FillConfigVector( fPSPosGen->GetRotx(), gen_rotx, dalloc );
  src_info.AddMember("rotx", gen_rotx, dalloc);
  rapidjson::Value gen_roty;
  FillConfigVector( fPSPosGen->GetRoty(), gen_roty, dalloc );
  src_info.AddMember("roty", gen_roty, dalloc);
  rapidjson::Value gen_rotz;
  FillConfigVector( fPSPosGen->GetRotz(), gen_rotz, dalloc );
  src_info.AddMember("rotz", gen_rotz, dalloc);

  if (fReferenceVolumeName.empty() == false) {
    rapidjson::Value jvol; 
    jvol.SetString(fReferenceVolumeName.data(), dalloc);
    src_info.AddMember("reference_volume", jvol, dalloc);
  }


  return src_info;
}

const rapidjson::Document SLArGPSVertexGenerator::ExportConfig() const {
  rapidjson::Document vtx_info; 
  vtx_info.SetObject(); 
  auto &dalloc = vtx_info.GetAllocator();

  G4String gen_type = fPSPosGen->GetPosDisType();

  rapidjson::Value str_gen_type;
  str_gen_type.SetString(gen_type, dalloc);
  vtx_info.AddMember("type", str_gen_type, dalloc); 

  if (fPSPosGen->GetConfined()) {
    G4String vol_name = fPSPosGen->GetConfineVolume();
    rapidjson::Value jvol; jvol.SetString(vol_name.data(), vol_name.size(), dalloc);
    vtx_info.AddMember("confined_to_volume", jvol, dalloc);
  }

  if ( gen_type == "Point" )
  {
    rapidjson::Document src_info = ExportConfigPointSource();
    rapidjson::Value jsrc; jsrc.SetObject();
    jsrc.CopyFrom( src_info, dalloc );
    vtx_info.AddMember("source", jsrc, dalloc);
  }
  else if ( gen_type == "Beam" )
  {
    rapidjson::Document src_info = ExportConfigBeamSource();
    rapidjson::Value jsrc; jsrc.SetObject();
    jsrc.CopyFrom( src_info, dalloc );
    vtx_info.AddMember("source", jsrc, dalloc);
  }
  else if ( gen_type == "Plane" )
  {
    rapidjson::Document src_info = ExportConfigPlaneSource();
    rapidjson::Value jsrc; jsrc.SetObject();
    jsrc.CopyFrom( src_info, dalloc );
    vtx_info.AddMember("source", jsrc, dalloc);
  }
  else if ( gen_type == "Surface" )
  {
    rapidjson::Document src_info = ExportConfigPlaneSource();
    rapidjson::Value jsrc; jsrc.SetObject();
    jsrc.CopyFrom( src_info, dalloc );
    vtx_info.AddMember("source", jsrc, dalloc);
  }

  else if ( gen_type == "Volume" )
  {
    rapidjson::Document src_info = ExportConfigVolumeSource();
    rapidjson::Value jsrc; jsrc.SetObject();
    jsrc.CopyFrom( src_info, dalloc );
    vtx_info.AddMember("source", jsrc, dalloc);
  }
  else
  {
    G4String msg = "SLArGPSVertexGenerator::ExportConfig: ";
    msg += ("unknown generator type " + gen_type + "\n");
    throw std::invalid_argument(msg);
  }

  auto dtime = fTimeGen.ExportConfig();
  rapidjson::Value jtime; jtime.SetObject();
  jtime.CopyFrom( dtime, vtx_info.GetAllocator() );
  vtx_info.AddMember("time", jtime, vtx_info.GetAllocator());

  return vtx_info; 
}

void SLArGPSVertexGenerator::Print() const 
{
  const G4String gen_type = fPSPosGen->GetPosDisType();
  printf("SLArGPSVertexGenerator configuration dump:\n"); 
  printf("generator type: %s\n", gen_type.data()); 

  if (gen_type == "Point") PrintConfigPointSource();
  else if (gen_type == "Beam") PrintConfigBeamSource();
  else if (gen_type == "Plane") PrintConfigPlaneSource();
  else if (gen_type == "Surface") PrintConfigPlaneSource();
  else if (gen_type == "Volume") PrintConfigVolumeSource();
  else {
    G4String msg = "SLArGPSVertexGenerator::Print: ";
    msg += ("unknown generator type " + gen_type + "\n");
    throw std::invalid_argument(msg);
  }

  return;
}

void SLArGPSVertexGenerator::PrintConfigPointSource() const 
{
  G4ThreeVector center = fPSPosGen->GetCentreCoords();
  printf("center set to %g, %g, %g mm\n", 
      center.x(), center.y(), center.z()); 
  return;
}

void SLArGPSVertexGenerator::PrintConfigBeamSource() const 
{
  G4ThreeVector center = fPSPosGen->GetCentreCoords();
  printf("center set to %g, %g, %g mm\n", 
      center.x(), center.y(), center.z()); 
  printf("sigma_r = %g mm\n", _beam_sigma_r);
  printf("sigma_x = %g mm\n", _beam_sigma_x);
  printf("sigma_y = %g mm\n", _beam_sigma_y);
  return;
}

void SLArGPSVertexGenerator::PrintConfigPlaneSource() const 
{
  G4String shape = fPSPosGen->GetPosDisShape();
  printf("shape set to %s\n", shape.data()); 
  G4ThreeVector center = fPSPosGen->GetCentreCoords();
  printf("center set to %g, %g, %g mm\n", 
      center.x(), center.y(), center.z()); 
  if (shape == "Square" || shape == "Rectangle" || shape == "Ellipse") {
    printf("halfx = %g mm\n", fPSPosGen->GetHalfX());
    printf("halfy = %g mm\n", fPSPosGen->GetHalfY());
  }
  else if (shape == "Circle") {
    printf("radius = %g mm\n", fPSPosGen->GetRadius());
  }
  else if (shape == "Annulus") {
    printf("radius = %g mm\n", fPSPosGen->GetRadius());
    printf("inner_radius = %g mm\n", fPSPosGen->GetRadius0());
  }
  else {
    G4String msg = "SLArGPSVertexGenerator::PrintConfigPlaneSource: ";
    msg += ("unknown shape " + shape + "\n");
    throw std::invalid_argument(msg);
  }
  return;
}

void SLArGPSVertexGenerator::PrintConfigVolumeSource() const {
  G4String shape = fPSPosGen->GetPosDisShape();
  printf("shape set to %s\n", shape.data()); 
  G4ThreeVector center = fPSPosGen->GetCentreCoords();
  printf("center set to %g, %g, %g mm\n", 
      center.x(), center.y(), center.z()); 
  if (shape == "Sphere") {
    printf("radius = %g mm\n", fPSPosGen->GetRadius());
  }
  else if (shape == "Cylinder") {
    printf("radius = %g mm\n", fPSPosGen->GetRadius());
    printf("halfz = %g mm\n", fPSPosGen->GetHalfZ());
  }
  else if (shape == "Ellipsoid" || shape == "Right") {
    printf("halfx = %g mm\n", fPSPosGen->GetHalfX());
    printf("halfy = %g mm\n", fPSPosGen->GetHalfY());
    printf("halfz = %g mm\n", fPSPosGen->GetHalfZ());
  }
  else {
    G4String msg = "SLArGPSVertexGenerator::PrintConfigVolumeSource: ";
    msg += ("unknown shape " + shape + "\n");
    throw std::invalid_argument(msg);
  }
  return;
}
}

}
