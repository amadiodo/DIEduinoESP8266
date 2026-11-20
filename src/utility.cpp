#include <global.h>
#include <Wire.h>

uint32_t epoch_millis;
unsigned long NTP_unixtime;
bool labico_parameters_live;
uint32_t status_print_interval = 1000;
uint32_t last_beacon_time, param_rew_old_time, TLM_old_time, print_old_time;
labico_parameters_sct labico_parameters;
cucina_TLM_sct cucina_TLM;
basement_SCT basement_TLM;
EPS_sct EPS_TLM;

void debug_prefix(char* outStr) {
  time_t epoch_t = NTP_unixtime + millis()/1000.0 - epoch_millis;
  struct tm  ts;
  ts = *localtime(&epoch_t);
  char buf[80];
  char buff[100];
  // strftime(buf, sizeof(buf), "[%Y-%m-%dT%H:%M:%S]", &ts);
  strftime(buf, sizeof(buf), "[%H:%M:%S]", &ts);
  if(millis() < 100000) {
    if(millis() < 10000) {
      sprintf(buff, "%s [  %.3f]\t", buf, millis()/1000.0);
    }
    else sprintf(buff, "%s [ %.3f]\t", buf, millis()/1000.0);
  }
  else sprintf(buff, "%s [%.3f]\t", buf, millis()/1000.0);
  strcpy(outStr, buff);
}

void debug(char text[]) {
  char buffr[100];
  sprintf(buffr, "%s",text);
  // if(prefix) {
    char str[100];
    debug_prefix(str);
  //   int length = 0;
  //   while (str[length] != '\0') {
  //       length++;
  //   }
  //   DEBUG_USART.print(str);
  //   texta(str);
  // }
  // texta(text);
  // DEBUG_USART.print(text);

  // char testo[100];
  strcat(str, text);
  text_to_web_page(str);
  // DEBUG_USART.println(str);
}


void parameters_request() {
  if(millis() - param_rew_old_time > 1000) {
    param_rew_old_time = millis();
    uint8_t a[2];
    a[0] = 0x41;
    KISS_translate(a, 2, 0x16);
  }
}

void telemetry() {
  if(millis() - TLM_old_time > 1000) {
    TLM_old_time = millis();
    // DEBUG_USART.println("telem");
    // if(garage_enable) {
    //     basement_TLM.seconds = millis()/1000.0;
    //     KISS_translate((uint8_t *)&basement_TLM, sizeof(basement_TLM), 0x26);
    // }
  }
}

void status_print() {
  if(millis() - print_old_time > status_print_interval) {
    print_old_time = millis();
    if(status_print_enable) {
      char debug_buffer[100];
      // if(cucina_enable) {
      //   sprintf(debug_buffer, "Mini: %d Casa: %d Temperaure %f", cucina_TLM.mini_ON, cucina_TLM.casa_ON, cucina_TLM.closed);
      //   debug(debug_buffer);
      // }
      // if(garage_enable) {
      //   sprintf(debug_buffer, "CC V: %f %f %f", basement_TLM.AC_voltage, basement_TLM.AC_current, basement_TLM.AC_power);
      //   debug(debug_buffer);
      // }
    }
  }
}

void I2C_scanner() {
  byte error, address;
  int nDevices;
  char debug_buffer[100];
  nDevices = 0;
  Wire.begin(SDA_PIN, SCL_PIN);
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      sprintf(debug_buffer, "I2C device at:\t\t%X", address);
      debug(debug_buffer);
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      sprintf(debug_buffer, "I2C error at:\t\t%X", address);
      debug(debug_buffer);
    }    
  }
  sprintf(debug_buffer, "I2C:\t\t\t%d devices", nDevices);
  debug(debug_buffer);    
}
