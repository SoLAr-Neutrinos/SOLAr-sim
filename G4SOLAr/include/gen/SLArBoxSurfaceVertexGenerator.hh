/**
 * @author      Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        SLArBoxSurfaceVertexGenerator.hh
 * @created     Tue Apr 11, 2023 09:58:25 CEST
 */

#ifndef SLARBOXSURFACEVERTEXGENERATOR_HH

#define SLARBOXSURFACEVERTEXGENERATOR_HH

#include <random>

#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "SLArGeoUtils.hh"
#include "gen/SLArVertexGenerator.hh"

namespace gen {
namespace vertex {

/**
 * @class SLArBoxSurfaceVertexGenerator
 * @brief Generates vertexes on the surface of a box volume
 *
 * This class generates vertexes on the surface of a box volume as defined by a G4VPhysicalVolume.
 *
 * The generator is configured using a JSON object, which should contain the following fields:
 * - `volume`: the name of the physical volume on which surface the vertexes are generated
 * - `origin_face`: the face of the box where the vertexes are generated. The face is specified
 *   using the `geo::EBoxFace` enum, which uses the following convention: 
 *   {kXplus=0, kXminus=1, kYplus=2, kYminus=3, kZplus=4, kZminus=5}.
 * - `time`: the time generator configuration. This is passed to the `SLArTimeGenerator` class.
 */
class SLArBoxSurfaceVertexGenerator : public SLArVertexGenerator
{
  public: 
    SLArBoxSurfaceVertexGenerator(); 
    SLArBoxSurfaceVertexGenerator(const SLArBoxSurfaceVertexGenerator&); 
    ~SLArBoxSurfaceVertexGenerator() override; 

    G4String GetType() const override {return "boxface_vertex_generator";}

    void FixVertexFace(const bool isFaceFixed) {fFixFace = isFaceFixed;}
    void SetVertexFace(const geo::EBoxFace face) {fVtxFace = face;}
    geo::EBoxFace GetVertexFace() const {return fVtxFace;}
    const G4LogicalVolume* GetBoxLogicalVolume() const; 
    void SetBoxLogicalVolume(const G4LogicalVolume*); 
    const G4VSolid* GetSolid() const; 
    const G4ThreeVector & GetSolidTranslation() const;
    void SetSolidTranslation(const G4ThreeVector &);
    const G4RotationMatrix & GetSolidRotation() const;
    const G4RotationMatrix & GetSolidInverseRotation() const;
    void SetSolidRotation(G4RotationMatrix* );
    double GetTolerance() const;
    void SetTolerance(double tolerance_);
    void SetRandomSeed(unsigned int seed_);
    void SetNoDaughters(bool no_daughters_);


    /**
     * @brief Get the surface area used for vertex generation
     * @return The surface area of the box volume in mm^2
     */
    G4double GetSurfaceGenerator() const; 

    void ShootVertex(G4ThreeVector & vertex_) override;
    void Config(const rapidjson::Value& config) override;
    void Config(const G4String& vol_face_name); 
    void Print() const override {
      printf("SLArBoxSurfaceVertexGenerator info dump:\n"); 
      printf("logical (solid) volume name: %s (%s)\n",
          fLogVol->GetName().data(), fSolid->GetName().data()); 
      printf("fixed face: %i - face number: %i\n", fFixFace, fVtxFace); 
      printf("box surface: %g mm2\n\n", GetSurfaceGenerator()); 
      return;
    }
    const rapidjson::Document ExportConfig() const override;

  private:
    // Configuration:
    const G4LogicalVolume * fLogVol = nullptr; ///< Reference to the logical volume
    G4ThreeVector fBulkTranslation; ///< The box position in world coordinate frame
    G4RotationMatrix fBulkRotation; ///< The box rotation in world coordinate frame
    G4Transform3D fBulkTransform; ///< The box transformation in world coordinate frame
    double fTolerance{1.0 * CLHEP::um}; ///< Geometrical tolerance (length)
    unsigned int fRandomSeed{0}; ///< Seed for the random number generator
    bool fNoDaughters = false; ///< Flag to reject vertexes generated from daughter volumes
    bool fFixFace = false; ///
    geo::EBoxFace fVtxFace = geo::kXplus; 

    // Working internals:
    const G4VSolid * fSolid = nullptr; ///< Reference to the solid volume from which are generated vertexes
    G4RotationMatrix fBulkInverseRotation; ///< The inverse box rotation
    G4double fSurface = 0.0; 
    std::array <G4double, 6> fSurfaceFaces = {};
    unsigned int fCounter = 0.0; // Internal vertex counter
}; 
}
}
#endif /* end of include guard SLARBOXSURFACEVERTEXGENERATOR_HH */

