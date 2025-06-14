
#ifndef SRC_SUPLA_SENSOR_WATERMETER_H_
#define SRC_SUPLA_SENSOR_WATERMETER_H_

#include <Arduino.h>

#include "supla/channel_element.h"
#include <supla/log_wrapper.h>

#include "esphome/core/optional.h"

namespace Supla {
namespace Sensor {
class WaterMeter : public ChannelElement {
public:
  WaterMeter(int type, int defa, const std::string meterType,
             const std::string meterId) {
    channel.setType(type);
    channel.setDefault(defa);
    channel.meterType = meterType;
    channel.meterId = meterId;
  }

  void iterateAlways() {
    if (millis() - lastReadTime > 10000) {
      lastReadTime = millis();
      if (this->value.has_value()) {
        channel.setNewValue((unsigned _supla_int64_t)(this->value.value()));
        this->value.reset();
      }
    }
  }

  void update(unsigned _supla_int64_t value) { this->value = value; }

protected:
  uint64_t lastReadTime;
  esphome::optional<unsigned _supla_int64_t> value{};
};

}; // namespace Sensor
}; // namespace Supla

#endif // SRC_SUPLA_SENSOR_WATERMETER_H_
