/**
 * @file    SLArEveGeometry.cc
 * @brief   Implementation of SLArEveGeometry.
 */

#include "SLArEveGeometry.hh"

#include "geo/SLArUnit.hpp"
#include "core/SLArDebugUtils.hh"

#include "Math/EulerAngles.h"
#include "TGeoBBox.h"
#include "TGeoTube.h"
#include "TGeoManager.h"
#include "TEveManager.h"

#include <stdexcept>

namespace display {

  // ─────────────────────────────────────────────────────────────────────────────
  // Public
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveGeometry::Configure(const rapidjson::Value& config)
  {
    if (config.HasMember("LArTarget") && config["LArTarget"].IsObject()) {
      try {
        ConfigureLArTarget(config["LArTarget"]);
      } catch (const std::exception& e) {
        printf("SLArEveGeometry: error configuring LAr target: %s\n", e.what());
        exit(EXIT_FAILURE);
      }
    }

    debug::require_json_member(config, "TPC");

    const auto& jtpc = config["TPC"];
    if (jtpc.IsObject()) {
      try { ConfigureTPC(jtpc); }
      catch (const std::exception& e) {
        printf("SLArEveGeometry: error configuring TPC: %s\n", e.what());
        exit(EXIT_FAILURE);
      }
    } else if (jtpc.IsArray()) {
      for (const auto& jtpc_entry : jtpc.GetArray()) {
        try { ConfigureTPC(jtpc_entry); }
        catch (const std::exception& e) {
          printf("SLArEveGeometry: error configuring TPC: %s\n", e.what());
          exit(EXIT_FAILURE);
        }
      }
    }
  }

  int SLArEveGeometry::GetTPCIndex(const int tpc_id) const
  {
    int idx = 0;
    for (const auto& tpc : fTPCs) {
      if (tpc.fID == tpc_id) return idx;
      ++idx;
    }
    return -1;
  }

  // ─────────────────────────────────────────────────────────────────────────────
  // Private builders
  // ─────────────────────────────────────────────────────────────────────────────

  void SLArEveGeometry::ConfigureLArTarget(const rapidjson::Value& jlar)
  {
    debug::require_json_member(jlar, "shape");
    debug::require_json_member(jlar, "dimensions");
    debug::require_json_member(jlar, "position");
    debug::require_json_member(jlar, "rot");

    fLArTarget.fShape = string_to_vol_shape(jlar["shape"].GetString());

    // ── Position ─────────────────────────────────────────────────────────────
    const auto& jpos = jlar["position"].GetObj();
    const double pos_unit =
      jpos.HasMember("unit") ? unit::Unit2Val(jpos["unit"]) : 1.0;
    fLArTarget.fPosition.SetX(jpos["xyz"].GetArray()[0].GetDouble() * pos_unit);
    fLArTarget.fPosition.SetY(jpos["xyz"].GetArray()[1].GetDouble() * pos_unit);
    fLArTarget.fPosition.SetZ(jpos["xyz"].GetArray()[2].GetDouble() * pos_unit);

    // ── Dimensions ───────────────────────────────────────────────────────────
    const auto& jdims = jlar["dimensions"];
    debug::require_json_type(jdims, rapidjson::kArrayType);

    if (fLArTarget.fShape == EVolShape::kBox) {
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "size_x");
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "size_y");
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "size_z");
      for (const auto& jdim : jdims.GetArray()) {
        const double d = unit::ParseJsonVal(jdim);
        const TString name = jdim["name"].GetString();
        if      (name == "size_x") fLArTarget.fDimension.SetX(d);
        else if (name == "size_y") fLArTarget.fDimension.SetY(d);
        else if (name == "size_z") fLArTarget.fDimension.SetZ(d);
      }
    } else if (fLArTarget.fShape == EVolShape::kTub) {
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "radius");
      debug::require_json_object_in_array(jdims, "name", rapidjson::kStringType, "length");
      for (const auto& jdim : jdims.GetArray()) {
        const double d = unit::ParseJsonVal(jdim);
        const TString name = jdim["name"].GetString();
        if      (name == "radius") fLArTarget.fRadius = d;
        else if (name == "length") fLArTarget.fHeight = d;
      }
    }

    // ── Rotation ─────────────────────────────────────────────────────────────
    const auto& jrot = jlar["rot"].GetObj();
    debug::require_json_member(jrot, "val");
    debug::require_json_type(jrot["val"], rapidjson::kArrayType);
    const double rot_unit =
      jrot.HasMember("unit") ? unit::Unit2Val(jrot["unit"]) : 1.0;
    const auto euler_arr = jrot["val"].GetArray();
    fLArTarget.fRotation = ROOT::Math::EulerAngles(
        euler_arr[0].GetDouble() * rot_unit,
        euler_arr[1].GetDouble() * rot_unit,
        euler_arr[2].GetDouble() * rot_unit);

    // ── TGeo shape ───────────────────────────────────────────────────────────
    TGeoShape* shape = nullptr;
    if (fLArTarget.fShape == EVolShape::kBox) {
      shape = new TGeoBBox("lar_box",
          0.5 * fLArTarget.fDimension.x(),
          0.5 * fLArTarget.fDimension.y(),
          0.5 * fLArTarget.fDimension.z());
    } else {
      shape = new TGeoTube("lar_cyl", 0., fLArTarget.fRadius, 0.5 * fLArTarget.fHeight);
    }

    auto* rot = new TGeoRotation();
    rot->SetAngles(
        fLArTarget.fRotation.Phi()   * TMath::RadToDeg(),
        fLArTarget.fRotation.Theta() * TMath::RadToDeg(),
        fLArTarget.fRotation.Psi()   * TMath::RadToDeg());

    fLArTarget.fTransform = new TGeoCombiTrans(
        fLArTarget.fPosition.x(),
        fLArTarget.fPosition.y(),
        fLArTarget.fPosition.z(), rot);

    // ── Eve volume ───────────────────────────────────────────────────────────
    fLArTarget.fVolume = std::make_unique<TEveGeoShape>("LArTarget");
    fLArTarget.fVolume->SetShape(shape);
    fLArTarget.fVolume->SetMainColor(kGray + 1);
    fLArTarget.fVolume->SetTransMatrix(*fLArTarget.fTransform);
    fLArTarget.fVolume->SetPickable(kTRUE);
    fLArTarget.fVolume->SetDrawFrame(kTRUE);
    fLArTarget.fVolume->SetMainTransparency(80);
    gEve->AddElement(fLArTarget.fVolume.get());

    // ── Scene bounds (generous 70 % padding) ─────────────────────────────────
    const double px = fLArTarget.fPosition.x();
    const double py = fLArTarget.fPosition.y();
    const double pz = fLArTarget.fPosition.z();
    const double dx = 0.7 * fLArTarget.fDimension.x();
    const double dy = 0.7 * fLArTarget.fDimension.y();
    const double dz = 0.7 * fLArTarget.fDimension.z();
    fXmin = static_cast<float>(px - dx);  fXmax = static_cast<float>(px + dx);
    fYmin = static_cast<float>(py - dy);  fYmax = static_cast<float>(py + dy);
    fZmin = static_cast<float>(pz - dz);  fZmax = static_cast<float>(pz + dz);
  }

  void SLArEveGeometry::MakeTPCBox(const rapidjson::Value& jcfg, GeoTPC_t& tpc)
  {
    for (const auto& jdim : jcfg["dimensions"].GetArray()) {
      const TString name = jdim["name"].GetString();
      const double  val  = unit::ParseJsonVal(jdim);
      if      (name == "tpc_x") tpc.fDimension.SetX(val);
      else if (name == "tpc_y") tpc.fDimension.SetY(val);
      else if (name == "tpc_z") tpc.fDimension.SetZ(val);
    }
    tpc.fVolume = std::make_unique<TEveGeoShape>(Form("TPC%i", tpc.fID));
    tpc.fVolume->SetShape(new TGeoBBox(
          0.5 * tpc.fDimension.x(),
          0.5 * tpc.fDimension.y(),
          0.5 * tpc.fDimension.z()));
  }

  void SLArEveGeometry::MakeTPCTub(const rapidjson::Value& jcfg, GeoTPC_t& tpc)
  {
    for (const auto& jdim : jcfg["dimensions"].GetArray()) {
      const TString name = jdim["name"].GetString();
      const double  val  = unit::ParseJsonVal(jdim);
      if      (name == "tpc_radius") tpc.fRadius = val;
      else if (name == "tpc_height") tpc.fHeight = val;
    }
    tpc.fVolume = std::make_unique<TEveGeoShape>(Form("TPC%i", tpc.fID));
    tpc.fVolume->SetShape(new TGeoTube(0., tpc.fRadius, 0.5 * tpc.fHeight));
  }

  void SLArEveGeometry::ConfigureTPC(const rapidjson::Value& jcfg)
  {
    debug::require_json_member(jcfg, "copyID");
    debug::require_json_member(jcfg, "position");
    debug::require_json_member(jcfg, "dimensions");
    debug::require_json_type(jcfg["dimensions"], rapidjson::kArrayType);

    GeoTPC_t tpc;
    if (jcfg.HasMember("shape"))
      tpc.fShape = string_to_vol_shape(jcfg["shape"].GetString());

    tpc.fID = jcfg["copyID"].GetInt();

    switch (tpc.fShape) {
      case EVolShape::kBox: MakeTPCBox(jcfg, tpc); break;
      case EVolShape::kTub: MakeTPCTub(jcfg, tpc); break;
      default:
                            throw std::invalid_argument(
                                Form("Unknown TPC shape: %s", jcfg["shape"].GetString()));
    }

    const auto& jpos      = jcfg["position"].GetObj();
    const double pos_unit = unit::Unit2Val(jpos["unit"]);
    tpc.fPosition.SetX(jpos["xyz"].GetArray()[0].GetDouble() * pos_unit);
    tpc.fPosition.SetY(jpos["xyz"].GetArray()[1].GetDouble() * pos_unit);
    tpc.fPosition.SetZ(jpos["xyz"].GetArray()[2].GetDouble() * pos_unit);

    tpc.fVolume->SetMainColor(kGray + 2);
    tpc.fVolume->SetMainTransparency(90);

    // TPC placement is relative to the LAr target; inherit its rotation.
    tpc.fTransform = new TGeoCombiTrans(
        tpc.fPosition.x(), tpc.fPosition.y(), tpc.fPosition.z(),
        fLArTarget.fTransform->GetRotation());

    tpc.fVolume->SetTransMatrix(*tpc.fTransform);
    fLArTarget.fVolume->AddElement(tpc.fVolume.get());

    fTPCs.push_back(std::move(tpc));
  }

} // namespace display
