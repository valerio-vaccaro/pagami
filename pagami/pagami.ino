#include <ArduinoJson.h>
#include <string.h>
#include "epaper.h"
#include "button.h"
#include "sd_fs.h"
#include "sound.h"
#include "net.h"
#include "ntptime.h"

File fsroot;
File logfile;

void setup() {
  // Start Serial
  Serial.begin(115200);
  Serial.println();

  // Setup Display
  Serial.println("Display setup...");
  display.init(115200); // enable diagnostic output on Serial
  display.setRotation(45);
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(GxEPD_BLACK, GxEPD_WHITE); // Set text to plot foreground and background colours
  display.setTextSize(0.5);

  // setup audio
  sound_init();

  // SD
  pinMode(LED_BUILTIN, OUTPUT);
  if (!SD.begin(SD_CS, SD_MOSI, SD_MISO, SD_CK)) {
    Serial.println("Failed to mount sd");
    Message("", "SDCard", "MOUNT FAIL", "");
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    esp_deep_sleep_start();
  }

  Message("", "SDCard OK", "", "");

  File myFile;

  myFile = SD.open("WIFI.TXT");
  if (!myFile) {
    // if the file didn't open, print an error:
    Serial.println("Error opening WIFI.TXT");
    Message("", "WIFI.TXT", "Error opening", "");
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    esp_deep_sleep_start();
  }

  StaticJsonDocument<256> root;
  DeserializationError error = deserializeJson(root, myFile);
  myFile.close();
  if (error)
  {
    Serial.println("Failed to read file, using default configuration");
    Message("", "Read file", "Failed", "");
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    esp_deep_sleep_start();
  }

  if (!root.containsKey("ssid") || !root.containsKey("password")) {
    Message("", "Missing ssid", "or password", "");
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    esp_deep_sleep_start();
  }

  String output;
  serializeJson(root, output);

  // Start WiFi Routine
  String ssid = root["ssid"];
  String password = root["password"];
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  // Set Cert
  client.setCACert(le_root_ca);
  Serial.println("Setup done");

  // time
  ntptime_init();
  ntptime_print();
}

void loop() {
  File myFile;
  char buf[100];

  myFile = SD.open("BTC.TXT");
  if (!myFile) {
    Serial.println("Error opening BTC.TXT");
    Message("", "BTC.TXT", "Error opening", "");
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    esp_deep_sleep_start();
  }

  StaticJsonDocument<1024> root;
  DeserializationError error = deserializeJson(root, myFile);
  if (error) {
    Serial.println(F("Failed to read file, using default configuration"));
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    esp_deep_sleep_start();
  }
  if (!root.containsKey("network") || !root.containsKey("addresses")) {
    Message("", "Missing network", "or addresses", "");
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    esp_deep_sleep_start();
  }

  // extract the values
  JsonArray array = root["addresses"].as<JsonArray>();
  String network_ = root["network"].as<String>();
  String myaddress;
  String unused_address;
  int first_unused_address = 1;
  bool found = false;
  long long satoshi = 0;
  long long mem_satoshi = 0;

  char network_str[20];
  if (network_ = 'testnet')
    snprintf(network_str, 20, "NET: TESTNET");
  else
    snprintf(network_str, 20, "NET: MAINNET");

  Message("", (char*)network_str, "Update balance", "");

  for (JsonVariant v : array) {
    myaddress = v.as<String>();

    if (network_ = 'testnet')
      snprintf(buf, 100, "https://blockstream.info/testnet/api/address/%s", myaddress.c_str());
    else
      snprintf(buf, 100, "https://blockstream.info/api/address/%s", myaddress.c_str());
    Serial.println(buf);

    HTTPClient http;
    http.begin(buf);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString(); //Get the response to the request
      //Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer

      StaticJsonDocument<400> http_root;
      DeserializationError http_error = deserializeJson(http_root, response);
      if (error) {
        Serial.println(F("Failed to read json from block explorer"));
      }

      byte tx_num_cha = http_root["chain_stats"]["funded_txo_count"].as<int>();
      byte tx_num_mem = http_root["mempool_stats"]["funded_txo_count"].as<int>();
      byte tx_num_tot = tx_num_cha + tx_num_mem;
      Serial.print(" tx_num_cha ");
      Serial.print(tx_num_cha);
      Serial.print(" tx_num_mem ");
      Serial.print(tx_num_mem);
      Serial.print(" tx_num_tot ");
      Serial.print(tx_num_tot);

      long long bal_in_cha = http_root["chain_stats"]["funded_txo_sum"];
      long long bal_out_cha = http_root["chain_stats"]["spent_txo_sum"];
      long long bal_tot_cha = bal_in_cha - bal_out_cha;
      char bal_tot_cha_str[20];
      snprintf(bal_tot_cha_str, 20, "%llu", bal_tot_cha);
      Serial.print(" bal_tot_cha_str ");
      Serial.print(bal_tot_cha_str);

      long long bal_in_mem = http_root["mempool_stats"]["funded_txo_sum"];
      long long bal_out_mem = http_root["mempool_stats"]["spent_txo_sum"];
      long long bal_tot_mem = bal_in_mem - bal_out_mem;
      char bal_tot_mem_str[20];
      snprintf(bal_tot_mem_str, 20, "%llu", bal_tot_mem);
      Serial.print(" bal_tot_mem_str ");
      Serial.print(bal_tot_mem_str);
      Serial.println("");
 
      satoshi += bal_tot_cha;
      mem_satoshi += bal_tot_mem;

      if (found == false && tx_num_tot == 0) {
        found = true;
        unused_address = myaddress;
      }

      if (found == false) {
        first_unused_address++;
      }

    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();  //Free resources
  }

  String progress = "Address: " + String(first_unused_address) + "/" + String(array.size());
  char satoshi_str[20];
  snprintf(satoshi_str, 20, "%llu sat.", satoshi);
  char mem_satoshi_str[20];
  snprintf(mem_satoshi_str, 20, "M %llu sat.", mem_satoshi);
  unused_address = "bitcoin:" + unused_address; // Bitcoin URI 
  QR((char *)unused_address.c_str(), network_str, (char *)progress.c_str(), satoshi_str, mem_satoshi_str);
  sound_ok();
  esp_sleep_enable_timer_wakeup(60 * 1000000);
  esp_deep_sleep_start();
}
