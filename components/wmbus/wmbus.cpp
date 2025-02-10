#include "wmbus.h"
#include "version.h"
#include "meters.h"
#include "address.h"
#include "esphome/core/application.h"
#include "qrcode.h"

#ifdef USE_CAPTIVE_PORTAL
#include "esphome/components/captive_portal/captive_portal.h"
#endif

#ifdef USE_ESP32
SET_LOOP_TASK_STACK_SIZE(32 * 1024);
#pragma message ( "Loop task stack increased." )
#endif
#ifdef USE_ESP8266
#error "ESP8266 not supported. Please use version 3.x: https://github.com/SzczepanLeon/esphome-components/issues/131"
#endif

namespace esphome {
namespace wmbus {

  static const char *TAG = "wmbus";

  void InfoComponent::setup() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.beginTransmission(DISPLAY_ADDR);
    if (Wire.endTransmission() == 0) {
        this->u8g2_ = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
        this->u8g2_->begin();
        this->u8g2_->clearBuffer();
        this->u8g2_->setFont(u8g2_font_VCR_OSD_mf);
        this->u8g2_->drawStr(65, 20, "ESP");
        this->u8g2_->drawStr(81, 38, "Home");
        this->u8g2_->setFont(u8g2_font_10x20_te);
        this->u8g2_->drawStr(67, 60, "wM-Bus");
        this->u8g2_->sendBuffer();
        
        QRCode qrcode;
        const int QRcode_Version = 3;
        const int QRcode_ECC = 0;
        uint8_t qrcodeData[qrcode_getBufferSize(QRcode_Version)];
        qrcode_initText(&qrcode, qrcodeData, QRcode_Version, QRcode_ECC, "https://github.com/SzczepanLeon/esphome-components");
        uint8_t x0 = 2;
        uint8_t y0 = 4;
        //display QRcode
        for (uint8_t y = 0; y < qrcode.size; y++) {
          for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y) == 1) {
              this->u8g2_->setDrawColor(1);
              this->u8g2_->drawPixel(x0 + 2 * x,     y0 + 2 * y);
              this->u8g2_->drawPixel(x0 + 2 * x + 1, y0 + 2 * y);
              this->u8g2_->drawPixel(x0 + 2 * x,     y0 + 2 * y + 1);
              this->u8g2_->drawPixel(x0 + 2 * x + 1, y0 + 2 * y + 1);
            } else {
              this->u8g2_->setDrawColor(0);
              this->u8g2_->drawPixel(x0 + 2 * x,     y0 + 2 * y);
              this->u8g2_->drawPixel(x0 + 2 * x + 1, y0 + 2 * y);
              this->u8g2_->drawPixel(x0 + 2 * x,     y0 + 2 * y + 1);
              this->u8g2_->drawPixel(x0 + 2 * x + 1, y0 + 2 * y + 1);
            }
          }
        }
        this->u8g2_->sendBuffer();

        delay(3000);
        this->u8g2_->setPowerSave(1);
    }
    return;
  }

  void WMBusComponent::setup() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.beginTransmission(DISPLAY_ADDR);
    if (Wire.endTransmission() == 0) {
      this->u8g2_ = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
      this->u8g2_->begin();
      this->u8g2_->clearBuffer();
      if (this->display_active_) {
        this->u8g2_->drawRFrame(0, 18, 128, 46, 5);
  #ifdef USE_WMBUS_MQTT
        this->u8g2_->setFont(u8g2_font_squeezed_b6_tr);
        this->u8g2_->setCursor(2, 7);
        this->u8g2_->print("MQ");
        this->u8g2_->setCursor(4, 14);
        this->u8g2_->print("TT");
        this->u8g2_->setFont(u8g2_font_open_iconic_www_2x_t);
        this->u8g2_->drawGlyph(17, 16, 65);
  #elif defined(USE_MQTT)
        this->u8g2_->setFont(u8g2_font_squeezed_b6_tr);
        this->u8g2_->setCursor(2, 7);
        this->u8g2_->print("MQ");
        this->u8g2_->setCursor(4, 14);
        this->u8g2_->print("TT");
        this->u8g2_->setFont(u8g2_font_open_iconic_www_2x_t);
        // this->u8g2_->drawGlyph(17, 16, 78);
  #endif

  #ifdef USE_API
        this->u8g2_->setFont(u8g2_font_squeezed_b6_tr);
        this->u8g2_->setCursor(55, 7);
        this->u8g2_->print("HOME");
        this->u8g2_->setCursor(45, 14);
        this->u8g2_->print("ASSISTANT");
  #endif

        // ETH / WiFi
        this->u8g2_->setFont(u8g2_font_open_iconic_www_2x_t);        
        if (this->net_component_->is_connected()) {
  #ifdef USE_ETHERNET
          this->u8g2_->drawGlyph(112, 16, 83);
  #elif defined(USE_WIFI)
          bool captive_portal_active{false};
  #ifdef USE_CAPTIVE_PORTAL
          captive_portal_active = (captive_portal::global_captive_portal != nullptr && captive_portal::global_captive_portal->is_active());
  #endif
          if (captive_portal_active) {
            this->u8g2_->setFont(u8g2_font_helvB12_te);
            this->u8g2_->setCursor(112, 14);
            this->u8g2_->print("AP");
          }
          else {
            this->u8g2_->drawGlyph(112, 16, 81);
          }
  #endif
        }
        else {
          this->u8g2_->drawGlyph(112, 16, 74);
        }

        this->u8g2_->setFont(u8g2_font_pxplusibmvga8_mr);
        this->u8g2_->setCursor(10, 32);
        this->u8g2_->print("ID:");
        this->u8g2_->setCursor(10, 46);
        this->u8g2_->print("RSSI:");
        this->u8g2_->setCursor(10, 60);
        this->u8g2_->print("Driver:");
        this->u8g2_->sendBuffer();
      } else {
        this->u8g2_->setPowerSave(1);
      }
    }
    this->high_freq_.start();
    if (this->led_pin_ != nullptr) {
      this->led_pin_->setup();
      this->led_pin_->digital_write(false);
      this->led_on_ = false;
    }
    if (!rf_mbus_.init(this->spi_conf_.mosi->get_pin(), this->spi_conf_.miso->get_pin(),
                       this->spi_conf_.clk->get_pin(),  this->spi_conf_.cs->get_pin(),
                       this->spi_conf_.gdo0->get_pin(), this->spi_conf_.gdo2->get_pin(),
                       this->frequency_, this->sync_mode_)) {
      this->mark_failed();
      ESP_LOGE(TAG, "Radio chip initialization failed");
      return;
    }
#ifdef USE_WMBUS_MQTT
    this->mqtt_client_.setClient(this->tcp_client_);
    this->mqtt_client_.setServer(this->mqtt_->ip, this->mqtt_->port);
    this->mqtt_client_.setBufferSize(1000);
#endif
  }

  void WMBusComponent::loop() {
    uint8_t forMe = 74;
    this->led_handler();
    if (rf_mbus_.task()) {
      ESP_LOGVV(TAG, "Have data from radio ...");
      WMbusFrame mbus_data = rf_mbus_.get_frame();

      std::string telegram = format_hex_pretty(mbus_data.frame);
      telegram.erase(std::remove(telegram.begin(), telegram.end(), '.'), telegram.end());

      this->frame_timestamp_ = this->time_->timestamp_now();
      send_to_clients(mbus_data);
      send_to_serial(mbus_data);
      Telegram t;
      if (t.parseHeader(mbus_data.frame) && t.addresses.empty()) {
        ESP_LOGE(TAG, "Address is empty! T: %s", telegram.c_str());
      }
      else {
        uint32_t meter_id = (uint32_t)strtoul(t.addresses[0].id.c_str(), nullptr, 16);
        bool meter_in_config = (this->wmbus_listeners_.count(meter_id) == 1) ? true : false;
        
        if (this->log_all_ || this->display_all_ || meter_in_config) { //No need to do sth if logging/display is disabled and meter is not configured

          auto detected_drv_info      = pickMeterDriver(&t);
          std::string detected_driver = (detected_drv_info.name().str().empty() ? "" : detected_drv_info.name().str().c_str());

          //If the driver was explicitly stated in meter config, use that driver instead on detected one
          auto used_drv_info      = detected_drv_info;
          std::string used_driver = detected_driver;
          if (meter_in_config) {
            auto *sensor = this->wmbus_listeners_[meter_id];
            used_driver = ((sensor->type).empty() ? detected_driver : sensor->type);
            if (!(sensor->type).empty()){
              auto *used_drv_info_ptr = lookupDriver(used_driver);
              if (used_drv_info_ptr == nullptr) {
                used_driver = detected_driver;
                used_drv_info = detected_drv_info;
                ESP_LOGW(TAG, "Selected driver %s doesn't exist, using %s", (sensor->type).c_str(), used_driver.c_str());
              }
              else{
                used_drv_info = *used_drv_info_ptr;
                ESP_LOGI(TAG, "Using selected driver %s (detected driver was %s)", used_driver.c_str(), detected_driver.c_str());
              }
            }
          }

          this->led_blink();
          if (this->log_all_ || meter_in_config) {
            ESP_LOGI(TAG, "%s [0x%08x] RSSI: %ddBm T: %s %c1 %c",
                      (used_driver.empty()? "Unknown!" : used_driver.c_str()),
                      meter_id,
                      mbus_data.rssi,
                      telegram.c_str(),
                      mbus_data.mode,
                      mbus_data.block);
          }

          std::pair<String, String> display_data;
          if (meter_in_config) {
            bool supported_link_mode{false};
            if (used_drv_info.linkModes().empty()) {
              supported_link_mode = true;
              ESP_LOGW(TAG, "Link modes not defined in driver %s. Processing anyway.",
                      (used_driver.empty()? "Unknown!" : used_driver.c_str()));
            }
            else {
              supported_link_mode = ( ((mbus_data.mode == 'T') && (used_drv_info.linkModes().has(LinkMode::T1))) ||
                                      ((mbus_data.mode == 'C') && (used_drv_info.linkModes().has(LinkMode::C1))) );
            }

            if (used_driver.empty()) {
              ESP_LOGW(TAG, "Can't find driver for T: %s", telegram.c_str());
            }
            else if (!supported_link_mode) {
              ESP_LOGW(TAG, "Link mode %c1 not supported in driver %s",
                      mbus_data.mode,
                      used_driver.c_str());
            }
            else {
              auto *sensor = this->wmbus_listeners_[meter_id];
              
              bool id_match;
              MeterInfo mi;
              mi.parse("ESPHome", used_driver, t.addresses[0].id + ",", sensor->myKey);
              auto meter = createMeter(&mi);
              std::vector<Address> addresses;
              AboutTelegram about{"Ultimate Reader", mbus_data.rssi, FrameType::WMBUS, this->frame_timestamp_};
              meter->handleTelegram(about, mbus_data.frame, false, &addresses, &id_match, &t);
              if (id_match) {
                forMe = 73;
                for (auto const &field : sensor->fields) {
                  std::string field_name = field.name;
                  std::string unit = field.unit;
                  bool display = field.display;
                  sensor::Sensor *esph_sensor = field.sensor;
                  if (field_name == "rssi") {
                    esph_sensor->publish_state(mbus_data.rssi);
                  }
                  else if (esph_sensor->get_unit_of_measurement().empty()) {
                    ESP_LOGW(TAG, "Fields without unit not supported as sensor, please switch to text_sensor.");
                  }
                  else {
                    Unit field_unit = toUnit(esph_sensor->get_unit_of_measurement());
                    if (field_unit != Uni:Unknown) {
                      double value  = meter->getNumericValue(field_name, field_unit);
                      if (!std::isnan(value)) {
                        esph_sensor->publish_state(value);
                        if (display) {
                          display_data = std::make_pair(String(field_name.c_str()) , String(strWithUnitLowerCase(value, field_unit).c_str()));
                        }
                      }
                      else {
                        ESP_LOGW(TAG, "Can't get requested field '%s' with unit '%s'", field_name.c_str(), unit.c_str());
                      }
                    }
                    else {
                      ESP_LOGW(TAG, "Can't get proper unit from '%s'", unit.c_str());
                    }
                  }
                }
                for (auto const &field : sensor->text_fields) {
                  std::string field_name = field.first;
                  text_sensor::TextSensor *esph_sensor = field.second;
                  if (meter->hasStringValue(field_name)) {
                    std::string value  = meter->getMyStringValue(field_name);
                    esph_sensor->publish_state(value);
                  }
                  else {
                    ESP_LOGW(TAG, "Can't get requested field '%s'", field_name.c_str());
                  }
                }
#ifdef USE_WMBUS_MQTT
                std::string json;
                meter->printJsonMeter(&t, &json, false);
                std::string mqtt_topic = (App.get_friendly_name().empty() ? App.get_name() : App.get_friendly_name()) + "/wmbus/" + t.addresses[0].id;
                if (this->mqtt_client_.connect("", this->mqtt_->name.c_str(), this->mqtt_->password.c_str())) {
                  this->mqtt_client_.publish(mqtt_topic.c_str(), json.c_str(), this->mqtt_->retained);
                  ESP_LOGV(TAG, "Publish(topic='%s' payload='%s' retain=%d)", mqtt_topic.c_str(), json.c_str(), this->mqtt_->retained);
                  this->mqtt_client_.disconnect();
                }
                else {
                  ESP_LOGV(TAG, "Publish failed for topic='%s' (len=%u).", mqtt_topic.c_str(), json.length());
                }
#elif defined(USE_MQTT)
                std::string json;
                meter->printJsonMeter(&t, &json, false);
                std::string mqtt_topic = this->mqtt_client_->get_topic_prefix() + "/wmbus/" + t.addresses[0].id;
                this->mqtt_client_->publish(mqtt_topic, json);
#endif
              }
              else {
                ESP_LOGE(TAG, "Not for me  %s", telegram.c_str());
              }
            }
          }
          else {
            // meter not in config
            forMe = 84;
          }
          if (this->display_active_) {
            if ((this->u8g2_ != nullptr) && (this->display_all_ || (display_data.first.length() > 0))) {
              this->u8g2_->clearBuffer();
              this->u8g2_->drawRFrame(0, 18, 128, 46, 5);

    #ifdef USE_WMBUS_MQTT
              this->u8g2_->setFont(u8g2_font_squeezed_b6_tr);
              this->u8g2_->setCursor(2, 7);
              this->u8g2_->print("MQ");
              this->u8g2_->setCursor(4, 14);
              this->u8g2_->print("TT");
              this->u8g2_->setFont(u8g2_font_open_iconic_www_2x_t);
              this->u8g2_->drawGlyph(17, 16, 65);
    #elif defined(USE_MQTT)
              this->u8g2_->setFont(u8g2_font_squeezed_b6_tr);
              this->u8g2_->setCursor(2, 7);
              this->u8g2_->print("MQ");
              this->u8g2_->setCursor(4, 14);
              this->u8g2_->print("TT");
    #endif

    #ifdef USE_API
              this->u8g2_->setFont(u8g2_font_squeezed_b6_tr);
              this->u8g2_->setCursor(55, 7);
              this->u8g2_->print("HOME");
              this->u8g2_->setCursor(45, 14);
              this->u8g2_->print("ASSISTANT");
    #endif

              this->u8g2_->setFont(u8g2_font_open_iconic_www_2x_t);
              this->u8g2_->drawGlyph(95, 16, forMe);

              // ETH / WiFi
              this->u8g2_->setFont(u8g2_font_open_iconic_www_2x_t);        
              if (this->net_component_->is_connected()) {
    #ifdef USE_ETHERNET
                this->u8g2_->drawGlyph(112, 16, 83);
    #elif defined(USE_WIFI)
                bool captive_portal_active{false};
    #ifdef USE_CAPTIVE_PORTAL
                captive_portal_active = (captive_portal::global_captive_portal != nullptr && captive_portal::global_captive_portal->is_active());
    #endif
                if (captive_portal_active) {
                  this->u8g2_->setFont(u8g2_font_helvB12_te);
                  this->u8g2_->setCursor(112, 14);
                  this->u8g2_->print("AP"); // ?? when
                }
                else {
                  this->u8g2_->drawGlyph(112, 16, 81);
                }
    #endif
              }
              else {
                this->u8g2_->drawGlyph(112, 16, 74);
              }
              this->u8g2_->setFont(u8g2_font_pxplusibmvga8_mr);
              this->u8g2_->setCursor(10, 32);
              this->u8g2_->print("ID:");
              if (display_data.first.length() == 0) {
                this->u8g2_->setCursor(10, 46);
                this->u8g2_->print("RSSI:");
                this->u8g2_->setCursor(10, 60);
                this->u8g2_->print("Driver:");
              }
              else {
                this->u8g2_->setFont(u8g2_font_crox1h_tr);
                this->u8g2_->setCursor( (this->u8g2_->getDisplayWidth() - this->u8g2_->getUTF8Width(display_data.first.c_str())) / 2, 46 );
                this->u8g2_->print(display_data.first);
                this->u8g2_->setCursor( (this->u8g2_->getDisplayWidth() - this->u8g2_->getUTF8Width(display_data.second.c_str())) / 2, 60 );
                this->u8g2_->print(display_data.second);
              }

              String myId  = "0x" + String(t.addresses[0].id.c_str());
              String rssi = String(mbus_data.rssi) + "dBm";
              String driver = String(used_drv_info.name().str().c_str());

              this->u8g2_->setFont(u8g2_font_crox1h_tr);
              this->u8g2_->setCursor( (this->u8g2_->getDisplayWidth() - this->u8g2_->getUTF8Width(myId.c_str())) - 10, 32 );
              this->u8g2_->print(myId);

              if (display_data.first.length() == 0) {
                this->u8g2_->setFont(u8g2_font_crox1h_tr);
                this->u8g2_->setCursor( (this->u8g2_->getDisplayWidth() - this->u8g2_->getUTF8Width(rssi.c_str())) - 10, 46 );
                this->u8g2_->print(rssi);

                this->u8g2_->setFont(u8g2_font_crox1h_tr);
                this->u8g2_->setCursor( (this->u8g2_->getDisplayWidth() - this->u8g2_->getUTF8Width(driver.c_str())) - 10, 60 );
                this->u8g2_->print(driver);
              }

              this->u8g2_->sendBuffer();
            }
          } else {
            this->u8g2_->setPowerSave(1);
          }
        }
      }
    }
  }

  void WMBusComponen:register_wmbus_listener(const uint32_t meter_id, const std::string type, const std::string key) {
    if (this->wmbus_listeners_.count(meter_id) == 0) {
      WMBusListener *listener = new wmbus::WMBusListener(meter_id, type, key);
      this->wmbus_listeners_.insert({meter_id, listener});
    }
  }

  void WMBusComponent::led_blink() {
    if (this->led_pin_ != nullptr) {
      if (!this->led_on_) {
        this->led_on_millis_ = millis();
        this->led_pin_->digital_write(true);
        this->led_on_ = true;
      }
    }
  }

  void WMBusComponent::led_handler() {
    if (this->led_pin_ != nullptr) {
      if (this->led_on_) {
        if ((millis() - this->led_on_millis_) >= this->led_blink_time_) {
          this->led_pin_->digital_write(false);
          this->led_on_ = false;
        }
      }
    }
  }

  void WMBusComponent::send_to_serial(WMbusFrame &mbus_data) {
    std::string telegram = format_hex_pretty(mbus_data.frame);
    telegram.erase(std::remove(telegram.begin(), telegram.end(), '.'), telegram.end());
    ESP_LOGI(TAG, "\n%c1;1;1;%s;%d;;;0x%s\n", mbus_data.mode, telegram_time, mbus_data.rssi, telegram.c_str());
  }

  void WMBusComponent::send_to_clients(WMbusFrame &mbus_data) {
    for (auto &client : this->clients_) {
      switch (client.format) {
        case FORMAT_HEX:
          {
            switch (client.transport) {
              case TRANSPORT_TCP:
                {
                  ESP_LOGV(TAG, "Will send HEX telegram to %s:%d via TCP", client.ip.str().c_str(), client.port);
                  if (this->tcp_client_.connect(client.ip.str().c_str(), client.port)) {
                    this->tcp_client_.write((const uint8_t *) mbus_data.frame.data(), mbus_data.frame.size());
                    this->tcp_client_.stop();
                  }
                  else {
                    ESP_LOGE(TAG, "Can't connect via TCP to %s:%d", client.ip.str().c_str(), client.port);
                  }
                }
                break;
              case TRANSPORT_UDP:
                {
                  ESP_LOGV(TAG, "Will send HEX telegram to %s:%d via UDP", client.ip.str().c_str(), client.port);
                  this->udp_client_.beginPacket(client.ip.str().c_str(), client.port);
                  this->udp_client_.write((const uint8_t *) mbus_data.frame.data(), mbus_data.frame.size());
                  this->udp_client_.endPacket();
                }
                break;
              default:
                ESP_LOGE(TAG, "Unknown transport!");
                break;
            }
          }
          break;
        case FORMAT_RTLWMBUS:
          {
            char telegram_time[24];
            strftime(telegram_time, sizeof(telegram_time), "%Y-%m-%d %H:%M:%S.00Z", gmtime(&(this->frame_timestamp_)));
            switch (client.transport) {
              case TRANSPORT_TCP:
                {
                  ESP_LOGV(TAG, "Will send RTLWMBUS telegram to %s:%d via TCP", client.ip.str().c_str(), client.port);
                  if (this->tcp_client_.connect(client.ip.str().c_str(), client.port)) {
                    this->tcp_client_.printf("%c1;1;1;%s;%d;;;0x",
                                             mbus_data.mode,
                                             telegram_time,
                                             mbus_data.rssi);
                    for (int i = 0; i < mbus_data.frame.size(); i++) {
                      this->tcp_client_.printf("%02X", mbus_data.frame[i]);
                    }
                    this->tcp_client_.print("\n");
                    this->tcp_client_.stop();
                  }
                  else {
                    ESP_LOGE(TAG, "Can't connect via TCP to %s:%d", client.ip.str().c_str(), client.port);
                  }
                }
                break;
              case TRANSPORT_UDP:
                {
                  ESP_LOGV(TAG, "Will send RTLWMBUS telegram to %s:%d via UDP", client.ip.str().c_str(), client.port);
                  this->udp_client_.beginPacket(client.ip.str().c_str(), client.port);
                  this->udp_client_.printf("%c1;1;1;%s;%d;;;0x",
                                           mbus_data.mode,
                                           telegram_time,
                                           mbus_data.rssi);
                  for (int i = 0; i < mbus_data.frame.size(); i++) {
                    this->udp_client_.printf("%02X", mbus_data.frame[i]);
                  }
                  this->udp_client_.print("\n");
                  this->udp_client_.endPacket();
                }
                break;
              default:
                ESP_LOGE(TAG, "Unknown transport!");
                break;
            }
          }
          break;
        default:
          ESP_LOGE(TAG, "Unknown format!");
          break;
      }
    }
  }

  const LogString *WMBusComponent::format_to_string(Format format) {
    switch (format) {
      case FORMAT_HEX:
        return LOG_STR("hex");
      case FORMAT_RTLWMBUS:
        return LOG_STR("rtl-wmbus");
      default:
        return LOG_STR("unknown");
    }
  }

  const LogString *WMBusComponent::transport_to_string(Transport transport) {
    switch (transport) {
      case TRANSPORT_TCP:
        return LOG_STR("TCP");
      case TRANSPORT_UDP:
        return LOG_STR("UDP");
      default:
        return LOG_STR("unknown");
    }
  }

  void WMBusComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "wM-Bus v%s-%s:", MY_VERSION, WMBUSMETERS_VERSION);
    if (this->clients_.size() > 0) {
      ESP_LOGCONFIG(TAG, "  Clients:");
      for (auto &client : this->clients_) {
        ESP_LOGCONFIG(TAG, "    %s: %s:%d %s [%s]",
                      client.name.c_str(),
                      client.ip.str().c_str(),
                      client.port,
                      LOG_STR_ARG(transport_to_string(client.transport)),
                      LOG_STR_ARG(format_to_string(client.format)));
      }
    }
    if (this->led_pin_ != nullptr) {
      ESP_LOGCONFIG(TAG, "  LED:");
      ESP_LOGCONFIG(TAG, "    Duration: %d ms", this->led_blink_time_);
    }
#ifdef USE_ESP32
    ESP_LOGCONFIG(TAG, "  Chip ID: %012llX", ESP.getEfuseMac());
#endif
    if (this->is_failed()) {
      ESP_LOGE(TAG, "   Check connection to radio module!");
    }
    std::string drivers = "";
    for (DriverInfo *p : allDrivers()) {
      drivers += p->name().str() + ", ";
    }
    drivers.erase(drivers.size() - 2);
    ESP_LOGCONFIG(TAG, "  Available drivers: %s", drivers.c_str());
    for (const auto &ele : this->wmbus_listeners_) {
      ele.second->dump_config();
    }
  }

  ///////////////////////////////////////

  void WMBusListener::dump_config() {
    std::string key = format_hex_pretty(this->key);
    key.erase(std::remove(key.begin(), key.end(), '.'), key.end());
    if (key.size()) {
      key.erase(key.size() - 5);
    }
    ESP_LOGCONFIG(TAG, "  Meter:");
    ESP_LOGCONFIG(TAG, "    ID: %zu [0x%08X]", this->id, this->id);
    ESP_LOGCONFIG(TAG, "    Type: %s", ((this->type).empty() ? "auto detect" : this->type.c_str()));
    ESP_LOGCONFIG(TAG, "    Key: '%s'", key.c_str());
    for (const auto &field : this->fields) {
      ESP_LOGCONFIG(TAG, "    Field: '%s' %s", field.name.c_str(), (field.display ? "[display]" : ""));
      LOG_SENSOR("     ", "Name:", field.sensor);
    }
    for (const auto &field : this->text_fields) {
      ESP_LOGCONFIG(TAG, "    Text field: '%s'", field.first.c_str());
      LOG_TEXT_SENSOR("     ", "Name:", field.second);
    }
  }

  WMBusListener::WMBusListener(const uint32_t id, const std::string type, const std::string key) {
    this->id = id;
    this->type = type;
    this->myKey = key;
    hex_to_bin(key, &(this->key));
  }

  int WMBusListener::char_to_int(char input)
  {
    if(input >= '0' && input <= '9') {
      return input - '0';
    }
    if(input >= 'A' && input <= 'F') {
      return input - 'A' + 10;
    }
    if(input >= 'a' && input <= 'f') {
      return input - 'a' + 10;
    }
    return -1;
  }

  bool WMBusListener::hex_to_bin(const char* src, std::vector<unsigned char> *target)
  {
    if (!src) return false;
    while(*src && src[1]) {
      if (*src == ' ' || *src == '#' || *src == '|' || *src == '_') {
        // Ignore space and hashes and pipes and underlines.
        src++;
      }
      else {
        int hi = char_to_int(*src);
        int lo = char_to_int(src[1]);
        if (hi<0 || lo<0) return false;
        target->push_back(hi*16 + lo);
        src += 2;
      }
    }
    return true;
  }

}  // namespace wmbus
}  // namespace esphome
