#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/log.h"

#include "mbus.h"
#include "utils_my.h"
#include "decode3of6.h"
#include "m_bus_data.h"

#include <string>
#include <stdint.h>

#include <RadioLib.h>

#include <vector>

#ifndef CONFIG_RADIO_FREQ
#define CONFIG_RADIO_FREQ           868.95
#endif
#ifndef CONFIG_RADIO_OUTPUT_POWER
#define CONFIG_RADIO_OUTPUT_POWER   17
#endif
#ifndef CONFIG_RADIO_BW
#define CONFIG_RADIO_BW             125.0
#endif

#define WMBUS_MODE_C_PREAMBLE      0x54
#define WMBUS_BLOCK_A_PREAMBLE     0xCD
#define WMBUS_BLOCK_B_PREAMBLE     0x3D

enum RxLoopState : uint8_t {
  INIT_RX       = 0,
  WAIT_FOR_SYNC = 1,
  WAIT_FOR_DATA = 2,
  READ_DATA     = 3,
};

typedef struct RxLoopData {
  volatile int bytesRx;
  uint8_t  lengthField;         // The L-field in the WMBUS packet
  uint16_t length;              // Total number of bytes to to be read from the RX FIFO
  bool complete;                // Packet received complete
  RxLoopState state;
} RxLoopData;

namespace esphome {
namespace wmbus {

  class RxLoop {
    public:
      bool init(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs,
                uint8_t gdo0, uint8_t gdo2, float freq, bool syncMode);
      bool task();
      WMbusFrame get_frame();

    private:
      bool syncMode{false};

      WMbusData data_in{0}; // Data from Physical layer decoded to bytes
      WMbusFrame returnFrame;

      RxLoopData rxLoop;

      // flag to indicate that a packet was received
      bool receivedFlag = false;

      Module *mod{nullptr};
      RADIO_TYPE *radio{nullptr};

      uint32_t sync_time_{0};
      uint8_t  extra_time_{20};
      uint8_t  max_wait_time_ = extra_time_;

  };

}
}
