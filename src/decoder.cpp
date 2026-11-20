// #include <Arduino.h>
#include <global.h>

void RX_decoder(uint8_t buf[], uint16_t size, uint8_t KISS_port) {
  char debug_buffer[100];
  switch(KISS_port) {
    case 0x90:
      switch(buf[0]) {
        case 0x60:
        sprintf(debug_buffer, "MCU reset");
        debug(debug_buffer);
        ESP.restart();
        break;
      case 0x61:
        // sprintf(debug_buffer, "MCU cores: %d Model: %d Revision: %d Freq:%d",ESP.getChipCores(), ESP.getChipCores(), ESP.getChipCores(), ESP.getCpuFreqMHz());
        // debug(debug_buffer);
        break;
      case 0x62:
        sprintf(debug_buffer, "Flash mode: %s size:%d speed: %d", ESP.getFlashChipMode(), ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
        debug(debug_buffer);
        break;
      case 0x63:
        // sprintf(debug_buffer, "Heap size: %d free: %d %d",ESP.getHeapSize()/1024, ESP.getFreeHeap()/1024, buf[2]);
        // debug(debug_buffer);
      case 0x64:
        I2C_scanner();
        break;
      case 0x65:
        // NTP_request();
        break;
        // DEBUG_USART.print("Heap size: ");
        // DEBUG_USART.print(ESP.getHeapSize()/1024);
        // DEBUG_USART.print(" free: ");
        // DEBUG_USART.print(ESP.getFreeHeap()/1024);
        // DEBUG_USART.print(" used: ");
        // DEBUG_USART.print(ESP.getHeapSize() - ESP.getFreeHeap());
        // DEBUG_USART.print(" PsRAM free: ");
        // DEBUG_USART.print(ESP.getFreePsram());
        // DEBUG_USART.print(" Free sketch: ");
        // DEBUG_USART.print(ESP.getFreeSketchSpace());
        // DEBUG_USART.print(" PsRAM size: ");
        // DEBUG_USART.println(ESP.getPsramSize());
      }
      break;
    case 0x92:
      switch(buf[0]) {
        case 0x80:
          EPS_TLM.V3V3_enable = buf[1];
          break;
        case 0x81:
          EPS_TLM.V5V_enable = buf[1];
          break;
        case 0x82:
          EPS_TLM.MOS_A_enable = buf[1];
          break;
        case 0x83:
          EPS_TLM.MOS_AUX_enable = buf[1];
          break;
        case 0x84:
          EPS_TLM.MOS_BIG_enable = buf[1];
          break;
      }
      break;
    case 0xA4:
      last_beacon_time = millis();
      if(size == sizeof(labico_parameters)) {
        memcpy(&labico_parameters, buf, sizeof(labico_parameters));
        // UPDATE_parameters();
        labico_parameters_live = true;
        // DEBUG_USART.println("Parame RCVD");
      }
      break;
    case 0x7B: {
        unsigned long highWord = word(buf[40], buf[41]);
        unsigned long lowWord = word(buf[42], buf[43]);
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        const unsigned long seventyYears = 2208988800UL;
        NTP_unixtime = secsSince1900 - seventyYears;
        epoch_millis = millis()/1000.0;
        sprintf(debug_buffer, "NTP received\t\t%d", NTP_unixtime);
        debug(debug_buffer);
      }
      break;
    default:
      char debug_buffer[100];
      sprintf(debug_buffer, "Packet size %d Kiss port: %d", size, KISS_port);
      debug(debug_buffer);
      break;
  }
}
