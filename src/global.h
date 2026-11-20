#ifndef global_H
#define global_H

#include <Arduino.h>
// #include <SoftwareSerial.h>

// #define DEBUG_ETHERNET_WEBSERVER_PORT       Serial
// #define _ETHERNET_WEBSERVER_LOGLEVEL_       4           // Debug Level from 0 to 4


// extern SoftwareSerial SOFTWAR_DEBUG_USART;
// #define DEBUG_USART SOFTWAR_DEBUG_USART
#define PZEM_SERIAL Serial

// #define MOS_EXT_PIN 13
// #define MOS_A1_PIN 14
// #define MOS_BIG_PIN 27
// #define DS18_PIN 32
#define SCL_PIN 32
#define SDA_PIN 33
#define ETH_MAX_SIZE 128
#define FEND 0xC0
#define FESC 0xDB
#define TFEND 0xDC
#define TFESC 0xDD
#define SERVERBIT_NAME   "diegoamadio.eu"
#define SERVERBIT_PORT 8093
#define HTTP_PORT 80
#define ONE_WIRE_BUS 14
#define INA219_ADDRESS 0x40
#define INA260_ADDRESS 0x40
#define INA3221_ADDRESS 0x40
// #define PIN_EN_3V 33
// #define PIN_EN_5V 4

extern uint32_t epoch_millis;
extern unsigned long NTP_unixtime;
// extern bool cucina_enable;
// extern bool garage_enable;
extern bool status_print_enable;
extern bool labico_parameters_live;
extern uint32_t last_beacon_time;
extern bool TCP_available;
extern uint32_t status_print_interval;

void debug_prefix(char* outStr);
void OTA_handler();
void UDP_server_activation();
void status_print();
void TCP_initialization();
void TCP_handler();
void server_initialization();
// uint32_t unix_seconds();
void parameters_request();
void RX_decoder(uint8_t buf[], uint16_t size, uint8_t KISS_port);
void KISS_decode(uint8_t KISS_array[], uint8_t RX_length);
// void UDP_client_initialization();
void ping_check();
void TCP_send(uint8_t TCP_TX_uffer[], uint16_t size);
void telemetry();
void KISS_translate(uint8_t buffer[], uint8_t size, uint8_t KISS_port);
void text_to_web_page(char buu[]);
void debug(char text[]);
void I2C_scanner();
void sendNTPPacket();
// void NTP_request();
// void pins_initialization();
// void pins_initialization();

struct labico_parameters_sct {
  uint8_t TGT_temperature;
  uint8_t fan;
  uint8_t temp;
  uint8_t fan_W_min;

  uint8_t ON_cycle;
  uint8_t cycle_multipler;
  uint8_t auto_ON;
  uint8_t FAN_THR;

  uint8_t temp1;
  uint8_t frigorifero;
  uint8_t temp2;
  uint8_t temp3;
} ;
extern labico_parameters_sct labico_parameters;

struct cucina_TLM_sct {
  uint8_t boot_cause;
  uint8_t temp;
  uint8_t PZEM_error;
  uint8_t caldaia_ON;

  uint8_t TV_ON;
  uint8_t casa_ON;
  uint8_t mini_ON;
  uint8_t AC_is_nan;

  uint32_t seconds;
  float AC_power;

  float closed;
  float esterno;
  float ritorno;
  float cucina;
} ;
extern cucina_TLM_sct cucina_TLM;

struct basement_SCT {
  uint8_t boot_count;
  uint8_t boot_cause;
  uint8_t AC_is_nan;
  uint8_t PZEM_error;
  float seconds;

  float AC_power;
  float AC_voltage;
  
  float AC_current;
  float AC_PF;
  float AC_frequency;
  float CC_voltage;

  float CC_current;
  float CC_power;
} ;
extern basement_SCT basement_TLM;

struct EPS_sct {
  uint8_t boot_count;
  uint8_t RSSI;
  uint8_t MOS_AUX_enable;
  uint8_t ETH_error;
  float seconds;

  uint8_t V3V3_enable;
  uint8_t V5V_enable;
  uint8_t MOS_BIG_enable;;
  uint8_t MOS_A_enable;
  float temp;

  float V5V_voltage;
  float V5V_current;
  
  float V3V3_voltage;
  float V3V3_current;
  
  float EXT_voltage;
  float EXT_current;
  
  float VIN_voltage;
  float VIN_current;
  
  float VIN_power;
  float temperature;
};
extern EPS_sct EPS_TLM;


// struct pic_struct {
//   uint8_t id;
//   uint8_t lenght;
//   uint16_t pkt_num;
//   uint8_t pic[picture_buffer_lenght];  
// } ;
// extern pic_struct photo;

#endif