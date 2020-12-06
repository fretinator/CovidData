/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <WiFi.h>
#include <HTTPClient.h>

// LCD constants
char ESC = 0xFE; // Used to issue command
const char CLS = 0x51;
const char CURSOR = 0x45;
const char LINE2_POS = 0x40;
const char LINE3_POS = 0x14;
const char LINE4_POS = 0x54;
const String NOT_FOUND = "n/a";

// WiFi constants
const char* ssid     = "Your Router";
const char* password = "Your Password";

// API constants
const char* api_endpoint = "https://covid19-api.com/country?name=USA&format=json";
const char* url = "/country?name=USA&format=json";
const char* host = "covid19-api.com";

// Loop variables
const int LOOP_DELAY = (30 * 60 * 1000); 
bool first_time = true;
 

// Value variables
String cases = "";
String deaths = "";

String parseValue(String field, String response) {
  if(field == NULL || field.length() == 0) {
    return NOT_FOUND;
  }
  
  int fieldLen = field.length();
  int fieldPos = response.indexOf(field);

  if(fieldPos < 0) {
    return NOT_FOUND;
  }

  // Value starts 3 characters after end of field - end quote, colon and space
  int valPos = fieldPos + fieldLen + 2;
  int endPos = response.indexOf(",", valPos);

  if(endPos < valPos) { // We are the last field in the collection
    endPos = response.indexOf("}", valPos);
  }

  if(endPos > valPos) {
    return response.substring(valPos, endPos);
  } else {
    return String(endPos) + "-" + String(valPos);
  }
}

// Make numbers easier to read
String convertNum(String num) {
  String ret = "";

  if(num.equals(NOT_FOUND)) {
    return num;
  }
  
  if(num.length() > 9) {
    ret = num.substring(0,num.length() - 6);
    ret = ret.substring(0, ret.length() - 3) + "." + ret.substring(ret.length() - 3) + "B";
  } else {
    if(num.length() > 6) {
      ret = num.substring(0,num.length() - 3);
      ret = ret.substring(0, ret.length() - 3) + "." + ret.substring(ret.length() - 3) + " M";      
    } else {
      if(num.length() > 3) {
        ret = num.substring(0, num.length() - 3) + "," + num.substring(num.length() - 3);
      } else {
        ret = num;
      }
    }
  }

  return ret;
}

void printScreen(const char* line1, const char* line2,
  const char* line3, const char* line4) {
  // Clear screen
  Serial2.write(ESC);
  Serial2.write(CLS);
  
  // Print line1 if present
  if(line1 != NULL) {
    Serial2.print(line1);
  }

  // Print line2 if present
  if(line2 != NULL) {
    Serial2.write(ESC);
    Serial2.write(CURSOR);
    Serial2.write(LINE2_POS);
    Serial2.print(line2);
  }

  // Print line3 if present
  if(line3 != NULL) {
    Serial2.write(ESC);
    Serial2.write(CURSOR);
    Serial2.write(LINE3_POS);
    Serial2.print(line3);
  }
  
  // Print line4 if present
  if(line4 != NULL) {
    Serial2.write(ESC);
    Serial2.write(CURSOR);
    Serial2.write(LINE4_POS);
    Serial2.print(line4);
  }

  delay(2000);
}

void setup()
{
  // Initiali
  Serial2.begin(9600);
  
  // Initialize LCD module
  Serial2.write(ESC);
  Serial2.write(0x41);
  Serial2.write(ESC);
  Serial2.write(0x51);
  
  // Set Contrast
  Serial2.write(ESC);
  Serial2.write(0x52);
  Serial2.write(40);
  
  // Set Backlight
  Serial2.write(ESC);
  Serial2.write(0x53);
  Serial2.write(8);
  
  Serial2.print(" NKC Electronics");
  
  // Set cursor line 2, column 0
  Serial2.write(ESC);
  Serial2.write(CURSOR);
  Serial2.write(LINE2_POS);
  
  Serial2.print(" 16x2 Serial LCD");
  
  delay(10);

  // We start by connecting to a WiFi network
  printScreen("Connecting to ", ssid, NULL, NULL);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial2.write(ESC);
      Serial2.write(CURSOR);
      Serial2.write(LINE3_POS);
      Serial2.print("'");
  }

  printScreen("WiFi connected", "IP address: ", WiFi.localIP().toString().c_str(), NULL);
}

int value = 0;

void loop()
{
  if(first_time) {
    first_time = false;
  }else {
    delay(LOOP_DELAY);
  }

  int err = 0;
  //WiFiClient wclient;
  HTTPClient http;
  
  // Send request
  http.useHTTP10(true);
  http.begin(api_endpoint);
  err = http.GET();

  if(err !=200) {
    printScreen("Error calling API", String(err).c_str(), NULL, NULL);
    return;
      
  }

  // Extract values
  String response = http.getString();

  if(response.length() < 20) {
    printScreen("BAD RESPONSE", response.c_str(), NULL, NULL);
  } else {
  
    String cases = parseValue("confirmed", response);
    String deaths = parseValue("deaths", response);
    String lastChange = parseValue("lastChange", response);
    cases.trim();
    deaths.trim();
    lastChange = lastChange.substring(1,17);
    cases = "Cases: " + convertNum(cases);
    deaths = "Deaths: " + convertNum(deaths);
     
    printScreen(
      cases.c_str(),
      deaths.c_str(), 
      "Last Change:",  
      lastChange.c_str());
  }
  
  // Disconnect
  http.end();
}
