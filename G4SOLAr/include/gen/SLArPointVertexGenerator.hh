/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : SLArPointVertexGenerator.hh
 * @created     : Monday Jun 23, 2025 14:55:56 CEST
 */

#ifndef SLARPOINTVERTEXGENERATOR_HH

#define SLARPOINTVERTEXGENERATOR_HH

#include "gen/SLArVertexGenerator.hh"

namespace gen {
namespace vertex {
  /**
   * @class SLArPointVertexGenerator
   * @brief Point vertex generator
   *
   * This class implements a point vertex generator. The vertex is set
   * to a fixed position in the 3D space. The position can be configured
   * using the `Config` method, which takes a JSON object as input.
   *
   * The JSON object should contain the following fields:
   * - `xyz`: the position of the vertex, given as a JSON object with
   *   `val` field containing an array of three values (x, y, z) and
   *   an optional `unit` field for the unit of the values.
   * - `volume` (optional): the name of the reference volume for the vertex position.
   * - `time` (optional): the time generator configuration, passed to the
   *
   */
  class SLArPointVertexGenerator : public SLArVertexGenerator {
    public: 
      inline SLArPointVertexGenerator() : fVertex(0.0, 0.0, 0.0) {}

      inline SLArPointVertexGenerator(const G4ThreeVector& v) {
        fVertex.set( v.x(), v.y(), v.z() ); 
      }

      inline ~SLArPointVertexGenerator() {}

      inline G4String GetType() const override {return "point_vertex_generator";}

      inline G4ThreeVector GetVertex() const {return fVertex;}

      void ShootVertex(G4ThreeVector& vertex) override; 

      /**
       * @brief Configure the vertex generator
       *
       * @param config The configuration object
       *
       * This method configures the vertex generator based on the provided
       * configuration object. The configuration object is expected to be
       * in JSON format and should contain the following fields:
       * - `xyz`: the position of the vertex, given as a JSON object with 
       *   `val` field containing an array of three values (x, y, z) and 
       *   an optional `unit` field for the unit of the values.
       * - `volume` (optional): the name of the reference volume for the vertex position.
       * - `time` (optional): the time generator configuration, passed to the
       *   `SLArTimeGenerator::SourceConfiguration` method.
       */
      void Config(const rapidjson::Value& config) override; 

      void Print() const override {
        printf("SLArPointVertexGenerator configuration dump:\n"); 
        printf("vertex set to %g, %g, %g mm\n\n", 
            fVertex.x(), fVertex.y(), fVertex.z()); 
        if (fReferenceVolumeName.size() > 0) {
          printf("Reference volume: %s\n", fReferenceVolumeName.data());
        }
        return;
      }

    const rapidjson::Document ExportConfig() const override;

    private: 
      G4ThreeVector fVertex; //!< The vertex position in the 3D space   
      std::vector<G4ThreeVector> fVertexList = {}; //!< List of vertices (if needed)
      UInt_t fNrOfEvPerVertex = 1; //!< Number of events per vertex (if needed)
      G4String fReferenceVolumeName = {}; //<!< Name of the reference volume for the vertex position
      G4Transform3D fTransform = {}; //!< Transform to the global frame of the reference volume

  };
}
}
#endif /* end of include guard SLARPOINTVERTEXGENERATOR_HH */

