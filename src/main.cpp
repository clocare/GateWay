#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "GateWay_Cfg.h"

/********************** Headers **************************/
#define Content_Type "application/json"

/*************** Global variables *************************/
String serverAddress = "http://obscure-temple-49041.herokuapp.com/api";
String ServerToken = "Bearer ";
int port = 80;
int httpCode;
SoftwareSerial STM_Serial(STM_SERIAL_RX, STM_SERIAL_TX); // RX , TX
String STM_CMD;
String postEmpLoging = "{\"email\": \"eman@gmail.com\" , \"password\":\"123456\"}";
StaticJsonDocument<512> doc;
DeserializationError error;
String NationalID, Password;
String getUrl;
char buffer[10];
HTTPClient http;
WiFiClient client;
/********************* local proto types ***********************/
void WIFIConfigure(void);
void SendPostRequest(String path, String contentType, String postData, bool withToken);
void SendGetRequest(String path, String contentType, bool withToken);
void Process_PaitentLogin();
void Process_EmpLogin();
void Process_PatientCheck();
void Process_Sensors();
void process_AddPatient();
/********************* Setup ****************************/
void setup()
{
  Serial.begin(115200);
  STM_Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WIFIConfigure(); 
}
/********************* Loop ****************************/
void loop()
{
  if (STM_Serial.available())
  {
    STM_CMD = STM_Serial.readStringUntil('\n');
    Serial.println();
    Serial.println(STM_CMD);
    if (STM_CMD == "CHK_ID")
    {
      Process_PatientCheck();
    }
    else if (STM_CMD == "ADD")
    {
      process_AddPatient();
    }
    else if (STM_CMD == "LOGIN_EMP")
    {
      Process_EmpLogin();
    }
    else if (STM_CMD == "LOGIN_PA")
    {
      Process_PaitentLogin();
    }
    else if (STM_CMD == "READS")
    {
      Process_Sensors();
    }
    else
    {
      // Shouldn't be here
    }
  }
}

/**************** local functions definitions ***********************/

void WIFIConfigure(void)
{
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void SendGetRequest(String path, String contentType, bool withToken)
{


  Serial.print("[HTTP] begin...\n");
  if (http.begin(client, path))
  { // HTTP

    http.addHeader("Content-Type", contentType);

    /***************************/
    if (withToken == true)
      http.addHeader("Authorization", ServerToken);

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString();
        doc.clear();
        error = deserializeJson(doc, payload);
        if (error)
          Serial.println("Couldnt extract JSON");
        else 
          serializeJsonPretty(doc , Serial);
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      String payload = http.errorToString(httpCode).c_str();
    }
    http.end();
    Serial.println("HTTP END");
  }
  else
  {
    Serial.printf("[HTTP} Unable to connect\n");
  }
}
void SendPostRequest(String path, String contentType, String postData, bool withToken)
{
  //  client.beginRequest();
  Serial.print("Path: ");
  Serial.println(path);
  Serial.print("Post Data: ");
  //Serial.println(postData);
  WiFiClient client;
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin(client, path); //HTTP
  http.addHeader("Content-Type", contentType);

  /***************************/
  if (withToken == true)
    http.addHeader("Authorization", ServerToken);
  /***************************/

  Serial.print("[HTTP] POST...\n");
  // start connection and send HTTP header and body
  httpCode = http.POST(postData);

  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      const String &payload = http.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
      doc.clear();
      error = deserializeJson(doc, payload);
    }
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void Process_PaitentLogin()
{
  doc["national_id"] = STM_Serial.readStringUntil('\n');
  doc["password"] = STM_Serial.readStringUntil('\n');

  String url = serverAddress + POST_LOGIN_PATIENT;
  String output;
  serializeJson(doc, output);
  serializeJsonPretty(doc , Serial);
  SendPostRequest(url, "application/json", output, 0);
  if (httpCode == HTTP_CODE_OK && error == 0)
  {
    STM_Serial.println("OK.");
    Serial.println("OK");
    const char *localToken = doc["api_token"];
    ServerToken = "Bearer ";
    ServerToken.concat(localToken);
    Serial.println(ServerToken);
  }
  else
  {
    STM_Serial.println("ERR");
    Serial.println("ERR");
  }
}
void Process_EmpLogin()
{
  String url = serverAddress + POST_LOGIN_EMP;
  SendPostRequest(url, "application/json", postEmpLoging, 0);
  if (error || (httpCode != HTTP_CODE_OK))
  {
    STM_Serial.println("ERR");
    Serial.println("ERR");
  }
  else
  {
    const char *localToken = doc["token"];
    ServerToken = "Bearer ";
    ServerToken.concat(localToken);
    Serial.println(ServerToken);
    STM_Serial.println("OK");
  }
}
void Process_PatientCheck()
{
  NationalID = STM_Serial.readStringUntil('\n');
  // Serialization
  getUrl = serverAddress + GET_CHECK_ID + NationalID;
  // Send get Request
  SendGetRequest(getUrl, Content_Type, true);
  // Parse Response
  // Parse JSON object
  if (error || (httpCode != HTTP_CODE_OK))
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    STM_Serial.println("ERR");
  }
  else
  {
    // Extract values
    Serial.println(F("Response:"));
    Serial.println(doc["name"].as<const char *>());
    Serial.println(doc["national_id"].as<const char *>());
    Serial.println(doc["phoneNumber"].as<const char *>());
    Serial.println(doc["height"].as<int>(), 6);
    Serial.println(doc["weight"].as<int>(), 6);
    Serial.println(doc["bloodType"].as<const char *>());
    // Send to STM
    STM_Serial.println("OK");
    STM_Serial.println(doc["name"].as<const char *>());
    STM_Serial.println(doc["national_id"].as<const char *>());
    STM_Serial.println(doc["phoneNumber"].as<const char *>());
    STM_Serial.println(doc["age"].as<int>(), 6);
    STM_Serial.println(doc["arriving_date"].as<int>(), 6);
    STM_Serial.println(doc["height"].as<int>(), 6);
    STM_Serial.println(doc["weight"].as<int>(), 6);
    STM_Serial.println(doc["bloodType"].as<const char *>());
  }
}
void process_AddPatient()
{
  doc["national_id"] = STM_Serial.readStringUntil('\n');
  doc["name"] = STM_Serial.readStringUntil('\n');
  doc["arriving_date"] = STM_Serial.readStringUntil('\n');
  STM_Serial.readBytesUntil('\n', buffer, sizeof(buffer));
  doc["phone_number"] = atoi(buffer);
  doc["age"] = STM_Serial.readStringUntil('\n');
  doc["blood_type"] = STM_Serial.readStringUntil('\n');
  STM_Serial.readBytesUntil('\n', buffer, sizeof(buffer));
  doc["height"] = atoi(buffer);
  STM_Serial.readBytesUntil('\n', buffer, sizeof(buffer));
  doc["weight"] = atoi(buffer);
  //serializeJsonPretty(doc, Serial);
  String url = serverAddress + POST_Add_PATIENT;
  String output;
  serializeJson(doc, output);
  SendPostRequest(url, "application/json", output, 1);
  if (httpCode == HTTP_CODE_OK)
  {
    STM_Serial.println("OK");
    Serial.println("OK");
  }
  else
  {
    STM_Serial.println("ERR");
    Serial.println("ERR");
  }
}
void Process_Sensors()
{
  STM_Serial.readBytesUntil('\n', buffer, sizeof(buffer));
  doc["temp"] = atof(buffer);
  STM_Serial.readBytesUntil('\n', buffer, sizeof(buffer));
  doc["spo2"] = atoi(buffer);
  STM_Serial.readBytesUntil('\n', buffer, sizeof(buffer));
  doc["heartRate"] = atoi(buffer);
  String url = serverAddress + POST_SENSORS;
  String output;
  serializeJson(doc, output);
  SendPostRequest(url, "application/json", output, 1);
  if (httpCode == HTTP_CODE_OK)
  {
    STM_Serial.println("OK.");
    Serial.println("OK");
  }
  else
  {
    STM_Serial.println("ERR");
    Serial.println("ERR");
  }
}
