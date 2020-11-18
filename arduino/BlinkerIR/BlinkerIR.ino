
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT              // 设置小爱灯类库
#define BLINKER_DUEROS_LIGHT            // 设置小度灯类库
#define BLINKER_ALIGENIE_LIGHT          // 设置天猫灯类库
#define BLINKER_WITHOUT_SSL

#define DEBUG_PRINTER Serial
#define MQTT_DEBUG

#include <Blinker.h>
#include <TimeLib.h>
#include <IRsend.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>



//#define ESP_01
#define NODEMCU

String version = "1.0.1";

bool oState = false;

#define BUTTON_1 "btn-abc"

BlinkerButton Button1(BUTTON_1);

char auth[] = "a7ac72a225a3";//这里面填写设备的密钥
char ssid[] = "panzujiMi10";//这里填写你的wifi名字
char pswd[] = "moto1984";//这里填写你的WiFi密码

//Protocol  : GREE Power: Off
//Code      : 0x110C2050004000B0 (64 Bits)
//Mesg Desc.: Model: 1 (YAW1F), Power: Off, Mode: 1 (Cool), Temp: 28C, Fan: 1 (Low), Turbo: Off, IFeel: Off, WiFi: On, XFan: Off, Light: On, Sleep: Off, Swing(V) Mode: Manual, Swing(V): 0 (Last), Timer: Off, Display Temp: 0 (Off)
uint16_t gREEPowerOffrawData[279] = {8978, 4502,  614, 1690,  616, 588,  618, 588,  614, 592,  616, 1696,  612, 588,  628, 572,  650, 546,  660, 542,  638, 588,  616, 1692,  638, 1666,  618, 582,  624, 590,  624, 580,  614, 590,  618, 582,  644, 550,  632, 588,  618, 590,  644, 562,  616, 1692,  616, 582,  648, 564,  614, 590,  642, 564,  620, 588,  614, 588,  620, 1690,  616, 584,  622, 1690,  618, 590,  614, 592,  614, 1688,  620, 584,  620, 20022,  618, 582,  624, 586,  642, 566,  616, 588,  622, 582,  640, 564,  616, 588,  642, 564,  618, 590,  618, 580,  620, 590,  618, 586,  620, 584,  618, 584,  620, 1692,  618, 584,  616, 550,  656, 584,  622, 588,  620, 586,  618, 582,  620, 592,  618, 580,  624, 584,  622, 590,  640, 564,  614, 590,  644, 542,  636, 1690,  640, 1664,  618, 590,  614, 1690,  618, 40034,  8956, 4524,  646, 1662,  616, 590,  642, 562,  640, 566,  616, 1690,  616, 584,  622, 588,  616, 590,  614, 590,  616, 590,  616, 1690,  616, 1690,  618, 588,  616, 588,  644, 562,  640, 550,  632, 588,  616, 588,  618, 588,  620, 586,  618, 586,  618, 1690,  616, 588,  642, 564,  616, 590,  616, 592,  614, 588,  644, 546,  632, 1692,  620, 1686,  616, 1690,  642, 566,  614, 590,  614, 1692,  616, 590,  614, 20028,  614, 590,  616, 590,  614, 592,  614, 590,  616, 610,  592, 592,  614, 590,  616, 610,  596, 612,  590, 590,  618, 588,  618, 586,  618, 588,  616, 590,  614, 592,  616, 590,  614, 590,  616, 592,  614, 590,  618, 588,  616, 1690,  616, 590,  618, 586,  618, 590,  638, 566,  642, 550,  632, 612,  594, 588,  616, 592,  638, 566,  612, 590,  616, 1692,  616};  // GREE
//uint8_t state[8] = {0x11, 0x0C, 0x20, 0x50, 0x00, 0x40, 0x00, 0xB0};

//Protocol  : GREE Power: On
//Code      : 0x190C205000400030 (64 Bits)
//Mesg Desc.: Model: 2 (YBOFB), Power: On, Mode: 1 (Cool), Temp: 28C, Fan: 1 (Low), Turbo: Off, IFeel: Off, WiFi: On, XFan: Off, Light: On, Sleep: Off, Swing(V) Mode: Manual, Swing(V): 0 (Last), Timer: Off, Display Temp: 0 (Off)
uint16_t gREEPowerOnrawData[279] = {8984, 4522,  646, 1660,  648, 538,  640, 584,  644, 1660,  650, 1658,  650, 556,  646, 558,  622, 586,  648, 558,  620, 584,  622, 1684,  648, 1658,  650, 556,  646, 558,  646, 560,  646, 560,  648, 560,  648, 558,  648, 556,  648, 556,  650, 556,  646, 1660,  624, 580,  650, 556,  624, 582,  622, 580,  652, 558,  648, 554,  652, 1658,  644, 562,  650, 1658,  648, 558,  622, 582,  648, 1660,  650, 556,  622, 20018,  650, 524,  680, 556,  648, 554,  648, 558,  648, 558,  648, 556,  650, 556,  652, 556,  648, 556,  650, 554,  648, 558,  654, 522,  678, 558,  652, 554,  654, 1652,  654, 550,  654, 554,  648, 558,  652, 552,  648, 556,  648, 558,  652, 554,  648, 558,  646, 558,  624, 582,  650, 556,  650, 556,  648, 556,  650, 1658,  652, 1656,  652, 552,  654, 550,  652, 40000,  8988, 4488,  658, 1650,  648, 556,  654, 552,  658, 1646,  656, 1652,  656, 550,  678, 526,  654, 528,  678, 552,  682, 524,  654, 1652,  660, 1648,  684, 524,  650, 554,  656, 512,  692, 522,  708, 526,  684, 522,  686, 518,  660, 546,  658, 550,  656, 1650,  654, 550,  656, 552,  658, 548,  682, 522,  682, 524,  684, 546,  656, 1650,  680, 1626,  684, 1622,  686, 522,  656, 552,  654, 1648,  688, 518,  692, 19946,  688, 516,  660, 544,  690, 518,  686, 518,  660, 546,  688, 520,  684, 522,  690, 518,  684, 518,  686, 520,  688, 518,  684, 522,  684, 520,  690, 432,  772, 518,  680, 526,  688, 518,  684, 520,  658, 546,  686, 520,  686, 1620,  688, 520,  686, 518,  690, 442,  758, 520,  686, 518,  688, 514,  688, 520,  688, 514,  688, 520,  684, 520,  684, 520,  688};  // GREE
//uint8_t state[8] = {0x19, 0x0C, 0x20, 0x50, 0x00, 0x40, 0x00, 0x30};

// ==================== start of TUNEABLE PARAMETERS ====================
// An IR detector/demodulator is connected to GPIO pin 14
// e.g. D5 on a NodeMCU board.
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
#ifdef ESP_01
//IR send  pin,esp8266-01/01s use 2,nodemcu use 14(D5);
const uint16_t kRecvPin = 2;
//IR send  pin,esp8266-01/01s use 0,nodemcu use 4(D2);
const uint16_t kSendPin = 0;
#endif
#ifdef NODEMCU
//IR send  pin,esp8266-01/01s use 2,nodemcu use 14(D5);
const uint16_t kRecvPin = 14;
//IR send  pin,esp8266-01/01s use 0,nodemcu use 4(D2);
const uint16_t kSendPin = 4;
#endif

IRsend irsend(kSendPin);  // Set the GPIO （D2）to be used to sending the message.
// The Serial connection baud rate.
const uint32_t kBaudRate = 115200;

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC
// Alternatives:
const uint16_t kMinUnknownSize = 12;
// Legacy (No longer supported!)
// Change to `true` if you miss/need the old "Raw Timing[]" display.
#define LEGACY_TIMING_INFO false
// ==================== end of TUNEABLE PARAMETERS ====================


// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;  // Somewhere to store the results

void setup()
{
    Serial.begin(115200);
    BLINKER_DEBUG.stream(Serial);

    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();
    
    Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
    Serial.printf("Flash real size: %u bytes\n\n", realSize);
    
    Serial.printf("Flash ide  size: %u bytes\n", ideSize);
    Serial.printf("Flash ide speed: %u Hz\n", ESP.getFlashChipSpeed());
    Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    if (ideSize != realSize) {
        Serial.println("Flash Chip configuration wrong!\n");
      } else {
        Serial.println("Flash Chip configuration ok.\n");
      }

    Serial.printf(PSTR("@@version %s, Free mem=%d\n"), version.c_str(), ESP.getFreeHeap());
    Blinker.begin(auth,ssid, pswd);
    Blinker.attachData(dataRead);
    BlinkerDuerOS.attachPowerState(duerPowerState);            //小度语音操作注册函数
    BlinkerMIOT.attachPowerState(miotPowerState);              //小爱语音操作注册函数
    BlinkerAliGenie.attachPowerState(aligeniePowerState);      //天猫语音操作注册函数
    Blinker.attachHeartbeat(heartbeat);              //app定时向设备发送心跳包, 设备收到心跳包后会返回设备当前状态进行语音操作和app操作同步。
    Button1.attach(button1_callback);
    Serial.println("工作模式开启");
  
  #if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    irrecv.setUnknownThreshold(kMinUnknownSize);
  #endif  // DECODE_HASH
    irrecv.enableIRIn();  // Start the receiver
    irsend.begin();
}

void loop()
{
    static int lastms = 0;
    if (millis()-lastms > 30000) {
      lastms = millis();
      Serial.printf(PSTR("Running version %s for %d Free mem=%d\n"),version.c_str(), lastms/1000, ESP.getFreeHeap());
    }
    Blinker.run();//运行Blinker

    // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_ "\n");
    // Display the basic output of what we found.
    String basic =  resultToHumanReadableBasic(&results);
    basic.replace(" ","");
    Serial.print(basic.c_str());
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    description.replace(" ","");
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    String results = resultToTimingInfo(&results);
    results.replace(" ","");
    String jsonstr = "{\"raw\":";
    jsonstr += results;
    jsonstr += "}";
    Serial.println(jsonstr.c_str());
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
    // Output the results as source code
    String sourcecode = resultToSourceCode(&results);
    Serial.println(sourcecode.c_str());
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
  }
}

   
//心跳回调
void heartbeat()
{
   BLINKER_LOG("heartbeat,state: ", digitalRead(LED_BUILTIN));

//    //较正app的按钮状态
//    if(HIGH == digitalRead(LED_BUILTIN) ){
//        Button1.print("on");
//    }else{
//       Button1.print("off");
//    }

}
bool dataParse(const JsonObject & data)
{
    String getData;

    serializeJson(data, getData);
    
    BLINKER_LOG("Get user command: ", getData);

    // if you parsed this data, return TRUE.
    // return true;
    return false;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果小度 有对设备进行操作就执行下面
void duerPowerState(const String & state)
{
    BLINKER_LOG("小度语音操作!");              //串口打印
    if (state == BLINKER_CMD_ON) {
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOnrawData), 38);
        BlinkerDuerOS.powerState("on");
        BLINKER_LOG("灯亮了!");        
        BlinkerDuerOS.print();
        oState = true;
    }
    else if (state == BLINKER_CMD_OFF) {
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOffrawData), 38);
        BlinkerDuerOS.powerState("off");
        BLINKER_LOG("灯关了!");        
        BlinkerDuerOS.print();
        oState = true;
   }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果小爱有对设备进行操作就执行下面
void miotPowerState(const String & state)
{
    BLINKER_LOG("小爱语音操作!");              //串口打印
    if (state == BLINKER_CMD_ON){
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOnrawData), 38);
        BlinkerMIOT.powerState("on");
        BLINKER_LOG("灯亮了!");
        BlinkerMIOT.print();      
    }
    else if (state == BLINKER_CMD_OFF){
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOffrawData), 38);
        BlinkerMIOT.powerState("off");
        BLINKER_LOG("灯关了!");
        BlinkerMIOT.print();     
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//*******如果天猫精灵有对设备进行操作就执行下面
void aligeniePowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);
    if (state == BLINKER_CMD_ON) {
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOnrawData), 38);
        BlinkerAliGenie.powerState("on");
        BlinkerAliGenie.print();
    }
    else if (state == BLINKER_CMD_OFF) {
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOffrawData), 38);
        BlinkerAliGenie.powerState("off");
        BlinkerAliGenie.print();
    }
}

void dataRead(const String & data){
  BLINKER_LOG("Blinker readString: ", data);

  Blinker.vibrate();
  
  uint32_t BlinkerTime = millis();
  
  Blinker.print("millis", BlinkerTime);
}


// 按下按键即会执行该函数
void button1_callback(const String & state) {
    BLINKER_LOG("get button1 state: ", state);
    BLINKER_LOG("LED_BUILTIN: ", LED_BUILTIN);

    if (state == BLINKER_CMD_ON) {
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOnrawData), 38);
        Serial.println("cfg.GREEPowerOnrawData");
        Button1.print("on");

    }
    else if (state == BLINKER_CMD_OFF) {
        irsend.sendRaw(gREEPowerOnrawData, sizeof(gREEPowerOffrawData), 38);
        Serial.println("cfg.gREEPowerOffrawData");
        Button1.print("off");
    }
}
