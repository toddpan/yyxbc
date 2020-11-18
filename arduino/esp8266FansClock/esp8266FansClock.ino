/* 代码修改：UP：Hans叫泽涵  bilibili UID:15481541
 *  感谢UP:会飞的阿卡林  bilibili UID:751219
 *  感谢UP:黄埔皮校大校长  bilibili UID:403928979
 *  感谢UP:F_KUN  bilibili UID:8515147
 *  感谢UP:啊泰_  bilibili UID:23106193
 *  感谢UP:硬核拆解  bilibili UID:427494870
 *  
 * 本代码适用于ESP8266 NodeMCU + 12864 SSD1306 OLED显示屏
 * 
 * 7pin SPI引脚，正面看，从左到右依次为GND、VCC、D0、D1、RES、DC、CS
 *    ESP8266 ---  OLED
 *      3V    ---  VCC
 *      G     ---  GND
 *      D7    ---  D1
 *      D5    ---  D0
 *      D2orD8---  CS
 *      D1    ---  DC
 *      RST   ---  RES
 *      
 * 4pin IIC引脚，正面看，从左到右依次为GND、VCC、SCL、SDA
 *      ESP8266  ---  OLED
 *      3.3V     ---  VCC
 *      G (GND)  ---  GND
 *      D1(GPIO5)---  SCL
 *      D2(GPIO4)---  SDA
 */
 
#include <TimeLib.h>

#include <WiFiClientSecureBearSSL.h>  
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <qrcode.h>

//https://blog.csdn.net/dpjcn1990/article/details/92831760
//https://www.iesdouyin.com/web/api/v2/user/info/?sec_uid=MS4wLjABAAAAFauf0z7gfzJVLTD6_q835WnGnLOJ47bmWR5NKu5dHQg



//如果是七线SPI的屏幕请替换成下面这行代码
//U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0,/* cs=*/4, /* dc=*/5, /* reset=*/3);

//如果是四线IIC的屏幕请替换成下面这行代码，此处 clock：14  data：2 是对应焊接的脚针
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,/* clock=*/ 14, /* data=*/ 2, /* reset=*/ U8X8_PIN_NONE);




//---------------修改此处""内的信息---------------------------------------------
const char ssid[] = "panzujiMi10";    //WiFi名
//const char ssid[] = "Xiaomi_10EE";    //WiFi名
const char pass[] = "moto1984";       //WiFi密码

//String sec_uid= "MS4wLjABAAAAFauf0z7gfzJVLTD6_q835WnGnLOJ47bmWR5NKu5dHQg";

//String sec_uid= "MS4wLjABAAAA97Xmqe9_hAusoefiv0veZyudrDXhCLrg1IJSQK_TyuM-IapFH4CcRnLM8JWJhr0S";
String sec_uid= "MS4wLjABAAAAonhiVxXz8CqcZ7UCo54jOPkKTNEGz5qVJufj5rfsfCo";
//-----------------------------------------------------------------------------

static const char ntpServerName[] = "ntp1.aliyun.com"; //NTP服务器，阿里云
const int timeZone = 8;                                //时区，北京时间为+8

const unsigned long HTTP_TIMEOUT = 30000;
WiFiClient client;
HTTPClient http;
WiFiUDP Udp;
unsigned int localPort = 8888; // 用于侦听UDP数据包的本地端口

time_t getNtpTime();
void sendNTPpacket(IPAddress& address);
void oledClockDisplay();
void sendCommand(int command, int value);
void initdisplay();

bool getJson();
bool parseJson(String json);

boolean isNTPConnected = false;

const unsigned char xing[] U8X8_PROGMEM = {
  0x00, 0x00, 0xF8, 0x0F, 0x08, 0x08, 0xF8, 0x0F, 0x08, 0x08, 0xF8, 0x0F, 0x80, 0x00, 0x88, 0x00,
  0xF8, 0x1F, 0x84, 0x00, 0x82, 0x00, 0xF8, 0x0F, 0x80, 0x00, 0x80, 0x00, 0xFE, 0x3F, 0x00, 0x00
};  /*星*/

const unsigned char liu[] U8X8_PROGMEM = { 
  0x40, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x02, 0x20, 0x04, 0x10, 0x08, 0x10, 0x10, 0x08, 0x10, 0x04, 0x20, 0x02, 0x20, 0x00, 0x00
};  /*六*/


String response;
int follower = 0;
int favorite = 0;
String nickName = "阳阳学编程";
int diffollower = 0;
int diffavorite = 0;

const int slaveSelect = 5;
const int scanLimit = 7;

int times = 0;


void setup()
{
  Serial.begin(115200);
  while (!Serial)
    continue;
  Serial.println("douyin fans NTP Clock version 1.0");
  Serial.println("by Hans");

  
  initdisplay();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setCursor(0, 14);
  u8g2.print("Waiting for WiFi");
  u8g2.setCursor(0, 30);
  u8g2.print("connection...");
  u8g2.setCursor(0, 47);
  u8g2.print("By Hans douyin");
  u8g2.setCursor(0, 64);
  u8g2.print("fans Clock v1.0");
  u8g2.sendBuffer();


  Serial.print("Connecting WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300); //每300秒同步一次时间
  isNTPConnected = true;


  // gen the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3 , ECC_LOW, "https://blog.craftyun.cn/");
}


void loop()
{
  if (times == 0) {
    if (timeStatus() != timeNotSet)
    {
        oledClockDisplay();
//        delay(2000);
//        genQrCode();
    }
  }

  if (times == 5) {
    if (WiFi.status() == WL_CONNECTED){
        if (getJson()){
            if (parseJson(response)){
                drawFlower(favorite,follower,nickName);
            }
         }
         delay(1000);
    }
  }
  
  times += 1;
  if (times >= 10) {
    times = 0;
  }
  delay(1000);
}

//显示粉丝数
void drawFlower(int favorite,int follower,String nickName )
{
      u8g2.clearBuffer();
//    u8g2.setFont(u8g2_font_unifont_t_chinese2);
//    u8g2.drawXBMP( 52 , 0 , 24 , 24 , bilibilitv_24u );     //根据你的图片尺寸修改
      u8g2.setFont(u8g2_font_wqy14_t_gb2312b);
      u8g2.setCursor(0, 14);
      u8g2.print(nickName);
      u8g2.setFont(u8g2_font_wqy14_t_gb2312b);
      u8g2.setCursor(0, 40);
      u8g2.print("粉丝:");
      u8g2.setFont(u8g2_font_unifont_t_chinese2);
      u8g2.setCursor(40, 40);
      u8g2.println(follower);
      if(diffollower > 0 ){
        u8g2.setFont(u8g2_font_wqy14_t_gb2312b);
        u8g2.setCursor(80, 40);
        u8g2.print("+");
        u8g2.setFont(u8g2_font_unifont_t_chinese2);
        u8g2.setCursor(70, 40);
        u8g2.println(diffollower);
      }

      u8g2.setFont(u8g2_font_wqy14_t_gb2312b);
      u8g2.setCursor(0, 60);
      u8g2.print("获赞:");
      u8g2.setFont(u8g2_font_unifont_t_chinese2);
      u8g2.setCursor(40, 60);
      u8g2.println(favorite);
      u8g2.sendBuffer();
}

//生成二维码，并显示
void genQrCode()
{
    // gen the QR code
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];

  qrcode_initText(&qrcode, qrcodeData, 3 , ECC_LOW, "https://blog.craftyun.cn/");

  // start draw
  u8g2.firstPage();
  do {
    // get the draw starting point,128 and 64 is screen size
    uint8_t x0 = (128 - qrcode.size * 2) / 2;
    uint8_t y0 = (64 - qrcode.size * 2) / 2;
    
    // get QR code pixels in a loop
    for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
        // Check this point is black or white
        if (qrcode_getModule(&qrcode, x, y)) {
          u8g2.setColorIndex(1);
        } else {
          u8g2.setColorIndex(0);
        }
        // Double the QR code pixels
        u8g2.drawPixel(x0 + x * 2, y0 + y * 2);
        u8g2.drawPixel(x0 + 1 + x * 2, y0 + y * 2);
        u8g2.drawPixel(x0 + x * 2, y0  + 1 + y * 2);
        u8g2.drawPixel(x0 + 1 + x * 2, y0 + 1 + y * 2);
      }
    }

  } while ( u8g2.nextPage() );
}

bool getJson()
{
    bool r = false;
    http.setTimeout(HTTP_TIMEOUT);

    const char* fingerprint = "16 34 40 b5 b0 0d 85 e0 11 84 5d b2 3e 76 7c 21 f3 31 5e 1b";
    //SHA1 Fingerprint=16:34:40:B5:B0:0D:85:E0:11:84:5D:B2:3E:76:7C:21:F3:31:5E:1B
   //followe
   //openssl s_client -connect www.iesdouyin.io:443
   //copy the certificate (from "-----BEGIN CERTIFICATE-----" to "-----END CERTIFICATE-----") and paste into a file (cert.perm).
//   openssl x509 -noout -in ./cert.perm -fingerprint -sha1
//    http.begin("https://www.iesdouyin.com/web/api/v2/user/info/?sec_uid=" + sec_uid, fingerprint);
// http.begin("http://api.bilibili.com/x/relation/stat?vmid=" + biliuid );
//http.begin("https://www.iesdouyin.com/web/api/v2/user/info/?sec_uid=MS4wLjABAAAAFauf0z7gfzJVLTD6_q835WnGnLOJ47bmWR5NKu5dHQg",fingerprint);
//    int httpCode = http.GET();
//     // HTTP header has been send and Server response header has been handled
//     Serial.printf("[HTTP] ... code: %d\n", httpCode);
//    if (httpCode > 0){
//        if (httpCode == HTTP_CODE_OK){
//
//          http.writeToStream(&Serial);
//            response = http.getString();
//        
//
//    
//            Serial.println(response);
//            r = true;
//        }
//    }else{
//        Serial.printf("[HTTP] GET JSON failed, error: %s\n", http.errorToString(httpCode).c_str());
//        r = false;
//    }
//    http.end();

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);  
//  const uint8_t fingerprint[20] = {0x16,0x34,0x40 ,0xb5 ,0xb0 ,0x0d ,0x85 ,0xe0 ,0x11 ,0x84 ,0x5d ,0xb2 ,0x3e ,0x76 ,0x7c ,0x21 ,0xf3 ,0x31 ,0x5e ,0x1b};  
//  client->setFingerprint(fingerprint);  
    client->setInsecure();  
    HTTPClient https;  
//"https://www.iesdouyin.com/web/api/v2/user/info/?sec_uid=" + sec_uid ,fingerprint);
    Serial.print("[HTTPS] begin...\n");  
    if (https.begin(*client, "https://www.iesdouyin.com/web/api/v2/user/info/?sec_uid=" + sec_uid )) {  // HTTPS  
  
      Serial.print("[HTTPS] GET...\n");  
      // start connection and send HTTP header  
      int httpCode = https.GET();  
  
      // httpCode will be negative on error  
      if (httpCode > 0) {  
        // HTTP header has been send and Server response header has been handled  
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);  
  
        // file found at server  
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {  
          response = https.getString();  
          Serial.println(response);  
          r = true;
        }  
      } else {  
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str()); 
        r = false; 
      }  
  
      https.end();  
    } else {  
      Serial.printf("[HTTPS] Unable to connect\n");  
      r = false;
    }

    return r;
}

/*
 * {
 "status_code": 0,
  "user_info": {
    "avatar_medium": {
      "uri": "tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c",
      "url_list": ["https://p6-dy-ipv6.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_720x720.jpeg?from=4010531038", "https://p3-dy-ipv6.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_720x720.jpeg?from=4010531038", "https://p9-dy.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_720x720.jpeg?from=4010531038"]
    },
    "verification_type": 1,
    "is_gov_media_vip": false,
    "aweme_count": 7,
    "following_count": 13,
    "total_favorited": "4990",
    "custom_verify": "",
    "unique_id": "dywp736vw3ai",
    "short_id": "3741241672",
    "nickname": "义乌市雯祥日用百货有限公司",
    "avatar_larger": {
      "uri": "tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c",
      "url_list": ["https://p3-dy-ipv6.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_1080x1080.jpeg?from=4010531038", "https://p26-dy.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_1080x1080.jpeg?from=4010531038", "https://p9-dy.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_1080x1080.jpeg?from=4010531038"]
    },
    "policy_version": null,
    "is_enterprise_vip": true,
    "original_musician": {
      "music_count": 0,
      "music_used_count": 0
    },
    "followers_detail": null,
    "region": "CN",
    "platform_sync_info": null,
    "secret": 0,
    "type_label": null,
    "uid": "430617207506007",
    "follower_count": 2530,
    "favoriting_count": 116,
    "signature": "感谢抖音绿色平台，给我带来快乐，让我认识好多朋友！",
    "avatar_thumb": {
      "uri": "tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c",
      "url_list": ["https://p6-dy-ipv6.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_100x100.jpeg?from=4010531038", "https://p26-dy.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_100x100.jpeg?from=4010531038", "https://p29-dy.byteimg.com/img/tos-cn-i-0813/ca7b7321790449c1b85806f9cb58d85c~c5_100x100.jpeg?from=4010531038"]
    },
    "geofencing": null
  },
  "extra": {
    "now": 1598943466000,
    "logid": "202009011457460101980621700300CC82"
  }
}

 * */

bool parseJson(String json)
{
    const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 1024;
    DynamicJsonDocument doc(2048);

    deserializeJson(doc, json);
    JsonObject obj = doc.as<JsonObject>();

    int code = obj["status_code"];

    if (code != 0){
        Serial.print("[API]Code:");
        Serial.print(code);
        return false;
    }

    JsonObject userInfo = obj["user_info"];
    unsigned long total_favorited = userInfo["total_favorited"];
    int follower_count = userInfo["follower_count"];
    String userName = userInfo["nickname"];
    Serial.print("userName: ");
    Serial.print(userName);
    Serial.print("follower: ");
    Serial.print(follower_count);
    Serial.print("favorited: ");
    Serial.println(total_favorited);
    
    if (follower_count == 0){
        Serial.println("[JSON] FORMAT ERROR");
        return false;
    }

    if(follower_count > 0 ){
          diffollower = follower_count - follower;
          follower = follower_count;
         
    }
 
    if(total_favorited > 0 ){
          diffavorite = total_favorited - favorite;
          favorite = total_favorited;
    }

    if( userName.length() > 0 ){
        nickName = userName;
    }

     // Lastly, you can print the resulting JSON to a String
//    String output;
//    serializeJson(doc, output);
  
    return true;
}


void initdisplay()
{
  u8g2.begin();
  u8g2.enableUTF8Print();
}

void oledClockDisplay()
{
  int years, months, days, hours, minutes, seconds, weekdays;
  years = year();
  months = month();
  days = day();
  hours = hour();
  minutes = minute();
  seconds = second();
  weekdays = weekday();
  Serial.printf("%d/%d/%d %d:%d:%d Weekday:%d\n", years, months, days, hours, minutes, seconds, weekdays);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setCursor(0, 14);
  if (isNTPConnected)
    u8g2.print("当前时间 (UTC+8)");
  else
    u8g2.print("无网络!"); //如果上次对时失败，则会显示无网络
  String currentTime = "";
  if (hours < 10)
    currentTime += 0;
  currentTime += hours;
  currentTime += ":";
  if (minutes < 10)
    currentTime += 0;
  currentTime += minutes;
  currentTime += ":";
  if (seconds < 10)
    currentTime += 0;
  currentTime += seconds;
  String currentDay = "";
  currentDay += years;
  currentDay += "/";
  if (months < 10)
    currentDay += 0;
  currentDay += months;
  currentDay += "/";
  if (days < 10)
    currentDay += 0;
  currentDay += days;

  u8g2.setFont(u8g2_font_logisoso24_tr);
  u8g2.setCursor(0, 44);
  u8g2.print(currentTime);
  u8g2.setCursor(0, 61);
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.print(currentDay);
  u8g2.drawXBM(80, 48, 16, 16, xing);
  u8g2.setCursor(95, 62);
  u8g2.print("期");
  if (weekdays == 1)
    u8g2.print("日");
  else if (weekdays == 2)
    u8g2.print("一");
  else if (weekdays == 3)
    u8g2.print("二");
  else if (weekdays == 4)
    u8g2.print("三");
  else if (weekdays == 5)
    u8g2.print("四");
  else if (weekdays == 6)
    u8g2.print("五");
  else if (weekdays == 7)
    u8g2.drawXBM(111, 49, 16, 16, liu);
  u8g2.sendBuffer();
}


/*-------- NTP 代码 ----------*/

const int NTP_PACKET_SIZE = 48;     // NTP时间在消息的前48个字节里
byte packetBuffer[NTP_PACKET_SIZE]; // 输入输出包的缓冲区


time_t getNtpTime()
{
  IPAddress ntpServerIP;          // NTP服务器的地址

  while (Udp.parsePacket() > 0);  // 丢弃以前接收的任何数据包
  Serial.println("Transmit NTP Request");
  // 从池中获取随机服务器
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      Serial.println("Receive NTP Response");
      isNTPConnected = true;
      Udp.read(packetBuffer, NTP_PACKET_SIZE); // 将数据包读取到缓冲区
      unsigned long secsSince1900;
      // 将从位置40开始的四个字节转换为长整型，只取前32位整数部分
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      Serial.println(secsSince1900);
      Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  
  Serial.println("No NTP Response :-("); //无NTP响应
  isNTPConnected = false;
  return 0; //如果未得到时间则返回0
}

// 向给定地址的时间服务器发送NTP请求
void sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123); //NTP需要使用的UDP端口号为123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
