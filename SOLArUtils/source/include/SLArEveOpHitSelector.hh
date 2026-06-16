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
 * and passed into the factory functions below.  The selectors themselves
 * contain no knowledge of EBacktracker or system names — they only hold
 * the resolved integer index.
 *
 * A record_idx of -1 means the backtracker is not available; every factory
 * function gracefully falls back to counting all hits in that case.
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

  // ── Selector alias ───────────────────────────────────────────────────────────

  template<typename ChannelT>
    using OpHitSelectorFn = std::function<OpHitSelection(const ChannelT&)>;

  using SiPMSelectorFn      = OpHitSelectorFn<SLArEventSiPM>;
  using OpDetSelectorFn = OpHitSelectorFn<SLArEventSuperCell>;

  // ── Shared implementation ─────────────────────────────────────────────────────

  namespace detail {
    template<typename ChannelT>
      OpHitSelection SelectAllImpl(const ChannelT& ch)
      {
        const int nhits = ch.GetNhits();
        const int tmin  = (nhits > 0)
          ? ch.GetConstHits().begin()->first
          : std::numeric_limits<int>::max();
        return {nhits, tmin};
      }

    template<typename ChannelT>
      OpHitSelection SelectByProcessImpl(const ChannelT& ch,
          int process_key,
          int record_idx)
      {
        const auto& bt_coll = ch.GetBacktrackerRecordCollection();
        if (bt_coll.empty() || record_idx < 0)
          return SelectAllImpl(ch);

        int nhits = 0;
        int tmin  = std::numeric_limits<int>::max();
        for (const auto& [time_bin, bt_vec] : bt_coll) {
          const auto& records = bt_vec.GetConstRecords();
          if (record_idx >= static_cast<int>(records.size())) continue;
          const auto& counter = records[record_idx].GetConstCounter();
          const auto  it      = counter.find(process_key);
          if (it != counter.end() && it->second > 0) {
            nhits += it->second;
            if (time_bin < tmin) tmin = time_bin;
          }
        }
        return {nhits, tmin};
      }

  } // namespace detail

  // ── Factory functions ─────────────────────────────────────────────────────────

  /// Accept all hits — does not require any backtracker.
  template<typename ChannelT>
    OpHitSelectorFn<ChannelT> MakeSelectAll()
    {
      return [](const ChannelT& ch) {
        return detail::SelectAllImpl(ch);
      };
    }

  /// Select hits from a specific optical process.
  /// @param proc        The EPhProcess value to select.
  /// @param record_idx  Index of the kOpticalProc record in fRecords.
  ///                    Pass -1 (or the result of GetBacktrackerRecordIndex
  ///                    when the backtracker is absent) to fall back to all hits.
  template<typename ChannelT>
    OpHitSelectorFn<ChannelT> MakeSelectByProcess(EPhProcess proc, int record_idx)
    {
      const int key = static_cast<int>(proc);
      return [key, record_idx](const ChannelT& ch) {
        return detail::SelectByProcessImpl(ch, key, record_idx);
      };
    }

  // Convenience specialisations

  inline SiPMSelectorFn      MakeSiPMSelectAll()
  { return MakeSelectAll<SLArEventSiPM>(); }

  inline OpDetSelectorFn MakeOpDetSelectAll()
  { return MakeSelectAll<SLArEventSuperCell>(); }

  inline SiPMSelectorFn MakeSiPMSelectByProcess(EPhProcess proc, int record_idx)
  { return MakeSelectByProcess<SLArEventSiPM>(proc, record_idx); }

  inline OpDetSelectorFn MakeOpDetSelectByProcess(EPhProcess proc, int record_idx)
  { return MakeSelectByProcess<SLArEventSuperCell>(proc, record_idx); }

} // namespace display

#endif // SLAR_EVE_OP_HIT_SELECTOR_HH
