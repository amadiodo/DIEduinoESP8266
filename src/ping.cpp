#include <global.h>
#include <ESP8266Ping.h>

scanner_TLM_sct scanner_TLM;
uint32_t ping_old_time;

// void ping_check() {
//   if(millis() - ping_old_time > 60000) {
//     ping_old_time = millis();
//     // DEBUG_USART.println("pi");
//     cucina_TLM.mini_ON = Ping.ping(IP_mini_eth, 1) || Ping.ping(IP_mini_WiFi, 1);
//     // DEBUG_USART.println("po");
//     cucina_TLM.casa_ON = Ping.ping(IP_casa, 1);
//     cucina_TLM.TV_ON = Ping.ping(IP_TV, 1);
//   }
// }

void ping_check () {
    if(millis() - ping_old_time > 20000) {
        ping_old_time = millis();
        for(uint8_t i = 2; i < 15; i++) {
            IPAddress IP_check = IPAddress(192, 168, 7 , i);
            scanner_TLM.IP[i] = Ping.ping(IP_check, 2);
            delay(100);
        }
    }
}

