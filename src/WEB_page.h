#include <global.h>

static const char *htmlContenta PROGMEM = R"(
<!DOCTYPE html>
<html><meta http-equiv="refresh" content="1">
<body>
   <h1>DIEduinoWT32</h1>
   <h2>MCU</h2>
   Seconds: <b>%seconds%</b> &nbsp; Unixtime: <b>%unixtime%</b> &nbsp; Luogo: <b>%luogo%</b>
   
   <div class=\"row\">
      <h2>Cucina</h2>
      Mini: <b>%mini%</b> &nbsp; Casa: <b>%casa%</b> &nbsp; TV: <b>%TV%</b> &nbsp; Ciclo: <b>%ciclo%</b> <span>&#8451;</span>&nbsp; Ritorno: <b>%ritorno%</b> <span>&#8451;</span> &nbsp; Esterno: <b>%esterno% <span>&#8451;</span></b> &nbsp; Frigorifero: <b>%frigorifero%</b>
   </div>

   <div class=\"row\">
      <h2>Garage</h2>
         12 V: <b>%12V_V% V</b> &nbsp; <b>%12V_I% I</b>&nbsp; <b>%AC_V% V </b>&nbsp; <b>%AC_P% W </b>&nbsp; <b>%AC_diff% mA </b>
   </div>

   <div class=\"row\">
      <h2>EPS</h2>
      <h3>INA3221</h3>
         3V3: <b>%3V3_enable%</b> &nbsp; <b>%3V3_V% V </b> &nbsp; <b>%3V3_I% A </b> &nbsp;
         5V: <b>%5V_enable%</b> &nbsp; <b>%5V_V% V </b> &nbsp; <b>%5V_I% A </b> &nbsp;
         EXT: <b>%3V3_enable%</b> &nbsp; <b>%EXT_V% V </b> &nbsp; <b>%EXT_I% A </b>
      <h3>INA260</h3>
         Input: <b>%VIN_enable%</b> &nbsp; <b>%VIN_V% V</b> &nbsp; <b>%VIN_I% I</b>
      <h3>Switches</h3>
         Mosfet A: <b>%MOS_A_enable%</b> &nbsp; Mosfet AUX: <b>%MOS_AUX%</b> &nbsp; Mosfet BIG: <b>%MOS_BIG%</b>
   </div>
</body>
</html>
)";

static const size_t htmlContentLengtha = strlen_P(htmlContenta);

const char HTML_PAGE[] PROGMEM = "\n"
"<!DOCTYPE html>\n"
"<html>\n"
"  <head>\n"
"  <meta charset=\"utf-8\">\n"
"  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
"  <title>WebSocket Test</title>\n"
"  <style>\n"
"     body{font:14px system-ui,sans-serif;margin:6px;color:#222}\n"
"     h1{font-size:18px;margin:0 0 8px}\n"
"     button{margin:2px;padding:6px 10px;border:1px solid #888;background:#f5f5f5;border-radius:4px;cursor:pointer}\n"
"     button:hover{background:#eaeaea}\n"
// "  pre{background:#111;color:#0f0;padding:8px;overflow:auto;height:460px;border-radius:4px}\n"
"     pre{font:10px system-ui,sans-serif;background:#111;color:#0f0;padding:6px;overflow:auto;width:96%;height:300px;border-radius:6px}\n"
"     input{width:80px}\n"
"     .row{margin:6px 0}\n"
"     code{background:#eee;padding:1px 4px;border-radius:3px}\n"
"  </style>\n"
"</head>\n"
"<body>\n"
// "<h1>WebSocket Test</h1>\n"
"  <pre id=\"log\"></pre>\n"
"  <script>\n"
"     const log = document.getElementById('log');\n"
"     function out(s){log.textContent += s + \"\\n\"; log.scrollTop = log.scrollHeight;}\n"
"     const ws = new WebSocket((location.protocol==='https:'?'wss:':'ws:') + '//' + location.host + '/ws');\n"
"     ws.onopen = () => {document.getElementById('status').textContent = 'WS: open'; };\n"
"     ws.onclose = () => {document.getElementById('status').textContent = 'WS: closed'; out('disconnected');};\n"
"     ws.onerror = e => out('error: ' + JSON.stringify(e));\n"
"     ws.onmessage = e => {\n"
"        if (e.data instanceof Blob) {\n"
"           e.data.arrayBuffer().then(b => out('bin(' + b.byteLength + '): ' + Array.from(new "
"           Uint8Array(b)).slice(0,16).map(x=>x.toString(16).padStart(2,'0')).join(' ') + (b.byteLength>16?' ...':'')));\n"
"        } else {\n"
"           let s = e.data;\n"
"           if (s.length > 80) s = s.slice(0,40) + '...[' + s.length + ']...' + s.slice(-40);\n"
"              out(s);\n"
"           }\n"
"        };\n"
// "        function cmd(c){if(ws.readyState===WebSocket.OPEN){ws.send(c); out('> ' + c);} else {out('WS not open');}}\n"
"        function cmd(c){if(ws.readyState===WebSocket.OPEN){ws.send(c); ;} else {out('WS not open');}}\n"
"  </script>\n"
   "\n"
"  <div class=\"row\">\n"
"     <h3>MCU</h3>"
"     <button onclick='cmd(\"TCP_connect\")'>TCP connect</button>\n"
"     <button onclick='cmd(\"TCP_disconnect\")'>TCP disconnect</button>\n"
"     <button onclick='cmd(\"UDP_activation\")'>UDP activation</button>\n"
"     <button onclick='cmd(\"NTP_request\")'>Request NTP</button>\n"
"     <button onclick='cmd(\"cucina\")'>Cucina</button>\n"
"     <button onclick='cmd(\"garage\")'>Garage</button>\n"
"     <button onclick='cmd(\"EPS\")'>EPS</button>\n"
"  </div>\n"
"  <div class=\"row\">\n"
"     <h3>Status print</h3>"
"     <button onclick='cmd(\"close\")'>close me</button>\n"
"     <button onclick='cmd(\"status_print_stop\")'>Stop</button>\n"
"     <button onclick='cmd(\"status_print_100msecs\")'>0.1 Secs</button>\n"
"     <button onclick='cmd(\"status_print_1sec\")'>1 Sec</button>\n"
"     <button onclick='cmd(\"status_print_10secs\")'>10 Sec</button>\n"
"  </div>\n"

"  <div class=\"row\">\n"
"     <h3>Inizialization</h3>"
"     <button onclick='cmd(\"DS18B20_initialization\")'>DS18B20</button>\n"
"     <button onclick='cmd(\"INA219_initialization\")'>INA219</button>\n"
"     <button onclick='cmd(\"INA3221_initialization\")'>INA3221</button>\n"
"     <button onclick='cmd(\"INA260_initialization\")'>INA260</button>\n"
"     <button onclick='cmd(\"PZEM_initialization\")'>PZEM</button>\n"
"  </div>\n"
"  <div class=\"row\">\n"
"     <h3>EPS</h3>"
"     <button onclick='cmd(\"3V3_enable\")'>3V ON</button>\n"
"     <button onclick='cmd(\"3V3_disable\")'>3V OFF</button>\n"
"     <button onclick='cmd(\"5V_enable\")'>5V OFF</button>\n"
"     <button onclick='cmd(\"5V_disable\")'>5V ON</button>\n"
"  </div>\n"


"  <div class=\"row\">\n"
"     <b>Custom:</b>\n"
"     <input id=\"custom\" style=\"width:240px\" placeholder=\"textall 1234\">\n"
"     <button onclick='cmd(document.getElementById(\"custom\").value)'>send</button>\n"
"  </div>\n"

"  <div id=\"status\">WS: connecting...</div>\n"
// "  </div>\n"
"</body>\n"
"</html>\n";