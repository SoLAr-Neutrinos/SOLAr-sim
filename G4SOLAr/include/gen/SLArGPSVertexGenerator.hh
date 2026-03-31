/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArGPSVertexGenerator.hh
 * @created     : Tuesday Dec 03, 2024 09:13:50 CET
 */

#ifndef SLARGPSVERTEXGENERATOR_HH

#define SLARGPSVERTEXGENERATOR_HH

#include "gen/SLArVertexGenerator.hh"
#include "G4SPSPosDistribution.hh"
#include "G4SPSRandomGenerator.hh"

namespace gen {
namespace vertex {

/**
 * @class SLArGPSVertexGenerator
 * @brief Interface to the G4SPSPosDistribution class
 *
 * This class is a wrapper around the G4SPSPosDistribution class, which is used to generate
 * random vertex positions in a Geant4 simulation. It provides methods to configure the
 * vertex generator, shoot vertices, and export the configuration in JSON format.
 *
 * The configuration is done using a JSON object, which is passed to the Config method.
 * The arguments are supposed to be in the same format used in GPS macro commands
 * and are: 
 * - `type`: the type of the source (`Point`, `Beam`, `Plane`, `Surface`, `Volume`)
 * - `shape`: only for `Surface` and `Volume` source type. 
 *   For "Surface" type, shapes `Circle`, `Annulus`, `Ellipse`, `Square`, `Rectangle` are available.
 *   For both `Surface` and `Volume` types, the shape can be set to 
 *   `Sphere`, `Ellipsoid`, `Cylinder`, `Para`
 * - `center`: the center of the source, given as array of three values with key "val". 
 *   The unit of the values can be set with the "unit" key (default is mm).
 * - `reference_volume` (optional): the name of the reference volume for the source placement.
 * - `halfx`, `halfy`, `radius`, `inner_radius`: the dimensions of the source, 
 *   depending on the type and shape. Values must be given in JSON object with key "val", 
 *   and optionally with key "unit" for the unit.
 * - `beam_sigma_r`, `beam_sigma_x`, `beam_sigma_y`: the beam size parameters 
 *   for the `Beam` source type. Values must be given in JSON object with
 *   key "val", and optionally with key "unit" for the unit.
 * - `rot1`, `rot2`: the rotation vectors for the source, given as arrays of three values.
 * - `alpha`: Used with a Parallelepiped. The angle [default 0 rad] α formed by 
 *   the y-axis and the plane joining the centre of the faces parallel to 
 *   the zx plane at y and +y.
 * - `theta`: Used with a Parallelepiped. Polar angle [default 0 rad] θ  
 *   of the line connecting the centre of the face at z to the centre of the face at +z.
 * - `phi`: Used with a Parallelepiped. The azimuth angle [default 0 rad] ϕ 
 *   of the line connecting the centre of the face at z with the centre of the face at +z. 
 * - `confine_to_volume`: the name of the physical volume to confine the source to.
 *
 */
class SLArGPSVertexGenerator : public SLArVertexGenerator
{
 public: 
    SLArGPSVertexGenerator(); 
    virtual ~SLArGPSVertexGenerator();

    void ShootVertex(G4ThreeVector& vertex) override;
    void Config(const rapidjson::Value& config) override;
    const rapidjson::Document ExportConfig() const override;
    G4String GetType() const override {return "gps_pos";}
    void Print() const override;
    G4double GetSurfaceGenerator() const;
    G4double GetVolumeGenerator() const;

  protected:
    std::unique_ptr<G4SPSPosDistribution> fPSPosGen = nullptr;
    std::unique_ptr<G4SPSRandomGenerator> fPSRandGen = nullptr;
    G4String fReferenceVolumeName = {};
    G4Transform3D fTransform = {};

    rapidjson::Document ExportConfigPointSource() const;
    rapidjson::Document ExportConfigPlaneSource() const;
    rapidjson::Document ExportConfigBeamSource() const;
    rapidjson::Document ExportConfigVolumeSource() const;
    void PrintConfigPointSource() const;
    void PrintConfigBeamSource() const;
    void PrintConfigPlaneSource() const;
    void PrintConfigVolumeSource() const;
    
    G4double _beam_sigma_r = 0.0;
    G4double _beam_sigma_x = 0.0;
    G4double _beam_sigma_y = 0.0;
};

}
}

#endif /* end of include guard SLARGPSVERTEXGENERATOR_HH */

