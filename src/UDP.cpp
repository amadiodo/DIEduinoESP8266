// #include <global.h>
// #include <time.h>

// char ReplyBuffer[] = "ACK";      // a string to send back

// char timeServer[]         = "time.nist.gov";  // NTP server
// #define NTP_REQUEST_PORT      123
// #define UDP_SERVER_PORT   3500
// const int NTP_PACKET_SIZE = 48;       // NTP timestamp is in the first 48 bytes of the message
// IPAddress timeServerIP = IPAddress(208, 81, 1, 244);
// IPAddress dke = IPAddress(192, 168, 7, 10);
// byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming and outgoing packets

// AsyncUDP Udp;

// void createNTPpacket(void)
// {
//   // set all bytes in the buffer to 0
//   memset(packetBuffer, 0, NTP_PACKET_SIZE);
//   // Initialize values needed to form NTP request
//   // (see URL above for details on the packets)

//   packetBuffer[0]   = 0b11100011;   // LI, Version, Mode
//   packetBuffer[1]   = 0;     // Stratum, or type of clock
//   packetBuffer[2]   = 6;     // Polling Interval
//   packetBuffer[3]   = 0xEC;  // Peer Clock Precision

//   // 8 bytes of zero for Root Delay & Root Dispersion
//   packetBuffer[12]  = 49;
//   packetBuffer[13]  = 0x4E;
//   packetBuffer[14]  = 49;
//   packetBuffer[15]  = 52;
// }

// void sendNTPPacket(void)
// {
//   createNTPpacket();
//   //Send unicast
//   Udp.write(packetBuffer, sizeof(packetBuffer));
// }

// void parsePacket(AsyncUDPPacket packet)
// {
//   struct tm  ts;
//   char       buf[80];

//   memcpy(packetBuffer, packet.data(), sizeof(packetBuffer));
//   Serial.print("RX UDP: ");
//   Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
//   Serial.print(" From: ");
//   Serial.print(packet.remoteIP());
//   Serial.print(" poo: ");
//   Serial.print(packet.localPort());
//   Serial.print(":");
//   Serial.println(packet.remotePort());

//   RX_decoder(packetBuffer, sizeof(packetBuffer), packet.remotePort());


// }

// void NTP_request() {
//     if (Udp.connect(timeServerIP, NTP_REQUEST_PORT)) {
//         // Serial.println("UDted");

//         // Udp.onPacket([](AsyncUDPPacket packet)
//         // {
//         // parsePacket(packet);
//         // });
//       sendNTPPacket();
//       Udp.close();
//     }
// }

// void UDP_server_activation() {
//   uint16_t a = UDP_SERVER_PORT;
//   char debug_buffer[100];
//   if (Udp.listen(a)) {
//     sprintf(debug_buffer, "UDP server:\t\t%d", a);
//     debug(debug_buffer);
//     Udp.onPacket([](AsyncUDPPacket packet) {
//       // Serial.print("UDP From: ");
//       // Serial.print(packet.remoteIP());
//       // Serial.print(":");
//       // Serial.print(packet.remotePort());
//       // Serial.print(", Length: ");
//       // Serial.print(packet.length());
//       // Serial.print(", Data: ");
//       // Serial.write(packet.data(), packet.length());
//       // Serial.println();
//       // //reply to the client
//       // packet.printf("Got %u bytes of data", packet.length());
      
//       if(packet.length() == 48) {
//         memcpy(packetBuffer, packet.data(), packet.length());
//         RX_decoder(packetBuffer, packet.length(), 0x7B);
//       }
//       else {
//         memcpy(packetBuffer, packet.data() + 1, packet.length() -1);
//         RX_decoder(packetBuffer, packet.length(),packet.data()[0]);
//       }
//     });
//   }
//   else {
//     sprintf(debug_buffer, "UDP server:\t\tError");
//     debug(debug_buffer);
//   }

//     if (Udp.connect(timeServerIP, NTP_REQUEST_PORT)) {
//         // Serial.println("UDted");

//         // Udp.onPacket([](AsyncUDPPacket packet)
//         // {
//         // parsePacket(packet);
//         // });
//       sendNTPPacket();
//       Udp.close();
//     }
// }