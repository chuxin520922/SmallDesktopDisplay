/* *****************************************************************
 * 
 * SmallDesktopDisplay
 * 小型桌面显示器
 * 
 * 作      者：Misaka
 * 讨   论  群:811058758、887171863
 * 创 建 日 期:2021.07.19
 * 最后更改日期:2021.11.04
 * 最后更改人:戮筱沫
 * 适合硬件版本:SD²
 * 软 件 版 本:V1.21
 * 
 * 更 改 说 明:V1.1添加串口调试，波特率115200\8\n\1；增加版本号显示。
 *            V1.2亮度和城市代码保存到EEPROM，断电可保存
 *            V1.21添加MQTT调光 添加初始化页面IP地址展示
 * 
 * 引 脚 分 配: SCK  GPIO14
 *             MOSI  GPIO13
 *             RES   GPIO2
 *             DC    GPIO0
 *             LCDBL GPIO5
 *             
 * *****************************************************************/
#include "ArduinoJson.h"
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h> 
#include <SPI.h>
#include <TJpg_Decoder.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <Ticker.h> 
#include "qr.h"
#include "number.h"
#include "weathernum.h"


//#include <ESP8266WiFi.h>          
//#include <DNSServer.h>
//#include <ESP8266WebServer.h>
//#include <WiFiManager.h>  



#define LCD_BL_PIN 5  //第五引脚


//---------------WIFI设置（可写死）--------------------
const char ssid[] = "";      //WIFI名称 修改这2个就可以了
const char pass[] = "";    //WIFI密码
//----------------------------------------------------



#include "font/ZdyLwFont_20.h"
#include "img/misaka.h"
#include "img/temperature.h"
#include "img/humidity.h"
#include "img/pangzi/i0.h"
#include "img/pangzi/i1.h"
#include "img/pangzi/i2.h"
#include "img/pangzi/i3.h"
#include "img/pangzi/i4.h"
#include "img/pangzi/i5.h"
#include "img/pangzi/i6.h"
#include "img/pangzi/i7.h"
#include "img/pangzi/i8.h"
#include "img/pangzi/i9.h"

TFT_eSPI tft = TFT_eSPI();  // 引脚请自行配置tft_espi库中的 User_Setup.h文件
TFT_eSprite clk = TFT_eSprite(&tft);

/*** Component objects ***/
Number      dig;
WeatherNum  wrat;


uint32_t targetTime = 0;   
uint16_t bgColor = 0x0000;
String cityCode = "101250101";  //天气城市代码 长沙:101250101株洲:101250301衡阳:101250401
int LCD_BL_PWM = 0;//屏幕亮度0-100
int tempnum = 0;   //温度百分比
int huminum = 0;   //湿度百分比
int tempcol =0xffff;
int humicol =0xffff;
int Anim = 0;
int prevTime = 0;
int AprevTime = 0;
int BL_addr = 1;//被写入数据的EEPROM地址编号  0亮度
int CC_addr = 10;//被写入数据的EEPROM地址编号  10城市

//NTP服务器
static const char ntpServerName[] = "ntp6.aliyun.com";
const int timeZone = 8;     //东八区

// 声明对象
Ticker ticker;
WiFiClient wificlient;
PubSubClient mqttClient(wificlient);

const char* mqttServer = "10.168.1.207";  //常量, mqtt服务器地址
const int port = 1883;//MQTT服务器端口号
int count;    // Ticker计数用的变量
String messageString = "";  //发布消息的变量,默认是关闭状态
String publishTopicName = "topicsend" ;
String subscribeTopicName = "topicreceive" ;



WiFiUDP Udp;
//WiFiClient wificlient; //99行代码存在重复代码 故此处注释掉
unsigned int localPort = 8000;
float duty=0;
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
String num2str(int digits);
void sendNTPpacket(IPAddress &address);


bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block
  return 1;
}

byte loadNum = 6;
void loading(byte delayTime)//绘制进度条
{
  clk.setColorDepth(8);
  
  clk.createSprite(200, 100);//创建窗口
  clk.fillSprite(0x0000);   //填充率

  clk.drawRoundRect(0,0,200,16,8,0xFFFF);       //空心圆角矩形
  clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);   //实心圆角矩形
  clk.setTextDatum(CC_DATUM);   //设置文本数据
  
  if(loadNum>97){
    if(WiFi.status() == WL_CONNECTED){
      clk.setTextColor(tft.color565(88,132,239), 0x0000);
      String ipadd = WiFi.localIP().toString();
      clk.drawString("IP "+ipadd,100,40,2);
    }else{
      clk.setTextColor(TFT_GREEN, 0x0000);
      clk.drawString("Connecting to WiFi......",100,40,2);
  }   
  }else{
    clk.setTextColor(TFT_GREEN, 0x0000);
    clk.drawString("Connecting to WiFi......",100,40,2);
  }
  clk.setTextColor(TFT_WHITE, 0x0000); 
  clk.drawRightString("SDD V1.21",180,60,2);
  clk.pushSprite(20,110);  //窗口位置
  
  //clk.setTextDatum(CC_DATUM);
  //clk.setTextColor(TFT_WHITE, 0x0000); 
  //clk.pushSprite(130,180);
  
  clk.deleteSprite();
  loadNum += 1;
  delay(delayTime);
}

void humidityWin()
{
  clk.setColorDepth(8);
  
  huminum = huminum/2;
  clk.createSprite(52, 6);  //创建窗口
  clk.fillSprite(0x0000);    //填充率
  clk.drawRoundRect(0,0,52,6,3,0xFFFF);  //空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.fillRoundRect(1,1,huminum,4,2,humicol);   //实心圆角矩形
  clk.pushSprite(45,222);  //窗口位置
  clk.deleteSprite();
}
void tempWin()
{
  clk.setColorDepth(8);
  
  clk.createSprite(52, 6);  //创建窗口
  clk.fillSprite(0x0000);    //填充率
  clk.drawRoundRect(0,0,52,6,3,0xFFFF);  //空心圆角矩形  起始位x,y,长度，宽度，圆弧半径，颜色
  clk.fillRoundRect(1,1,tempnum,4,2,tempcol);   //实心圆角矩形
  clk.pushSprite(45,192);  //窗口位置
  clk.deleteSprite();
}

void SmartConfig(void)//微信配网
{
  WiFi.mode(WIFI_STA);    //设置STA模式
  //tft.pushImage(0, 0, 240, 240, qr);
  tft.pushImage(0, 0, 240, 240, qr);
  Serial.println("\r\nWait for Smartconfig...");    //打印log信息
  WiFi.beginSmartConfig();      //开始SmartConfig，等待手机端发出用户名和密码
  while (1)
  {
    Serial.println("等待中...");
    delay(1000);                   // wait for a second
    if (WiFi.smartConfigDone())//配网成功，接收到SSID和密码
    {
    Serial.println("SmartConfig Success");
    Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
    Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
    break;
    }
  }
  loadNum = 194;
}

String SMOD = "";//0亮度

void Serial_set()//串口设置
{
  String incomingByte = "";
  if(Serial.available()>0)
  {
    
    while(Serial.available()>0)//监测串口缓存，当有数据输入时，循环赋值给incomingByte
    {
      incomingByte += char(Serial.read());//读取单个字符值，转换为字符，并按顺序一个个赋值给incomingByte
      delay(2);//不能省略，因为读取缓冲区数据需要时间
    }    
    if(SMOD=="0x01")//设置1亮度设置
    {
      int LCDBL = atoi(incomingByte.c_str());//int n = atoi(xxx.c_str());//String转int
      if(LCDBL>=0 && LCDBL<=100)
      {
        EEPROM.write(BL_addr, LCDBL);//亮度地址写入亮度值
        EEPROM.commit();//保存更改的数据
        delay(5);
        LCD_BL_PWM = EEPROM.read(BL_addr); 
        delay(5);
        SMOD = "";
        Serial.printf("亮度调整为：");
        analogWrite(LCD_BL_PIN, 1023 - (LCD_BL_PWM*10));
        Serial.println(LCD_BL_PWM);
        Serial.println("");
      }
      else
        Serial.println("亮度调整错误，请输入0-100");
    } 
    if(SMOD=="0x02")//设置2地址设置
    {
      int CityCODE = 0;
      int CityC = atoi(incomingByte.c_str());//int n = atoi(xxx.c_str());//String转int
      if(CityC>=101000000 && CityC<=102000000 || CityC == 0)
      {
        for(int cnum=0;cnum<5;cnum++)
        {
          EEPROM.write(CC_addr+cnum,CityC%100);//城市地址写入城市代码
          EEPROM.commit();//保存更改的数据
          CityC = CityC/100;
          delay(5);
        }
        for(int cnum=5;cnum>0;cnum--)
        {          
          CityCODE = CityCODE*100;
          CityCODE += EEPROM.read(CC_addr+cnum-1); 
          delay(5);
        }
        
        cityCode = CityCODE;
        
        if(cityCode == "0")
        {
          Serial.println("城市代码调整为：自动");
          getCityCode();  //获取城市代码
        }
        else
        {
          Serial.printf("城市代码调整为：");
          Serial.println(cityCode);
        }
        Serial.println("");
        getCityWeater();//更新城市天气  
        SMOD = "";
      }
      else
        Serial.println("城市调整错误，请输入9位城市代码，自动获取请输入0");
    }   
    else
    {
      SMOD = incomingByte;
      delay(2);
      if(SMOD=="0x01")
        Serial.println("请输入亮度值，范围0-100");
      else if(SMOD=="0x02")
        Serial.println("请输入9位城市代码，自动获取请输入0"); 
      else
      {
        Serial.println("");
        Serial.println("请输入需要修改的代码：");
        Serial.println("亮度设置输入    0x01");
        Serial.println("地址设置输入    0x02");
        Serial.println("");
      }
    }
  }
}




// 发送HTTP请求并且将服务器响应通过串口输出
void getCityCode(){
 String URL = "http://wgeo.weather.com.cn/ip/?_="+String(now());
  //创建 HTTPClient 对象
  HTTPClient httpClient;
 
  //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.begin(wificlient,URL); 
  
  //设置请求头中的User-Agent
  httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  //启动连接并发送HTTP请求
  int httpCode = httpClient.GET();
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  if (httpCode == HTTP_CODE_OK) {
    String str = httpClient.getString();
    
    int aa = str.indexOf("id=");
    if(aa>-1)
    {
       //cityCode = str.substring(aa+4,aa+4+9).toInt();
       cityCode = str.substring(aa+4,aa+4+9);
       Serial.println(cityCode); 
       getCityWeater();
    }
    else
    {
      Serial.println("获取城市代码失败");  
    }
    
    
  } else {
    Serial.println("请求城市代码错误：");
    Serial.println(httpCode);
  }
 
  //关闭ESP8266与服务器连接
  httpClient.end();
}



// 获取城市天气
void getCityWeater(){
 //String URL = "http://d1.weather.com.cn/dingzhi/" + cityCode + ".html?_="+String(now());//新
 String URL = "http://d1.weather.com.cn/weather_index/" + cityCode + ".html?_="+String(now());//原来
  //创建 HTTPClient 对象
  HTTPClient httpClient;
  
  httpClient.begin(URL); 
  
  //设置请求头中的User-Agent
  httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  //启动连接并发送HTTP请求
  int httpCode = httpClient.GET();
  Serial.println("正在获取天气数据");
  Serial.println(URL);
  
  //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
  if (httpCode == HTTP_CODE_OK) {

    String str = httpClient.getString();
    int indexStart = str.indexOf("weatherinfo\":");
    int indexEnd = str.indexOf("};var alarmDZ");

    String jsonCityDZ = str.substring(indexStart+13,indexEnd);
    //Serial.println(jsonCityDZ);

    indexStart = str.indexOf("dataSK =");
    indexEnd = str.indexOf(";var dataZS");
    String jsonDataSK = str.substring(indexStart+8,indexEnd);
    //Serial.println(jsonDataSK);

    
    indexStart = str.indexOf("\"f\":[");
    indexEnd = str.indexOf(",{\"fa");
    String jsonFC = str.substring(indexStart+5,indexEnd);
    //Serial.println(jsonFC);
    
    weaterData(&jsonCityDZ,&jsonDataSK,&jsonFC);
    Serial.println("获取成功");
    
  } else {
    Serial.println("请求城市天气错误：");
    Serial.print(httpCode);
  }
 
  //关闭ESP8266与服务器连接
  httpClient.end();
}


String scrollText[7];
//int scrollTextWidth = 0;
//天气信息写到屏幕上
void weaterData(String *cityDZ,String *dataSK,String *dataFC)
{
  //解析第一段JSON
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();

  //TFT_eSprite clkb = TFT_eSprite(&tft);
  
  /***绘制相关文字***/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);
  
  //温度
  clk.createSprite(58, 24); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(tft.color565(0,197,255), bgColor); //温度字体颜色
  clk.drawString(sk["temp"].as<String>()+"℃",28,13);
  clk.pushSprite(100,184);
  clk.deleteSprite();
  tempnum = sk["temp"].as<int>();
  tempnum = tempnum+10;
  //温度颜色值
  if(tempnum<10)
    tempcol=0x00FF;
  else if(tempnum<28)
    tempcol=0x0AFF;
  else if(tempnum<34)
    tempcol=0x0F0F;
  else if(tempnum<41)
    tempcol=0xFF0F;
  else if(tempnum<49)
    tempcol=0xF00F;
  else
  {
    tempcol=0xF00F;
    tempnum=50;
  }
  tempWin();
  
  //湿度
  clk.createSprite(58, 24); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(tft.color565(88,132,239), bgColor); //湿度字体颜色
  clk.drawString(sk["SD"].as<String>(),28,13);
  //clk.drawString("100%",28,13);
  clk.pushSprite(100,214);
  clk.deleteSprite();
  //String A = sk["SD"].as<String>();
  huminum = atoi((sk["SD"].as<String>()).substring(0,2).c_str());
  //湿度颜色值 
  if(huminum>90)
    humicol=0x00FF;
  else if(huminum>70)
    humicol=0x0AFF;
  else if(huminum>40)
    humicol=0x0F0F;
  else if(huminum>20)
    humicol=0xFF0F;
  else
    humicol=0xF00F;//红色
  //humicol = tft.color565(74,213,240);
  humidityWin();

  
  //城市名称
  clk.createSprite(94, 30); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor); 
  clk.drawString(sk["cityname"].as<String>(),44,16);
  clk.pushSprite(15,15);
  clk.deleteSprite();

  //PM2.5空气指数
  uint16_t pm25BgColor = tft.color565(64,192,87);//优
  String aqiTxt = "优";
  int pm25V = sk["aqi"];
  if(pm25V>200){
    pm25BgColor = tft.color565(136,11,32);//重度
    aqiTxt = "重度";
  }else if(pm25V>150){
    pm25BgColor = tft.color565(224,49,49);//中度
    aqiTxt = "中度";
  }else if(pm25V>100){
    pm25BgColor = tft.color565(247,103,7);//轻
    aqiTxt = "轻度";
  }else if(pm25V>50){
    pm25BgColor = tft.color565(130,201,30);//良
    aqiTxt = "良";
  }
  clk.createSprite(56, 24); 
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0,0,50,24,4,pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, pm25BgColor); //设置字体颜色
  clk.drawString(aqiTxt,25,13);
  clk.pushSprite(104,18);
  clk.deleteSprite();
  
  scrollText[0] = "实时天气 "+sk["weather"].as<String>();
  scrollText[1] = "空气质量 "+aqiTxt;
  scrollText[2] = "风向 "+sk["WD"].as<String>()+sk["WS"].as<String>();

  //scrollText[6] = atoi((sk["weathercode"].as<String>()).substring(1,3).c_str()) ;

  //天气图标
  wrat.printfweather(170,15,atoi((sk["weathercode"].as<String>()).substring(1,3).c_str()));

  
  //左上角滚动字幕
  //解析第二段JSON
  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  //Serial.println(sk["ws"].as<String>());
  //横向滚动方式
  //String aa = "今日天气:" + dz["weather"].as<String>() + "，温度:最低" + dz["tempn"].as<String>() + "，最高" + dz["temp"].as<String>() + " 空气质量:" + aqiTxt + "，风向:" + dz["wd"].as<String>() + dz["ws"].as<String>();
  //scrollTextWidth = clk.textWidth(scrollText);
  //Serial.println(aa);
  scrollText[3] = "今日"+dz["weather"].as<String>();
  
  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();
  
  scrollText[4] = "最低温度"+fc["fd"].as<String>()+"℃";
  scrollText[5] = "最高温度"+fc["fc"].as<String>()+"℃";
  
  //Serial.println(scrollText[0]);
  
  clk.unloadFont();
}

int currentIndex = 0;
TFT_eSprite clkb = TFT_eSprite(&tft);

void scrollBanner(){
  //if(millis() - prevTime > 2333) //3秒切换一次
  if(second()%2 ==0&& prevTime == 0)
  { 
    if(scrollText[currentIndex])
    {
      clkb.setColorDepth(8);
      clkb.loadFont(ZdyLwFont_20);
      clkb.createSprite(150, 30); 
      clkb.fillSprite(bgColor);
      clkb.setTextWrap(false);
      clkb.setTextDatum(CC_DATUM);
      clkb.setTextColor(TFT_WHITE, bgColor); 
      clkb.drawString(scrollText[currentIndex],74, 16);
      clkb.pushSprite(10,45);
       
      clkb.deleteSprite();
      clkb.unloadFont();
      
      if(currentIndex>=5)
        currentIndex = 0;  //回第一个
      else
        currentIndex += 1;  //准备切换到下一个        
    }
    prevTime = 1;
  }
}

void imgAnim()
{
  int x=160,y=160;
  if(millis() - AprevTime > 37) //x ms切换一次
  {
    Anim++;
    AprevTime = millis();
  }
  if(Anim==10)
    Anim=0;

  switch(Anim)
  {
    case 0:
      TJpgDec.drawJpg(x,y,i0, sizeof(i0));
      break;
    case 1:
      TJpgDec.drawJpg(x,y,i1, sizeof(i1));
      break;
    case 2:
      TJpgDec.drawJpg(x,y,i2, sizeof(i2));
      break;
    case 3:
      TJpgDec.drawJpg(x,y,i3, sizeof(i3));
      break;
    case 4:
      TJpgDec.drawJpg(x,y,i4, sizeof(i4));
      break;
    case 5:
      TJpgDec.drawJpg(x,y,i5, sizeof(i5));
      break;
    case 6:
      TJpgDec.drawJpg(x,y,i6, sizeof(i6));
      break;
    case 7:
      TJpgDec.drawJpg(x,y,i7, sizeof(i7));
      break;
    case 8: 
      TJpgDec.drawJpg(x,y,i8, sizeof(i8));
      break;
    case 9: 
      TJpgDec.drawJpg(x,y,i9, sizeof(i9));
      break;
    default:
      Serial.println("显示Anim错误");
      break;
  }
}

unsigned char Hour_sign   = 60;
unsigned char Minute_sign = 60;
unsigned char Second_sign = 60;
void digitalClockDisplay()
{ 
  int timey=82;
  if(hour()!=Hour_sign)//时钟刷新
  {
    dig.printfW3660(20,timey,hour()/10);
    dig.printfW3660(60,timey,hour()%10);
    Hour_sign = hour();
  }
  if(minute()!=Minute_sign)//分钟刷新
  {
    dig.printfO3660(101,timey,minute()/10);
    dig.printfO3660(141,timey,minute()%10);
    Minute_sign = minute();
  }
  if(second()!=Second_sign)//分钟刷新
  {
    dig.printfW1830(182,timey+30,second()/10);
    dig.printfW1830(202,timey+30,second()%10);
    Second_sign = second();
  }
  
  /***日期****/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);
  
  //星期
  clk.createSprite(58, 30);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);
  clk.drawString(week(),29,16);
  clk.pushSprite(102,150);
  clk.deleteSprite();
  
  //月日
  clk.createSprite(95, 30);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, bgColor);  
  clk.drawString(monthDay(),49,16);
  clk.pushSprite(5,150);
  clk.deleteSprite();
  
  clk.unloadFont();
  /***日期****/
}

//星期
String week()
{
  String wk[7] = {"日","一","二","三","四","五","六"};
  String s = "周" + wk[weekday()-1];
  return s;
}

//月日
String monthDay()
{
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP时间在消息的前48字节中
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  //Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  //Serial.print(ntpServerName);
  //Serial.print(": ");
  //Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 无法获取时间时返回0
}

// 向NTP服务器发送请求
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


/**
*MQTT设置灯亮度
*/
//设置lcd屏幕亮度值
int web_lcdbl = 0;
void setlcdlv(){
  if(web_lcdbl>0 && web_lcdbl<=100)
  {
    EEPROM.write(BL_addr, web_lcdbl);//亮度地址写入亮度值
    Serial.printf("写入亮度值为：");
    Serial.println(web_lcdbl);
    
    analogWrite(LCD_BL_PIN, 1023 - (LCD_BL_PWM*10));
    EEPROM.commit();//保存更改的数据
    delay(5);
    LCD_BL_PWM = EEPROM.read(BL_addr); 
    delay(5);
    Serial.printf("亮度调整为：");
    analogWrite(LCD_BL_PIN, 1023 - (LCD_BL_PWM*10));
    Serial.println(LCD_BL_PWM);
    messageString = "成功写入亮度值：" + web_lcdbl; 
  }
}

//计数
void tickerCount(){
  count++;
}

// 连接MQTT服务器
void connectMQTTServer(){
    // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
    String clientId = "esp8266-" + WiFi.macAddress(); 
    
    if (mqttClient.connect(clientId.c_str())) { 
      Serial.println("MQTT Server Connected.");
      Serial.print("Server Address:");
      Serial.println(mqttServer);
      Serial.print("ClientId:");
      Serial.println(clientId);
      subscribeTopic(); // ****订阅指定主题****
    } else {
      Serial.print("MQTT Server Connect Failed. Client State:");
      Serial.println(mqttClient.state());
      delay(3000);
    }   
}
 
// 发布信息
void pubMQTTmsg(){   
    // 建立发布主题
    char publishTopic[publishTopicName.length() + 1];  
    strcpy(publishTopic, publishTopicName.c_str());
   
    // 建立发布信息:设备的状态    
    char publishMsg[messageString.length() + 1];   
    strcpy(publishMsg, messageString.c_str());
    
    // 实现ESP8266向主题发布信息,并在串口监视器显示出来
    if(mqttClient.publish(publishTopic, publishMsg)){
      Serial.print("发布主题:");
      Serial.println(publishTopic);
      Serial.print("发布消息:");
      Serial.println(publishMsg); 
    } else {
      Serial.println("消息发布失败。"); 
    }
}


// 订阅指定主题
void subscribeTopic(){ 
    char subTopic[subscribeTopicName.length() + 1];  
    strcpy(subTopic, subscribeTopicName.c_str());
    
    // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
    if(mqttClient.subscribe(subTopic)){
      Serial.print("订阅:");
      Serial.println(subTopic);
    } else {
      Serial.print("Subscribe Fail...");
    }  
}
 
// 收到信息后的回调函数
void receiveCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("接收到订阅消息：[");
    Serial.print(topic);
    Serial.print("] ");
    String msg = "";
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      msg+=(char)payload[i];
    }
    Serial.println("");
    Serial.print("消息长度(Bytes) ");
    Serial.println(length);
    //接收到消息内容
    Serial.print("接收到消息内容：");
    Serial.println(msg);
    
    web_lcdbl = msg.toInt(); 
    setlcdlv();
    pubMQTTmsg(); //调用函数发布消息
    

    
}




void setup()
{
  Serial.begin(115200);
  EEPROM.begin(1024);

  if(EEPROM.read(BL_addr)>0&&EEPROM.read(BL_addr)<100)
    LCD_BL_PWM = EEPROM.read(BL_addr); 
  
  pinMode(LCD_BL_PIN, OUTPUT);
  analogWrite(LCD_BL_PIN, 1023 - (LCD_BL_PWM*10));
  
  tft.begin(); /* TFT init */
  tft.invertDisplay(1);//反转所有显示颜色：1反转，0正常
  tft.fillScreen(0x0000);
  tft.setTextColor(TFT_BLACK, bgColor);

  targetTime = millis() + 1000; 
  Serial.println("");
  Serial.println("正在连接WIFI...");
  Serial.print("SSID:");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  while (WiFi.status() != WL_CONNECTED) 
  {
    loading(70);  
      
    if(loadNum>=194)
    {
      Serial.println("进入配置页面");
      SmartConfig();   
      break;
    }
  }
  delay(10); 
  while(loadNum < 194) //让动画走完
  { 
    loading(1); 
  }
 

  Serial.print("本地IP： ");
  Serial.println(WiFi.localIP());
  Serial.println("启动UDP");
  Udp.begin(localPort);
  Serial.println("等待同步...");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);


  /*
  *显示logo
  */
  //tft.pushImage(0,0,240,240,misaka);
  //TJpgDec.drawJpg(120,120,misaka, sizeof(misaka));//显示logo
  //delay(1000); 
  int CityCODE = 0;
  for(int cnum=5;cnum>0;cnum--)
  {          
    CityCODE = CityCODE*100;
    CityCODE += EEPROM.read(CC_addr+cnum-1); 
    delay(5);
  }
  if(CityCODE>=101000000 && CityCODE<=102000000) 
    cityCode = CityCODE;  
  else
    getCityCode();  //获取城市代码
   
  tft.fillScreen(TFT_BLACK);//清屏
  
  TJpgDec.drawJpg(15,183,temperature, sizeof(temperature));  //温度图标
  TJpgDec.drawJpg(15,213,humidity, sizeof(humidity));  //湿度图标

  getCityWeater();



  // 设置MQTT服务器和端口号 
  mqttClient.setServer(mqttServer, port); 
  // 收到信息后的回调函数
  mqttClient.setCallback(receiveCallback);
  // 连接MQTT服务器
  connectMQTTServer(); 
  // Ticker定时对象
  ticker.attach(1, tickerCount);
  
}
time_t prevDisplay = 0; // 显示时间
unsigned long weaterTime = 0;

void loop()
{
  
  if (now() != prevDisplay) {
    prevDisplay = now();
    digitalClockDisplay();
    prevTime=0;  
  }
  
  if(millis() - weaterTime > 300000){ //5分钟更新一次天气
    weaterTime = millis();
    getCityWeater();
  }
  scrollBanner();
  imgAnim();
  Serial_set();

  //MQTT服务连接检测
  if (mqttClient.connected()) { // 如果开发板成功连接服务器
    // 每隔10秒钟发布一次信息
    if (count >= 10){
      pubMQTTmsg();
      count = 0;
    }    
    // 保持心跳
    mqttClient.loop();
  } else {                  // 如果开发板未能成功连接服务器
    connectMQTTServer();    // 则尝试连接服务器
  }
  
}
