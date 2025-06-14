#include <meter.h>

Meter::Meter(const uint32_t id, const std::string type, const std::string key) {
  this->id = id;
  this->type = type;
  hex_to_bin(key, &(this->key));
}

bool Meter::hex_to_bin(const char* src, std::vector<unsigned char> *target)
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

int Meter::char_to_int(char input)
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