#pragma once

#include <string>
#include <vector>

static char format_hex_char(uint8_t v) { return v >= 10 ? 'a' + (v - 10) : '0' + v; }
std::string format_hex(const uint8_t *data, size_t length) {
  std::string ret;
  ret.resize(length * 2);
  for (size_t i = 0; i < length; i++) {
    ret[2 * i] = format_hex_char((data[i] & 0xF0) >> 4);
    ret[2 * i + 1] = format_hex_char(data[i] & 0x0F);
  }
  return ret;
}
std::string format_hex(const std::vector<uint8_t> &data) { return format_hex(data.data(), data.size()); }

static char format_hex_pretty_char(uint8_t v) { return v >= 10 ? 'A' + (v - 10) : '0' + v; }
std::string format_hex_pretty(const uint8_t *data, size_t length) {
  if (length == 0)
    return "";
  std::string ret;
  ret.resize(3 * length - 1);
  for (size_t i = 0; i < length; i++) {
    ret[3 * i] = format_hex_pretty_char((data[i] & 0xF0) >> 4);
    ret[3 * i + 1] = format_hex_pretty_char(data[i] & 0x0F);
    if (i != length - 1)
      ret[3 * i + 2] = '.';
  }
  if (length > 4)
    return ret + " (" + std::to_string(length) + ")";
  return ret;
}
std::string format_hex_pretty(const std::vector<uint8_t> &data) { return format_hex_pretty(data.data(), data.size()); }

std::string format_hex_pretty(const uint16_t *data, size_t length) {
  if (length == 0)
    return "";
  std::string ret;
  ret.resize(5 * length - 1);
  for (size_t i = 0; i < length; i++) {
    ret[5 * i] = format_hex_pretty_char((data[i] & 0xF000) >> 12);
    ret[5 * i + 1] = format_hex_pretty_char((data[i] & 0x0F00) >> 8);
    ret[5 * i + 2] = format_hex_pretty_char((data[i] & 0x00F0) >> 4);
    ret[5 * i + 3] = format_hex_pretty_char(data[i] & 0x000F);
    if (i != length - 1)
      ret[5 * i + 2] = '.';
  }
  if (length > 4)
    return ret + " (" + std::to_string(length) + ")";
  return ret;
}
std::string format_hex_pretty(const std::vector<uint16_t> &data) { return format_hex_pretty(data.data(), data.size()); }
