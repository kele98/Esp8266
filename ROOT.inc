
/**
   PostHTTPClient.ino

    Created on: 21.11.2016

*/
//正则表达式头文件
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
//屏幕显示
#include <Arduino.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
/* this can be run with an emulated server on host:
        cd esp8266-core-root-dir
        cd tests/host
        make ../../libraries/ESP8266WebServer/examples/PostServer/PostServer
        bin/PostServer/PostServer
   then put your PC's IP address in SERVER_IP below, port 9080 (instead of default 80):
*/
//#define SERVER_IP "10.0.1.7:9080" // PC address with emulation on host
#ifndef STASSID
#define STASSID "kele"
#define STAPSK  "Luo21800"
#endif
//全局变量 需要的数据
String words="";
String times="";
String date="";
String payload="";
String city="";
String weather="";
String temperature="";
String url="";

void setup() {
  //设置串口波特率
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
  //得到时间等数据
  post();
  //得到天气等数据
  get();
  display();
  words = "";
  times = "";
  date = "";
  url = "";

}
void display() {
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);  // use chinese2 for all the glyphs of "你好世界"
  u8g2.begin();
  u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
Serial.println("display");
  if (times == "" && date == "" && words == "")
  {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10);
    u8g2.print("日期数据接收错误");
     u8g2.sendBuffer();
      delay(5000);
  }
  else {
   
    for(int i=0;i<50;i++)
    {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10);
      u8g2.print(times);
      u8g2.setCursor(0, 30);
      u8g2.print(date);
      u8g2.setCursor(-i*3, 50);
      u8g2.print(words);
      u8g2.sendBuffer();
       delay(25);
      };

   
  }

  if (city == "" && weather == "" && temperature == "")
  { u8g2.clearBuffer();
    u8g2.setCursor(0, 10);
    u8g2.print("天气数据接收错误");
    u8g2.sendBuffer();
     delay(5000);
  }
  else {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10);
    u8g2.print("所在城市：" + city);
    u8g2.setCursor(0, 30);
    u8g2.print("今日天气：" + weather);
    u8g2.setCursor(0, 50);
    u8g2.print("最高温度：" + temperature+"°c");
    u8g2.sendBuffer();
    delay(5000);

  }

}
//对服务器返回的字符串进行解析
void handle() {
  unsigned int iSize = payload.length();
  unsigned int i = 0;
  int iCount = 0;
  int flag = 0;
  for (i = 0; i < iSize; i++)
  {
    if (payload[i] == '&' && flag == 0) {
      flag = 1;
      continue;
    }
    if (payload[i] == '&' && flag == 1) {
      flag = 2;
      continue;
    }
    if (payload[i] == '&' && flag == 2) {
      flag = 3;
      continue;
    }
    else if (flag == 0) {
      date += payload[i];
    }
    else if (flag == 1) {
      times += payload[i];
    }

    else if (flag == 2) {
      words += payload[i];
    }
    else if (flag == 3) {
      url += payload[i];
    }
  }

}

void post() {

  // 等待wifi连接成功
  if ((WiFi.status() == WL_CONNECTED)) {
    //
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "//服务器Respose网址"); //HTTP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST("");
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        handle();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
void get() {

  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, url)) { // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          char json[payload.length() + 1];
          payload.toCharArray(json, payload.length() + 1);
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, json);
          //把城市赋值给city
          String value = doc["results"][0]["location"]["name"];
          city = value;
          String value2 = doc["results"][0]["now"]["text"];
          weather = value2;
          //解析Json得到温度
          String value1 = doc["results"][0]["now"]["temperature"];
          temperature = value1;
          delay(500);
          //name[std::atoi(code.c_str())]得到的是天气
          //Serial.println("天气：" + name[std::atoi(code.c_str())]);

          Serial.println(payload.length());
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  delay(1000);
}
