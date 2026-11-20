#include <global.h>
#include <ESPAsyncWebServer.h>

bool TCP_serverbit_enable = true;
static AsyncClient TCPclient;
uint32_t TCP_connection_time;
bool TCP_available;

void TCP_initialization() {
  if(TCP_serverbit_enable) {
    TCPclient.onDisconnect([](void *arg, AsyncClient *client) {
      char debug_buffer[100];
      sprintf(debug_buffer, "TCP Serverbit:\t\tDisconnected");
      debug(debug_buffer);
      TCP_available = false;
    });

    TCPclient.onError([](void *arg, AsyncClient *client, int8_t error) {
      char debug_buffer[100];
      sprintf(debug_buffer, "TCP Serverbit:\t\t%s", client->errorToString(error));
      debug(debug_buffer);
    });
    TCPclient.onData([](void *arg, AsyncClient *client, void *data, size_t len) {
      KISS_decode((uint8_t *)data, len);
    });

    TCPclient.onConnect([](void *arg, AsyncClient *client) {
      TCP_available = true;
      char debug_buffer[100];
      sprintf(debug_buffer, "TCP Serverbit\t\tConeected");
      debug(debug_buffer);
      uint8_t buff[2];
      buff[0] = 0x30;
      // if(cucina_enable)buff[1] = 14;
      // if(garage_enable)buff[1] = 10;
      KISS_translate(buff, sizeof(buff), 0x90);
    });

    // client.onAck([](void *arg, AsyncClient *client, size_t len, uint32_t time) {
    //   Serial.printf("Acked %u bytes in %" PRIu32 " ms\n", len, time);
    //   assert(waitingAck >= len);
    //   waitingAck -= len;
    // });

    TCPclient.setRxTimeout(20000);
    TCPclient.setNoDelay(true);
  }
}

void TCP_send(uint8_t TCP_TX_uffer[], uint16_t size) {
  if(TCP_available) TCPclient.write((char *)TCP_TX_uffer, size);
}

void TCP_handler() {
  if(TCP_serverbit_enable) {
    if(!TCP_available) {
      if(millis() - TCP_connection_time > 2000) {
        TCP_connection_time = millis();
        TCPclient.connect(SERVERBIT_NAME, SERVERBIT_PORT);
      }
    }
    else parameters_request();
  }
}

