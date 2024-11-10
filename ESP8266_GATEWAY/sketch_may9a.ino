#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "AES.h"
#include <string.h>
#include <SoftwareSerial.h>
#include "stdint.h"


#define CHIP_ID (0X23)
#define SESSION_CONTROL (0X10)  //5A + LEN + CHIP_ID + OPERATION + 1BYTE SESSION  LEN = 3
#define PROGRAMMING_SESSION (0x03)
#define DOWNLOAD_REQUEST (0X34)  //5A + LEN + CHIP_ID + OPERATION + SIZE_CODE + TYPE(FULL/PATCH) LEN = 7
#define TRANSFER_DATA (0X36)     //5A + LEN + CHIP_ID + OPERATION + 128BYTES DATA     LEN = 130
#define TRANSFER_EXIT (0X37)     //5A + LEN + CHIP_ID + OPERATION           LEN = 2

#define START_APP_CMD				(0X31)    //5A + LEN + CHIP_ID + OPERATION            //LEN = 2

#define READ_FLASH_ADDRESS (0x40)  //5A + LEN + CHIP_ID + OPERATION + 4BYTES READ  //LEN = 6
#define ERASE_FLASH (0x41)         //5A + LEN + CHIP_ID + OPERATION             //LEN = 2
#define ROLL_BACK (0x45)           //5A + LEN + CHIP_ID + OPERATION            //LEN = 2
#define GET_CHIP_ID (0x20)         //5A + LEN + CHIP_ID + OPERATION            //LEN = 2
#define RESPONSE_TIMEOUT (5000)


typedef unsigned long long ull;

const char* ssid = "LAPTOP-VCJHES9H 4996";  // Replace with your Wi-Fi SSID
const char* password = "00000000";          // Replace with your Wi-Fi password

ESP8266WebServer server(80);
SoftwareSerial mySerial(D5, D6);  // RX, TX

ull get_value_of_add(String addr);
bool waitForResponse(uint8_t expectedLength, uint8_t expectedResponse);
bool START_SESSION_CONTROL();
bool START_DOWNLOAD_REQUEST(uint32_t code_size, uint8_t type);
bool START_TRANSFER_DATA(uint8_t* data, uint8_t len);
bool START_TRANSFER_EXIT();

bool START_READ_FLASH_ADDRESS(uint32_t address);
bool START_ERASE_FLASH();
bool START_ROLL_BACK();
bool START_GET_CHIP_ID();
bool START_APP();


const BYTE key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
BYTE data[16];
BYTE out_data[16] = { 0 };
WORD res[44];
volatile uint8_t buffer[255];
static int first_frame = 0;

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  key_expansion(res, key);
  delay(100);
  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());



  // Setup routes
  server.on("/read", HTTP_GET, handleIDRead);
  server.on("/get_chip_id", HTTP_GET, handleIDGet);
  server.on("/erase_flash", HTTP_GET, handleIDErase);
  server.on("/roll_back", HTTP_GET, handleROLLBACK);
  server.on("/start_app", HTTP_GET, handleSTART_APP);
  server.on("/update_flash", HTTP_POST, handleUpdateFlash);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}


bool waitForResponse(uint8_t expectedLength, uint8_t expectedResponse) {
  unsigned long startTime = millis();
  while (!mySerial.available() || mySerial.peek() != expectedResponse) {
    if (millis() - startTime >= RESPONSE_TIMEOUT) {
      Serial.println("Response timeout");
      return false;  // Timeout after 5 seconds
    }
    delay(10);
  }

  uint8_t response[expectedLength];
  mySerial.readBytes(response, expectedLength);

  // Handle the response
  Serial.print("Response: ");
  for (uint8_t i = 0; i < expectedLength; i++) {
    Serial.print(response[i], HEX);
    buffer[i] = response[i];
    Serial.print(" ");
  }
  Serial.println();

  return (response[0] == expectedResponse);
}

bool START_SESSION_CONTROL() {
  Serial.println("Starting session control");
  mySerial.flush();
  uint8_t sessionControl[] = { 0x5A, 0x03, CHIP_ID, SESSION_CONTROL, PROGRAMMING_SESSION };
  for (int i = 0; i < 5; i++) {
    mySerial.write(sessionControl[i]);
    if(i<4)delay(10);
  }
  //mySerial.write(sessionControl, sizeof(sessionControl));
  return waitForResponse(1, SESSION_CONTROL + 0X40);
}

bool START_DOWNLOAD_REQUEST(uint32_t code_size, uint8_t type) {
  //CODE SIZE WILL SEND FIRST BYTE THEN SECOND BYTE THEN THIRD BYTE THEN FOURTH BYTE THEN TYPE
  Serial.println("Starting download request");
  mySerial.flush();
  uint8_t* ptr_2_size = (uint8_t*)&code_size;
  uint8_t downloadRequest[] = { 0x5A, 0x07, CHIP_ID, DOWNLOAD_REQUEST, ptr_2_size[0], ptr_2_size[1], ptr_2_size[2], ptr_2_size[3], type };
  for (int i = 0; i < 9; i++) {
    mySerial.write(downloadRequest[i]);
    if(i<8)delay(10);
  }
  //mySerial.write(downloadRequest, sizeof(downloadRequest));
  return waitForResponse(1, DOWNLOAD_REQUEST + 0X40);
}

bool START_TRANSFER_DATA(uint8_t* data, uint8_t len) {
  Serial.println("Starting transfer data");
  mySerial.flush();
  uint8_t transferData[132];
  transferData[0] = 0x5A;
  transferData[1] = 0x82;
  transferData[2] = CHIP_ID;
  transferData[3] = TRANSFER_DATA;
  int i = 0;
  for (i = 0; i < len; i++) {
    transferData[i + 4] = data[i];
  }
  if (len < 128) {
    for (i; i < 128; i++) {
      transferData[i + 4] = 0xff;
    }
  }
  for (int i = 0; i < 132; i++) {
    mySerial.write(transferData[i]);
    if(i<131)delay(1);
  }
  //mySerial.write(transferData, sizeof(transferData));
  return waitForResponse(1, TRANSFER_DATA + 0X40);
}

bool START_TRANSFER_EXIT() {
  Serial.println("Starting transfer exit");
  mySerial.flush();
  uint8_t transferExit[] = { 0x5A, 0x02, CHIP_ID, TRANSFER_EXIT };
  for (int i = 0; i < 4; i++) {
    mySerial.write(transferExit[i]);
    if(i<3)delay(5);
  }
  //mySerial.write(transferExit, sizeof(transferExit));
  return waitForResponse(1, TRANSFER_EXIT + 0X40);
}

bool START_READ_FLASH_ADDRESS(uint32_t address) {
  Serial.println("Starting read flash address");
  mySerial.flush();
  uint8_t* ptr_2_address = (uint8_t*)&address;
  Serial.println("Address: " + String(ptr_2_address[0], HEX) + " " + String(ptr_2_address[1], HEX) + " " + String(ptr_2_address[2], HEX) + " " + String(ptr_2_address[3], HEX));
  uint8_t readFlashAddress[] = { 0x5A, 0x06, CHIP_ID, READ_FLASH_ADDRESS, ptr_2_address[0], ptr_2_address[1], ptr_2_address[2], ptr_2_address[3] };
  for (int i = 0; i < 8; i++) {
    mySerial.write(readFlashAddress[i]);
    if(i<7)delay(10);
  }
  //mySerial.write(readFlashAddress, sizeof(readFlashAddress));
  return waitForResponse(5, READ_FLASH_ADDRESS + 0X40);
}

bool START_ERASE_FLASH() {
  Serial.println("Starting erase flash");
  mySerial.flush();
  uint8_t eraseFlash[] = { 0x5A, 0x02, CHIP_ID, ERASE_FLASH };
  for (int i = 0; i < 4; i++) {
    mySerial.write(eraseFlash[i]);
    if(i<3)delay(10);
  }
  //mySerial.write(eraseFlash, sizeof(eraseFlash));
  return waitForResponse(1, ERASE_FLASH + 0X40);
}

bool START_ROLL_BACK() {
  Serial.println("Starting roll back");
  mySerial.flush();
  uint8_t rollBack[] = { 0x5A, 0x02, CHIP_ID, ROLL_BACK };
  for (int i = 0; i < 4; i++) {
    mySerial.write(rollBack[i]);
    if(i<3)delay(10);
  }
  //mySerial.write(rollBack, sizeof(rollBack));
  return waitForResponse(1, ROLL_BACK + 0X40);
}

bool START_GET_CHIP_ID() {
  Serial.println("Starting get chip id");
  mySerial.flush();
  uint8_t getChipID[] = { 0x5A, 0x02, CHIP_ID, GET_CHIP_ID };
  for (int i = 0; i < 4; i++) {
    mySerial.write(getChipID[i]);
    if(i<3)delay(10);
  }
  //mySerial.write(getChipID, sizeof(getChipID));
  return waitForResponse(3, GET_CHIP_ID + 0X40);
}

bool START_APP() {
  Serial.println("Starting app");
  mySerial.flush();
  uint8_t startApp[] = { 0x5A, 0x02, CHIP_ID, START_APP_CMD };
  for (int i = 0; i < 4; i++) {
    mySerial.write(startApp[i]);
    if(i<3)delay(10);
  }
  //mySerial.write(startApp, sizeof(startApp));
  return waitForResponse(1, START_APP_CMD + 0X40);
}







//NOW WE NEED TO ENCRYPT THE DATA BEFORE SENDING IT TO THE SERVER
String enc_string(String data_BEFORE) {
  String output = "";
  int len = data_BEFORE.length();
  int append_null = len % 16 ? 16 - (len % 16) : 0;
  if (append_null != 0) {
    for (int i = 0; i < append_null; i++) {
      data_BEFORE += '\0';
    }
  }
  memset(out_data, 0, 16);
  for (int i = 0; i < data_BEFORE.length(); i += 16) {
    aes128_enc(out_data, (BYTE*)&data_BEFORE[i], res);
    for (int j = 0; j < 16; j++) {
      output += (char)out_data[j];
    }
  }
  return output;
}
//BEFORE ANY OPERATION WE NEED TO CALL THE SESSION CONTROL FUNCTION
void handleIDRead() {
  mySerial.flush();
  if (server.hasArg("address") && server.hasArg("ID_DEVICE")) {
    Serial.println("Handling ID_READ request");
    String address = server.arg("address");
    String ID_DEVICE = server.arg("ID_DEVICE");

    Serial.println("Handling read_address request for address: " + address + " from device: " + ID_DEVICE);
    ull value = get_value_of_add(address);
    ull ret = 0xffffffff;
    Serial.println(value, HEX);
    if (START_SESSION_CONTROL() || START_SESSION_CONTROL() ) {
      Serial.println("Session control started successfully");
      delay(100);
      if (START_READ_FLASH_ADDRESS(value)) {
        Serial.println("Read flash address started successfully");
        ret = (buffer[1] | buffer[2] << 8 | buffer[3] << 16 | buffer[4] << 24);
        Serial.println("Value at address " + address + " is: " + String(ret, HEX));
        DynamicJsonDocument responseJson(200);
        responseJson["operation"] = "ID_READ";
        responseJson["ID_DEVICE"] = ID_DEVICE;
        responseJson["data"] = String(ret, HEX);

        String responseStr;
        serializeJson(responseJson, responseStr);
        Serial.println(responseStr);
        server.send(200, "application/json", "{\"encrypted\":\"" + enc_string(responseStr) + "\"}");
        return;
      }
    }
    server.send(400, "application/json", "{\"error\":\"Error in reading flash address\"}");
  } else {
    Serial.println("ERROR IN ID_READ REQUEST");
    server.send(400, "application/json", "{\"error\":\"Missing address or ID_DEVICE parameter\"}");
  }
}

void handleSTART_APP()
{
  mySerial.flush();
  if (server.hasArg("ID_DEVICE")) {
    Serial.println("Handling START_APP request");
    String ID_DEVICE = server.arg("ID_DEVICE");
    Serial.println("Handling START_APP request for device: " + ID_DEVICE);
    String status = "FAILED";
    if (START_SESSION_CONTROL() || START_SESSION_CONTROL()) {
      Serial.println("Session control started successfully");
      delay(100);
      if (START_APP()) {
        Serial.println("START_APP started successfully");
        status = "SUCCESS";
        DynamicJsonDocument responseJson(200);
        responseJson["operation"] = "START_APP";
        responseJson["ID_DEVICE"] = ID_DEVICE;
        responseJson["status"] = status;
        String responseStr;
        serializeJson(responseJson, responseStr);
        Serial.println(responseStr);
        server.send(200, "application/json", "{\"encrypted\":\"" + enc_string(responseStr) + "\"}");
      }
    } else {
      Serial.println("Error in starting session control");
      server.send(400, "application/json", "{\"error\":\"Error in starting session control\"}");
    }
  } else {
    Serial.println("ERROR IN START_APP REQUEST");
    server.send(400, "application/json", "{\"error\":\"Missing ID_DEVICE parameter\"}");
  }
}

void handleIDGet() {
  mySerial.flush();
  if (server.hasArg("ID_DEVICE")) {
    Serial.println("Handling ID_GET request");
    String ID_DEVICE = server.arg("ID_DEVICE");

    if (START_SESSION_CONTROL() ||  START_SESSION_CONTROL() ) {
      delay(100);
      Serial.println("Session control started successfully");
      if (START_GET_CHIP_ID()) {
        Serial.println("Get chip id started successfully");
        uint32_t chip_id = (buffer[1] | buffer[2] << 8);
        delay(10);
        Serial.println("Chip ID: " + String(chip_id, HEX));

        DynamicJsonDocument responseJson(200);
        responseJson["operation"] = "ID_GET";
        responseJson["ID_DEVICE"] = String(chip_id, HEX);

        String responseStr;
        serializeJson(responseJson, responseStr);
        Serial.println(responseStr);
        server.send(200, "application/json", "{\"encrypted\":\"" + enc_string(responseStr) + "\"}");
        return;
      }
    }
    Serial.println("Error in getting chip id");
    server.send(400, "application/json", "{\"error\":\"Error in getting chip id\"}");
  } else {
    Serial.println("ERROR IN ID_GET REQUEST");
    server.send(400, "application/json", "{\"error\":\"Missing ID_DEVICE parameter\"}");
  }
}

void handleIDErase() 
{
  mySerial.flush();
  if (server.hasArg("ID_DEVICE")) {
    Serial.println("Handling ID_ERASE request");
    String ID_DEVICE = server.arg("ID_DEVICE");
    Serial.println("Handling ID_ERASE request for device: " + ID_DEVICE);
    String status = "FAILED";
    if (START_SESSION_CONTROL() || START_SESSION_CONTROL()) {
      delay(100);
      Serial.println("Session control started successfully");
      if (START_ERASE_FLASH()) {
        Serial.println("Erase flash started successfully");
        delay(10);
        status = "SUCCESS";
        DynamicJsonDocument responseJson(200);
        responseJson["operation"] = "ID_ERASE";
        responseJson["ID_DEVICE"] = ID_DEVICE;
        responseJson["status"] = status;
        String responseStr;
        serializeJson(responseJson, responseStr);
        Serial.println(responseStr);
        server.send(200, "application/json", "{\"encrypted\":\"" + enc_string(responseStr) + "\"}");
      }
    } else {
      Serial.println("Error in starting session control");
      server.send(400, "application/json", "{\"error\":\"Error in starting session control\"}");
    }
  } else {
    Serial.println("ERROR IN ID_ERASE REQUEST");
    server.send(400, "application/json", "{\"error\":\"Missing ID_DEVICE parameter\"}");
  }
}

void handleROLLBACK() 
{
  mySerial.flush();
  if (server.hasArg("ID_DEVICE")) {
    Serial.println("Handling ROLLBACK request");
    String ID_DEVICE = server.arg("ID_DEVICE");
    Serial.println("Handling ROLLBACK request for device: " + ID_DEVICE);
    String status = "FAILED";
    if (START_SESSION_CONTROL() || START_SESSION_CONTROL()) {
      Serial.println("Session control started successfully");
      delay(100);
      if (START_ROLL_BACK()) {
        Serial.println("Roll back started successfully");
        status = "SUCCESS";
        DynamicJsonDocument responseJson(200);
        responseJson["operation"] = "ROLLBACK";
        responseJson["ID_DEVICE"] = ID_DEVICE;
        responseJson["status"] = status;
        String responseStr;
        serializeJson(responseJson, responseStr);
        server.send(200, "application/json", "{\"encrypted\":\"" + enc_string(responseStr) + "\"}");
      }
    } else {
      Serial.println("Error in starting session control");
      server.send(400, "application/json", "{\"error\":\"Error in starting session control\"}");
    }
  } else {
    Serial.println("ERROR IN ROLLBACK REQUEST");
    server.send(400, "application/json", "{\"error\":\"Missing ID_DEVICE parameter\"}");
  }
}

/*
void handleUpdateFlash() {
  if(server.method()==HTTP_POST)
  {
    Serial.println("Handling UPDATE_FLASH request");
    String ID_DEVICE = server.arg("ID_DEVICE");
    String ID_OPERATION = server.arg("ID_OPERATION");
    String CODESIZE = server.arg("CODESIZE");
    int FRAME_NUM = server.arg("FRAME_NUM").toInt();
    int NUM_FRAMES = server.arg("NUM_FRAMES").toInt();
    if (!ID_DEVICE.length() || !ID_OPERATION.length() || !CODESIZE.length()) {
      server.send(400, "application/json", "{\"error\":\"Missing ID_DEVICE, ID_OPERATION, or CODESIZE parameter\"}");
      return;
    }

    Serial.printf("Received firmware frame %d/%d\n", FRAME_NUM, NUM_FRAMES);

    int contentLength = server.arg("plain").length();
    if (contentLength != 1024) {
      server.send(400, "application/json", "{\"error\":\"Invalid frame size. Expected 1024 bytes.\"}");
      return;
    }

    String firmwareData = server.arg("plain");
    Serial.println("firmware" + firmwareData);
    DynamicJsonDocument responseJson(200);
    responseJson["status"] = "Frame received successfully";
    server.send(200, "application/json", "{\"OK\":\"Method Allowed\"}");
  } else {
    server.send(405, "application/json", "{\"error\":\"Method Not Allowed\"}");
  }
}*/

/*
void handleUpdateFlash() {
  if (server.method() == HTTP_POST) {
    Serial.println("Handling UPDATE_FLASH request");
  if( server.hasArg("ID_DEVICE"))
  {
    String ID_DEVICE = server.arg("ID_DEVICE");
    Serial.println("ID_DEVICE : " + ID_DEVICE);
    Serial.println("ID_OPERATION : " + server.arg("ID_OPERATION"));
    Serial.println("CODESIZE : " + server.arg("CODESIZE"));
    Serial.println("FRAME_NUM : " + server.arg("FRAME_NUM"));
    String response = "{\"data correct\":\"" + String("data data") + "\"}";
    server.send(200, "application/json", response);    
    return;
  }

    // Read the JSON body
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON payload\"}");
      return;
    }

    // Extract parameters from JSON
    if (doc.containsKey("ID_DEVICE") && doc.containsKey("ID_OPERATION") && doc.containsKey("CODESIZE") && doc.containsKey("FRAME_NUM") && doc.containsKey("TOTAL_FRAMES")) {
      String ID_DEVICE = doc["ID_DEVICE"].as<String>();
      String ID_OPERATION = doc["ID_OPERATION"].as<String>();
      String CODESIZE = doc["CODESIZE"].as<String>();
      int FRAME_NUM = doc["FRAME_NUM"];
      int TOTAL_FRAMES = doc["TOTAL_FRAMES"];

      Serial.printf("Received firmware frame %d/%d\n", FRAME_NUM, TOTAL_FRAMES);

      // Read the binary data from the request
      WiFiClient client = server.client();
      uint8_t firmwareData[1024];
      int bytesRead = 0;
      while (client.available() && bytesRead < 1024) {
        firmwareData[bytesRead++] = client.read();
      }

      if (bytesRead != 1024) {
        server.send(400, "application/json", "{\"error\":\"Invalid frame size. Expected 1024 bytes.\"}");
        return;
      }

      // Process and write firmwareData to flash memory
      // Example: write_flash_data(firmwareData, 1024);

      // Send response
      DynamicJsonDocument responseJson(200);
      responseJson["status"] = "Frame received successfully";
      String responseStr;
      serializeJson(responseJson, responseStr);
      server.send(200, "application/json", "{\"encrypted\":\"" + enc_string(responseStr) + "\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"ID_DEVICE, ID_OPERATION, and CODESIZE parameters are required\"}");
    }
  } else {
    server.send(405, "application/json", "{\"error\":\"Method Not Allowed\"}");
  }
}
*/

void handleUpdateFlash() {
  if (server.method() == HTTP_POST) {
    Serial.println("Handling UPDATE_FLASH request");

    // Extract parameters from the request
    String ID_DEVICE = server.arg("ID_DEVICE");
    String ID_OPERATION = server.arg("ID_OPERATION");
    int CODESIZE = server.arg("CODESIZE").toInt();
    int type = server.arg("type_update").toInt();  //1 patch 2 full
    int FRAME_NUM = server.arg("FRAME_NUM").toInt();
    int TOTAL_FRAMES = server.arg("TOTAL_FRAMES").toInt();
    String plain_ = server.arg("plain");

    WiFiClient client = server.client();
    int bytesRead = 0;
    uint8_t firmwareData[1024];
    static uint8_t status_download = 0;
    static uint8_t error_download = 0;

    // Read the binary data from the request
    for (int i = 0; i < 1024 && i < plain_.length(); i++) {
      firmwareData[i] = (uint8_t)plain_[i];
      bytesRead++;
    }
    Serial.print("\n");
    if (bytesRead == 0) {
      server.send(400, "application/json", "{\"error\":\"No data received.\"}");
      Serial.println("Error: No data received.");
      return;
    }

    if (bytesRead != 1024 && FRAME_NUM != TOTAL_FRAMES) {
      server.send(400, "application/json", "{\"error\":\"Invalid frame size. Expected 1024 bytes.\"}");
      Serial.println("Error: Invalid frame size. Expected 1024 bytes.");
      return;
    }
    // Process firmwareData (e.g., write to flash memory)
    Serial.printf("Received %d bytes in this frame.\n", bytesRead);
    for (int i = bytesRead; (i % 128) != 0; i++) { firmwareData[i] = 0xff; }

    // Start session control -> start download request -> start transfer data -> start transfer exit
    if (FRAME_NUM == 1) {
      if (START_SESSION_CONTROL() ||  START_SESSION_CONTROL()) {
        Serial.println("Session control started successfully");
        delay(100);
        if (START_DOWNLOAD_REQUEST(CODESIZE, type)) {
          Serial.println("Download request started successfully");
          delay(100);
          status_download = 1;
        } else {
          Serial.println("Error in starting download request");
          server.send(400, "application/json", "{\"error\":\"Error in starting download request\"}");
          return;
        }
      } else {
        Serial.println("Error in starting session control");
        server.send(400, "application/json", "{\"error\":\"Error in starting session control\"}");
        return;
      }
    }
    if (status_download == 1) {
      for (int i = 0; i < bytesRead; i += 128) {
        if (START_TRANSFER_DATA(firmwareData + i, 128)) {
          delay(10);
        } else {
          Serial.println("Error in START_TRANSFER_DATA" + String(FRAME_NUM) + "/ " + String(TOTAL_FRAMES));
          status_download = 0;
          server.send(400, "application/json", "{\"error\":\"Error in starting transfer exit\"}");
          return;
        }
      }
      //Serial.println("Transfer data started successfully");
      if (FRAME_NUM == TOTAL_FRAMES) {
        delay(10);
        if (START_TRANSFER_EXIT()) {
          Serial.println("Transfer exit started successfully");
          status_download = 0;
          delay(10);
          mySerial.write(0xff);
          mySerial.write(0xff);
        } else {
          status_download = 0;
          Serial.println("Error in starting transfer " + String(FRAME_NUM) + "/ " + String(TOTAL_FRAMES));
          server.send(400, "application/json", "{\"error\":\"Error in starting transfer exit\"}");
          return;
        }
      }
      // Send response
      DynamicJsonDocument responseJson(200);
      responseJson["status"] = "Frame received successfully";
      String responseStr;
      serializeJson(responseJson, responseStr);
      server.send(200, "application/json", responseStr);
    } else {
      server.send(405, "application/json", "{\"error\":\"Method Not Allowed\"}");
    }
  }
}


ull get_value_of_add(String addr) {
  ull ret_val = 0, addr_val = 0;
  int digit;
  for (int i = 0; i < 8; i++) {
    char c = addr[i];
    if ('0' <= c && c <= '9') digit = c - '0';
    else if ('a' <= c && c <= 'f') digit = 10 + c - 'a';
    else if ('A' <= c && c <= 'F') digit = 10 + c - 'A';
    else return 0;

    addr_val = (addr_val << 4) + digit;
  }
  ret_val = *(ull*)(&addr_val);
  return ret_val;
}

int get_ID_DEVICE(String ID_DEVICE) {
  Serial.println("id_chip" + String(ESP.getChipId()));
  return ESP.getChipId();
}