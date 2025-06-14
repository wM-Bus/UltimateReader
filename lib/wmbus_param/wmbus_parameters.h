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

#ifndef SRC_SUPLA_NETWORK_HTML_WMBUS_PARAMETERS_H_
#define SRC_SUPLA_NETWORK_HTML_WMBUS_PARAMETERS_H_

#include <supla/network/html_element.h>
#include <supla/storage/config.h>

class SuplaDeviceClass;

namespace Supla {

namespace Html {

class WmbusParameters : public HtmlElement {
 public:
  WmbusParameters();
  virtual ~WmbusParameters();
  void send(Supla::WebSender* sender) override;
  bool handleResponse(const char* key, const char* value) override;

  private:
  Supla::Config* cfg{nullptr};
  virtual bool setMeterId(const char* meter, uint32_t meter_id);
  virtual int32_t getMeterId(const char* meter);
  virtual bool isMeterSelected(const char* meter, const char* meter_type);
  virtual bool setMeter(const char* meter, int8_t meter_id);
  virtual int8_t getMeter(const char* meter);
  virtual bool setMeterKey(const char* meter, const char* key);
  virtual bool getMeterKey(const char* meter, char* result);
};

};  // namespace Html
};  // namespace Supla

#endif  // SRC_SUPLA_NETWORK_HTML_WMBUS_PARAMETERS_H_
