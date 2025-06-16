#include <Arduino.h>

#include <SuplaDevice.h>
#include <supla/control/action_trigger.h>
#include <supla/control/button.h>
#include <supla/device/register_device.h>
#include <supla/device/status_led.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/events.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/storage/littlefs_config.h>

#include <supla/network/esp_wifi.h>

#define STATUS_LED_GPIO 37
#define BUTTON_CFG 0

///////

#include "esp_mqtt.h"

// wMBus lib
#include "m_bus_data.h"
#include "rf_mbus.hpp"
#include <wmbus_parameters.h>

#include <map>

#include "drivers.h"
#include "meter.h"

#include "utils_my.h"

#include "format_hex.h"
#include "water_meter.h"

// for restart
#include <supla/clock/clock.h>
#include <supla/tools.h>

#include <iostream>
#include <sstream>

std::map<std::string, Driver *> drivers_{};
std::map<uint32_t, Meter *> meters_{};

void add_meter(Meter *meter) { meters_[meter->id] = meter; }

void add_driver(Driver *driver) { drivers_[driver->get_name()] = driver; }

bool decrypt_telegram(std::vector<unsigned char> &telegram,
                      std::vector<unsigned char> &key) {
  bool ret_val = true;
  int ci_field = telegram[10];
  switch (ci_field) {
  case 0x8D: {
    if (esphome::wmbus::decrypt_ELL_AES_CTR(telegram, key)) {
      static const uint8_t offset{17};
      uint8_t payload_len =
          telegram.size() - 2 - offset; // telegramFrameSize - CRC - offset
      ESP_LOGV(TAG, "Validating CRC for ELL payload");
      if (!esphome::wmbus::crcValid(
              (esphome::wmbus::safeButUnsafeVectorPtr(telegram) + offset), 0,
              payload_len)) {
        ret_val = false;
      }
    } else {
      ESP_LOGVV(TAG, "Decrypting ELL AES CTR failed!");
      ret_val = false;
    }
  } break;

  default: {
    if (!esphome::wmbus::decrypt_TPL_AES_CBC_IV(telegram, key)) {
      ESP_LOGVV(TAG, "Decrypting TPL AES CBC IV failed!");
      ret_val = false;
    }
  } break;
  }
  return ret_val;
}

esphome::wmbus::RxLoop rf_mbus_;

Supla::Sensor::WaterMeter *miernik_1;
Supla::Sensor::WaterMeter *miernik_2;
Supla::Sensor::WaterMeter *miernik_3;
Supla::Sensor::WaterMeter *miernik_4;
Supla::Sensor::WaterMeter *miernik_5;
Supla::Sensor::WaterMeter *miernik_6;
Supla::Sensor::WaterMeter *miernik_7;
Supla::Sensor::WaterMeter *miernik_8;

bool rf_ready = false;

//////

// Choose where Supla should store roller shutter data in persistent memory
// We recommend to use external FRAM memory
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom;

Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla(2048);

Supla::Device::StatusLed statusLed(STATUS_LED_GPIO, false);
Supla::EspWebServer suplaServer;

// HTML www component (they appear in sections according to creation
// sequence)
Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;
Supla::Html::StatusLedParameters htmlStatusLed;
Supla::Html::WmbusParameters htmlWmbusParameters;

template <typename T, int m, int n>
bool isEqual(T (&first)[m], T (&second)[n]) {
  if (m != n) {
    return false;
  }

  for (int i = 0; i < m; i++) {
    if (first[i] != second[i]) {
      return false;
    }
  }
  return true;
}

void add_meter_from_config(uint8_t id, Supla::Sensor::WaterMeter *miernik,
                           Supla::Config *cfg) {
  std::string meterEna = "meter_enabled_" + std::to_string(id);
  char meter_enabled[10] = {};
  cfg->getString(meterEna.c_str(), meter_enabled, 10);
  SUPLA_LOG_INFO("setup %s | %s", meterEna.c_str(), meter_enabled);
  if (strcmp("on", meter_enabled) == 0) {
    std::string meterId = "meter_id_" + std::to_string(id);
    uint32_t meter_id = 0;
    cfg->getUInt32(meterId.c_str(), &meter_id);
    if (meter_id > 0) {
      std::string meterType = "meter_type_" + std::to_string(id);
      char meter_type[30] = {};
      cfg->getString(meterType.c_str(), meter_type, 30);
      std::string meterSens = "meter_sensor_" + std::to_string(id);
      char meter_sensor[50] = {};
      cfg->getString(meterSens.c_str(), meter_sensor, 30);
      std::string meterKey = "meter_key_" + std::to_string(id);
      char meter_key[33] = {};
      cfg->getString(meterKey.c_str(), meter_key, 33);
      std::ostringstream ss;
      ss << "0x" << std::hex << meter_id;
      std::string str_meter_id = ss.str();
      SUPLA_LOG_INFO("  setup mamy miernik: %d [0x%08X] %s | %s | %s | %s",
                     meter_id, meter_id, str_meter_id.c_str(), meter_type,
                     meter_sensor, meter_key);
      miernik = new Supla::Sensor::WaterMeter(SUPLA_CHANNELTYPE_IMPULSE_COUNTER,
                                              SUPLA_CHANNELFNC_IC_WATER_METER,
                                              meter_sensor, str_meter_id);
      if (meters_.count(meter_id) > 0) {
        meters_[meter_id]->add_sensor(meter_sensor, miernik);
      } else {
        auto meter = new Meter(meter_id, meter_type, meter_key);
        meter->add_sensor(meter_sensor, miernik);
        add_meter(meter);
      }
    }
  }
}

void setup() {

  Serial.begin(115200);

  SuplaDevice.addClock(new Supla::Clock);

  auto buttonCfgRelay = new Supla::Control::Button(BUTTON_CFG, true, true);

  buttonCfgRelay->configureAsConfigButton(&SuplaDevice);

  // configure defualt Supla CA certificate
  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);

  SuplaDevice.setName("wM-Bus SUPLA");
  SuplaDevice.setSwVersion("25.02-0.0.1");

  auto mqtt = new Supla::Protocol::EspMqtt(&SuplaDevice);

  SuplaDevice.begin();

  // Supla::Storage::Init();
  auto cfg = Supla::Storage::ConfigInstance();

  uint32_t first_config = 0;
  cfg->getUInt32("first_config", &first_config);
  if (first_config != 1441) {
    cfg->setUInt32("first_config", 1441);
  }

  if (rf_mbus_.init(0, 0, 0, 0, 0, 0, 868.9, true)) {
    SUPLA_LOG_INFO("  RF chip initialization OK.");
    rf_ready = true;
  } else {
    SUPLA_LOG_INFO("  RF chip initialization failed.");
  }

  add_driver(new Amiplus());
  add_driver(new Apator08());
  add_driver(new Apator162());
  add_driver(new ApatorEITN());
  add_driver(new Bmeters());
  add_driver(new C5isf());
  add_driver(new Compact5());
  add_driver(new Dme07());
  add_driver(new Elf());
  add_driver(new Evo868());
  add_driver(new FhkvdataIII());
  add_driver(new Flowiq2200());
  add_driver(new Hydrocalm3());
  add_driver(new Hydrus());
  add_driver(new Hydrodigit());
  add_driver(new Iperl());
  add_driver(new Itron());
  add_driver(new Izar());
  add_driver(new Kamheat());
  add_driver(new Mkradio3());
  add_driver(new Mkradio4());
  add_driver(new Mkradio4a());
  add_driver(new Multical21());
  add_driver(new Qheat());
  add_driver(new Qwater());
  add_driver(new Rfmtx1());
  add_driver(new Sharky774());
  add_driver(new TopasESKR());
  add_driver(new Ultrimis());
  add_driver(new Unismart());
  add_driver(new Vario451());

  add_meter_from_config(1, miernik_1, cfg);
  add_meter_from_config(2, miernik_2, cfg);
  add_meter_from_config(3, miernik_3, cfg);
  add_meter_from_config(4, miernik_4, cfg);
  add_meter_from_config(5, miernik_5, cfg);
  add_meter_from_config(6, miernik_6, cfg);
  add_meter_from_config(7, miernik_7, cfg);
  add_meter_from_config(8, miernik_8, cfg);

  mqtt->channelsCount = Supla::RegisterDevice::getChannelCount();
}

void loop() {
  SuplaDevice.iterate();
  if (rf_ready && rf_mbus_.task()) {
    esphome::wmbus::WMbusFrame mbus_data = rf_mbus_.get_frame();
    std::vector<unsigned char> frame = mbus_data.frame;
    std::string telegram = format_hex_pretty(frame);

    telegram.erase(std::remove(telegram.begin(), telegram.end(), '.'),
                   telegram.end());

    uint32_t meter_id = ((uint32_t)frame[7] << 24) |
                        ((uint32_t)frame[6] << 16) | ((uint32_t)frame[5] << 8) |
                        ((uint32_t)frame[4]);

    SUPLA_LOG_INFO("Meter ID [0x%08X] RSSI: %d dBm T: %s", meter_id,
                   mbus_data.rssi, telegram.c_str());

    if (meters_.count(meter_id) > 0) {
      auto *meter = meters_[meter_id];
      if (drivers_.count(meter->type) > 0) {
        std::string myKey = format_hex_pretty(meter->key);
        SUPLA_LOG_INFO("Key: '%s'", myKey.c_str());
        if (meter->key.size()) {
          decrypt_telegram(frame, meter->key);
        }

        auto selected_driver = drivers_[meter->type];
        SUPLA_LOG_INFO("Using driver '%s' for ID [0x%08X] RSSI: %d dBm T: %s",
                       selected_driver->get_name().c_str(), meter_id,
                       mbus_data.rssi, telegram.c_str());

        auto mapValues = selected_driver->get_values(frame);
        if (mapValues.has_value()) {
          for (const auto &ele : mapValues.value()) {
            if (meter->sensors_.count(ele.first) > 0) {
              unsigned _supla_int64_t pomiar =
                  (unsigned _supla_int64_t)(ele.second);
              SUPLA_LOG_INFO("Uzyskalem dane %s = %.3f", ele.first.c_str(),
                             ele.second);
              pomiar = (unsigned _supla_int64_t)(ele.second * 1000);
              meter->sensors_[ele.first]->update(pomiar);
            }
          }
        }
      } else {
        SUPLA_LOG_INFO("Brak drivera dla meter_id 0x%08X w konfiguracji",
                       meter_id);
      }
    }
  }
}