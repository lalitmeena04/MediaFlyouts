#include "PowerAlertPolicy.h"

namespace mf {

std::optional<ChargerAlert> PowerAlertPolicy::update(const PowerSnapshot& snapshot) {
    // Unknown reading keeps the last source. Do not guess a transition.
    if(snapshot.source == PowerSource::Unknown)
        return std::nullopt;

    const PowerSource previous = m_source;
    m_source = snapshot.source;
    if(previous == PowerSource::Unknown || previous == snapshot.source)
        return std::nullopt;

    ChargerAlert alert;
    alert.event = snapshot.source == PowerSource::AC ? ChargerEvent::PluggedIn : ChargerEvent::Unplugged;
    alert.percent = snapshot.percent;
    alert.bucket = bucketOf(snapshot.percent);
    return alert;
}

} // namespace mf
