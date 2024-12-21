#include "rf_sx1276.h"

namespace esphome {
namespace wmbus {

  static const char *TAG = "rxLoop";

  volatile bool dataInFifo = false;

void setupBoards()
{
  SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
}

// this function is called when the radio receive buffer
// is full and ready to be read
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void IRAM_ATTR fifoGet(void) {
  dataInFifo = true;
}

  bool RxLoop::init(uint8_t mosi, uint8_t miso, uint8_t clk, uint8_t cs,
                    uint8_t gdo0, uint8_t gdo2, float freq, bool syncMode) {
    bool retVal = true;
    this->syncMode = syncMode;

    setupBoards();
    delay(1500);

    int state = radio.beginFSK();

    if (state != RADIOLIB_ERR_NONE) {
      return false;
    }

    radio.setFifoFullAction(fifoGet);
    radio.setFifoThreshold(0x3);
    radio.fixedPacketLengthMode(0);
    if (radio.setFrequency(CONFIG_RADIO_FREQ) == RADIOLIB_ERR_INVALID_FREQUENCY) {
      return false;
    }
    if (radio.setBandwidth(CONFIG_RADIO_BW) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
      return false;
    }
    if (radio.setOOK(false) != RADIOLIB_ERR_NONE) {
      return false;
    }
    if (radio.setFrequencyDeviation(50.0) != RADIOLIB_ERR_NONE) {
      return false;
    }
    if (radio.setBitRate(100.0) != RADIOLIB_ERR_NONE) {
      return false;
    }
    if (radio.setPreambleLength(32) != RADIOLIB_ERR_NONE) {
      return false;
    }
    uint8_t syncWord[] = {0x54, 0x3D};
    if (radio.setSyncWord(syncWord, 2) != RADIOLIB_ERR_NONE) {
      return false;
    }
    if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
      return false;
    }

    rxLoop.state = INIT_RX;

    delay(1000);

    ESP_LOGI(TAG, "Radio Starting to listen ...");
    state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE) {
      return false;
    }

    return true;
  }

  bool RxLoop::task() {
    do {
      bool reinit_needed = ((millis() - sync_time_) > max_wait_time_) ? true: false;
      if (reinit_needed) {
        rxLoop.state = INIT_RX;
      }

    switch (rxLoop.state) {
      case INIT_RX:
          // Initialize RX info variable
          rxLoop.lengthField = 0;              // Length Field in the wM-Bus packet
          rxLoop.length      = 100;            // Total length of bytes to receive packet
          rxLoop.bytesRx     = 0;              // Bytes read from Rx FIFO
          rxLoop.complete    = false;          // Packet received

          std::fill( std::begin( data_in.data ), std::end( data_in.data ), 0 );
          data_in.length      = 0;
          data_in.lengthField = 0;
          data_in.mode        = 'X';
          data_in.block       = 'X';
          // Serial.print("I");
          radio.setFifoThreshold(0x3);
          radio.standby();
          radio.startReceive();
          rxLoop.state = WAIT_FOR_DATA;
          sync_time_ = millis();
          max_wait_time_ = extra_time_;
          break;

      // waiting for enough data in Rx FIFO buffer
      case WAIT_FOR_DATA:
          if (dataInFifo) { // assert when Rx FIFO buffer threshold reached
            // Serial.print("W");
            dataInFifo = false;
            uint8_t preamble[2];
            radio.setFifoThreshold(RADIOLIB_SX127X_FIFO_THRESH);
            // Read the 3 first bytes,
            radio.fifoGet(data_in.data, 3, &rxLoop.bytesRx);
            // Serial.print(": ");
            // Serial.print(data_in.data[0], HEX);
            // Serial.print(data_in.data[1], HEX);
            // Serial.print(data_in.data[2], HEX);
            // Serial.print(data_in.data[3], HEX);
            const uint8_t *currentByte = data_in.data;
            // Mode C
            if (*currentByte == WMBUS_MODE_C_PREAMBLE) {
                currentByte++;
                data_in.mode = 'C';
                // Block A
                if (*currentByte == WMBUS_BLOCK_A_PREAMBLE) {
                  // Serial.println(" C1 A ");
                  currentByte++;
                  rxLoop.lengthField = *currentByte;
                  rxLoop.length = 2 + packetSize(rxLoop.lengthField);
                  data_in.block = 'A';
                }
                // Block B
                else if (*currentByte == WMBUS_BLOCK_B_PREAMBLE) {
                  // Serial.println(" C1 B ");
                  currentByte++;
                  rxLoop.lengthField = *currentByte;
                  rxLoop.length = 2 + 1 + rxLoop.lengthField;
                  data_in.block = 'B';
                }
                // Unknown type, reinit loop
                else {
                  // Serial.println(" C1 unknown ");
                  rxLoop.state = INIT_RX;
                  return false;
                // break;
                }
                // don't include C "preamble"
                data_in.data[0] = rxLoop.lengthField;
                rxLoop.bytesRx = 1; // ??
            }
            // Mode T Block A
            else if (decode3OutOf6(data_in.data, preamble)) {
              // Serial.print(" T1 A L: ");
              rxLoop.lengthField  = preamble[0];
              data_in.lengthField = rxLoop.lengthField;
              rxLoop.length  = byteSize(packetSize(rxLoop.lengthField));
              // Serial.print(rxLoop.length);
              // Serial.print(" ");
              data_in.mode   = 'T';
              data_in.block  = 'A';
            }
            // Unknown mode, reinit loop
            else {
              rxLoop.state = INIT_RX;
              // Serial.print(" L 3of6! ");
              // Serial.print(data_in.data[0], HEX);
              // Serial.print(data_in.data[1], HEX);
              // Serial.print(data_in.data[2], HEX);
              // Serial.println(data_in.data[3], HEX);
              return false;
                // break;
            }
            rxLoop.state = READ_DATA;
            max_wait_time_ += extra_time_;
          }
          break;

      // waiting for more data in Rx FIFO buffer
      case READ_DATA:
          if (dataInFifo) { // assert when Rx FIFO buffer threshold reached
            dataInFifo = false;
            max_wait_time_    += extra_time_;
            receivedFlag = radio.fifoGet(data_in.data, rxLoop.length, &rxLoop.bytesRx);

            // if (receivedFlag) {
            //   Serial.print("D");
            // }
            // else {
            //   Serial.print("d");
            // }
          }

          rxLoop.state = READ_DATA;
          break;
      }

      // check if the flag is set
      if (receivedFlag) {
          // Serial.println("R");
          // reset flag
          receivedFlag = false;
          data_in.length  = (uint16_t)rxLoop.bytesRx;
          data_in.lengthField = rxLoop.lengthField;

          returnFrame.frame.clear();
          returnFrame.rssi  = radio.getRSSI();
          returnFrame.lqi   = 0;
          returnFrame.mode  = 'X';
          returnFrame.block = 'X';

          if (mBusDecode(data_in, returnFrame)) {
            rxLoop.complete = true;
            returnFrame.mode  = data_in.mode;
            returnFrame.block = data_in.block;
          }
          rxLoop.state = INIT_RX;
      }
    } while ((this->syncMode) && (rxLoop.state > WAIT_FOR_DATA));

    return rxLoop.complete;
  }

  WMbusFrame RxLoop::get_frame() {
    return this->returnFrame;
  }

}
}
