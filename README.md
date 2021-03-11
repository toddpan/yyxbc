#微信：yyxbc2010  
阳阳学编程抖音号视频中的实验源代码  
项目说明：  
1，arduino/esp8266FansClock  
    抖音粉丝灯牌，esp8266开发板接入oled显示屏和获取抖音粉丝个数，获赞数用显示屏显示。  

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
2、arduino/BlinkerIR  
    esp8266+红外模块，实现遥控器功能   
    功能描述：  
      1.模拟家用电器的遥控器功能。  
      2.接入blinker平台，通过手机可以远程控制。  
      3.接入小爱同学、小度和天猫精灵，可以动动嘴就能控制你的家用电器。    
      4.带红外学习功能，只要用遥控器的电器设备都可以接入。  

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
3、arduino/wificar   
    nodemcu(esp8266-12e) + L298N 电机驱动板 ，wifi遥控小车，app inventor   
    开发的手机app wifi遥控器。

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
4、arduino/blinker_esp8266-01s_mi_duer_aligenie_physicsSwitch
   NodeMcu或esp-01/01s控制1路继电器，集成blinker平台，
   用手机app控制1路继电器开关，添加一路物理开关控制，继电器用常开（NO）模式

   固件有网页配网功能，固件烧写工具：
   链接:https://pan.baidu.com/s/1WsE_2gERyjaUZFbwgJnsOw 提取码:56r8 

  说明：

   1，继电器高电平触发时，YYXBC_HIGH = 1，YYXBC_LOW  = 0
   继电器低电平触发时，YYXBC_HIGH = 0，YYXBC_LOW  = 1
   const int YYXBC_HIGH = 0 ;
   const int YYXBC_LOW  = 1 ;

   2，用esp-01时，物理开关接在vcc 和gpio2上，继电器接在gpio0上
   用nodeMcu时，物理开关接在vcc 和D4上，继电器接在D3上，下面的宏
   定义只能同时有一个生效

   //NodeMCU 继电器接D3,物理开关接D4
   #define LED_BUILTIN_LIGHT 0
   #define LED_BUILTIN_K2 2

   //Esp-01/01s，继电器接GPIO0,物理开关接GPIO2
   //#define LED_BUILTIN_LIGHT D3
   //#define LED_BUILTIN_K2 D4

   3，YYXBC_BUTTON_TYPE = 1时表示物理开关为点动模式，0时表示自锁模式
   const int YYXBC_BUTTON_TYPE = 1;

   获得具体连接电路图和更多资源，请参阅阳阳学编程网站 www.sooncore.com。
 
