#pragma once

#include <string>
#include <vector>
#include <map>

// #include "supla/channel_element.h"
#include "water_meter.h"

class Meter {
  public:
    Meter(const uint32_t id, const std::string type, const std::string key);
    uint32_t id;
    std::string type;
    std::vector<unsigned char> key{};
    std::map<std::string, Supla::Sensor::WaterMeter *> sensors_{};
    void add_sensor(std::string type, Supla::Sensor::WaterMeter *sensor) {
      this->sensors_[type] = sensor;
    };
    int char_to_int(char input);
    bool hex_to_bin(const std::string &src, std::vector<unsigned char> *target) { return hex_to_bin(src.c_str(), target); };
    bool hex_to_bin(const char* src, std::vector<unsigned char> *target);
};