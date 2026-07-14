/**
 * @file    SLArEveGeometry.hh
 * @brief   Geometry description (LAr target + TPCs) constructed from a
 *          rapidJSON configuration block and exposed to the other display
 *          components via const accessors.
 *
 * Ownership model
 * ---------------
 *  - All TEveGeoShape objects are owned through std::unique_ptr held inside
 *    the GeoLArTarget_t / GeoTPC_t value types.
 *  - TGeoCombiTrans raw pointers inside those structs are owned by ROOT's
 *    TGeoManager and must NOT be deleted manually.
 *  - SLArEveGeometry itself is owned by SLArEveDisplay (by value or
 *    unique_ptr); it does not inherit from any ROOT framework class and
 *    carries no implicit ROOT ownership.
 */

#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Math/EulerAngles.h"
#include "Math/Vector3D.h"
#include "TEveGeoShape.h"
#include "TGeoMatrix.h"
#include "rapidjson/document.h"

namespace display {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers shared between geometry and renderers
// ─────────────────────────────────────────────────────────────────────────────

enum class EVolShape { kBox, kTub };

inline EVolShape string_to_vol_shape(const std::string& s)
{
    if (s == "box") return EVolShape::kBox;
    if (s == "tub") return EVolShape::kTub;
    throw std::invalid_argument("Unknown volume shape: " + s);
}

inline TString vol_shape_to_string(const EVolShape shape)
{
    switch (shape) {
        case EVolShape::kBox: return "box";
        case EVolShape::kTub: return "tub";
        default:              return "unknown";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Plain geometry descriptors
// ─────────────────────────────────────────────────────────────────────────────

/** Geometry and Eve representation of a single TPC drift volume. */
struct GeoTPC_t {
    std::unique_ptr<TEveGeoShape>  fVolume    = {};
    ROOT::Math::XYZVectorD         fPosition  = {};  ///< centre in world frame [mm]
    ROOT::Math::XYZVectorD         fDimension = {};  ///< full extents [mm] (box)
    EVolShape                      fShape     = EVolShape::kBox;
    Double_t                       fRadius    = 0.;  ///< cylinder only
    Double_t                       fHeight    = 0.;  ///< cylinder only
    Int_t                          fID        = {};
    TGeoCombiTrans*                fTransform = nullptr; ///< non-owning; ROOT-managed
};

/** Geometry and Eve representation of the active LAr volume. */
struct GeoLArTarget_t {
    std::unique_ptr<TEveGeoShape>  fVolume    = {};
    ROOT::Math::XYZVectorD         fPosition  = {};
    ROOT::Math::XYZVectorD         fDimension = {};
    ROOT::Math::EulerAngles        fRotation  = {};
    EVolShape                      fShape     = EVolShape::kBox;
    double                         fRadius    = 0.;
    double                         fHeight    = 0.;
    TGeoCombiTrans*                fTransform = nullptr; ///< non-owning; ROOT-managed
};

// ─────────────────────────────────────────────────────────────────────────────
// SLArEveGeometry
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @class SLArEveGeometry
 * @brief Constructs and owns the Eve geometry hierarchy (LAr target + TPCs)
 *        from a rapidJSON configuration.
 *
 * Call order:
 *   1. Configure(config)      — builds the LAr target and all TPC sub-volumes.
 *   2. Accessors              — hand const refs to renderer classes.
 *
 * The Eve scene graph is wired internally: each TPC volume is added as a child
 * of the LAr target volume, which is in turn added to the global Eve scene.
 */
class SLArEveGeometry {
public:
    SLArEveGeometry()  = default;
    ~SLArEveGeometry() = default;

    // Non-copyable; moveable so it can be stored in containers if needed.
    SLArEveGeometry(const SLArEveGeometry&)            = delete;
    SLArEveGeometry& operator=(const SLArEveGeometry&) = delete;
    SLArEveGeometry(SLArEveGeometry&&)                 = default;
    SLArEveGeometry& operator=(SLArEveGeometry&&)      = default;

    // ── Configuration ────────────────────────────────────────────────────────

    /**
     * Build the full geometry from a JSON object that must contain
     * keys "LArTarget" (optional) and "TPC" (required, object or array).
     */
    void Configure(const rapidjson::Value& config);

    // ── Accessors ────────────────────────────────────────────────────────────

    const GeoLArTarget_t&        GetLArTarget()  const { return fLArTarget; }
    const std::vector<GeoTPC_t>& GetTPCs()       const { return fTPCs; }

    /** Returns the index into fTPCs for the given TPC copy ID, or -1. */
    int  GetTPCIndex(const int tpc_id) const;

    /** Axis-aligned bounding box of the LAr target (70 % inflated). */
    float XMin() const { return fXmin; }
    float XMax() const { return fXmax; }
    float YMin() const { return fYmin; }
    float YMax() const { return fYmax; }
    float ZMin() const { return fZmin; }
    float ZMax() const { return fZmax; }

private:
    // ── Internal builders ────────────────────────────────────────────────────

    void ConfigureLArTarget(const rapidjson::Value& lar_config);
    void ConfigureTPC      (const rapidjson::Value& tpc_config);
    void MakeTPCBox        (const rapidjson::Value& tpc_config, GeoTPC_t& geo_tpc);
    void MakeTPCTub        (const rapidjson::Value& tpc_config, GeoTPC_t& geo_tpc);

    // ── Data members ─────────────────────────────────────────────────────────

    GeoLArTarget_t         fLArTarget;
    std::vector<GeoTPC_t>  fTPCs;

    float fXmin = 0.f, fXmax = 0.f;
    float fYmin = 0.f, fYmax = 0.f;
    float fZmin = 0.f, fZmax = 0.f;
};

} // namespace display
