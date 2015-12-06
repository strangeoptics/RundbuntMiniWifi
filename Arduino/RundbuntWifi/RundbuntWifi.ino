#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <NeoPixelBus.h>

#define DBG_OUTPUT_PORT Serial

//const char* ssid = "Rundbunt";
//const char* ssid = "HACKFFM.DE";
const char* password = "Metaspace61440Workers";
const char* host = "oros";

// Rundbunt Mini UI
String program = "one_color";
int programInt = 0;

// 0:intensity, 1:color, 2:speed, 3:saturation
int params[] = {1023, 250, 150, 1023};
int params_old[4];
float params_flt[4];
int hasDelay = false;

MDNSResponder mdns;
ESP8266WebServer server(80);
//holds the current upload
File fsUploadFile;


#define NUM_LEDS 64 // this example assumes 4 pixels, making it smaller will cause a failure
#define NUM_LEDS_ROW 8
#define pixelPin 2

NeoPixelBus strip = NeoPixelBus(NUM_LEDS, pixelPin);
HsbColor leds[NUM_LEDS_ROW][NUM_LEDS_ROW];

RgbColor black = RgbColor(0);
HsbColor hsbBlack( black );


//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  
  if(SPIFFS.exists(pathWithGz) || 
    SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    Serial.println("handleFileRead: " + path);
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    Serial.println("streamFile done");
    file.close();
    return true;
  }
  return false;
}

void handleFileUpdate(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    DBG_OUTPUT_PORT.print("Upload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("Upload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    DBG_OUTPUT_PORT.print("Upload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = server.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (!entry) break;
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  server.send(200, "text/json", output);
}

void handleRest(){
  String resturi = server.uri();
  DBG_OUTPUT_PORT.print("uri: "); DBG_OUTPUT_PORT.println(resturi);
  
  String rest = resturi.substring(resturi.indexOf('/',1));
  DBG_OUTPUT_PORT.print("rest: "); DBG_OUTPUT_PORT.println(rest);

  String service = resturi.substring(resturi.indexOf('/',1)+1,resturi.length());
  DBG_OUTPUT_PORT.print("service: "); DBG_OUTPUT_PORT.println(service);

  if(service.startsWith("state")) {
    String message = "{\"program\":\""+program+"\",\"intensity\":\""+params[0]+"\",\"color\":\""+params[1]+"\",\"speed\":\""+params[2]+"\"}";
    server.send(200, "application/json", message); 
  } else {
    String value = service.substring(service.indexOf('/')+1, service.length());
    DBG_OUTPUT_PORT.print("value: "); DBG_OUTPUT_PORT.println(value);
    if(service.startsWith("intensity")) {
      params[0] = value.toInt();
    } else
    if(service.startsWith("color")) {
      params[1] = value.toInt();
    } else
    if(service.startsWith("speed")) {
      params[2] = value.toInt();
    } else
    if(service.startsWith("saturation")) {
      params[3] = value.toInt();
    } else
    if(service.startsWith("program")) {
      program = value;
      if(program.startsWith("one_color")) {
        programInt = 0;
        params[1]++;
      } else
      if(program.startsWith("lines_vertical")) {
        programInt = 3;
      } else
      if(program.startsWith("rotor_stripes")) {
        programInt = 4;
      } else
      if(program.startsWith("plasma")) {
        programInt = 1;
      } else
      if(program.startsWith("code_rain")) {
        params[1] = 336;
        programInt = 2;
      }
    }
  }  
}

void setup(void){
  //DBG_OUTPUT_PORT.begin(115200);
  //DBG_OUTPUT_PORT.print("\n");
  //Serial.setDebugOutput(true);  
  SPIFFS.begin();
  //SPIFFS INIT
//  DBG_OUTPUT_PORT.printf("\n==== SPIFFS Info ====\n");
//  DBG_OUTPUT_PORT.printf("FS Mount: %d\n", FS.mount());
//  DBG_OUTPUT_PORT.printf("FS Size: %s\n", formatBytes(FS.size()));
//  DBG_OUTPUT_PORT.printf("FS Bytes: total: %s, used: %s\n", formatBytes(FS.totalBytes()), formatBytes(FS.usedBytes()));
//  DBG_OUTPUT_PORT.printf("FS Blocks: total: %d, free: %d, size: %s\n", FS.totalBlocks(), FS.freeBlocks(), formatBytes(FS.blockSize()));
//  DBG_OUTPUT_PORT.printf("FS Pages: allocated: %d, deleted: %d, size: %s\n", FS.allocatedPages(), FS.deletedPages(), formatBytes(FS.pageSize()));
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, type: %s, size: %s\r\n", fileName.c_str(), "file", formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\r\n");
  }
 
  //WIFI INIT
  //DBG_OUTPUT_PORT.printf("Connecting to %s\r\n", ssid);

  WiFi.mode(WIFI_AP);
  WiFi.softAP("RundBuntMini");
  //WiFi.begin(ssid, password); 
  //WiFi.softAP(ssid, password);
  //IPAddress myIP = WiFi.softAPIP();
  IPAddress myIP = WiFi.localIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  //SERVER INIT
  server.on("/rest/", HTTP_POST, handleRest);
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //called after file upload
  server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); });
  //called when a file is received inside POST data
  server.onFileUpload(handleFileUpdate);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(server.uri().startsWith("/rest/")) {
      handleRest();
    } else {
      if(!handleFileRead(server.uri()))
        server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"analog\":"+String(analogRead(A0));
    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");

  strip.Begin();
  clear();
  strip.Show();
}
 


void clear() {
  for(int i=0; i<NUM_LEDS; i++) {
    strip.SetPixelColor(i, hsbBlack);
  }
}

void matrixToLine() {
  for(uint8_t x=0; x<8; x++) {
    if ( !(x & 0x01) ) {    // even
      for(uint8_t y=0; y<8; y++) {  
        strip.SetPixelColor(x*8+y, leds[x][y]);
      }
    } else {                  // odd
      for(uint8_t y=0; y<8; y++) {  
         strip.SetPixelColor(x*8+y^7, leds[x][y]);
      }
    }
  }
}



// 0:intensity, 1:color, 2:speed, 3:saturation
void calculateParams() {
  if(params[0] != params_old[0] || params[1] != params_old[1] || params[3] != params_old[3]) {
    params_old[0] = params[0];
    params_old[1] = params[1];
    params_old[3] = params[3];
    params_flt[0] = float(map(params[0], 0, 1023, 0, 100))/100.0;
    params_flt[1] = float(map(params[1], 0, 1023, 0, 100))/100.0;
    params_flt[3] = float(map(params[3], 0, 1023, 0, 100))/100.0;
  }
}


void drawColor() {
    for(int x = 0; x < 8; x++) {
      for(int y = 0; y < 8; y++) {
        leds[x][y] = HsbColor(params_flt[1], params_flt[3], params_flt[0]);   
      }
    }
    matrixToLine();
    strip.Show();
    hasDelay = false;
}

float t = 0;
void drawLinesVertical() {
  t += float(params[2])/3072;
  float c = float(params[1])/100;
  for(int x = 0; x < 8; x++) {
    float col = pow(sin(float(x)/c+t), 2);
    for(int y = 0; y < 8; y++) {
      leds[x][y] = HsbColor(col, params_flt[3], params_flt[0]);
    }
  } 
  matrixToLine();
  strip.Show();
  hasDelay = false;
}


void drawPlasma02() {
  t += float(params[2])/2048;
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {    
      float col = sin(x*0.5+t);
            col += sin(0.5*(x*sin(t/2)+y*cos(t/3))+t);
       float cx = x - 3.5 + 5.0*sin(t/3.0);
       float cy = y - 3.5 + 5.0*cos(t/3.0);
            col += sin(sqrt(params_flt[1]*(cx*cx+cy*cy)+0.0)+t);
            col = (2.5+col)/5;
      leds[x][y] = HsbColor(col, params_flt[3], params_flt[0]);
    }
  } 
  matrixToLine();
  strip.Show();
  hasDelay = false;
}


void drawPlasma05() {
  t += 0.1;
  for(int x = 0; x < 8; x++) {    
    for(int y = 0; y < 8; y++) {
      float col = 0.5+sin((x*sin(t/2)+y*cos(t/3))+t)/2;
      leds[x][y] = HsbColor(col, params_flt[3], params_flt[0]);
    }
  } 
  matrixToLine();
  strip.Show();
  hasDelay = false;
}


int pos[8] = {0,0,0,0,0,0,0,0};
int lng[8] = {1,1,1,1,1,1,1,1};
void drawCodeRain() {
  HsbColor c_bg = HsbColor(params_flt[1],0.80,0); 
  HsbColor c_t = HsbColor(params_flt[1],0.90,0.35);
  HsbColor c_p = HsbColor(params_flt[1],1,0.70);
  
  for(uint8_t x=0; x<8; x++) {
     int h=pos[x];   
     int r=lng[x];
     if(h<0) r = r+h;
     
     for(uint8_t y=0; y<8; y++) {  
       if(h == y) {
         leds[x][y] = c_p;
       } else
       if(h < y) {   
         if(r<y) {
           leds[x][y] = c_bg;
         } else {
           leds[x][y] = c_t;            
         }
       } else {
         leds[x][y] = c_bg;
       }
     }

     if(lng[x] > -pos[x]) {
       pos[x]--;
     } else {
       pos[x] = random(8, 30);
       lng[x] = random(6,12);
     }
  }
  
  matrixToLine();
  strip.Show();
  delay(map(params[2], 0,1024, 250,50));
  hasDelay = true;
}

void loop(void){
  server.handleClient();

  calculateParams();
  switch(programInt) {
    case 0:
      drawColor();
      break;
    case 1:
      drawPlasma02();
      break;
    case 2:
      //drawPlasma05();
      drawCodeRain();
      break;
    case 3:
      drawLinesVertical();
      break;
    case 4:
      drawPlasma05();
      break;
  }
  
  if(!hasDelay) delay(50);
}
