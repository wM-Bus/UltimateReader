#pragma once

#include "esphome/core/log.h"

#include "mbus.h"
#include "utils_my.h"
#include "decode3of6.h"
#include "m_bus_data.h"

#include <string>
#include <stdint.h>

#include <RadioLib.h>

#include <vector>


#define UNUSED_PIN                   (0)

#ifdef USE_ETHERNET
#define I2C_SDA                     18
#define I2C_SCL                     17
#define OLED_RST                    UNUSED_PIN

#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              3
#define RADIO_MOSI_PIN              6
#define RADIO_CS_PIN                7

#define SDCARD_MOSI                 11
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13

#define BOARD_LED                   37
#define LED_ON                      HIGH

#define BUTTON_PIN                  0
#define ADC_PIN                     1

#define RADIO_RST_PIN               8

#define RADIO_BUSY_PIN              33      //DIO1

#define RADIO_DIO0_PIN              9
#define RADIO_DIO1_PIN              33
#define RADIO_DIO2_PIN              34
#define RADIO_DIO3_PIN              21
#define RADIO_DIO4_PIN              10
#define RADIO_DIO5_PIN              36

#define BOARD_VARIANT_NAME          "T3 S3 V1.2"
#else
#define ETH_MISO_PIN                     47
#define ETH_MOSI_PIN                     21
#define ETH_SCLK_PIN                     48
#define ETH_CS_PIN                       45
#define ETH_INT_PIN                      14
#define ETH_RST_PIN                      -1
#define ETH_ADDR                         1

#define SPI_MISO_PIN                     9
#define SPI_MOSI_PIN                     11
#define SPI_SCLK_PIN                     10

#define SD_MISO_PIN                     SPI_MISO_PIN
#define SD_MOSI_PIN                     SPI_MOSI_PIN
#define SD_SCLK_PIN                     SPI_SCLK_PIN
#define SD_CS_PIN                       12

#define I2C_SDA_PIN                     17
#define I2C_SCL_PIN                     18

#define RADIO_MISO_PIN                  SPI_MISO_PIN
#define RADIO_MOSI_PIN                  SPI_MOSI_PIN
#define RADIO_SCLK_PIN                  SPI_SCLK_PIN
#define RADIO_CS_PIN                    40
#define RADIO_RST_PIN                   46
#define RADIO_DIO0_PIN                   8
#define RADIO_DIO1_PIN                  16

#define ADC_BUTTONS_PIN                 7

#define MODEM_RX_PIN                    4
#define MODEM_TX_PIN                    6
#define MODEM_DTR_PIN                   5
#define MODEM_RI_PIN                    1
#define MODEM_PWRKEY_PIN                3

#define GPS_RX_PIN                      39
#define GPS_TX_PIN                      42

#define LED_PIN                         38
#define LED_ON                          HIGH

#define BOARD_VARIANT_NAME          "T-ETH-ELite"
#endif

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

      SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN);

      uint32_t sync_time_{0};
      uint8_t  extra_time_{20};
      uint8_t  max_wait_time_ = extra_time_;

  };

}
}
