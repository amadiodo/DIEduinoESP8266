#include <global.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ElegantOTA.h>
#include <WEB_page.h>

static const char *WS_URL = "/ws";
static const size_t MAX_WS_CLIENTS = 4;
// Periodic background broadcaster interval (ms).  Keeps the queues busy so
// fragmentation / tearing paths are exercised even with no user input.
static const uint32_t BG_BROADCAST_MS = 2000;
// Size of the periodic background broadcast (large enough to fragment).
static const size_t BG_BROADCAST_LEN = 4000;
static AsyncWebSocket ws(WS_URL);
AsyncWebServer server(HTTP_PORT);

// Monotonic message counter baked into deterministic payloads so a receiver
// can verify ordering / completeness across fragments and bursts.
static uint32_t g_seq = 0;

void OTA_handler() {
  ElegantOTA.loop(); 
}

// ...........................................................................
// Command parser (text frames from clients)
// ...........................................................................
static void handleCommand(AsyncWebSocketClient *client, const char *cmd) {
  char debug_buffer[100];
  while (*cmd == ' ' || *cmd == '\t') cmd++;
  if (!*cmd) return;

  // Split first token.
  const char *sp = strchr(cmd, ' ');
  size_t tokLen = sp ? (size_t)(sp - cmd) : strlen(cmd);
  // ESP8266's String has no (const char*, size_t) constructor, so build the
  // verb explicitly in a portable way.
  String verb;
  verb.reserve(tokLen);
  for (size_t i = 0; i < tokLen; i++) {
    verb.concat(cmd[i]);
  }
  const char *rest = sp ? sp + 1 : "";
  while (*rest == ' ') rest++;
  // char  aa[100];
  // sprintf(aa, "%s", verb.c_str());
  // DEBUG_USART.printf("[cmd] client #%" PRIu32 ": %s %s\n", client->id(), verb.c_str(), rest);

  sprintf(debug_buffer, "WEB command:\t\t%s", verb.c_str());
  debug(debug_buffer);

  if (verb == "status_print_stop") {
    status_print_enable = false;
    // ws.closeAll(1011, "server going away");
  } else if (verb == "status_print_100msecs") {
    status_print_enable = true;
    status_print_interval = 100;
  } else if (verb == "status_print_1sec") {
    status_print_enable = true;
    status_print_interval = 1000;
  } else if (verb == "status_print_10secs") {
    status_print_enable = true;
    status_print_interval = 10000;
  } else if (verb == "3V3_enable") {
    EPS_TLM.V3V3_enable = true;
  } else if (verb == "3V3_disable") {
    EPS_TLM.V3V3_enable = false;
  } else if (verb == "5V_enable") {
    EPS_TLM.V5V_enable = true;
  } else if (verb == "5V_disable") {
    EPS_TLM.V5V_enable = false;
  } else if (verb == "TCP_connect") {
    TCP_initialization();
  } else if (verb == "TCP_disconnect") {
    TCP_initialization();
  } else if (verb == "UDP_activation") {
    // UDP_server_activation();
  } else if (verb == "NTP_request") {
    // NTP_request();
  } else if (verb == "drop") {
    unsigned long id = strtoul(rest, nullptr, 10);
    ws.close((uint32_t)id, 1000, "dropped by command");
  } else if (verb == "status") {
    char string_buffer[100];
    sprintf(string_buffer, "WS clients: %u connected / %u in list\n", (unsigned)ws.count(), (unsigned)ws.getClients().size() );
    // DEBUG_USART.println(string_buffer);
    // Serial.printf("WS clients: %u connected / %u in list\n", (unsigned)ws.count(), (unsigned)ws.getClients().size());
    for (auto &c : ws.getClients()) {
      sprintf(string_buffer,"  id=%" PRIu32 " status=%d queueLen=%u queueFull=%d\n", c.id(), (int)c.status(), (unsigned)c.queueLen(), c.queueIsFull() ? 1 : 0);
      // DEBUG_USART.println(string_buffer);
      // Serial.printf("  id=%" PRIu32 " status=%d queueLen=%u queueFull=%d\n", c.id(), (int)c.status(), (unsigned)c.queueLen(), c.queueIsFull() ? 1 : 0);
    }
  } else {
    client->text("ERR: unknown command");
  }
}

// ...........................................................................
// WebSocket event handler
// ...........................................................................

static void onWsEvent(AsyncWebSocket *srv, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  char debug_buffer[100];
  switch (type) {
    case WS_EVT_CONNECT: {
      char buf[100];
      sprintf(debug_buffer, "HTTP client Id: %d(%d)\t%s:%u ", client->id(), (unsigned)ws.count(), client->remoteIP().toString().c_str(), client->remotePort() );
      debug(debug_buffer);

      // Serial.printf(
      //   "[ws] connect id=%" PRIu32 " from %s:%u (now %u client(s))\r\n", client->id(), client->remoteIP().toString().c_str(), client->remotePort(),
      //   (unsigned)ws.count()
      // );
      // Enable application-level keepalive: every 10s the library will emit a
      // ping with the AWSC_PING_PAYLOAD; the pong reply exercises the new
      // pong-data forwarding path added in PR #462.
      client->keepAlivePeriod(10);
      // Probe the new client immediately: a small ping exercises control-frame
      // queuing on a fresh connection.
      uint32_t t = micros();
      client->ping((const uint8_t *)&t, sizeof(t));
      break;
    }
    case WS_EVT_DISCONNECT: {
        char buf[100];
        sprintf(buf, "[ws] disconnect id=%" PRIu32 " (now %u client(s))", client->id(), (unsigned)ws.count());
        // DEBUG_USART.println(buf);
        break;
    }
    case WS_EVT_ERROR: Serial.printf("[ws] error id=%" PRIu32 "\n", client->id()); break;

    case WS_EVT_PONG: {
      // PR #462 forwards the pong payload to the callback.  If it's our
      // 4-byte micros() timestamp, we can measure RTT.
      if (len == sizeof(uint32_t)) {
        uint32_t sent;
        memcpy(&sent, data, sizeof(sent));
        char buf[100];
        // sprintf(buf, "[ws] pong id=%" PRIu32 " rtt=%" PRIu32 " us", client->id(), (uint32_t)(micros() - sent));
        // DEBUG_USART.println(buf);
        // Serial.printf("[ws] pong id=%" PRIu32 " rtt=%" PRIu32 " us\r\n", client->id(), (uint32_t)(micros() - sent));
      } else {
        Serial.printf("[ws] pong id=%" PRIu32 " len=%u\r\n", client->id(), (unsigned)len);
      }
      break;
    }

    case WS_EVT_DATA:
    {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      // We only care about fully-assembled TEXT messages for commands.
      if (info->final && info->index == 0 && info->len == len && info->message_opcode == WS_TEXT) {
        // Null-terminate (library guarantees it for text frames, but be safe).
        char *text = (char *)data;
        size_t textLen = strnlen(text, len);
        text[textLen] = '\0';
        handleCommand(client, text);
      } else if (info->message_opcode == WS_BINARY) {
        // Echo binary frames back as binary, so clients can round-trip checks.
        client->binary(data, len);
      } else {
        // Log fragmented reception (useful when a client sends a >MSS frame).
        Serial.printf(
          "[ws] fragmented frame id=%" PRIu32 " num=%" PRIu32 " idx=%" PRIu64 " len=%" PRIu64 " final=%u op=%u\n", client->id(), info->num, info->index,
          info->len, info->final, info->message_opcode
        );
      }
      break;
    }

    default: break;
  }
  (void)srv;
}

// ...........................................................................
// HTTP control page
// ...........................................................................


// JSON snapshot of live clients, for curl-based monitoring.
static String clientsJson() {
  String s = "{\"count\":";
  s += (unsigned)ws.count();
  s += ",\"clients\":[";
  bool first = true;
  for (auto &c : ws.getClients()) {
    if (!first) {
      s += ",";
    }
    first = false;
    s += "{\"id\":";
    s += (unsigned)c.id();
    s += ",\"status\":";
    s += (int)c.status();
    s += ",\"queueLen\":";
    s += (unsigned)c.queueLen();
    s += "}";
  }
  s += "]}";
  return s;
}

void server_initialization() {
    // client.setBufferSize(255);
    LittleFS.begin();
    {
    File f = LittleFS.open("/template.html", "w");
    assert(f);
    f.print(htmlContenta);
    f.close();
    }

    server.serveStatic("/template.html", LittleFS, "/template.html");
    server.serveStatic("/dynamic.html", LittleFS, "/template.html").setTemplateProcessor([](const String &var) -> String {
        if (var == "seconds") {
          return String(millis()/1000.0, 3);
        }        
        if (var == "luogo") {
          char HTTP_buffer[10];
          // if(cucina_enable) sprintf(HTTP_buffer, "Cucina");
          // if(garage_enable) sprintf(HTTP_buffer, "Garage");
          return HTTP_buffer;
        }
        if (var == "ciclo") {
          return String(cucina_TLM.closed, 1);
        }
        if (var == "ritorno") {
          return String(cucina_TLM.ritorno, 1);
        }
        if (var == "12V_V") {
          return String(basement_TLM.CC_voltage, 2);
        }
        if (var == "12V_I") {
          return String(basement_TLM.CC_current, 3);
        }
        if (var == "AC_V") {
          return String(basement_TLM.AC_voltage, 1);
        }
        if (var == "AC_P") {
          return String(basement_TLM.AC_power, 1);
        }
        if (var == "AC_diff") {
          return String(basement_TLM.AC_current*1000, 1);
        }
        if (var == "esterno") {
          return String(cucina_TLM.esterno, 1);
        }
        if (var == "unixtime") {
          uint32_t unixtime = NTP_unixtime + millis()/1000.0 - epoch_millis;
          return String(unixtime);
        }
        if (var == "3V3_enable") {
          char HTTP_buffer[10];
          if(EPS_TLM.V3V3_enable) sprintf(HTTP_buffer, "ON");
          else sprintf(HTTP_buffer, "OFF");
          return HTTP_buffer;
        }
        if (var == "frigorifero") {
          char HTTP_buffer[10];
          if(cucina_TLM.AC_power > 50) sprintf(HTTP_buffer, "ON");
          else sprintf(HTTP_buffer, "OFF");
          return HTTP_buffer;
        }
        if (var == "3V3_V") {
          return String(EPS_TLM.V3V3_voltage,2);
        }
        if (var == "3V3_I") {
          return String(EPS_TLM.V3V3_voltage,2);
        }
        if (var == "5V_enable") {
          char HTTP_buffer[10];
          if(EPS_TLM.V5V_enable) sprintf(HTTP_buffer, "ON");
          else sprintf(HTTP_buffer, "OFF");
          return HTTP_buffer;
        }
        if (var == "5V_V") {
          return String(EPS_TLM.V5V_voltage,2);
        }
        if (var == "5V_I") {
          return String(EPS_TLM.V5V_voltage,2);
        }
        if (var == "EXT_V") {
          return String(EPS_TLM.EXT_voltage,2);
        }
        if (var == "EXT_I") {
          return String(EPS_TLM.EXT_current,2);
        }
        if (var == "mini") {
          char HTTP_buffer[10];
          if(cucina_TLM.mini_ON) sprintf(HTTP_buffer, "ON");
          else sprintf(HTTP_buffer, "OFF");
          return HTTP_buffer;
        }
        if (var == "casa") {
          char HTTP_buffer[10];
          if(cucina_TLM.casa_ON) sprintf(HTTP_buffer, "ON");
          else sprintf(HTTP_buffer, "OFF");
          return HTTP_buffer;
        }
        if (var == "TV") {
          char HTTP_buffer[10];
          if(cucina_TLM.TV_ON) sprintf(HTTP_buffer, "ON");
          else sprintf(HTTP_buffer, "OFF");
          return HTTP_buffer;
        }
        if (var == "VIN_V") {
          return String(EPS_TLM.VIN_voltage,2);
        }
        if (var == "VIN_I") {
          return String(EPS_TLM.VIN_current,2);
        }
        return asyncsrv::emptyString;
    });    
     
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {req->send(200, "text/html", HTML_PAGE);});
    server.on("/clients", HTTP_GET, [](AsyncWebServerRequest *req) {req->send(200, "application/json", clientsJson());});
    ws.onEvent(onWsEvent);

    // Per-client keepAlive (auto-ping) and explicit ping/pingall commands both
    // exercise the control-frame queue and the pong-data forwarding added in
    // PR #462.  keepAlive is set on each new client at connect time inside the
    // event handler.

    // Gate the number of concurrent WS clients via middleware.  This exercises
    // the upgrade path being rejected mid-handshake (HTTP 503) -- a corner case
    // for the request lifecycle.
    server.addHandler(&ws).addMiddleware([](AsyncWebServerRequest *request, ArMiddlewareNext next) {
    // ws.count() does NOT include the in-flight upgrade request yet.
        if (ws.count() >= MAX_WS_CLIENTS) {
            request->send(503, "text/plain", "WS server full");
        } 
        else next();
    });
    // server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    //   request->send(200, "text/plain", "DIEduinoWT32.");
    // });

    ElegantOTA.begin(&server);
    server.begin();
    char debug_buffer[100];
    sprintf(debug_buffer, "HTTP server\t\t%d",HTTP_PORT);
    debug(debug_buffer);
}

void text_to_web_page(char buu[]) {
    ws.textAll(buu);
}

// static AsyncWebSocketSharedBuffer makeShared(size_t len, uint32_t seq) {
//   auto buffer = std::make_shared<std::vector<uint8_t>>();
//   if (!fillDeterministic(*buffer, len, seq)) {
//     return nullptr;  // allocation failed -- caller should check
//   }
//   return buffer;
// }

// Hex-decode a space-separated hex string (e.g. "deadbeef") into out.
// Returns false on bad input.
// static bool hexDecode(const char *s, std::vector<uint8_t> &out) {
//   out.clear();
//   while (*s) {
//     while (*s == ' ' || *s == '\t') {
//       s++;
//     }
//     if (!*s) {
//       break;
//     }
//     char hi = *s++;
//     if (!*s) {
//       return false;
//     }
//     char lo = *s++;
//     auto nib = [](char c, uint8_t &v) -> bool {
//       if (c >= '0' && c <= '9') {
//         v = c - '0';
//         return true;
//       }
//       if (c >= 'a' && c <= 'f') {
//         v = c - 'a' + 10;
//         return true;
//       }
//       if (c >= 'A' && c <= 'F') {
//         v = c - 'A' + 10;
//         return true;
//       }
//       return false;
//     };
//     uint8_t h, l;
//     if (!nib(hi, h) || !nib(lo, l)) {
//       return false;
//     }
//     out.push_back((h << 4) | l);
//   }
//   return true;
// }

// ...........................................................................
// Test actions (server -> client(s))
// ...........................................................................

// Per-client masked broadcast of ONE shared buffer to every connected client. Each client gets its own random mask key; if the library ever mutates the
// shared buffer while masking, clients will receive different decoded bytes and the test fails.  (RFC 6455 5.1 forbids server-side masking, so this is
// a deliberate conformance violation -- use a tolerant client like websocat4 --ws-ignore-invalid-masks.)
// static size_t sendMaskedBroadcast(size_t len) {
//   AsyncWebSocketSharedBuffer buffer = makeShared(len, g_seq++);
//   if (!buffer) {
//     // DEBUG_USART.printf("[masked] ERROR: could not allocate %u bytes\n", (unsigned)len);
//     return 0;
//   }
//   size_t sent = 0;
//   for (auto &client : ws.getClients()) {
//     if (client.status() == WS_CONNECTED && client.message(buffer, WS_TEXT, /*mask=*/true)) {
//       sent++;
//     }
//   }
//   Serial.printf("[masked] len=%u sent to %u client(s), buffer use_count=%ld\n", (unsigned)len, (unsigned)sent, (long)buffer.use_count());
//   return sent;
// }

// Enqueue `n` text frames of `len` bytes to one client, back to back. Puts the queue under load so the "one outstanding frame" invariant and
// resumable-send logic both get exercised.  Stops early if the WS queue is full (the library drops/closes the client) or if an allocation fails (heap
// exhaustion) so we never trigger a std::bad_alloc abort.
// static void sendBurst(AsyncWebSocketClient *client, size_t n, size_t len) {
//   for (size_t i = 0; i < n; i++) {
//     AsyncWebSocketSharedBuffer buffer = makeShared(len, g_seq++);
//     if (!buffer) {
//       Serial.printf("[burst] ALLOC FAILED at %u/%u (len=%u)\n", (unsigned)i, (unsigned)n, (unsigned)len);
//       return;
//     }
//     if (!client->text(buffer)) {
//       Serial.printf("[burst] queue full at %u/%u (len=%u)\n", (unsigned)i, (unsigned)n, (unsigned)len);
//       return;
//     }
//   }
//   Serial.printf("[burst] enqueued %u x %u bytes\n", (unsigned)n, (unsigned)len);
// }

// static bool fillDeterministic(std::vector<uint8_t> &out, size_t len, uint32_t seq) {
//   char header[48];
//   const int written = snprintf(header, sizeof(header), "SEQ=%" PRIu32 " LEN=%u\n", seq, (unsigned)len);
//   const size_t headerLen = std::min((size_t)std::max(written, 0), sizeof(header));
//   // Avoid std::bad_alloc: some cores (RP2040, ESP8266) build with -fno-exceptions
//   // so resize() would abort the whole program on heap exhaustion.  Pre-check the
//   // free heap and bail out gracefully instead so the test keeps running.
//   out.clear();
//   if (len) {
// #if defined(ESP32) || defined(ESP8266) || defined(LIBRETINY)
//     if (ESP.getFreeHeap() < len + 256) {
//       return false;
//     }
// #endif
//     out.reserve(len);
//     if (out.capacity() < len) {
//       return false;
//     }
//     out.resize(len);
//   }
//   const size_t n = std::min(headerLen, len);
//   memcpy(out.data(), header, n);
//   for (size_t i = n; i < len; i++) {
//     out[i] = '0' + ((i - n) % 10);
//   }
//   return true;
// }


// Variables used for dynamic cacheable template
// static unsigned uptimeInMinutes = 0;
// static AsyncStaticWebHandler *uptimeHandler = nullptr;

// Utility function for performing that update
// static void setUptimeInMinutes(unsigned t) {
//   uptimeInMinutes = t;
//   // Update caching header with a new value as well
//   if (uptimeHandler) {
//     uptimeHandler->setLastModified();
//   }
// }