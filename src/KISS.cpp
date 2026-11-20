#include <global.h>

uint32_t ETH_KISS_timer;
uint8_t new_KISS_ETH_packet = 1;
uint16_t ETH_KISS_index = 0;
uint8_t ERH_KISS_escape = 0;
uint8_t ETH_KISS_packet[ETH_MAX_SIZE];

/**************************************************************************/
/*!
    @brief  Decode ethernet KISS data stream
*/
/**************************************************************************/
void KISS_decode(uint8_t KISS_array[], uint8_t RX_length) {
  for (int byte = 0; byte < RX_length; byte++) {
    uint8_t in = KISS_array[byte];
    if(millis() - ETH_KISS_timer > 100 && new_KISS_ETH_packet == 0 )  {
      // uint64_t dii = millis() - ETH_KISS_timer;
      // DEBUG_USART.print(F("KISS timeout\r\n"));
      new_KISS_ETH_packet = 1;
      ETH_KISS_index = 0;
    }
    if (ERH_KISS_escape) {
      if (in == TFEND)  {
          ETH_KISS_packet[--ETH_KISS_index] = FEND;
          ETH_KISS_index++;
      }
      if (in == TFESC)  {   
          ETH_KISS_packet[--ETH_KISS_index] = FESC;
          ETH_KISS_index++;
      }
      ERH_KISS_escape = 0;
    }
    else {		
      switch (in) {
        case FEND:
          if (new_KISS_ETH_packet)  {
            memset(ETH_KISS_packet, 0, ETH_MAX_SIZE);
            new_KISS_ETH_packet = 0;
            // printf("%s KISS start %llu %d \n", time_string , unix_milliseconds(), index_93);
            ETH_KISS_timer = millis();
            ETH_KISS_index = 0;
          }
          else  {
            new_KISS_ETH_packet = 1;
            if(ETH_KISS_index == 0) continue;
            uint8_t KISS_port = ETH_KISS_packet[0];
            uint8_t KISS_length = ETH_KISS_index-1;
            uint8_t KISS_packet[KISS_length];
            // DEBUG_USART.print(F("KISS RX port: "));
            // DEBUG_USART.print(KISS_port, HEX);
            // DEBUG_USART.print(F(" Size: "));
            memcpy(KISS_packet, ETH_KISS_packet + 1, KISS_length);
            RX_decoder(KISS_packet, KISS_length, KISS_port);
          }
          break;
        case FESC:
            ERH_KISS_escape = 1;
        default:
          ETH_KISS_packet[ETH_KISS_index++] = in;
          break;
      }
    }
  }
}

void KISS_translate(uint8_t buffer[], uint8_t size, uint8_t KISS_port) { 
  uint8_t bufere[ETH_MAX_SIZE];
  bufere[0] = FEND;
  bufere[1] = KISS_port;
  uint16_t KISS_index = 2;
  for(uint16_t n = 0 ; n < size ; n++) {
    if(buffer[n] == FEND) {
      bufere[KISS_index++] = FESC;
      bufere[KISS_index++] = TFEND;
    }
    else if(buffer[n] == FESC) {
      bufere[KISS_index++] = FESC;
      bufere[KISS_index++] = TFESC;
    }
    else bufere[KISS_index++] = buffer[n];
  }
  bufere[KISS_index++] = FEND;
  TCP_send(bufere, KISS_index);
}