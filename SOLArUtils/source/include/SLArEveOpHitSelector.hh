/**
 * @file    SLArEveOpHitSelector.hh
 * @brief   Channel-level hit selectors for the optical hit renderer.
 *
 * A selector extracts a filtered {nhits, time_min} from a readout channel
 * (SLArEventSiPM or SLArEventSuperCell) by querying the backtracker records
 * stored in fBacktrackerCollections.
 *
 * The record index for each backtracker type is determined at display
 * configuration time by SLArEveEventReader::GetBacktrackerRecordIndex(),
 * and passed into the factory functions below.  A record_idx of -1 means
 * the backtracker is not available; every factory falls back to SelectAll.
 *
 * Backtracker record key conventions (from SLArBacktracker.cc):
 *   kOpticalProc record  → key = EPhProcess integer
 *   kWavelength  record  → key = wavelength bin index (bin width = kWvlBinWidth nm)
 */
#ifndef SLAR_EVE_OP_HIT_SELECTOR_HH
#define SLAR_EVE_OP_HIT_SELECTOR_HH

#include <functional>
#include <limits>

#include "event/SLArEventSiPM.hh"
#include "event/SLArEventSuperCell.hh"
#include "event/SLArEventPhotonHit.hh"   // EPhProcess

namespace display {

// ── Result type ──────────────────────────────────────────────────────────────

struct OpHitSelection {
    int nhits    = 0;
    int time_min = std::numeric_limits<int>::max();
};

// ── Selector type alias ───────────────────────────────────────────────────────

template<typename ChannelT>
using OpHitSelectorFn = std::function<OpHitSelection(const ChannelT&)>;

using SiPMSelectorFn      = OpHitSelectorFn<SLArEventSiPM>;
using OpDetSelectorFn = OpHitSelectorFn<SLArEventSuperCell>;

/// Wavelength bin width assumed when decoding the kWavelength backtracker key.
/// Must match the value used during simulation. Adjust if needed.
static constexpr float kWvlBinWidth = 10.f;  // nm per bin

// ── Shared implementation helpers ─────────────────────────────────────────────

namespace detail {

template<typename ChannelT>
inline OpHitSelection SelectAllImpl(const ChannelT& ch)
{
    const int nhits = ch.GetNhits();
    const int tmin  = (nhits > 0)
        ? ch.GetConstHits().begin()->first
        : std::numeric_limits<int>::max();
    return {nhits, tmin};
}

/// Sum hits from a set of process keys in backtracker record @p rec_idx.
template<typename ChannelT>
inline OpHitSelection SelectByProcessSetImpl(
    const ChannelT&                ch,
    const std::vector<EPhProcess>& procs,
    int                            rec_idx)
{
    const auto& bt_coll = ch.GetBacktrackerRecordCollection();
    if (bt_coll.empty() || rec_idx < 0)
        return SelectAllImpl(ch);

    int nhits = 0;
    int tmin  = std::numeric_limits<int>::max();
    for (const auto& [time_bin, bt_vec] : bt_coll) {
        const auto& records = bt_vec.GetConstRecords();
        if (rec_idx >= static_cast<int>(records.size())) continue;
        const auto& counter = records[rec_idx].GetConstCounter();
        for (EPhProcess proc : procs) {
            const auto it = counter.find(static_cast<int>(proc));
            if (it != counter.end() && it->second > 0) {
                nhits += it->second;
                if (time_bin < tmin) tmin = time_bin;
                // Don't break — multiple processes can contribute per time_bin
            }
        }
    }
    return {nhits, tmin};
}

/// Sum hits whose wavelength bin falls within [wvl_min, wvl_max] nm.
template<typename ChannelT>
inline OpHitSelection SelectByWavelengthRangeImpl(
    const ChannelT& ch,
    float           wvl_min,
    float           wvl_max,
    int             rec_idx)
{
    const auto& bt_coll = ch.GetBacktrackerRecordCollection();
    if (bt_coll.empty() || rec_idx < 0)
        return SelectAllImpl(ch);

    int nhits = 0;
    int tmin  = std::numeric_limits<int>::max();
    for (const auto& [time_bin, bt_vec] : bt_coll) {
        const auto& records = bt_vec.GetConstRecords();
        if (rec_idx >= static_cast<int>(records.size())) continue;
        for (const auto& [wvl_key, count] : records[rec_idx].GetConstCounter()) {
            const float wvl = wvl_key * kWvlBinWidth;
            if (wvl >= wvl_min && wvl <= wvl_max && count > 0) {
                nhits += count;
                if (time_bin < tmin) tmin = time_bin;
            }
        }
    }
    return {nhits, tmin};
}

/// Combined process + wavelength filter.
/// Hit is accepted if it passes both the process set AND the wavelength range.
/// NOTE: this requires two separate backtracker records; hits passing both
/// are counted via the intersection of their time-bin contributions.
/// If either record index is -1 that constraint is dropped (falls back to
/// the single available filter).
template<typename ChannelT>
inline OpHitSelection SelectCombinedImpl(
    const ChannelT&                ch,
    const std::vector<EPhProcess>& procs,
    int                            proc_rec_idx,
    float                          wvl_min,
    float                          wvl_max,
    int                            wvl_rec_idx)
{
    // Degrade gracefully when one record is missing
    if (proc_rec_idx < 0)
        return SelectByWavelengthRangeImpl(ch, wvl_min, wvl_max, wvl_rec_idx);
    if (wvl_rec_idx < 0)
        return SelectByProcessSetImpl(ch, procs, proc_rec_idx);

    const auto& bt_coll = ch.GetBacktrackerRecordCollection();
    if (bt_coll.empty())
        return SelectAllImpl(ch);

    // Strategy: a time bin contributes min(proc_count, wvl_count) hits,
    // since each photon is recorded once in each backtracker.
    int nhits = 0;
    int tmin  = std::numeric_limits<int>::max();

    for (const auto& [time_bin, bt_vec] : bt_coll) {
        const auto& records = bt_vec.GetConstRecords();
        const int n_rec = static_cast<int>(records.size());
        if (proc_rec_idx >= n_rec || wvl_rec_idx >= n_rec) continue;

        // Count from process record
        int proc_count = 0;
        for (EPhProcess proc : procs) {
            const auto it = records[proc_rec_idx].GetConstCounter()
                                .find(static_cast<int>(proc));
            if (it != records[proc_rec_idx].GetConstCounter().end())
                proc_count += it->second;
        }
        if (proc_count == 0) continue;

        // Count from wavelength record
        int wvl_count = 0;
        for (const auto& [wvl_key, count] :
                records[wvl_rec_idx].GetConstCounter()) {
            const float wvl = wvl_key * kWvlBinWidth;
            if (wvl >= wvl_min && wvl <= wvl_max)
                wvl_count += count;
        }
        if (wvl_count == 0) continue;

        // Conservative estimate: photons satisfying both constraints
        // cannot exceed the minimum of the two counts.
        nhits += std::min(proc_count, wvl_count);
        if (time_bin < tmin) tmin = time_bin;
    }
    return {nhits, tmin};
}

} // namespace detail

// ── Factory functions — SiPM ──────────────────────────────────────────────────

inline SiPMSelectorFn MakeSiPMSelectAll()
{
    return [](const SLArEventSiPM& ch) {
        return detail::SelectAllImpl(ch);
    };
}

inline SiPMSelectorFn MakeSiPMSelectByProcessSet(
    const std::vector<EPhProcess>& procs, int rec_idx)
{
    return [procs, rec_idx](const SLArEventSiPM& ch) {
        return detail::SelectByProcessSetImpl(ch, procs, rec_idx);
    };
}

inline SiPMSelectorFn MakeSiPMSelectByWavelengthRange(
    float wvl_min, float wvl_max, int rec_idx)
{
    return [wvl_min, wvl_max, rec_idx](const SLArEventSiPM& ch) {
        return detail::SelectByWavelengthRangeImpl(ch, wvl_min, wvl_max, rec_idx);
    };
}

inline SiPMSelectorFn MakeSiPMSelectCombined(
    const std::vector<EPhProcess>& procs, int proc_rec,
    float wvl_min, float wvl_max,         int wvl_rec)
{
    return [procs, proc_rec, wvl_min, wvl_max, wvl_rec](const SLArEventSiPM& ch) {
        return detail::SelectCombinedImpl(ch, procs, proc_rec,
                                          wvl_min, wvl_max, wvl_rec);
    };
}

// ── Factory functions — OpDets ─────────────────────────────────────────────

inline OpDetSelectorFn MakeOpDetSelectAll()
{
    return [](const SLArEventSuperCell& ch) {
        return detail::SelectAllImpl(ch);
    };
}

inline OpDetSelectorFn MakeOpDetSelectByProcessSet(
    const std::vector<EPhProcess>& procs, int rec_idx)
{
    return [procs, rec_idx](const SLArEventSuperCell& ch) {
        return detail::SelectByProcessSetImpl(ch, procs, rec_idx);
    };
}

inline OpDetSelectorFn MakeOpDetSelectByWavelengthRange(
    float wvl_min, float wvl_max, int rec_idx)
{
    return [wvl_min, wvl_max, rec_idx](const SLArEventSuperCell& ch) {
        return detail::SelectByWavelengthRangeImpl(ch, wvl_min, wvl_max, rec_idx);
    };
}

inline OpDetSelectorFn MakeOpDetSelectCombined(
    const std::vector<EPhProcess>& procs, int proc_rec,
    float wvl_min, float wvl_max,          int wvl_rec)
{
    return [procs, proc_rec, wvl_min, wvl_max, wvl_rec](const SLArEventSuperCell& ch) {
        return detail::SelectCombinedImpl(ch, procs, proc_rec,
                                           wvl_min, wvl_max, wvl_rec);
    };
}

} // namespace display

#endif // SLAR_EVE_OP_HIT_SELECTOR_HH
