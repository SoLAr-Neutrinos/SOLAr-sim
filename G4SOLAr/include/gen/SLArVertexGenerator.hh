/**
 * @author      : Daniele Guffanti (University and INFN Milano-Bicocca)
 * @file        : SLArVertexGenerator.hh
 * @created     : Friday Mar 29, 2024 14:58:43 CET
 * @brief       : Vertex generator interface class
 */

#ifndef SLARVERTEXGENERATOR_HH

#define SLARVERTEXGENERATOR_HH

#include <cstdio>
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RandomTools.hh"
#include "rapidjson/document.h"
#include "SLArGeoUtils.hh"
#include "SLArUnit.hpp"

namespace gen {
  namespace time {
    enum ETimeGeneratorMode {kFixed = 0, kWindow = 1}; 
    static const std::map<G4String, ETimeGeneratorMode> timeGenMap = {
      {"fixed", ETimeGeneratorMode::kFixed}, 
      {"window", ETimeGeneratorMode::kWindow}
    };

   /**
    * @brief Get the time generator mode from its label string
    *
    * @param str The string representing the time generator mode (e.g., "fixed", "window")
    * @return The corresponding `ETimeGeneratorMode` value
    *
    * This function checks if the provided string matches any of the known
    * time generator modes and returns the corresponding enum value.
    */ 
    static inline ETimeGeneratorMode GetTimeGeneratorMode(const G4String& str) {
      ETimeGeneratorMode kGen = ETimeGeneratorMode::kFixed;
      if (timeGenMap.find(str) != timeGenMap.end()) {
        kGen = timeGenMap.find(str)->second;
      }
      return kGen;
    }

    /**
     * @brief Get the string label of a time generator given its mode enumerator
     *
     * @param kMode The time generator mode enumerator (e.g., kFixed, kWindow)
     * @return The corresponding string label (e.g., "fixed", "window")
     */
    static inline G4String  GetTimeGeneratorModeString(const ETimeGeneratorMode kMode) {
      if (kMode == ETimeGeneratorMode::kFixed) {return G4String("fixed");}
      else if (kMode == ETimeGeneratorMode::kWindow) {return G4String("window"); }
      else {
        fprintf(stderr, "WARNING: Unkown time generator mode %i\n", kMode); 
        return G4String("");
      }
    }

    /**
     * @class SLArTimeGenerator
     * @brief Class for generating vertex times for Geant4 primary particles
     *
     * This class is used to generate the time of the primary particles in 
     * the SoLAr simulation framework. It provides methods to configure the time
     * generator, sample times, and export the configuration in JSON format.
     *
     * The configuration is done using a JSON object, which is passed to the
     * SourceConfiguration method. The arguments are:
     * - `mode`: the mode of the time generator (`fixed` or `window`)
     * - `event_time`: the fixed time value (in ns) for the `fixed` mode
     * - `event_window_limits`: the time window limits for the `window` mode. 
     *   This field must contain two subfields: `t0` and `t1`, which define the
     *   start and end of the time window. `t0` and `t1` are JSON 
     *   objects with the fields `val` and `unit`, where `val` is the time value
     *   and `unit` is the unit of the time value (default is ns).
     */
    class SLArTimeGenerator {
      public: 
        /**
         * @struct TimeGenConfig_t
         * @brief Configuration structure for the time generator
         *  
         * This structure contains the configuration parameters for the time generator.
         */
        struct TimeGenConfig_t {
          ETimeGeneratorMode mode = ETimeGeneratorMode::kFixed; //!< Time generator mode
          G4double time = 0.0; //!< Time value (in ns) for the fixed mode
          G4double time_min = 0.0; //!< Start of the time window (in ns) for the window mode
          G4double time_max = 0.0; //!< End of the time window (in ns) for the window mode
        };

        inline SLArTimeGenerator() {};
        inline ~SLArTimeGenerator() {};
        inline TimeGenConfig_t& GetTimeConfig() {return fConfig;}
        inline const TimeGenConfig_t& GetTimeConfig() const {return fConfig;}

        /**
         * @brief Configure the time generator
         *
         * @param jconfig The JSON configuration object for the time generator
         *
         * This method configures the time generator based on the provided
         * configuration. The "mode" of the generator is mandatory.
         * Available modes are:
         * - "fixed" for a fixed time value
         * - "window" for a time window
         * 
         * The "event_time" field is used for the `fixed` mode, and it must
         * be a JSON object with the fields `val` and `unit`, where `val` is
         * the time value and `unit` is the unit of the time value (default is ns).
         * 
         * The "event_window_limits" field is used for the `window` mode, and it
         * must be a JSON object with two subfields: `t0` and `t1`, which define
         * the start and end of the time window. `t0` and `t1` are JSON
         * objects with the fields `val` and `unit`, where `val` is the time value
         * and `unit` is the unit of the time value (default is ns).
         */
        void SourceConfiguration(const rapidjson::Value& jconfig) {
          assert(jconfig.HasMember("mode")); 
          fConfig.mode = GetTimeGeneratorMode( jconfig["mode"].GetString() ); 
          if (jconfig.HasMember("event_time")) {
            fConfig.time = unit::ParseJsonVal( jconfig["event_time"] ); 
          }
          else if (jconfig.HasMember("event_window_limits")) {
            const auto& jwindow = jconfig["event_window_limits"];
            fConfig.time_min = unit::ParseJsonVal( jwindow["t0"] );
            fConfig.time_max = unit::ParseJsonVal( jwindow["t1"] ); 
            //G4cout << "Time min: " << fConfig.time_min << G4endl;
            //G4cout << "Time max: " << fConfig.time_max << G4endl;
            //getchar();
          }
        }
        
        /**
         * @brief Calculate the time interval associated to the time generator
         */
        G4double CalculateTotalTime() const {
          if (fConfig.mode == ETimeGeneratorMode::kFixed) {
            return 0.0;
          }
          else {
            return fConfig.time_max - fConfig.time_min;
          }
        }

        /**
         * @brief Sample a time value
         *
         * @return The sampled time value (in ns)
         *
         * This method samples a time value based on the configured mode.
         */
        inline G4double SampleTime() {
          switch (fConfig.mode) {
            case time::kFixed : {
              return fConfig.time;
              break;
            }
            case time::kWindow : {
              fConfig.time = CLHEP::RandFlat::shoot(fConfig.time_min, fConfig.time_max);
              return fConfig.time;
              break;
            }
            default : {
              fprintf(stderr, "SLArTimeGenerator::SampleTime() WARNING. Undefined vertex time sampling. Setting t = 0 ns\n"); 
              return 0.0; 
            }
          }
          return fConfig.time;
        }

        /**
         * @brief Export the configuration in JSON format
         *
         * @return The JSON document containing the configuration
         *
         * Exports the configuration of the time generator in JSON format for 
         * storing the configuration in the output file.
         */
        const rapidjson::Document ExportConfig() const {
          rapidjson::Document d; 
          d.SetObject(); 
          std::string mode_str = GetTimeGeneratorModeString(fConfig.mode).data();
          char buffer[50]; 
          int len = snprintf(buffer, sizeof(buffer), "%s", mode_str.data());
          rapidjson::Value jmode;
          jmode.SetString(buffer, len, d.GetAllocator());
          d.AddMember("mode", jmode, d.GetAllocator());
          if (fConfig.mode == ETimeGeneratorMode::kFixed) {
            d.AddMember("event_time_ns", fConfig.time, d.GetAllocator());
          }
          else if (fConfig.mode == ETimeGeneratorMode::kWindow) {
            rapidjson::Value jwindow; jwindow.SetArray();
            jwindow.PushBack( fConfig.time_min, d.GetAllocator()); 
            jwindow.PushBack( fConfig.time_max, d.GetAllocator()); 
            d.AddMember("event_window_ns", jwindow, d.GetAllocator());
          }
          return d;
        }

      private: 
        TimeGenConfig_t fConfig;
    };
  }

  namespace vertex {

  enum EVertexGenerator {
    kUndefinedVtxGen = -1, 
    kPoint = 0, 
    kBulk = 1, 
    kBoxVolSurface = 2, 
    kGPSPos = 3};

  static const std::map<G4String, EVertexGenerator> vtxGenMap = {
    {"point", EVertexGenerator::kPoint}, 
    {"bulk", EVertexGenerator::kBulk}, 
    {"boxsurface", EVertexGenerator::kBoxVolSurface}, 
    {"gps_pos", EVertexGenerator::kGPSPos}
  };

  /**
   * @brief Get the vertex generator type from its label string
   *
   * @param str The string representing the vertex generator type (e.g., "point", "gps_pos")
   * @return The corresponding `EVertexGenerator` value
   *
   * This function checks if the provided string matches any of the known
   * vertex generator types and returns the corresponding enum value.
   */
  static inline EVertexGenerator getVtxGenIndex(G4String str) {
    EVertexGenerator kGen = EVertexGenerator::kUndefinedVtxGen;
    if (vtxGenMap.find(str) != vtxGenMap.end()) {
      kGen = vtxGenMap.find(str)->second;
    }
    return kGen;
  }

  /**
   * @brief Get the string label of a vertex generator given its type enumerator
   *
   * @param kGen The vertex generator type enumerator (e.g., kPoint, kGPSPos)
   * @return The corresponding string label (e.g., "point", "gps_pos")
   */
  static inline void printVtxGeneratorType() {
    printf("Available vertex generators:\n");
    for (const auto& vgen : vtxGenMap) {
      printf("\t-[%i] %s\n", getVtxGenIndex(vgen.first), vgen.first.data());
    }
    return;
  }

  /**
   * @class SLArVertexGenerator
   * @brief Base class for vertex generators
   * 
   * This class provides an interface for generating vertices in the SoLAr
   * simulation framework. It defines the basic methods that all vertex
   * generators must implement, such as `ShootVertex`, `Config`, and
   * `ExportConfig`.
   */
  class SLArVertexGenerator {
    public:
      SLArVertexGenerator() = default;

      virtual ~SLArVertexGenerator() = default;

      /**
       * Check if the generator can provide at least one more vertex (default: return true)
       */
      inline virtual bool HasNextVertex() const {return true;}
      /**
       * Get the type of the vertex generator
       */
      virtual G4String GetType() const = 0; 
      /**
       * @brief Generate a vertex
       * @param vertex_ The generated vertex
       */
      virtual void ShootVertex(G4ThreeVector&  vertex_) = 0;

      /**
       * @brief Configure the vertex generator
       */
      virtual void Config(const rapidjson::Value&) = 0;
      /**
       * @brief Export the configuration in JSON format
       * @return The JSON document containing the configuration
       */
      virtual const rapidjson::Document ExportConfig() const = 0;  
      /**
       * @brief Print the configuration to the standard output
       *
       * This method prints the configuration of the vertex generator to the standard output.
       */
      virtual void Print() const = 0;
      /**
       * @brief Get the vertex time generator
       * @return The time generator object
       */
      inline time::SLArTimeGenerator& GetTimeGenerator() {return fTimeGen;}
      /**
       * @brief Get the vertex time generator (const version)
       * @return The time generator object
       */
      inline const time::SLArTimeGenerator& GetTimeGenerator() const {return fTimeGen;}

      time::SLArTimeGenerator fTimeGen = {}; 

      protected:
      inline void FillConfigVector(const G4ThreeVector& vec, 
          rapidjson::Value& jsonArray, 
          rapidjson::Document::AllocatorType& allocator) const
      {
        jsonArray.SetArray();
        jsonArray.PushBack(vec.x(), allocator);
        jsonArray.PushBack(vec.y(), allocator);
        jsonArray.PushBack(vec.z(), allocator);
        return;
      }
  };
  }
}


#endif /* end of include guard SLARVERTEXTGENERATOR_HH */

