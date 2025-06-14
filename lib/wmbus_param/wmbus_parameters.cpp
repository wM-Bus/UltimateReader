/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <string.h>
#include <supla/network/network.h>
#include <supla/network/web_sender.h>
#include <supla/tools.h>

#include "wmbus_parameters.h"

#include <supla/log_wrapper.h>

#include <string>
#include <vector>

namespace Supla {

namespace Html {

WmbusParameters::WmbusParameters() : HtmlElement(HTML_SECTION_PROTOCOL) {
  cfg = Supla::Storage::ConfigInstance();
}
WmbusParameters::~WmbusParameters() {}
void WmbusParameters::send(Supla::WebSender *sender) {
  // METER
  for (int i = 1; i <= 8; i++) {
    std::string myMeterId = "meter_id_" + std::to_string(i);
    uint32_t meter_id = this->getMeterId(myMeterId.c_str());
    char meter_id_hex[10] = {};
    sprintf(meter_id_hex, "%X", meter_id); // convert number to hex
    sender->send("<div class=\"box\">");
    sender->send("<h3>Meter #");
    sender->send(i);
    sender->send(" configuration</h3>");

    // ENABLED
    std::string enabledKey = "meter_enabled_" + std::to_string(i);
    std::string enabledId = "meter_en_" + std::to_string(i);
    sender->send("<div class=\"form-field right-checkbox\">");
    sender->sendLabelFor(enabledKey.c_str(), "Enabled");
    sender->send("<label>");
    sender->send("<div class=\"switch\">");
    sender->send("<input type=\"hidden\" value=\"off\" ");
    sender->sendName(enabledKey.c_str());
    sender->sendId(enabledId.c_str());
    sender->send(">");
    //
    sender->send(
        "<input type=\"checkbox\" value=\"on\" onchange=\"meterChanged(");
    sender->send(std::to_string(i).c_str());
    sender->send(");\" ");
    sender->send(checked(this->isMeterSelected(enabledKey.c_str(), "on")));
    sender->sendNameAndId(enabledKey.c_str());
    sender->send(">");
    sender->send("<span class=\"slider\"></span>");
    sender->send("</div>");
    sender->send("</label>");
    sender->send("</div>");

    sender->send("<div id=\"wmbus_m_");
    sender->send(std::to_string(i).c_str());
    sender->send("\" >");

    // ID
    // form-field START
    sender->send("<div class=\"form-field\">");
    std::string idKey = "meter_id_" + std::to_string(i);
    sender->sendLabelFor(idKey.c_str(), "ID 0x");
    sender->send("<input maxlength=\"8\"");
    sender->sendNameAndId(idKey.c_str());
    sender->send("value=\"");
    sender->send(meter_id_hex);
    sender->send("\">");
    sender->send("</div>");
    // form-field END

    // TYPE
    std::string typeKey = "meter_type_" + std::to_string(i);

    // int size = this->cfg->getStringSize(typeKey.c_str());
    // char *typeBuf = nullptr;
    // if (size > 0) {
    //   typeBuf = new char[size + 1];
    //   memset(typeBuf, 0, size + 1);
    //   cfg->getString(typeKey.c_str(), typeBuf, size);
    // }

    // SUPLA_LOG_INFO("TYPE %s | %s", typeKey.c_str(), typeBuf);

    std::vector<std::string> all_types{
        "amiplus",    "apatoreitn", "apator08", "apator162", "bmeters",
        "c5isf",      "compact5",   "elf",      "evo868",    "fhkvdataiii",
        "hydrocalm3", "hydrus",     "itron",    "izar",      "mkradio3",
        "mkradio4",   "qheat",      "qwater",   "sharky774", "topaseskr",
        "ultrimis",   "unismart",   "vario451",
    };

    // form-field BEGIN
    sender->send("<div class=\"form-field\" style=\"border-top:0px\">");
    sender->sendLabelFor(typeKey.c_str(), "type");
    sender->send("<select ");
    sender->sendNameAndId(typeKey.c_str());
    sender->send(">");
    for (const auto &meter_type : all_types) {
      sender->send("<option value=\"");
      sender->send(meter_type.c_str());
      sender->send("\"");
      sender->send(
          selected(this->isMeterSelected(typeKey.c_str(), meter_type.c_str())));
      sender->send(">");
      sender->send(meter_type.c_str());
      sender->send("</option>");
    }
    sender->send("</select>");
    sender->send("</div>");
    // form-field END

    // KEY
    char keyBuf[33] = {};
    std::string keyKey = "meter_key_" + std::to_string(i);
    this->getMeterKey(keyKey.c_str(), keyBuf);
    // form-field START
    sender->send("<div class=\"form-field\" style=\"border-top:0px\">");
    sender->sendLabelFor(keyKey.c_str(), "key");
    sender->send("<input maxlength=\"32\"");
    sender->sendNameAndId(keyKey.c_str());
    sender->send("value=\"");
    sender->send(keyBuf);
    sender->send("\">");
    sender->send("</div>");
    // form-field END

    // SENSOR
    std::vector<std::string> all_sensors{
        //  "rssi_dbm",
        "total_water_m3",
        "last_month_total_water_m3",
        "current_month_total_water_l",
        "total_energy_consumption_kwh",
        "current_power_consumption_kw",
        "total_energy_production_kwh",
        "current_power_production_kw",
        "current_hca",
        "previous_hca",
        "temp_room_avg_c",
        "total_heating_kwh",
        "current_heating_kwh",
        "previous_heating_kwh",
        "total_gas_m3",
        "flow_temperature_c",
        "return_temperature_c",
        "voltage_at_phase_1_v",
        "voltage_at_phase_2_v",
        "voltage_at_phase_3_v",
        "transmit_period_s",
        "remaining_battery_life_y",
        "current_alarms",
        "previous_alarms",
    };

    // form-field BEGIN
    sender->send("<div class=\"form-field\" style=\"border-top:0px\">");
    std::string sensorKey = "meter_sensor_" + std::to_string(i);
    sender->sendLabelFor(sensorKey.c_str(), "sensor");
    sender->send("<select ");
    sender->sendNameAndId(sensorKey.c_str());
    sender->send(">");
    for (const auto &meter_sensor : all_sensors) {
      sender->send("<option value=\"");
      sender->send(meter_sensor.c_str());
      sender->send("\"");
      sender->send(selected(
          this->isMeterSelected(sensorKey.c_str(), meter_sensor.c_str())));
      sender->send(">");
      sender->send(meter_sensor.c_str());
      sender->send("</option>");
    }
    sender->send("</select>");
    sender->send("</div>");
    // form-field END

    sender->send("</div>");

    sender->send("</div>");

    sender->send("<script>"
                 "function meterChanged(meter_id){"
                 "var myName = 'wmbus_m_';"
                 "myName += meter_id;"
                 "var myEnName = 'meter_enabled_';"
                 "myEnName += meter_id;"
                 "var e=document.getElementById(myEnName),"
                 "t=document.getElementById(myName),"
                 "show_meter=true;"
                 "if(e==null){return false;}"
                 "show_meter=e.checked;"
                 "t.style.display=show_meter?\"block\":\"none\";"
                 "}"
                 "meterChanged(1);"
                 "meterChanged(2);"
                 "meterChanged(3);"
                 "meterChanged(4);"
                 "</script>");
  }
}

bool WmbusParameters::handleResponse(const char *key, const char *value) {
  std::string myKey(key);
  myKey.pop_back();
  if (myKey.compare("meter_id_") == 0) {
    uint32_t myValue = hexStringToInt(value, strlen(value));
    this->setMeterId(key, myValue);
    return true;
  } else if (myKey.compare("meter_type_") == 0) {
    this->setMeterKey(key, value);
    return true;
  } else if (myKey.compare("meter_key_") == 0) {
    this->setMeterKey(key, value);
    return true;
  } else if (myKey.compare("meter_sensor_") == 0) {
    this->setMeterKey(key, value);
    return true;
  } else if (myKey.compare("meter_enabled_") == 0) {
    this->setMeterKey(key, value);
    return true;
    // } else if (strcmp(key, "miso_pin") == 0) {
    //   uint8_t pin = stringToUInt(value);
    //   this->cfg->setUInt8(key, pin);
    //   return true;
    // } else if (strcmp(key, "mosi_pin") == 0) {
    //   uint8_t pin = stringToUInt(value);
    //   this->cfg->setUInt8(key, pin);
    //   return true;
    // } else if (strcmp(key, "clk_pin") == 0) {
    //   uint8_t pin = stringToUInt(value);
    //   this->cfg->setUInt8(key, pin);
    //   return true;
    // } else if (strcmp(key, "cs_pin") == 0) {
    //   uint8_t pin = stringToUInt(value);
    //   this->cfg->setUInt8(key, pin);
    //   return true;
    // } else if (strcmp(key, "gdo0_pin") == 0) {
    //   uint8_t pin = stringToUInt(value);
    //   this->cfg->setUInt8(key, pin);
    //   return true;
    // } else if (strcmp(key, "gdo2_pin") == 0) {
    //   uint8_t pin = stringToUInt(value);
    //   this->cfg->setUInt8(key, pin);
    //   return true;
  }

  return false;
}

bool WmbusParameters::setMeterId(const char *meter, uint32_t meter_id) {
  return this->cfg->setUInt32(meter, meter_id);
}

int32_t WmbusParameters::getMeterId(const char *meter) {
  uint32_t result = 0;
  this->cfg->getUInt32(meter, &result);
  return result;
}

bool WmbusParameters::isMeterSelected(const char *meter,
                                      const char *meter_type) {
  // int8_t result = -1;
  char result[80] = {};
  this->cfg->getString(meter, result, 80);
  // this->cfg->getInt8(meter, &result);
  // SUPLA_LOG_INFO("isMeterSelected %s | %s | %s", meter, meter_type, result);
  if (strcmp(meter_type, result) == 0) {
    // if (meter_type == result) {
    return true;
  } else {
    return false;
  }
}

bool WmbusParameters::setMeter(const char *meter, int8_t meter_id) {
  return this->cfg->setInt8(meter, meter_id);
}

int8_t WmbusParameters::getMeter(const char *meter) {
  int8_t result = -1;
  this->cfg->getInt8(meter, &result);
  return result;
}

bool WmbusParameters::setMeterKey(const char *meter, const char *key) {
  // SUPLA_LOG_INFO("setMeterKey %s | %d", meter, strlen(key));
  if (strlen(key) > 33) {
    return false;
  }
  return this->cfg->setString(meter, key);
}

bool WmbusParameters::getMeterKey(const char *meter, char *result) {
  return this->cfg->getString(meter, result, 33);
}

}; // namespace Html
}; // namespace Supla
