/**
 * @author      Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        SLArBulkVertexGenerator.hh
 * @created     Thur Jun 23, 2022 15:31:58 CEST
 */

#ifndef SLARBULKVERTEXGENERATOR_HH

#define SLARBULKVERTEXGENERATOR_HH

// Standard library
#include <random>

#include "gen/SLArVertexGenerator.hh"

#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

namespace gen {
namespace vertex {


/**
 * @class SLArBulkVertexGenerator
 * @brief Generates vertexes in a bulk volume
 *
 * This class generates vertexes in a bulk volume as defined by a G4VPhysicalVolume.
 *
 * The generator is configured using a JSON object, which should contain the following fields:
 * - `volume`: the name of the physical volume where the vertexes are generated
 * - `fiducial_fraction`: the fraction of the volume that is used for vertex generation. 
 *   (only works for box-shape volumes). Setting this value to 1.0 means that the whole 
 *   volume is used. If smaller than 1, the volume is shrinked by the same amount 
 *   in all directions until reaching the requested volume fraction.
 * - `avoid_daughters`: if set to true, the generator will avoid generating vertexes
 *   inside daughter volumes of the specified volume.
 * - `material`: the material of the volume. If set, the generator will only generate vertexes
 *   where the material matches the specified one.
 * - `time`: the time generator configuration. This is passed to the `SLArTimeGenerator` class.
 *
 */
class SLArBulkVertexGenerator: public SLArVertexGenerator
{
public:

  SLArBulkVertexGenerator();

  SLArBulkVertexGenerator(const SLArBulkVertexGenerator&);
  
  ~SLArBulkVertexGenerator() override;

  G4String GetType() const override {return "bulk_vertex_generator";}

  inline G4double GetBulkVolumeMass() const {return fMass;} ///< Get the mass of the bulk volume
  
  const G4LogicalVolume * GetBulkLogicalVolume() const;
  
  void SetBulkLogicalVolume(G4LogicalVolume *, int counter);
   
  const G4VSolid * GetSolid() const;
 
  const G4ThreeVector & GetSolidTranslation() const;
  
  void SetSolidTranslation(const G4ThreeVector &);
 
  const G4RotationMatrix & GetSolidRotation() const;
 
  const G4RotationMatrix & GetSolidInverseRotation() const;
  
  void SetSolidRotation(G4RotationMatrix* );
 
  double GetTolerance() const;
  
  void SetTolerance(double tolerance_);

  void SetRandomSeed(unsigned int seed_);

  void SetNoDaughters(bool no_daughters_);

  void SetFiducialFraction(double fvf) {fFVFraction = fvf;} 

  double GetFiducialFraction() {return fFVFraction;}

  void ShootVertex(G4ThreeVector & vertex_) override;
    
  void Config(const rapidjson::Value& cfg) override;

  void Config(const G4String& target_volume_name); 

  void Config(const G4String& target_volume_name, const G4String& mother_volume_name);

  inline G4double GetCubicVolumeGenerator() const {
    return fSolid->GetCubicVolume();
  }

    inline void Print() const override {
    printf("SLArBulkVertexGenerator info dump:\n"); 
    printf("logical (solid) volume name: %s (%s)\n",
        fLogVol->GetName().data(), fSolid->GetName().data()); 
    printf("Reject daughters: %i\nFiducial fraction: %g\n\n", 
        fNoDaughters, fFVFraction); 
    return;
  }

  const rapidjson::Document ExportConfig() const override;

private:
  // Configuration:
  G4LogicalVolume * fLogVol = nullptr; ///< Reference to the logical volume
  G4String fMotherVolumeName;
  G4String fTargetVolumeName;
  G4ThreeVector fBulkTranslation; ///< The box position in world coordinate frame
  G4RotationMatrix fBulkRotation; ///< The box rotation in world coordinate frame
  std::vector<G4Transform3D> fBulkTransformVec; ///< The vectors box transformation in world coordinate frame
  double fTolerance{1.0 * CLHEP::um}; ///< Geometrical tolerance (length)
  unsigned int fRandomSeed{0}; ///< Seed for the random number generator
  bool fNoDaughters = false; ///< Flag to reject vertexes generated from daughter volumes
  double fFVFraction{1.0}; //!< Volume fraction 
  G4bool fRequireMaterialMatch = false;
  G4String fMaterial = {};
  G4double fMass; //!< Parameter that sets up the volume mass
  
  // Working internals:
  G4VSolid * fSolid = nullptr; ///< Reference to the solid volume from which are generated vertexes
  G4RotationMatrix fBulkInverseRotation; ///< The inverse box rotation
  unsigned int fCounter = 0.0; // Internal vertex counter

  inline G4double GetMassVolumeGenerator() const {return fLogVol->GetMass();}

  double ComputeDeltaX(const G4ThreeVector& lo, const G4ThreeVector& hi) const;
  double ComputeDeltaX(const G4ThreeVector& lo, const G4ThreeVector& hi, const G4double fv) const;
     
};
}
}

#endif /* end of include guard SLARBULKVERTEXGENERATOR_HH */

