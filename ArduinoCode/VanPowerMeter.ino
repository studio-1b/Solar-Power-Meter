/*
Comments below this comment block, are all credits to people whose code I copied to do this project.


These variables in THIS FILE need to be updated (See right column)

ServerPurpose	Protocol	Type		Variablename
-------------	--------	----		============
heartbeat	http		string		VANPI_WEB_SERVER="ip4 address"
Get time	ntp		IPAddress	gpsServer=IPAddress(0,0,0,0)
						I have private GPSD server,
						but can use public NIST servers
AP association	wifi		IPAddress	THIS_IP=IPAddress(0,0,0,0)
also web server
also sntp

The wifi AP name, and password, need to be updated in arduino_secrets.h

BY: 
1. Yes, all the comments below, are for the .h files
2. Yes, most of the code below is cut-and-paste from example reference code 
   for the specific module supporting the hardware
3. Except the NTP client and server code, which I shamelessly ripped off.
4. And yes, this is my first C program since college.
5. Yes, much of the code is debugging statements, 
   which are controlled by #define DEBUG_??? set to true

For Arduino 33 Nano IOT
Features:
-HTTP Server on port 80 for delivering data, for a datalogger
-NTP client for getting accurate time, for the logger
 -you can configure your own NTP server, if not on internet
-SNTP server, for forwarding time to any other devices
 -drift calculator of emulated clock
-reading analog pins from voltage and current sensors, to be logged
-Epaper UI, to show logged data

The code I copied, I credit with copying their comment headers, before the include libraries they used.
*/



/***************************************************
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

  https://github.com/adafruit/Adafruit-GFX-Library
  https://github.com/adafruit/Adafruit_EPD
  
 ****************************************************/
/*----------- epaper includes ---------------------*/
#include <SD.h>
#include "Adafruit_EPD.h"

/*
 * https://www.arduino.cc/en/Tutorial/ScheduledWiFiSSLWebClientfor
  Scheduled WiFi SSL Web Client for MKR1000

  This sketch connects to the Arduino website every minute and downloads the ASCII logo to display it on the serial monitor

  created 19 Jan 2016
  by Arturo Guadalupi <a.guadalupi@arduino.cc>r

  http://arduino.cc/en/Tutorial/

  This code is in the public domain.
*/

#include <SPI.h>
#include <WiFiNINA.h>
#include <RTCZero.h>
#include "arduino_secrets.h" 

/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe

 This code is in the public domain.

 */
#include <WiFiUdp.h>



/* LM: The following is based on NTP_Server_01.ino, downloaded from:
 *     https://forum.arduino.cc/index.php?topic=197870.0
 *     
 *     Original source is named NTP_Server_01.
 *     This sketch includes modifications as summarized below.
 *     
 *                LM:   Platform Arduion Uno + u-blox NEO-M8N
 *     Modifications:   GPS com via _software_ serial (D5 and D6)
 *                      Substitute GPS date/time crack() for library method
 *                      Fix leap year bug for year > 1970
 *                      Invert order of NTP and GPS polling
 *                      Minor trivial changes
 */

/*
  NTP Time Server:
 
 This code is in the public domain.
 */


/*
  Battery Monitor

  This example creates a BLE peripheral with the standard battery service and
  level characteristic. The A0 pin is used to calculate the battery level.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>


/*
BY: end of includes, and credits
*/





// wifinina lib updated 3/14
#define VERSION               String("0.10.21")
#define SAFE_EPD              false

#define DEBUG_SETUP           false
#define DEBUG_LOOP            false
#define DEBUG_FUNCTIONS       false
#define DEBUG_WIFI            false
#define DEBUG_NTP             false
#define DEBUG_NTPS            false
#define DEBUG_RTC             false
#define DEBUG_HTTP_OUTGOING   false
#define DEBUG_HTTP_INCOMING   false
#define DEBUG_SAMPLING        false
#define DEBUG_BATT_AMP_SAMPL  false
#define DEBUG_BATT_VOLT_SAMPL false
#define DEBUG_SOL_AMP_SAMPL   false
#define DEBUG_SOL_VOLT_SAMPL  false
#define DEBUG_UI              false
#define DEBUG_BMP             false
#define DEBUG_BLE             false
#define DEBUG_FREE_MEMORY     false

#define DEBUG_18V             false
bool isledon = false;

/*----------- epaper defines and global varaiables ---------------------*/
#ifdef ESP8266
   #define SD_CS    2
   #define SRAM_CS 16
   #define EPD_CS   0
   #define EPD_DC   15
#endif
#ifdef ESP32
  #define SD_CS       14
  #define SRAM_CS     32
  #define EPD_CS      15
  #define EPD_DC      33  
#endif
#if defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined(ARDUINO_FEATHER_M4) || defined (__AVR_ATmega328P__) || defined(ARDUINO_NRF52840_FEATHER)
  #define SD_CS       5
  #define SRAM_CS     6
  #define EPD_CS      9
  #define EPD_DC      10  
#endif
#ifdef TEENSYDUINO
  #define SD_CS       8
  #define SRAM_CS     3
  #define EPD_CS      4
  #define EPD_DC      10  
#endif
#ifdef ARDUINO_STM32_FEATHER
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
#endif
#ifdef ARDUINO_NRF52832_FEATHER
  #define SD_CS       27
  #define SRAM_CS     30
  #define EPD_CS      31
  #define EPD_DC      11  
#endif
#ifdef ARDUINO_SAMD_NANO_33_IOT  //https://forum.arduino.cc/index.php?topic=636237.0 OR C:\Users\Bob\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.8.4\boards.txt
  #define EPD_DC      9  // https://learn.adafruit.com/adafruit-eink-display-breakouts/arduino-code
  #define EPD_RESET   5  // Normally not defined, and set below
  #define EPD_CS      10 // https://learn.adafruit.com/adafruit-eink-display-breakouts/arduino-code
  #define SRAM_CS     8  // https://learn.adafruit.com/adafruit-eink-display-breakouts/arduino-code
  #define EPD_BUSY    3  // Normally not defined, and set below

  #define SD_CS       4  // Personal Choice to connect to epaper.
#endif

//#define EPD_RESET   -1 // can set to -1 and share with microcontroller Reset!
//#define EPD_BUSY    -1 // can set to -1 to not use a pin (will wait a fixed delay)

/* Uncomment the following line if you are using 2.13" tricolor EPD */
//Adafruit_IL0373 epd(212, 104 ,EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
/* Uncomment the following line if you are using 2.13" monochrome 250*122 EPD */
Adafruit_SSD1675 epd(250, 122, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

//bool drawBitmap = false;
long lastEPDupdatemillis = 0;
bool disableClear = false;



/*----------- Wifi defines and global variables---------------------*/
const String dotToStr = ".";
const String trueToStr = "true";
const String falseToStr = "false";
const String spToStr = " ";

const char null_client_detected_skipping_checking_for_response[] = "null client detected, skipping checking for response";
const char checking_for_http_response[] = "checking for http response";
const char http_response_loop_exit[] = "http response loop exit";
const char received_[] = "received=";
const char __Web[] = "--Web";
const char End_of_HTTP_request_response_parsing_Free_memory_[] = "End of HTTP request/response parsing Free memory=";
const char checking_for_https_response[] = "checking for https response";
const char https_response_loop_exit[] = "https response loop exit";
const char End_of_HTTPS_request_response_parsing_Free_memory_[] = "End of HTTPS request/response parsing, Free memory=";

//char ssid[] = "House";      //  your network SSID (name)
//char pass[] = "@11thAvenue";       // your network password
int keyIndex = 0;                  // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Initialize the Wifi Web client library
//WiFiSSLClient client;
// server address:

//reset
void(*resetFunc)(void) = 0;

// defined web pages to make http requests to
typedef bool (*httpsResponseReceiver)(WiFiSSLClient client);
typedef bool (*httpResponseReceiver)(WiFiClient client);
long uptimeAtAP = 0;

char ARDUINO_WEB_SERVER[] = "www.arduino.cc";
unsigned int ARDUINO_WEB_PORT = 80;
char ARDUINO_LOGO_WEBLOCATION[] = "/asciilogo.txt";

//https://raw.githubusercontent.com/adafruit/Adafruit_Learning_System_Guides/master/EInk_Autostereograms/images/adafruit.bmp
char ADAFRUIT_WEB_SERVER[] = "raw.githubusercontent.com";
unsigned int ADAFRUIT_WEB_PORT = 443;
char ADAFRUIT_LOGO_WEBLOCATION[] = "/adafruit/Adafruit_Learning_System_Guides/master/EInk_Autostereograms/images/adafruit.bmp";

char VANPI_WEB_SERVER[] = GPSD_IP_STR;
unsigned int VANPI_WEB_PORT = 80;
char VANPI_LOGO_WEBLOCATION[] = "/roadtripbusinessend.bmp";

//bool sendRequest = true; // used to understand if the http request must be sent
bool isInternetConnected = false;
bool isNetworkConfigured = false;

/*----------Web Server-----------*/
WiFiServer webserver(80);
//#define REQUESTIN_BUFFER_SIZE 8192
//char httprequestin[REQUESTIN_BUFFER_SIZE]; 


const char HTTP1_1_200_OK[] = "HTTP/1.1 200 OK";
const char Content_Type_text_html[] = "Content-Type: text/html";
const char Connection_close[]="Connection: close";
const char HTML_DOCTYPE_HTML[]="<!DOCTYPE HTML>";
const char HTML_html[]="<html>";
const char HTML_br[]="<br/>";
const char EOL[]="<br />";
const char analog_input[]="analog input ";
const char _is_[]=" is ";
const char  Timestamp_Category_Value_Units_tag_eor[]="Timestamp,Category,Value,Units,tag,<br/>";
const char _SystemVoltage_[]=",SystemVoltage,";
const char milli_[]=",milli-";
const char _BatteryRate_[]=",BatteryRate,";
const char _BatteryNetCharge_[]=",BatteryNetCharge,";
const char _SolarStatus_[]=",SolarStatus,";
const char _SolarSupply_[]=",SolarSupply,";
const char _EstFullCapacity_[]=",EstFullCapacity,";
const char _EstAvailCapacity_[]=",EstAvailCapacity,";
const char _mAh[]=",mAh";
const char EOR[]=",<br />";
const char Timestamp_BatteryAmp_SystemVoltage_SolarStatus_SolarSupply_eor[]="Timestamp,BatteryAmp,SystemVoltage,SolarStatus,SolarSupply,<br />";
const char hourUTC_hourlocal_batterynet_eor[]="hour(UTC),hour(local),batterynet,<br />";
const char TEXT_COMMA[]=",";
const char First_NTP_sync_epoch_[]="First NTP sync epoch = ";
const char Local_time_of_last_outgoing_NTP_sync_request_[]="Local time of last outgoing NTP sync request=";
const char Unix_Epoch_calculated_from_incoming_response_from_NTP_[]="Unix Epoch calculated from incoming response from NTP=";
const char Last_outgoing_NTP_sync_response_sent_to_IP_[] = "Last outgoing NTP sync response sent to IP=";
const char TEXT_PERIOD[]=".";
const char Last_outgoing_NTP_sync_response_sent_epoch_value_[]="Last outgoing NTP sync response sent epoch value=";
const char Number_of_NTP_requests_served_[]="Number of NTP requests served=";
const char Adjustments_to_RTC_actual_elapsed_[]="Adjustments to RTC (actual elapsed)=";
const char _rtc_elapsed___[] = "+(rtc elapsed)*(";
const char TEXT_SLASH[]="/";
const char __eor[]="),<br />";
const char Current_drift_adjustment_to_outgoing_NTP_response[]="Current drift adjustment to outgoing NTP response=";
const char rtc_returned_seconds_elapsed_actual_seconds_elapsed_from_NTP_br[] = "#,rtc returned seconds elapsed, actual seconds elapsed from NTP,<br />";
const char NTP_Sync_Request_received_br[]="NTP Sync Request received<br>";
const char Current_time_is_[] = "Current time is ";
const char NTP_Sync_Request_DENIED_bc_sync_was_just_run_[] = "NTP Sync Request DENIED b/c sync was just run ";
const char seconds_ago_br[] = "seconds ago<br>";
const char br_free_memory_[] = "<br>free memory=";
const char br_raw_[] = "<br>raw=";
const char br_methodcode_[] = "<br>methodcode=";
const char br_line1_[]= "<br>line1=";
const char br_uri_[]= "<br>uri=";
const char br_url_[]= "<br>url=";
const char br_query_[] = "<br>query=";
const char This_url_does_not_exist__No_404_error__But_you_need_to_select_one_of_these_adc_live_powermeter_powerhistory_hourly_ntp[] = "This url does not exist.  No 404 error.  But you need to select one of these: /adc,/live, /powermeter,/powerhistory,/hourly,/ntp";
const char HTML_CHTML[] = "</html>";

const String URL_adc = "/adc";
const String URL_live = "/live";
const String URL_powermeter = "/powermeter";
const String URL_powerhistory = "/powerhistory";
const String URL_hourly = "/hourly";
const String URL_ntp = "/ntp";
const String URL_timesync = "/timesync";



/*----------NTP Client-----------*/
const char Sending_NTP_packet[] = "Sending NTP packet";
const char packet_received[] = "packet received";
const char Seconds_since_Jan_1_1900_[] = "Seconds since Jan 1 1900 = ";
const char Unix_time_[] = "Unix time = ";
const char Epoch_or_date_in_seconds_when_last_synced_with_NTP_[] = "Epoch (or date in seconds) when last synced with NTP = ";
const char Epoch_or_date_in_seconds_of_RTC_[] = "Epoch (or date in seconds) of RTC = ";
const char Seconds_passed_on_RTC_therefore_Expected_from_NTP_[] = "Seconds passed on RTC (therefore Expected from NTP) = ";
const char Epoch_or_date_in_seconds_of_NTP_response_[]  = "Epoch (or date in seconds) of NTP response = ";
const char Seconds_passed_according_to_NTP_therefore_actual_[] = "Seconds passed according to NTP (therefore actual) = ";
const char Data_bin_for_drift_calculation_[] = "Data bin for drift calculation = ";
const char The_UTC_time_is[] = "The_UTC_time_is";
const char __NTP[] = "--NTP";
const char rtc_clock_drift_calculated[] = "rtc clock drift calculated";
const char rtcclock_returned_year_month_day_hr_min_sec_[] = "rtcclock() returned year,month,day,hr,min,sec:";
const char RTC_adjusted_for_drift_to_NTP_server_[] = "RTC adjusted for drift to NTP server: ";
const char sec[] = "sec";
const char End_of_NTPRequest_Free_memory_[] = "End of NTPRequest(), Free memory =";
const char No_NTP_response_received[] = "No NTP response received";

/* Create an rtc object, to handle */
RTCZero rtc;
//const int GMT = 2; //change this to adapt it to your time zone

/*internal storage of timestamps*/
struct utcdatetime {
    unsigned long epoch;
    unsigned int year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// NTP sync detail variables
struct utcdatetime datetimeAtConnect;
struct utcdatetime datetimeAtNTP;
unsigned long uptimeAtNTP = 0; 
#define UDP_RESPONSE_PORT 2390        // local port to listen for UDP response
//unsigned int localPort = 2390;      // local port to listen for UDP response
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress nistServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress gpsServer(GPSD_IP_1, GPSD_IP_2, GPSD_IP_3, GPSD_IP_4); // GPSD NTP server
//IPAddress timeServer(128,138,141,172); // utcnist2.colorado.edu NTP server

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp2390;
// Global buffer for UDP response packets, and protocol adapter
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

//Time zone of local
//unsigned long tz = -5; //New York/EST
long tz = -8; //Los Angeles/PST
long tz2 = 0;
String tzabbr = "PST(-8)";

byte rtcdriftindex = 0;
long rtcdriftactual[48];
long rtcdriftexpected[48];
long excludeEpochFromDrift = 0;


/*----------Sensor readings-----------*/
struct linearconv {
    long Bn;
    long Bd;
    long A;
    String units;
};
struct analogsamplesum {
    unsigned long ms;
    unsigned int pin;
    unsigned long count;
    unsigned long sum;
    unsigned int mx;
    unsigned int mn;
    unsigned int avg;
    int calibrate;
    unsigned int tovolts;
    struct linearconv* converter;
    int converted;
};
int fudge_for_ammeter = 1676-1686; //1700, 1676-1710; //-30 //bottom one
int fudge_for_ammeter2 = 1676-1672; //1676-1686 //top one
int fudge_for_voltameter = 2474-2502; //2545; //-69
int sample_count = 300; // 300 is biggest sample for 12bit dac before overflow long numerator of average : 300*4095*3300=4,054,050,000
int sample_delay = 0;
struct linearconv ANALOG_VOLT_TO_AMP = {100,1,-167600, "amps"};  //every millivolt is an amp
//1676*1000 = 1676000
struct linearconv ANALOG_VOLT_TO_12V = {5,1,0, "volts"};  //every millivolt is an amp
//10.81 multimeter on battery
//2.162 multimeter on divider lead
//1.823 on potentiometer lead
//  for 0.84181313 ratio between the 2 voltage divider (1st is 5:1 ratio)
// 1/5 * 1.823/2.162 = 1.823/(5*2.162) = 1.823/10.81 = 1/5.929786 is ratio on second divider
struct linearconv ANALOG_VOLT_TO_18V = {593,100,0, "volts"};  //200Kohm resister on divider made ratio 


// sampling interval control variables
struct aggregate {
    long count;
    long sum;
    int mx;
    int mn;
    int avg;
    struct linearconv* converter;
    int converted;
};
unsigned long nowAhMillis;
long totalAh;
long nowAh;
long nowAhCount;
long nowAhInterval = 60000;
long dischargedAh=0; // should be negative or 0.  Never positive
bool isdischargeestimate = true;
long batteryCapacityAh = 300000;
long lostCapacityAh    = 0;
long sampleStartMillis=0;
unsigned long samplesetMillis;
struct utcdatetime totalAhDatetime;
typedef unsigned long (*MillisGenerator)();
typedef unsigned int (*HourGenerator) ();
MillisGenerator currentMillis;
HourGenerator prevHour;
HourGenerator nowHour;


struct aggregate hourlyAmpSum[24];

struct analogsamplesum amperesMinuteTot = {0, A1, 1, 2048, 0 ,0, 2048, fudge_for_ammeter2, 1676, &ANALOG_VOLT_TO_AMP, 0};
struct analogsamplesum voltageMinuteTot = {0, A6, 0, 0, 0 ,0, 0, fudge_for_voltameter, 0, &ANALOG_VOLT_TO_12V, 0};
struct analogsamplesum solarMinuteTotA = {0, A2, 1, 2048, 0 ,0, 2048, fudge_for_ammeter,1676, &ANALOG_VOLT_TO_AMP, 0};
struct analogsamplesum solarMinuteTotV = {0, A7, 0, 0, 0 ,0, 0, fudge_for_voltameter, 0, &ANALOG_VOLT_TO_18V, 0};

struct analogsamplesum amperesForWeb;
struct analogsamplesum voltageForWeb;
struct analogsamplesum solarAmpForWeb;
struct analogsamplesum solarVoltForWeb;


/* high level drawing routines */
//drawgraph() enumerations
#define NO_BORDER 0
#define SHOW_XY_AXIS 1
#define SHOW_XY_AXIS_W_BORDERS 2
#define DATA_COUNT 144

//data to send to drawgraph() to recreate graph
byte graphdatahead = 0;
//struct utcdatetime datatime[DATA_COUNT]; 
int batteryamppxgraph[DATA_COUNT]; 
int batteryvoltpxgraph[DATA_COUNT]; 
int solaramppxgraph[DATA_COUNT];
int solarvoltpxgraph[DATA_COUNT]; 
//int graphactualmah[DATA_COUNT];

struct downloadable_history {
    //unsigned long epoch;
    struct utcdatetime utc;
    
    int batterymA; //milliamps
    int batterymV; //millivolts
    int solarmA; //milliamps
    int solarmV; //millivolts
}; // it's stored in millivolts and milliamps, but the resolution of the analog readings isn't that good.  Maybe to the tenths place.  The extra numbers are just to support arithmetic and rounding support.
// 12-bit adc means 4096 values, which to have milli-resolution, the thing being measured can only have range 0 to 4.1
struct downloadable_history readings[DATA_COUNT];

// just counters for debugging what is a zero level for amp sensor
long debugginglongtermA = 0;
long debugginglongtermCount = 0;
long debugginglongSolarA = 0;


#define MIN_PER_GRAPH_PX  10 // 144 graph data points.  So 1 means >2hours on graph.  10 means 1440min or 24hours.

typedef void (*DataPoint) (const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1);
typedef void (*AxesTick) (const int l, const int t, const int color);

typedef int (*ToPx) (const int y);
struct graphproperties {
    unsigned int zeroX;
    unsigned int zeroY;
    
    int rangePlusY;
    int rangeNegY;
    int rangePlusX;
    int rangeNegX;

    unsigned int lastDataX;
    unsigned int lastDataY;
};

//       overflow for long/ nominal voltage reading = number of readings before overflow
//decide 4000000000/(12000) = 333333.333
//333333.333/60 = 5555.555, assuming 1min per reading, 5555 hours before overflow
//5555.55 / 24 = 231 days before overflow
//so let's log every minute's Ah for an hour, then add that value to a uptime aggregate Ah value.  
//...There is no way that this overflows for a van.  The battery should run out of power.

// 12.75 to 11.50, 
//unsigned byte soc[120] = {};
//trying to set an initial maxAh, from boot

//what to graph, if NTP is unavailable?

// for NTP vs RTC drift calculator
struct linearconv rtcdrift = {0,21600,0,"sec"}; //6 hours = 21600, so the least sqr regression should return in fraction of 21600


/*----------NTP server-----------*/
// All code copied from http://lloydm.net/Demos/NTP/NTP-GPS_Server.ino.txt
//
// buffers for receiving and sending data
const int NTPS_PACKET_SIZE = 48; //same as above sending, and receiving response, but separate instance
byte packetBuffer2[NTPS_PACKET_SIZE];  // same as sending, and receiving response

const char Checking_if_NTP_request_received_on_port_123___[] = "Checking if NTP request received on port 123...";
const char Received_UDP_packet_size_[] = "Received UDP packet size ";
const char From_[] = "From ";
const char __port_[] = ", port ";
const char LI_Vers_Mode_[] = "  LI, Vers, Mode :";
const char Stratum_[] = "  Stratum :";
const char Polling_[] = "  Polling :";
const char Precision_[] = "  Precision :";
const char packet_dump_[] = "packet dump:";
const char Local_time_is_[] = "Local time is =";
const char UTC_Time_in_seconds_[] ="UTC Time in seconds="; 
const char Drift_of_clock_calculated_is_[] = "Drift of clock calculated is="; 
const char UTC_Time_sent_[] = "UTC Time sent="; 
const char Time_sent_is_supposed_to_be_[] = "Time sent is supposed to be =";
const char No_NTP_request_received[] = "No NTP request received";

byte lastremoteip[4];
unsigned long lastremoteepoch;
unsigned long ntpscount;


/*----------Bluetooth-----------*/


BLEService btService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEUnsignedIntCharacteristic systemVoltageCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead);
BLEUnsignedIntCharacteristic solarVoltageCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead);
BLEIntCharacteristic batterymAmpsCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1216", BLERead);
BLEIntCharacteristic solarmAmpsCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1217", BLERead);
BLELongCharacteristic batteryNetmAhCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1218", BLERead);
BLELongCharacteristic batteryTopDischargemAhCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1219", BLERead);

//BLEUnsignedCharCharacteristic









/*---------------------------------------------------
----------------------------------------------------

Run once at boot

-----------------------------------------------------
----------------------------------------------------*/

void setup() {
  //setDebugMessageLevel(3); // used to set a level of granularity in information output [0...4]
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  /*----------- epaper ---------------------*/
  Serial.begin(115200);

  if(DEBUG_SETUP) {
    for(int i=0; i<2000; i++)
      Serial.print(".");
    Serial.println();
    Serial.println();
    
    Serial.println(F("Van Power Meter Starting"));
    Serial.println(F("setup() enter"));
    
    Serial.print(F("Initializing SD card..."));
  }
  bool drawBitmap =false;
  if (!SD.begin(SD_CS)) {
    if(DEBUG_SETUP) Serial.println(F("failed!"));
  } else {
    drawBitmap = true;
    File root = SD.open("/");
    printDirectory(root, 0);
    if(DEBUG_SETUP) Serial.println(F("done!"));
  }

  if(DEBUG_SETUP) {
    Serial.println(F("Big object sizes..."));


    long bigobjects = 0;
//    Serial.println(F("Graph Data x-axis dates:"));
//    Serial.print(sizeof(datatime));
//    bigobjects += sizeof(datatime);
//    Serial.print(F(" / "));
//    Serial.println(bigobjects);
    
    Serial.println(F("Graph Data y-axis voltage values:"));
    Serial.print(sizeof(batteryvoltpxgraph));
    bigobjects += sizeof(batteryvoltpxgraph);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.println(F("Graph Data y-axis amp values:"));
    Serial.print(sizeof(solaramppxgraph));
    bigobjects += sizeof(solaramppxgraph);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.println(F("Graph Data y-axis solar voltage values:"));
    Serial.print(sizeof(solarvoltpxgraph));
    bigobjects += sizeof(solarvoltpxgraph);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    //Serial.println(F("Graph Data integration of y-values :"));
    //Serial.print(sizeof(graphactualmah));
    //bigobjects += sizeof(graphactualmah);
    //Serial.print(F(" / "));
    //Serial.println(bigobjects);


    Serial.println(F("Daily amp (dis)charge avg by hour :"));
    Serial.print(sizeof(hourlyAmpSum));
    bigobjects += sizeof(hourlyAmpSum);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    
//    Serial.println("History Timestamps :");
//    Serial.print(sizeof(historytimestamp));
//    bigobjects += sizeof(utcdatetime) * HISTORY_LENGTH;
//    Serial.print(" / ");
//    Serial.println(bigobjects);

    Serial.println(F("History Readings :"));
//    Serial.print(sizeof(historyamp) + sizeof(historyvolt) + sizeof(historysolarvolt) + sizeof(historysolaramp) );
//    bigobjects += sizeof(historyamp) + sizeof(historyvolt) + sizeof(historysolarvolt) + sizeof(historysolaramp);
    Serial.print(sizeof(readings));
    bigobjects += sizeof(readings);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.println(F("Total array initialization:"));
    Serial.print(bigobjects);
    Serial.println(F("/32,000 SRAM for Arduino Nano 33 IoT"));
    //https://www.mouser.com/new/arduino/arduino-nano-33-iot/

    
    Serial.println(F("Strings in constant area :"));

    Serial.print(dotToStr);
    Serial.print(sizeof(dotToStr) + dotToStr.length());
    bigobjects += sizeof(dotToStr + dotToStr.length());
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(trueToStr);
    Serial.print(sizeof(trueToStr) + trueToStr.length());
    bigobjects += sizeof(trueToStr + trueToStr.length());
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(falseToStr);
    Serial.print(sizeof(falseToStr) + falseToStr.length());
    bigobjects += sizeof(falseToStr + falseToStr.length());
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(spToStr);
    Serial.print(sizeof(spToStr) + spToStr.length());
    bigobjects += sizeof(spToStr + spToStr.length());
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    
    Serial.print(Checking_if_NTP_request_received_on_port_123___);
    Serial.print(sizeof(Checking_if_NTP_request_received_on_port_123___));
    bigobjects += sizeof(Checking_if_NTP_request_received_on_port_123___);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Received_UDP_packet_size_);
    Serial.print(sizeof(Received_UDP_packet_size_) );
    bigobjects += sizeof(Received_UDP_packet_size_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(From_);
    Serial.print(sizeof(From_));
    bigobjects += sizeof(From_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(__port_);
    Serial.print(sizeof(__port_));
    bigobjects += sizeof(__port_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(LI_Vers_Mode_);
    Serial.print(sizeof(LI_Vers_Mode_));
    bigobjects += sizeof(LI_Vers_Mode_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Stratum_);
    Serial.print(sizeof(Stratum_));
    bigobjects += sizeof(Stratum_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(Polling_);
    Serial.print(sizeof(Polling_));
    bigobjects += sizeof(Polling_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(Precision_);
    Serial.print(sizeof(Precision_));
    bigobjects += sizeof(Precision_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(packet_dump_);
    Serial.print(sizeof(packet_dump_));
    bigobjects += sizeof(packet_dump_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Local_time_is_);
    Serial.print(sizeof(Local_time_is_));
    bigobjects += sizeof(Local_time_is_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(UTC_Time_in_seconds_);
    Serial.print(sizeof(UTC_Time_in_seconds_));
    bigobjects += sizeof(UTC_Time_in_seconds_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Drift_of_clock_calculated_is_);
    Serial.print(sizeof(Drift_of_clock_calculated_is_));
    bigobjects += sizeof(Drift_of_clock_calculated_is_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(UTC_Time_sent_);
    Serial.print(sizeof(UTC_Time_sent_));
    bigobjects += sizeof(UTC_Time_sent_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Time_sent_is_supposed_to_be_);
    Serial.print(sizeof(Time_sent_is_supposed_to_be_));
    bigobjects += sizeof(Time_sent_is_supposed_to_be_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(No_NTP_request_received);
    Serial.print(sizeof(No_NTP_request_received));
    bigobjects += sizeof(No_NTP_request_received);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(null_client_detected_skipping_checking_for_response);
    Serial.print(sizeof(null_client_detected_skipping_checking_for_response));
    bigobjects += sizeof(null_client_detected_skipping_checking_for_response);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(checking_for_http_response);
    Serial.print(sizeof(checking_for_http_response));
    bigobjects += sizeof(checking_for_http_response);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(http_response_loop_exit);
    Serial.print(sizeof(http_response_loop_exit));
    bigobjects += sizeof(http_response_loop_exit);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(received_);
    Serial.print(sizeof(received_));
    bigobjects += sizeof(received_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(__Web);
    Serial.print(sizeof(__Web));
    bigobjects += sizeof(__Web);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(End_of_HTTP_request_response_parsing_Free_memory_);
    Serial.print(sizeof(End_of_HTTP_request_response_parsing_Free_memory_));
    bigobjects += sizeof(End_of_HTTP_request_response_parsing_Free_memory_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(checking_for_https_response);
    Serial.print(sizeof(checking_for_https_response));
    bigobjects += sizeof(checking_for_https_response);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(https_response_loop_exit);
    Serial.print(sizeof(https_response_loop_exit));
    bigobjects += sizeof(https_response_loop_exit);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(End_of_HTTPS_request_response_parsing_Free_memory_);
    Serial.print(sizeof(End_of_HTTPS_request_response_parsing_Free_memory_));
    bigobjects += sizeof(End_of_HTTPS_request_response_parsing_Free_memory_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);


    Serial.print(Sending_NTP_packet);
    Serial.print(sizeof(Sending_NTP_packet));
    bigobjects += sizeof(Sending_NTP_packet);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(packet_received);
    Serial.print(sizeof(packet_received));
    bigobjects += sizeof(packet_received);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Seconds_since_Jan_1_1900_);
    Serial.print(sizeof(Seconds_since_Jan_1_1900_));
    bigobjects += sizeof(Seconds_since_Jan_1_1900_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Unix_time_);
    Serial.print(sizeof(Unix_time_));
    bigobjects += sizeof(Unix_time_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Epoch_or_date_in_seconds_when_last_synced_with_NTP_);
    Serial.print(sizeof(Epoch_or_date_in_seconds_when_last_synced_with_NTP_));
    bigobjects += sizeof(Epoch_or_date_in_seconds_when_last_synced_with_NTP_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Epoch_or_date_in_seconds_of_RTC_);
    Serial.print(sizeof(Epoch_or_date_in_seconds_of_RTC_));
    bigobjects += sizeof(Epoch_or_date_in_seconds_of_RTC_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Seconds_passed_on_RTC_therefore_Expected_from_NTP_);
    Serial.print(sizeof(Seconds_passed_on_RTC_therefore_Expected_from_NTP_));
    bigobjects += sizeof(Seconds_passed_on_RTC_therefore_Expected_from_NTP_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Epoch_or_date_in_seconds_of_NTP_response_);
    Serial.print(sizeof(Epoch_or_date_in_seconds_of_NTP_response_));
    bigobjects += sizeof(Epoch_or_date_in_seconds_of_NTP_response_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(Seconds_passed_according_to_NTP_therefore_actual_);
    Serial.print(sizeof(Seconds_passed_according_to_NTP_therefore_actual_));
    bigobjects += sizeof(Seconds_passed_according_to_NTP_therefore_actual_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(Data_bin_for_drift_calculation_);
    Serial.print(sizeof(Data_bin_for_drift_calculation_));
    bigobjects += sizeof(Data_bin_for_drift_calculation_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(The_UTC_time_is);
    Serial.print(sizeof(The_UTC_time_is));
    bigobjects += sizeof(The_UTC_time_is);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(__NTP);
    Serial.print(sizeof(__NTP));
    bigobjects += sizeof(__NTP);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(rtc_clock_drift_calculated);
    Serial.print(sizeof(rtc_clock_drift_calculated));
    bigobjects += sizeof(rtc_clock_drift_calculated);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(rtcclock_returned_year_month_day_hr_min_sec_);
    Serial.print(sizeof(rtcclock_returned_year_month_day_hr_min_sec_));
    bigobjects += sizeof(rtcclock_returned_year_month_day_hr_min_sec_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(RTC_adjusted_for_drift_to_NTP_server_);
    Serial.print(sizeof(RTC_adjusted_for_drift_to_NTP_server_));
    bigobjects += sizeof(RTC_adjusted_for_drift_to_NTP_server_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(sec);
    Serial.print(sizeof(sec));
    bigobjects += sizeof(sec);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(End_of_NTPRequest_Free_memory_);
    Serial.print(sizeof(End_of_NTPRequest_Free_memory_));
    bigobjects += sizeof(End_of_NTPRequest_Free_memory_);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    Serial.print(No_NTP_response_received);
    Serial.print(sizeof(No_NTP_response_received));
    bigobjects += sizeof(No_NTP_response_received);
    Serial.print(F(" / "));
    Serial.println(bigobjects);

    
    Serial.print(URL_adc);
    Serial.print(sizeof(URL_adc));
    bigobjects += sizeof(URL_adc);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(URL_live);
    Serial.print(sizeof(URL_live));
    bigobjects += sizeof(URL_live);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(URL_powermeter);
    Serial.print(sizeof(URL_powermeter));
    bigobjects += sizeof(URL_powermeter);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(URL_powerhistory);
    Serial.print(sizeof(URL_powerhistory));
    bigobjects += sizeof(URL_powerhistory);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(URL_hourly);
    Serial.print(sizeof(URL_hourly));
    bigobjects += sizeof(URL_hourly);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(URL_ntp);
    Serial.print(sizeof(URL_ntp));
    bigobjects += sizeof(URL_ntp);
    Serial.print(F(" / "));
    Serial.println(bigobjects);
    
    Serial.print(URL_timesync);
    Serial.print(sizeof(URL_timesync));
    bigobjects += sizeof(URL_timesync);
    Serial.print(F(" / "));
    Serial.println(bigobjects);


    Serial.println(F("Partial string initialization + array:"));
    Serial.print(bigobjects);
    Serial.println(F("/32,000 SRAM for Arduino Nano 33 IoT"));
    //https://www.mouser.com/new/arduino/arduino-nano-33-iot/

  }
  

  if(DEBUG_SETUP) Serial.println(F("EPD begin"));
  epd.begin();
  epd.setTextWrap(true);
  epd.setTextSize(1);

  if(DEBUG_SETUP) Serial.println(F("Starting e-paper startup self-diagnostic... it just draws random stuff so you know it's working"));
  // Draw a bitmap
  if (drawBitmap) {
    epd.clearBuffer();  
    epd.fillScreen(EPD_WHITE);
    bmpDraw("/blinka.bmp",0,0);
  
    delay(15*1000);
  }
  startupdraw();

  //initialize critical method calls, to see if the stack space is sufficient.
  //initializing here, puts any variables that seem persistent on the heap, at the bottom of the heap
  //so that fragmentation can be fixed.
  if(DEBUG_FREE_MEMORY) {
    Serial.print(F("Before intializing critical methods, Free memory="));
    Serial.println(freeMemory());    
  }
  //for listenTo...(), the isNetworkConfigured is currently false, so it will abort checking for HTTP response, regardless of input parameters
  listenToClient(NULL);
  listenToSslClient(NULL);
  if(DEBUG_FREE_MEMORY) {
    Serial.print(F("After intializing critical methods, Free memory="));
    Serial.println(freeMemory());    
  }


  /*----------- Wifi ---------------------*/
  connectToAP();    // connect the board to the access point

  // always start whether or not connection is made, waiting for it to connect later.  
  // restarting these services at reconnect might cause malfunction.  
  // Unfortunately, no way to tell if after wifi connection and disconnection, to check these services are still ok
  if(DEBUG_SETUP) Serial.println(F("starting HTTP server"));
  webserver.begin(); 
  if(DEBUG_SETUP) Serial.println(F("starting response listener for NTP request"));
  Udp2390.begin(UDP_RESPONSE_PORT); // unfortunately to be lazy at code, we listen even when we don't send a request.  Just ignore, and flush it before we send a NTP request
  if(DEBUG_SETUP) Serial.println(F("starting NTPS"));
  startServerNTP();
      
  if(isWifiConnected()) {
    if(DEBUG_SETUP) Serial.println(F("setup() Wifi connected"));
    printWifiStatus();

    /*-- Webserver... It's going to be at least 45seconds before next service --*/
    if(DEBUG_SETUP) Serial.println(F("initial HTTP request check"));
    httprequesthandle();
  
    /*------------NTP UDP--------------------------*/
    if(DEBUG_SETUP) Serial.println(F("sending NTP request for current time"));
    
    // unsigned long epoch = WiFi.getTime(); //used the old examples for NTP b/c I can't figure out where to change the NTP server in this call
    // rtc.setEpoch(epoch); //so this line is in ntprequest(), after custom coded call to NTP
    rtc.begin();
    ntprequest(); // this will assume NIST server, so van boot will take a little longer NIST doesnt respond.
    delay(15000); //force a delay here...

    if(DEBUG_FREE_MEMORY) {
      Serial.print(F("After starting NTPS, Free memory="));
      Serial.println(freeMemory());    
    }
      
    // Check connectivity to Internet thru website
    if(DEBUG_SETUP) Serial.println(F("making http request"));
    configureNetwork();
    //httpRequest(ARDUINO_WEB_SERVER, ARDUINO_LOGO_WEBLOCATION, ARDUINO_WEB_PORT);
    //disableClear = true;
    //httpsRequest(ADAFRUIT_WEB_SERVER, ADAFRUIT_LOGO_WEBLOCATION, ADAFRUIT_WEB_PORT);
    //disableClear = false;
  }

  /*------------ Sensor Data collection setup --------------------------*/
  analogReadResolution(12);

  for(int i=0; i<DATA_COUNT; i++) {
    readings[i].utc.epoch=0;
  }

  //1676*4095/3300 = 2079.7636
  struct aggregate zeroreadingAh = {0 ,0, 0, 0, 0, NULL, 0};
  for(int i=0; i<24; i++)
    hourlyAmpSum[i] = zeroreadingAh;
  
  // there are two counters.  one that resets at top of second hand, another resets at top of hour hand.
  if(validclock()) { // we want samplesets to reset on the minute hand of clock
    currentMillis = rtcMinuteMillis;
    prevHour = rtcPrevHour;
    nowHour = rtcNowHour;

    samplesetMillis = millis();
    struct utcdatetime now = rtcclock();

    samplesetMillis -= now.second*1000;
    nowAhMillis = samplesetMillis; // - now.minute*60000UL;
    totalAhDatetime = now;
    nowAhInterval = samplesetMillis + (60-now.minute)*60000UL;
    
  } else {
    currentMillis = uptimeMinuteMillis;
    prevHour = uptimePrevHour;
    nowHour = uptimeNowHour;

    samplesetMillis = millis(); //we have no clock, so we can't tell which minute is is, anymore.
    nowAhMillis = samplesetMillis;
  }
  

  if(DEBUG_SETUP) Serial.println(F("Initial sensor readings"));
  struct analogsamplesum amperes = amperesForWeb = getsample(A1, fudge_for_ammeter2, &ANALOG_VOLT_TO_AMP);
  struct analogsamplesum voltage = voltageForWeb = getsample(A6, fudge_for_voltameter, &ANALOG_VOLT_TO_12V);
  struct analogsamplesum solarA = solarAmpForWeb = getsample(A2, fudge_for_ammeter, &ANALOG_VOLT_TO_AMP);
  struct analogsamplesum solarV = solarVoltForWeb = getsample(A7, fudge_for_voltameter, &ANALOG_VOLT_TO_18V);
  // absAmpReading(&solarA);
  solarAmpForWeb = solarA;

//struct analogsamplesum amperesForWeb;
//struct analogsamplesum voltageForWeb;
//struct analogsamplesum solarAmpForWeb;
//struct analogsamplesum solarVoltForWeb;

  if(DEBUG_SETUP) {
    struct analogsamplesum tmp = getsample(A2, fudge_for_ammeter, &ANALOG_VOLT_TO_AMP);
    for(int i=0; i<4096; i++) {
      tmp.sum = tmp.count * i;
      computesampleAvg(&tmp);
      computesampleVolts(&tmp);
      computesampleConversion(&tmp);
      Serial.print(F("Test reading="));
      Serial.print(tmp.avg);
      Serial.print(F(" mv="));
      Serial.print(tmp.tovolts);
      Serial.print(F(" amps="));
      Serial.print(tmp.converted);
      absAmpReading(&tmp);
      Serial.print(F("       ABS'ed="));
      Serial.print(tmp.avg);
      Serial.print(F(" mv="));
      Serial.print(tmp.tovolts);
      Serial.print(F(" amp="));
      Serial.println(tmp.converted);
    }
  }
  
  // estimate battery state of change and current capacity supported
  // first establish depth of discharge. maxAh = -50ah
  // then use amp hours from then on, another -50ah = -100ah, or 33%
  // then use amp house from then on, another + 70ah = -30ah, etc
  // but when amps <1 and voltage > 13, this means it's fullly charged
  // if ah =-30, interpret real capacity has decreased and this is new capacity    
  if(DEBUG_SETUP) {
    Serial.println(F("Initializing depth of discharge"));
    Serial.println(F("discharge table"));
    Serial.print("     ");
    for(int amp=0; amp<20000; amp+=2000) {
      Serial.print(amp);
      Serial.print(F("mA|"));
    }
    Serial.println();
    for(int volt=12700; volt>11500; volt-=200) {
      Serial.print(volt);
      Serial.print(F("mv|"));
      for(int discharge=0; discharge<20000; discharge+=2000){
        int soc = estimateStateOfCharge(volt, -discharge);
        Serial.print(soc);
        Serial.print(F("|"));
      }
      Serial.println();
    }
    Serial.println(F("charging table"));
    for(int volt=11300; volt<13000; volt+=200) {
      Serial.print(volt);
      Serial.print(F("v|"));
      for(int charge=0; charge<20000; charge+=2000){
        int soc = estimateStateOfCharge(volt, charge);
        Serial.print(soc);
        Serial.print(F("|"));
      }
      Serial.println();
    }
  }
  updatestateofcharge(0, amperes.converted, voltage.converted);
  if(DEBUG_SETUP) {   
    Serial.print(F("Voltage read = "));
    Serial.println(voltage.converted);
    Serial.print(F("Initial Estimated Battery State of charge = "));
    Serial.println(dischargedAh);
    Serial.print(F("/"));
    Serial.println(batteryCapacityAh);
  }


  addToGraph(amperes, voltage, solarA, solarV);

  if(DEBUG_SETUP) {   
    Serial.println(F("setup() exit"));
  }
}





//the cooperative multitasker... if one of the tasks stall, oh well...
void loop() {
  if(DEBUG_LOOP)
      Serial.println(F("Next loop"));
  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("Free memory="));
      Serial.println(freeMemory());    
  }

  if(isledon)
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  else
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  isledon=!isledon;
  
  // double check that WIFI is still connected WL_CONNECTED || WL_IDLE_STATUS
  //  show update, when it fails
  if(!isWifiConnected()) {
    if(DEBUG_LOOP)
      Serial.println(F("Wifi lost... attempting to reconnect..."));
    connectToAP();
    // show wifi connection
    if(isWifiConnected()) {
      printWifiStatus();
      // has e-paper display() call, and everyone should have 15 second delay built in

      /*------------NTP UDP, to time servers, receiving response on UDP_RESPONSE_PORT --------------------------*/
      rtc.begin(); 
      Udp2390.begin(UDP_RESPONSE_PORT);
      ntprequest();

      /*------------checking if on internet, or Roadtrip2017 Van --------------------------*/
      configureNetwork();
      
      /*------------NTP listening for requests on port 123 (pretending to be time server)--------------------------*/
      startServerNTP();

      refreshGraph();
    }
    else if(DEBUG_LOOP) {
      Serial.println(F("... unable to reconnect"));
    } else {
      if(uptimeAtAP!=0 && millis()-uptimeAtAP >= 3600000 ) {
        Serial.println(F("... rebooting ..."));
        resetFunc();
      }
    }
  } else {
    // double check time doesnt need updating  
    // Send request to NTP
    //  show update, when it occurs
    if(DEBUG_LOOP) {
      Serial.print(F("Checking clock..."));
      Serial.println(isclockstale() ? F("Stale") : F("OK"));
    }
    if(isclockstale()) { // isWifiConnected() implied
      ntprequest();
      refreshGraph();
    } else if(DEBUG_LOOP) {
      Serial.print(F("Time is "));
      String timestr = formattedlocal(rtcclock());
      Serial.println(timestr);
      timestr.~String();
    }

    // double check that required server is still up every 60 seconds
    //  show update, when it fails
    // Check connectivity to Internet thru website
    //httpRequest();
    //listenToClient();

    if(DEBUG_LOOP) Serial.println(F("checking for HTTP request"));
    httprequesthandle();

    if(DEBUG_LOOP) Serial.println(F("checking for NTP request"));
    handleNTPOn123();
  }

  // Sampling is done on several levels of summation.  Partly to avoid overflow issues
  // L1 is a seconds-long sample of analog readings, averaged and converted to measured units.
  // L2, the L1 samples are reduced it's average and added to the L2 aggregate of samples within either a minute, or whatever the interval is going to be finally
  // L3, the L2 aggregates are added up into the hourly Ah totals, and total uptime Ah totals
  if(DEBUG_LOOP) Serial.println(F("L1 Sampling - or as many samples as possible w/o overflowing the counters"));
  struct analogsamplesum amperes = amperesForWeb = getsample(A1, fudge_for_ammeter2, &ANALOG_VOLT_TO_AMP);
  struct analogsamplesum voltage = voltageForWeb = getsample(A6, fudge_for_voltameter, &ANALOG_VOLT_TO_12V);
  struct analogsamplesum solarA = solarAmpForWeb = getsample(A2, fudge_for_ammeter, &ANALOG_VOLT_TO_AMP);
  struct analogsamplesum solarV = solarVoltForWeb = getsample(A7, fudge_for_voltameter, &ANALOG_VOLT_TO_18V);
  debugginglongSolarA += solarA.converted;
  //absAmpReading(&solarA);
  solarAmpForWeb = solarA;

  //...
  // no real worry about overflow here.
  // this is just to determine the true zero reading level
  // when a current is actually detected, this will be meaningless, and ever-overflowing.
  debugginglongtermA += amperes.converted;
  debugginglongtermCount ++;
  //...

  if(DEBUG_BATT_AMP_SAMPL) Serial.println("amp sample=" + String(amperes.converted) + ", from volts=" + String(amperes.tovolts) + ", readings=" + String(amperes.avg));
  if(DEBUG_BATT_VOLT_SAMPL) Serial.println("volt sample=" + String(voltage.converted) + ", from volts=" + String(voltage.tovolts) + ", readings=" + String(voltage.avg));
  if(DEBUG_SOL_AMP_SAMPL) Serial.println("sol amp sample=" + String(solarA.converted) + ", from volts=" + String(solarA.tovolts) + ", readings=" + String(solarA.avg));
  if(DEBUG_SOL_VOLT_SAMPL) Serial.println("sol volt sample=" + String(solarV.converted) + ", from volts=" + String(solarV.tovolts) + ", readings=" + String(solarV.avg));
  
  if(DEBUG_LOOP) Serial.println(F("Check if Aggregating L2 into L3 and L2 reset"));
  bool rollover = false;
  if(millis() - samplesetMillis > 60000 * MIN_PER_GRAPH_PX) {
    if(DEBUG_LOOP) Serial.println("New sample set old samplesetMillis =" + String(samplesetMillis));
    rollover = true;
    if(validclock()) { // we want samplesetMilliss to reset on the minute hand of clock
      samplesetMillis = millis();
      struct utcdatetime now = rtcclock();
      samplesetMillis -= now.second*1000;
      if(DEBUG_LOOP) Serial.println("...new L2 sample interval start/id by rtc =" + String(samplesetMillis));
      if(millis() - nowAhMillis > nowAhInterval) {
        nowAhMillis = samplesetMillis - now.minute*60000; //minute should always be zero
        long netah = (nowAh/nowAhCount)*(nowAhInterval/1000)/3600; // the weird division is to avoid overflow, so we multiply by a smaller interval
        totalAh+=netah; 
        nowAh=0;
        nowAhCount=0;
        nowAhInterval = 3600000;

        //if(totalAh>maxAh) {
        //  maxAh = totalAh;
        //  sinceMaxAh = 0;
        //}
        int hr = now.hour;
        if(now.minute<15)
          hr = (hr+23)%24;
        struct aggregate *bin = &hourlyAmpSum[hr]; //using time to determine bin
        //struct analogsamplesum *bin = &hourlyAmpSum[graphdatahead/6]; //using the graph to determine bin, always 
        autoadd2(bin, netah); 
        if(DEBUG_LOOP) {
          Serial.println("hourly sum updated");
          for(int i=0; i>24; i++){
            Serial.print(i);
            Serial.print("H f(");
            Serial.print(hourlyAmpSum[i].sum);
            Serial.print("/ ");
            Serial.print(hourlyAmpSum[i].count);
            Serial.print(") =");
            Serial.println(hourlyAmpSum[i].converted);
          }
        }
        
        if(DEBUG_LOOP) Serial.println(F("on the hour, sending http to GPSD/WEBSERVER connectivity notification"));
        char test[] = "/heartbeat.txt";
        httpRequest(VANPI_WEB_SERVER, test, VANPI_WEB_PORT, 0);
        
      } //if(millis() - nowAhMillis > nowAhInterval) // 
      if(totalAhDatetime.epoch==0) {
        totalAhDatetime = now;
        totalAhDatetime.epoch -= millis()/1000; //hopefully it doesn't take 50 days after startup to connect to NTP
      }
    } else {
      unsigned long now = millis();
      unsigned long fraction = (now-samplesetMillis)%60000;
      samplesetMillis = now - fraction;
      if(DEBUG_LOOP) Serial.println("...new L2 sample interval start/id by millis() =" + String(samplesetMillis));
      if(millis() - nowAhMillis > nowAhInterval) {
        nowAhMillis = samplesetMillis;
        long netah = (nowAh/nowAhCount)*(nowAhInterval/1000)/3600; // the weird division is to avoid overflow, so we multiply by a smaller interval
        totalAh+=netah;
        nowAh=0;
        nowAhCount=0;
        nowAhInterval = 3600000;
        
        struct aggregate *bin = &hourlyAmpSum[now/36000000];
        autoadd2(bin, netah);
        if(DEBUG_LOOP) {
          Serial.println("hourly sum updated");
          for(int i=0; i>24; i++){
            Serial.print(i);
            Serial.print("H f(");
            Serial.print(hourlyAmpSum[i].sum);
            Serial.print("/ ");
            Serial.print(hourlyAmpSum[i].count);
            Serial.print(") =");
            Serial.println(hourlyAmpSum[i].converted);
          }
        }
      }
    } //if(millis() - samplesetMillis > 60000 * MIN_PER_GRAPH_PX) // if graph pixel has elapsed


    // keep track of this hour's amperage samples to produce an average at end of hour.
    // no need to do integration.  The sample interval is an hour, so the average value is same as area under sample curve. 
    nowAh += amperesMinuteTot.converted; //L2 average totalled and incremented.
    nowAhCount++;

    // update battery charge/discharge
    // you need to make this an average
    if(sampleStartMillis!=0) {
      unsigned long sampleInterval = millis() - sampleStartMillis;
      long sampleAh = (long)amperesMinuteTot.converted * (long)sampleInterval/3600000L;
      // long netah = (nowAh/nowAhCount)*(nowAhInterval/1000)/3600; // the weird division is to avoid overflow, so we multiply by a smaller interval
      if(sampleInterval%3600000 >= 1800000)
          sampleAh++; //round up
      if(DEBUG_LOOP) {
        Serial.print("before discharged mAh=");
        Serial.println(dischargedAh);
      }
      //updatestateofcharge(sampleAh, amperes.converted, voltage.converted);
      if(DEBUG_LOOP) {
        Serial.print("Sample interval=");
        Serial.println(sampleInterval);
        Serial.print("recorded mA=");
        Serial.println(amperesMinuteTot.converted);
        Serial.print("calculated mAh=");
        Serial.println(sampleAh);
      }
    }
    sampleStartMillis = millis();

    
    //addToGraph(amperes, voltage);
    addToGraph(amperesMinuteTot, voltageMinuteTot, solarMinuteTotA, solarMinuteTotV);
    
    zeroOut(&amperesMinuteTot);
    zeroOut(&voltageMinuteTot);
    zeroOut(&solarMinuteTotA);
    zeroOut(&solarMinuteTotV);
  } //if(millis() - samplesetMillis > 60000 * MIN_PER_GRAPH_PX)

  if(DEBUG_BATT_AMP_SAMPL) {
    Serial.println(F("Ah calculation parts below:"));
    Serial.println("1.existing aggregate, sum=" + String(amperesMinuteTot.sum) + " count=" + String(amperesMinuteTot.count) + " avg=" + String(amperesMinuteTot.avg) + " volts=" + String(amperesMinuteTot.tovolts) + " measured=" + String(amperesMinuteTot.converted));
    Serial.println("2.new sample, sum=" + String(amperes.sum) + " count=" + String(amperes.count) + " avg=" + String(amperes.avg) + " volts=" + String(amperes.tovolts) + " measured=" + String(amperes.converted));
  }
  if(DEBUG_LOOP) Serial.println(F("Aggregating L1 Samples into L2"));

  //Serial.println("count before=" + String(solarMinuteTotV.count));
  //Serial.println("sum before=" + String(solarMinuteTotV.sum));
  //Serial.println("addend before=" + String(solarV.avg));

  if(MIN_PER_GRAPH_PX*DATA_COUNT>sample_count) { 
    //this will have effect of lowering the weight of the earlier readings, and twice more weight on the later half of readings.
    if(amperesMinuteTot.count>=sample_count) reduceToAvg(&amperesMinuteTot);
    if(voltageMinuteTot.count>=sample_count) reduceToAvg(&voltageMinuteTot);
    if(solarMinuteTotA.count>=sample_count) reduceToAvg(&solarMinuteTotA);
    if(solarMinuteTotV.count>=sample_count) reduceToAvg(&solarMinuteTotV);
    
    //(a+b+c+d+...+z)/n    sum/count
    //sum+sum+sum/count+count+count = and sum already is going to overflow
  }
  inclInAvg(&amperesMinuteTot, amperes);
  inclInAvg(&voltageMinuteTot, voltage);
  inclInAvg(&solarMinuteTotA, solarA);
  inclInAvg(&solarMinuteTotV, solarV);

  

  //Serial.println("count after=" + String(solarMinuteTotV.count));
  //Serial.println("sum after=" + String(solarMinuteTotV.sum));
  if(DEBUG_SAMPLING) {
    Serial.println("new aggregate, sum=" + String(amperesMinuteTot.sum) + " count=" + String(amperesMinuteTot.count) + " avg=" + String(amperesMinuteTot.avg) + " volts=" + String(amperesMinuteTot.tovolts) + " measured=" + String(amperesMinuteTot.converted));
    Serial.println();
  }
  if(DEBUG_BATT_AMP_SAMPL) Serial.println("amp for this min/L2=" + String(amperesMinuteTot.converted));
  if(DEBUG_BATT_VOLT_SAMPL) Serial.println("volt for this min/L2=" + String(voltageMinuteTot.converted));
  if(DEBUG_SOL_AMP_SAMPL) Serial.println("solar amp for this min/L2=" + String(solarMinuteTotA.converted));
  if(DEBUG_SOL_VOLT_SAMPL) Serial.println("solar volt for this min/L2=" + String(solarMinuteTotV.converted));
  if(DEBUG_BATT_AMP_SAMPL) {
    Serial.print("amp-hour drift/reading, if on zero amps the entire uptime, is:");
    Serial.println(debugginglongtermA/debugginglongtermCount);
  }
  if(DEBUG_SOL_AMP_SAMPL) {
    Serial.print("sol amp-hour drift/reading, if on zero amps the entire uptime, is:");
    Serial.println(debugginglongSolarA/debugginglongtermCount);
  }
  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("End of loop() Free memory="));
      Serial.println(freeMemory());
      unsigned long ms = millis();
      String newobjref = String(ms);
      Serial.print(F("address of last newly allocated string="));
      Serial.println((unsigned int)&newobjref);
      const char* newobjref2 = newobjref.c_str();      
      Serial.print(F("address of last newly allocated c_string="));
      Serial.println((unsigned int)newobjref2);
      // https://stackoverflow.com/questions/8843604/string-c-str-deallocation-necessary
      //meaning newobjref2 just points to memory alloc'ed by String().  So when it goes out of scope, the pointer might point to invalid data used by some other object bc it was marked as released, is what I think they mean.
      Serial.print(F("address of long variable that should be allocated on stack="));
      Serial.println((unsigned int)&ms);
      // https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram
      // see memory management tips and why I output the address of variable allocated on stack, and one I think should be on heap
      // I'm of course assuming no virtual memory translation shananigans, that an modern multitasking operating system implements
  }
  if(DEBUG_LOOP)
    Serial.println(F("loop() complete"));  
}

void absAmpReading(struct analogsamplesum *ampreading) {
  long zeroampreading = (1676-ampreading->calibrate)*4095/3300+1;
  long zeroampreadingmult = ampreading->count*zeroampreading;
  long diff = zeroampreadingmult - ampreading->sum;
  long delta = abs(diff);
  long reext = zeroampreadingmult+delta;

  ampreading->sum = reext;

  computesampleAvg(ampreading);
  computesampleVolts(ampreading);
  computesampleConversion(ampreading);
}

unsigned long rtcMinuteMillis() {
  unsigned long ms = millis();
  struct utcdatetime now = rtcclock();
  ms -= now.second*1000;
  return ms;
}
unsigned long uptimeMinuteMillis() {
  //unsigned long ms = millis();
  //unsigned long fraction = (ms-samplesetMillis)%60000;
  //ms = ms - fraction;
  return millis()%60000;
}
unsigned int rtcPrevHour(){
  return (rtcclock().hour+23)%24;
}
unsigned int rtcNowHour(){
  return rtcclock().hour;
}
unsigned int uptimePrevHour(){
  return ((millis()%86400000)+23)%24;
}
unsigned int uptimeNowHour(){
  return (millis()%86400000)/3600000;
}

void startupdraw(){

  epd.clearBuffer();
  //epd.fillScreen(EPD_BLACK);
  //250x122
  unsigned int w = epd.width()-1;
  unsigned int h = epd.height()-1;
  unsigned int x = millis() % 40;
  unsigned int y = millis() % 20;

  //fills
  epd.fillRect(x, y, w-42, h-22, EPD_BLACK);
  epd.fillRect(x, y, w-42, h-22, EPD_INVERSE);
  epd.drawRect(x+2, y+2, w-46, h-26, EPD_BLACK);

  //shapes
  epd.drawLine(50+w/3 + millis() % w/2, h/3 + millis() % h/2, millis() % w/2, millis() % h/2, EPD_BLACK);

  epd.drawFastHLine(50+millis() % h, millis() % w, millis() % w, EPD_BLACK);
  epd.drawFastVLine(50+millis() % w, millis() % h, millis() % h, EPD_BLACK);

// useless, EPD_WHITE has no routine handling it, in drawPixel()
//  epd.drawFastHLine(millis() % h, millis() % w, millis() % w, EPD_WHITE);
//  epd.drawFastVLine(millis() % w, millis() % h, millis() % h, EPD_WHITE);

//  epd.fillCircle(2*millis() % h/3, 2*millis() % w/3, millis() % w/4, EPD_WHITE);
//  epd.drawCircle(2*millis() % h/3, 2*millis() % w/3, millis() % w/4, EPD_WHITE);
  //epd.fillCircle(2*millis() % h/3, 2*millis() % w/3, millis() % w/4, EPD_BLACK);
  epd.fillCircle(50, 20, 75, EPD_BLACK);
  epd.drawCircle(50+2*millis() % h/3, 2*millis() % w/3, millis() % w/4, EPD_BLACK);

  //outline text
  epd.setCursor(19,4);
  epd.setTextColor(EPD_BLACK);
  epd.print(F("Lorem ipsum dolor sit amet"));
  epd.setCursor(19,4);
  epd.setTextColor(EPD_INVERSE);
  epd.print(F("Lorem ipsum dolor sit amet"));
  
  epd.setCursor(21,4);
  epd.setTextColor(EPD_BLACK);
  epd.print(F("Lorem ipsum dolor sit amet"));
  epd.setCursor(21,4);
  epd.setTextColor(EPD_INVERSE);
  epd.print(F("Lorem ipsum dolor sit amet"));
  
  epd.setCursor(21,6);
  epd.setTextColor(EPD_BLACK); 
  epd.print(F("Lorem ipsum dolor sit amet"));
  epd.setCursor(21,6);
  epd.setTextColor(EPD_INVERSE);
  epd.print(F("Lorem ipsum dolor sit amet"));
  
  epd.setCursor(19,6);
  epd.setTextColor(EPD_BLACK);
  epd.print(F("Lorem ipsum dolor sit amet"));
  epd.setCursor(19,6);
  epd.setTextColor(EPD_INVERSE);
  epd.print(F("Lorem ipsum dolor sit amet"));

  epd.setCursor(20,5);
  epd.setTextColor(EPD_BLACK);
  epd.print(F("Lorem ipsum dolor sit amet"));

  //title
  //epd.drawRoundRect(w/2 + millis() % w/2, h/2 + millis() % h/2, millis() % w/2, millis() % h/2, EPD_BLACK);
  epd.fillRect(50, 100, 150, 20, EPD_BLACK);
  epd.setCursor(60,106);
  epd.setTextColor(EPD_INVERSE);
  char v[] = "Version:                ";
  updateCStr(v,9,14,VERSION);
  epd.print(v);

  epd.setCursor(20,24);
  epd.setTextColor(EPD_INVERSE);
  epd.print(F("Arduino Nano 33 IoT Auto Power Meter\nFeatures:\n-Wifi Access\n-Voltage Graphing\n-Amp Graphing\n-NTP Time Server\n-Web Server 24hr Data"));

  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("End of Splash startupdraw() Free memory="));
      Serial.println(freeMemory());    
  }

  
  epd.display();
  delay(15000);
}


// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

// There are 2 BMP draw functions... this one is provided by ADAfruit and can read BMP from SD Card
// It will read a 24bit RGB type BMP, drawwing the image straight to Image buffer, just using a few bytes in array to speed reads thru block reads.
// The other is at bottom of this file, and primarily is focused on decoding 1bpp monochrome BMP's, 
//   which are implicitly compressed (though marked as uncompressed RBG) as 8 pixels per byte.
// The bottom method takes an array of char to transfer the entire copy to image buffer (via EPD.draw???? methods).

#define BUFFPIXEL 20

bool bmpDraw(char *filename, int16_t x, int16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col, x2, y2, bx1, by1;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= epd.width()) || (y >= epd.height())) return false;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return false;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        x2 = x + bmpWidth  - 1; // Lower-right corner
        y2 = y + bmpHeight - 1;
        if((x2 >= 0) && (y2 >= 0)) { // On screen?
          w = bmpWidth; // Width/height of section to load/display
          h = bmpHeight;
          bx1 = by1 = 0; // UL coordinate in BMP file
  
          for (row=0; row<h; row++) { // For each scanline...
  
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
            else     // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + (row + by1) * rowSize;
            pos += bx1 * 3; // Factor in starting column (bx1)
            if(bmpFile.position() != pos) { // Need seek?
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
            }
            for (col=0; col<w; col++) { // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) { // Indeed
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
              }
              // Convert pixel from BMP to EPD format, push to display
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];

              uint8_t c = 0;
              if ((r < 0x80) && (g < 0x80) && (b < 0x80)) {
                 c = EPD_BLACK; // try to infer black
              } else if ((r >= 0x80) && (g >= 0x80) && (b >= 0x80)) {
                 c = EPD_WHITE;
              } else if (r >= 0x80) {
                c = EPD_RED; //try to infer red color
              }
              
              epd.writePixel(col, row, c);
            } // end pixel
          } // end scanline
        } // end onscreen
        epd.display();
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) {
    Serial.println(F("BMP format not recognized."));
    return false;
  }
  return true;
}

void drawTextAsBlips(int left, int top, char* text, int startindex, int endindex) {
  unsigned int x=0;
  unsigned int y=0;
//  unsigned int len = text.length();
  for(unsigned int i=startindex; i<endindex; i++)
  {
    char ch = text[i];
    if(ch>=33 && ch<127) { //visible letters 
      epd.drawPixel(x + left,y+top,EPD_BLACK);
      x++;
    } else if(ch=='\n') {
      x=0;
      y++;
    } else if(ch==' ')
      x++;
  }
  /*
  epd.setCursor(50, 70);
  epd.setTextColor(EPD_INVERSE);
  epd.print("--Web");
  epd.display();
  
  delay(15000);*/
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}


void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
void epdDisplayDeferredDelay() {
  if(SAFE_EPD) {
    epd.display();
    delay(15000); //always delay 15seconds after display.  It's not worth the risk to the epaper.
  } else {
    unsigned long span = millis()-lastEPDupdatemillis;
    if(span < 15000) {
      Serial.println("Warning: Epaper delay:" + String(span));
      delay(15000);
    }
    lastEPDupdatemillis = millis();
  }
  epd.display();
}

long lowmem;

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
 
int freeMemory() {
  char top;
#ifdef __arm__
  int freem = &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  int freem = &top - __brkval;
#else  // __arm__
  int freem = __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
  if(lowmem==0 || (freem < lowmem))
    lowmem = freem;
  if(DEBUG_FREE_MEMORY){
      Serial.print(F("(Lowest memory detected="));
      Serial.print(lowmem);    
      Serial.print(")");
  }
  if(freem < 2000) {
    if(DEBUG_FREE_MEMORY) {
      Serial.println(F("Less than 2k detected.  Please examine"));
      Serial.println(freem);
      halt();
    }
  }
  return freem;
}
void halt() {
  pinMode(LED_BUILTIN, OUTPUT);
  while(true) { //halt
    // initialize digital pin LED_BUILTIN as an output.
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);                       // wait for a second
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  if(DEBUG_WIFI) {
    Serial.print(F("SSID: "));
    Serial.println(WiFi.SSID());
  }
  
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  if(DEBUG_WIFI) {
    Serial.print(F("IP Address: "));
    Serial.println(ip);
  }

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  if(DEBUG_WIFI) {
    Serial.print(F("signal strength (RSSI):"));
    Serial.print(rssi);
    Serial.println(F(" dBm"));
  }

  byte mac[6];                     // the MAC address of your Wifi shield
  WiFi.macAddress(mac);

  // Draw some text
  epd.clearBuffer();
  epd.setCursor(0, 10);
  epd.setTextColor(EPD_BLACK);
  epd.setTextSize(2);
  char mesg[] = "SSID:_             \nIP Addr:_              \nSignal(RSSI):    dBm\nMAC:__.__.__.__.__.__\nWeb Server:_        ";
  char *ptr = &mesg[0];
  char old = ' ';
  unsigned int spot = 0;
  String ssid = String(WiFi.SSID());
  String ipaddr = formatIP(ip);
  String sign = String(rssi);
  String websvr = webserver.status()==1 ? trueToStr : falseToStr;
  updateCStr(ptr, 5, 14, ssid);
  updateCStr(ptr, 28, 15, ipaddr);
  updateCStr(ptr, 57, 6, sign);
  //updateCStr(ptr, 70, 13, websvr);
  puthexInCStr(ptr,mac[5],69);
  puthexInCStr(ptr,mac[4],72);
  puthexInCStr(ptr,mac[3],75);
  puthexInCStr(ptr,mac[2],78);
  puthexInCStr(ptr,mac[1],81);
  puthexInCStr(ptr,mac[0],84);
  updateCStr(ptr, 98, 7, websvr);
  mesg[sizeof(mesg)-1] = 0;
  //String mesg = "SSID:" + String(WiFi.SSID()) + "\nIP Addr:" + formatIP(ip)+ "\nSignal(RSSI):" + String(rssi) + "dBm\n\nWeb Server:" + String(webserver.status());
  epd.print(mesg);
  epd.display();
  ipaddr.~String();

  epd.setTextSize(1);
  delay(15000);
}
String formatIP(IPAddress ip) {
  String byt1 = String(ip[0]);
  String byt2 = String(ip[1]);
  String byt3 = String(ip[2]);
  String byt4 = String(ip[3]);
  
  String formatted = byt1 + dotToStr + byt2 + dotToStr + byt3 + dotToStr + byt4;

  //for the day, when memory leaks are so bad, I need to remove all the references to string, just as a rule out, that it is cause of the fragmentation - which I don't under why initializing all the functions at start, doesn't solve.  I don't store strings anywhere in global variables.  It should all be collected moving up and down the stack.
  //char dot[] = ".";
  //char format[] = "0.0.0.0        "; //actually a buffer, but having the format there, is self documenting
  //char* ptr = &format[0];
  //ptr+=putnumInCStr(format, ip[0], 0);
  //ptr++[0] = '.';
  //ptr+=putnumInCStr(format, ip[1], 0);
  //ptr++[0] = '.';
  //ptr+=putnumInCStr(format, ip[2], 0);
  //ptr++[0] = '.';
  //ptr+=putnumInCStr(format, ip[3], 0);
  //return String(format);
  
  return formatted;
}
unsigned int updateCStr(char* toupdate, unsigned int at, unsigned int mx, String with) {
  toupdate+=at;
  unsigned int spot = with.length();
  char old = toupdate[spot];
  with.toCharArray(toupdate, mx); //5
  toupdate[spot] = old; //assuming this is where it places a null character

  return spot;
}
// https://arduino.stackexchange.com/questions/42986/convert-int-to-char
unsigned int putnumInCStr(char* toupdate, int num, unsigned int at) {
  toupdate += at;

  int copy;
  unsigned int spot=0;
  if(copy!=0)
    while(copy!=0) {
      spot++;
      copy/=10;
    }
  else
    spot=1;
  spot++;
  char old = toupdate[spot];
  
  char* val = itoa(num,toupdate,10);
  if(val==0)
    return 0;
  if(toupdate[spot]==0)
    toupdate[spot] = old;

  return spot;
}
unsigned int puthexInCStr(char* toupdate, int num, unsigned int at) {
  toupdate+=at;

  int copy;
  unsigned int spot=0;
  if(copy!=0)
    while(copy!=0) {
      spot++;
      copy/=16;
    }
  else
    spot=1;
  spot++;
  char old = toupdate[spot];
  
  char* val = itoa(num,toupdate,16);
  if(val==0)
    return 0;
  if(toupdate[spot]==0)
    toupdate[spot] = old;

  return spot;
}

void connectToAP() {
  status = WiFi.status();
  if(DEBUG_WIFI) {
    Serial.print(F("Current Wifi status code="));
    Serial.print(status);
    Serial.println();
  }
  
  // check for the presence of the shield:
  if (status == WL_NO_SHIELD) {
    if(DEBUG_WIFI) Serial.println(F("WiFi shield not present"));
    // don't continue:
    //while (true); //Bob - not bothering to abort.  Anything that uses network, needs to checl
  }

  // attempt to connect to Wifi network:
  int retries = 5;
  while ( status != WL_CONNECTED && (retries--)>0) {  //Bob - added a limit of 5 retries, then move on.  No wifi, no big deal.
    if(DEBUG_WIFI) {
      Serial.print(F("Attempting to connect to SSID: "));
      Serial.println(SECRET_SSID);
    }
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(SECRET_SSID, SECRET_PASS);

    // wait 1 second for connection:
    delay(1000);
  }
  //if(isWifiConnected())
  //  webserver.begin();
  if(isWifiConnected())
    uptimeAtAP = millis();

  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("End of connectToAP() Free memory="));
      Serial.println(freeMemory());    
  }
}
bool isWifiConnected() {
  int last = WiFi.status();
  return (last == WL_CONNECTED); // || (last == WL_IDLE_STATUS);
}



void httprequesthandle() {
  // listen for incoming clients
  WiFiClient tcpconnect = webserver.available();
  if (tcpconnect) {
    if(DEBUG_HTTP_INCOMING) Serial.println(F("new client"));
    
    /*
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (tcpconnect.connected()) {
      if (tcpconnect.available()) {
        char c = tcpconnect.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          tcpconnect.println("HTTP/1.1 200 OK");
          tcpconnect.println("Content-Type: text/html");
          tcpconnect.println("Connection: close");  // the connection will be closed after completion of the response
          //tcpconnect.println("Refresh: 5");  // refresh the page automatically every 5 sec
          tcpconnect.println();
          tcpconnect.println("<!DOCTYPE HTML>");
          tcpconnect.println("<html>");
          tcpconnect.println(formattedlocal(rtcclock()));
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            tcpconnect.print("analog input ");
            tcpconnect.print(analogChannel);
            tcpconnect.print(" is ");
            tcpconnect.print(sensorReading);
            tcpconnect.println("<br />");
          }
          tcpconnect.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    */
    if(tcpconnect.connected()) {
      String raw = serverrequest(tcpconnect);
      String line1 = httprequestline(raw);
      String methodcode = String(httpmethod(line1));
      String uri = httprequesturi(line1);
      String url = requesturl(uri);
      String query = requestquerystring(uri);

      tcpconnect.println(HTTP1_1_200_OK);
      tcpconnect.println(Content_Type_text_html);
      tcpconnect.println(Connection_close);  // the connection will be closed after completion of the response
      //tcpconnect.println("Refresh: 5");  // refresh the page automatically every 5 sec
      tcpconnect.println();
      tcpconnect.print(HTML_DOCTYPE_HTML);
      tcpconnect.print(HTML_html);
      String timstr = formattedlocal(rtcclock());
      tcpconnect.print(timstr);
      timstr.~String();
      tcpconnect.println(EOL);

      if(url==URL_adc) {
        // output the value of each analog input pin
        for (int analogChannel = 0; analogChannel < 8; analogChannel++) {
          int sensorReading = analogRead(analogChannel);
          tcpconnect.print(analog_input);
          tcpconnect.print(analogChannel);
          tcpconnect.print(_is_);
          tcpconnect.print(sensorReading);
          tcpconnect.println(EOL);
        }
      } else if(url==URL_live) {
          tcpconnect.println(Timestamp_Category_Value_Units_tag_eor);
          String now = formattedlocal(rtcclock());          
          struct analogsamplesum voltage = getsample(A6, fudge_for_voltameter, &ANALOG_VOLT_TO_12V);
          tcpconnect.print(now);
          tcpconnect.print(_SystemVoltage_);
          tcpconnect.print(voltage.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(voltage.converter->units);
          tcpconnect.println(EOR);
          
          struct analogsamplesum amperes = getsample(A1, fudge_for_ammeter2, &ANALOG_VOLT_TO_AMP);          
          tcpconnect.print(now);
          tcpconnect.print(_BatteryRate_);
          tcpconnect.print(amperes.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(amperes.converter->units);
          tcpconnect.println(EOR);

          long ah = getUptimeNetBatteryAh();
          tcpconnect.print(now);
          tcpconnect.print(_BatteryNetCharge_);
          tcpconnect.print(ah);
          tcpconnect.print(milli_);
          tcpconnect.print(amperes.converter->units);
          tcpconnect.println(EOR);

          struct analogsamplesum solarstatus = getsample(A7, fudge_for_voltameter, &ANALOG_VOLT_TO_18V);
          tcpconnect.print(now);
          tcpconnect.print(F(",SolarStatus,"));
          tcpconnect.print(solarstatus.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(solarstatus.converter->units);
          tcpconnect.println(EOR);
          
          struct analogsamplesum solarsupply = getsample(A2, fudge_for_ammeter, &ANALOG_VOLT_TO_AMP);
          tcpconnect.print(now);
          tcpconnect.print(_SolarSupply_);
          tcpconnect.print(solarsupply.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(solarsupply.converter->units);
          tcpconnect.println(EOR);

          now.~String();

      } else if(url==URL_powermeter) {
          tcpconnect.println(Timestamp_Category_Value_Units_tag_eor);
          String now = formattedlocal(rtcclock());          
          tcpconnect.print(now);
          tcpconnect.print(_SystemVoltage_);
          tcpconnect.print(voltageForWeb.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(voltageForWeb.converter->units);
          tcpconnect.println(EOR);
          
          tcpconnect.print(now);
          tcpconnect.print(_BatteryRate_);
          tcpconnect.print(amperesForWeb.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(amperesForWeb.converter->units);
          tcpconnect.println(EOR);

          long ah = getUptimeNetBatteryAh();
          tcpconnect.print(now);
          tcpconnect.print(_BatteryNetCharge_);
          tcpconnect.print(ah);
          tcpconnect.print(_mAh);
          tcpconnect.println(EOR);

          tcpconnect.print(now);
          tcpconnect.print(_SolarStatus_);
          tcpconnect.print(solarVoltForWeb.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(solarVoltForWeb.converter->units);
          tcpconnect.println(EOR);
          
          tcpconnect.print(now);
          tcpconnect.print(_SolarSupply_);
          tcpconnect.print(solarAmpForWeb.converted);
          tcpconnect.print(milli_);
          tcpconnect.print(solarAmpForWeb.converter->units);
          tcpconnect.println(EOR);

          long trueCapacityAh = batteryCapacityAh - lostCapacityAh;
          long availCapacityAh = trueCapacityAh+dischargedAh;
          tcpconnect.print(now);
          tcpconnect.print(_EstFullCapacity_);
          tcpconnect.print(trueCapacityAh);
          tcpconnect.print(_mAh);
          tcpconnect.println(EOR);

          tcpconnect.print(now);
          tcpconnect.print(_EstAvailCapacity_);
          tcpconnect.print(availCapacityAh);
          tcpconnect.print(_mAh);
          tcpconnect.println(EOR);

          now.~String();
      } else if(url==URL_powerhistory) {
        tcpconnect.println(Timestamp_BatteryAmp_SystemVoltage_SolarStatus_SolarSupply_eor);
        for (int i = 0; i< DATA_COUNT; i++) {
          int index = (graphdatahead+i) % DATA_COUNT;
          tcpconnect.print(readings[index].utc.epoch);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(readings[index].batterymA);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(readings[index].batterymV);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(readings[index].solarmA);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(readings[index].solarmV);
          tcpconnect.println(EOR);
        }
      } else if(url==URL_hourly) {
        tcpconnect.println(hourUTC_hourlocal_batterynet_eor);
        for (int i = 0; i < 24; i++) {
          tcpconnect.print(i);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print((i+tz+24) % 24); // we are not reflecting partial hours b/c we can't divide the array into partials
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(hourlyAmpSum[i].avg);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(hourlyAmpSum[i].sum);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(hourlyAmpSum[i].count);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(hourlyAmpSum[i].mx);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(hourlyAmpSum[i].mn);

          tcpconnect.println(EOR);
        }
      } else if(url==URL_ntp) {
        // show time of boot
        tcpconnect.print(First_NTP_sync_epoch_);
        tcpconnect.print(datetimeAtConnect.epoch);
        tcpconnect.println(EOR);
        
        // last NTP sync
        tcpconnect.print(Local_time_of_last_outgoing_NTP_sync_request_);
        String timestr = formattedlocal(datetimeAtNTP);
        tcpconnect.print(timestr);
        timestr.~String();
        tcpconnect.println(EOL);
        tcpconnect.print(Unix_Epoch_calculated_from_incoming_response_from_NTP_);
        tcpconnect.print(datetimeAtNTP.epoch);
        tcpconnect.println(EOR);

        // last NTP request
        tcpconnect.print(Last_outgoing_NTP_sync_response_sent_to_IP_);
        tcpconnect.print(lastremoteip[0]);
        tcpconnect.print(TEXT_PERIOD);
        tcpconnect.print(lastremoteip[1]);
        tcpconnect.print(TEXT_PERIOD);
        tcpconnect.print(lastremoteip[2]);
        tcpconnect.print(TEXT_PERIOD);
        tcpconnect.print(lastremoteip[3]);
        tcpconnect.println(EOL);
        tcpconnect.print(Last_outgoing_NTP_sync_response_sent_epoch_value_);
        tcpconnect.print(lastremoteepoch);
        tcpconnect.println(EOL);
        tcpconnect.print(Number_of_NTP_requests_served_);
        tcpconnect.print(ntpscount);
        tcpconnect.println(EOR);
        
        // drift adjustments
        tcpconnect.print(Adjustments_to_RTC_actual_elapsed_);
        tcpconnect.print(rtcdrift.A);
        tcpconnect.print(_rtc_elapsed___);
        tcpconnect.print(rtcdrift.Bn);
        tcpconnect.print(F("/"));
        tcpconnect.print(rtcdrift.Bd);
        tcpconnect.println(__eor);

        // current drift adjustment
        tcpconnect.print(Current_drift_adjustment_to_outgoing_NTP_response);
        tcpconnect.print(getdrift());
        tcpconnect.println(EOR);
        
        // data used for drift calculations
        tcpconnect.println(rtc_returned_seconds_elapsed_actual_seconds_elapsed_from_NTP_br);
        for (int i = 0; i < 48; i++) {
          tcpconnect.print(i);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(rtcdriftexpected[i]);
          tcpconnect.print(TEXT_COMMA);
          tcpconnect.print(rtcdriftactual[i]);
          tcpconnect.println(EOR);
        }
      } else if(url==URL_timesync) {
        tcpconnect.println(F("NTP Sync Request received<br>"));
        if(rtcclock().epoch-datetimeAtNTP.epoch>10*60) {
          ntprequest();
          tcpconnect.println(Current_time_is_);
          String retimstr = formattedlocal(rtcclock());
          tcpconnect.print(retimstr);
          retimstr.~String();
          tcpconnect.println(EOL);
        } else {
          tcpconnect.println(NTP_Sync_Request_DENIED_bc_sync_was_just_run_);
          tcpconnect.println(rtcclock().epoch-datetimeAtNTP.epoch);
          tcpconnect.println(seconds_ago_br);
        }
      } else {
        int free = freeMemory();
        tcpconnect.println(br_free_memory_);
        tcpconnect.println(free);
        tcpconnect.println(br_raw_);
        tcpconnect.println(raw);
        tcpconnect.println(br_methodcode_);
        tcpconnect.println(methodcode);
        tcpconnect.println(br_line1_);
        tcpconnect.println(line1);
        tcpconnect.println(br_uri_);
        tcpconnect.println(uri);
        tcpconnect.println(br_url_);
        tcpconnect.println(url);
        tcpconnect.println(br_query_);
        tcpconnect.println(query);
        tcpconnect.println(EOL);
        //char millisinceAP[] = "Millisec since wifi connect=";
        //tcpconnect.println(millisinceAP-millis());
        //tcpconnect.println(uptimeAtAP);
        //tcpconnect.println(EOL);
        tcpconnect.println(This_url_does_not_exist__No_404_error__But_you_need_to_select_one_of_these_adc_live_powermeter_powerhistory_hourly_ntp);
      }
      tcpconnect.println(HTML_CHTML);

      raw.~String();
      line1.~String();
      methodcode.~String();
      uri.~String();
      url.~String();
      query.~String();
    }

    // give the web browser time to receive the data
    //delay(1);

    // close the connection:
    tcpconnect.stop();
    tcpconnect.~WiFiClient();

    if(DEBUG_HTTP_INCOMING) Serial.println(F("client disonnected"));
  }
}

#define REQUESTIN_BUFFER_SIZE 1024 //8192
String serverrequest(WiFiClient tcpconnect) {
  char httprequestin[REQUESTIN_BUFFER_SIZE]; 

  unsigned int index=0;
  boolean currentLineIsBlank = true;
  while (tcpconnect.connected()) {
      if (tcpconnect.available()) {
        char c = tcpconnect.read();
        if(c!=-1) {
          httprequestin[index++] = c;
          if(DEBUG_HTTP_INCOMING) Serial.write(c);
        }
        if (c == '\n' && currentLineIsBlank) {
          // break out of loop and return the request
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
  }
  httprequestin[index] = 0;
  if(DEBUG_HTTP_INCOMING) Serial.println(F("request returned"));

  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("End of HTTP request handler Free memory="));
      Serial.println(freeMemory());    
  }

  return String(httprequestin);
}
String httprequestline(String raw) {
  unsigned int len = raw.length();
  for(int i=0; i<len; i++)
  if(raw[i]=='\n')
    return String(raw).substring(0,i);
  return raw;
}
String httpversion(String requestline) {
  return "Not implemented";
}

#define HTTP_METHOD_UNKNOWN 0
#define HTTP_METHOD_GET     1
#define HTTP_METHOD_HEAD    2
#define HTTP_METHOD_POST    3
#define HTTP_METHOD_PUT     4
#define HTTP_METHOD_DELETE  5
#define HTTP_METHOD_CONNECT 6
#define HTTP_METHOD_OPTIONS 7
#define HTTP_METHOD_TRACE   8
int httpmethod(String requestline) {
    // GET Method. A GET request retrieves data from a web server by specifying parameters in the URL portion of the request. ...
    // HEAD Method. ...
    // POST Method. ...
    // PUT Method. ...
    // DELETE Method. ...
    // CONNECT Method. ...
    // OPTIONS Method. ...
    // TRACE Method.
  if(requestline[0]=='G' && requestline.startsWith("GET "))
    return HTTP_METHOD_GET;
  else if(requestline[0]=='H' && requestline.startsWith("HEAD "))
    return HTTP_METHOD_HEAD;
  else if(requestline[0]=='P') {
    if(requestline[1]=='O' && requestline.startsWith("POST "))
      return HTTP_METHOD_POST;
    if(requestline[1]=='U' && requestline.startsWith("PUT "))
      return HTTP_METHOD_PUT;
  }
  else if(requestline[0]=='D' && requestline.startsWith("DELETE "))
    return HTTP_METHOD_DELETE;
  else if(requestline[0]=='C' && requestline.startsWith("CONNECT "))
    return HTTP_METHOD_CONNECT;
  else if(requestline[0]=='O' && requestline.startsWith("OPTIONS "))
    return HTTP_METHOD_OPTIONS;
  else if(requestline[0]=='T' && requestline.startsWith("TRACE "))
    return HTTP_METHOD_TRACE;
  return HTTP_METHOD_UNKNOWN;
}
String httprequesturi(String requestline) {
  unsigned int len = requestline.length();
  unsigned int start=4;
  unsigned int method=httpmethod(requestline);
  if(method==HTTP_METHOD_GET)
    start=4;
  else if(method==HTTP_METHOD_HEAD)
    start=5;
  else if(method==HTTP_METHOD_POST)
    start=5;
  else if(method==HTTP_METHOD_PUT)
    start=4;
  else if(method==HTTP_METHOD_DELETE)
    start=7;
  else if(method==HTTP_METHOD_CONNECT)
    start=8;
  else if(method==HTTP_METHOD_OPTIONS)
    start=8;
  else if(method==HTTP_METHOD_TRACE)
    start=6;
  else
    for(unsigned int i=0; i<len-1; i++)
      if(requestline[i]==' ') {
        start = i+1;
        break;
      }
      else if(requestline[i]=='\n')
        break;
  //Serial.println(start);
  //Serial.println(len);
  //Serial.println(requestline[start]);
  //Serial.println(requestline.substring(start,len-start));
  
  for(unsigned int i=start; i<len-1; i++)
    if(requestline[i]==' ') {
      //Serial.println(i);
      //Serial.println(requestline.substring(start,i-1));
      //delay(15*1000);
      return requestline.substring(start, i);
    }
  //delay(15*1000);
  return requestline.substring(start, len);
}
String requesturl(String requesturi) {
  unsigned int len = requesturi.length();
  for(unsigned int i=0; i<len; i++)
    if(requesturi[i]=='?')
      return requesturi.substring(i, i);
  return requesturi;
}
String requestquerystring(String requesturi) {
  unsigned int len = requesturi.length();
  for(unsigned int i=0; i<len; i++)
    if(requesturi[i]=='?')
      return requesturi.substring(i, len);
  return "";
}



void configureNetwork() {
  isNetworkConfigured = true;
  
  // char easteregg[] ="/easteregg.bmp";
  // char eastereggsvr[] = "192.168.0.76";
  // httpRequest(eastereggsvr, easteregg, VANPI_WEB_PORT);

  // Check connectivity to Internet thru website
  bool a = httpRequest(ARDUINO_WEB_SERVER, ARDUINO_LOGO_WEBLOCATION, ARDUINO_WEB_PORT, listenToClient);
  disableClear = true;
  bool b = httpsRequest(ADAFRUIT_WEB_SERVER, ADAFRUIT_LOGO_WEBLOCATION, ADAFRUIT_WEB_PORT, listenToSslClient);
  disableClear = false;

  isInternetConnected = a || b;

  if(isInternetConnected)
    timeServer = nistServer;
  else if(httpRequest(VANPI_WEB_SERVER, VANPI_LOGO_WEBLOCATION, VANPI_WEB_PORT, listenToClient)) {
    timeServer = gpsServer;

    // https://forum.arduino.cc/index.php?topic=565994.0
    IPAddress THIS_IP(THIS_IP_1, THIS_IP_2, THIS_IP_3, THIS_IP_4);
    WiFi.config(THIS_IP); //<- static IP
    // wait 10 seconds for connection:
    delay(10000);
    printWifiStatus();
  }

    
  if(!validclock()) 
    ntprequest();  

  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("End of configureNetwork() Free memory="));
      Serial.println(freeMemory());    
  }
}

// this method makes a HTTP connection to the server:
//void httpRequest() {
//  WiFiSSLClient client;
//  unsigned int port = 443;
  // server address:
//  char server[] = "www.arduino.cc";
//  char resource[] = "/asciilogo.txt";
  //bool sendRequest = true; // used to understand if the http request must be sent
bool httpsRequest(char server[], char location[], unsigned int port, httpsResponseReceiver listenHandler) {
  WiFiSSLClient client;

  // Print request time
  if(DEBUG_HTTP_OUTGOING) {
    Serial.println(server);
    Serial.println(location);
    Serial.print(F("Secure Request sent @ "));
    Serial.println(millis());
  }


  if (client.connect(server, port)) {
    // Make a HTTP request:
    client.print("GET ");
    client.print(location);
    client.println(" HTTP/1.1");

    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();


    if(DEBUG_FREE_MEMORY) {
        Serial.print(F("HTTPS request sent, before response, Free memory="));
        Serial.println(freeMemory());    
    }
    if(listenHandler!=0)
      return listenHandler(client); //listenToSslClient(client);
    else
      return true;
  }
  else {
    if(DEBUG_HTTP_OUTGOING) Serial.println(F("connection failed"));
  }
  return false;
}
bool httpRequest(char server[], char location[], unsigned int port, httpResponseReceiver listenHandler) {
  WiFiClient client;

  // Print request time
  if(DEBUG_HTTP_OUTGOING) {
    Serial.println(server);
    Serial.println(location);
    Serial.print(F("Request sent @ "));
    Serial.print(millis());
  }

  if (client.connect(server, port)) {
    // Make a HTTP request:
    client.print("GET ");
    client.print(location);
    client.println(" HTTP/1.1");

    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();

    if(DEBUG_FREE_MEMORY) {
        Serial.print(F("HTTP request sent, before response, Free memory="));
        Serial.println(freeMemory());    
    }
    if(listenHandler!=0)
      return listenHandler(client); // listenToClient(client);
     else
      return true;
  }
  else {
    if(DEBUG_HTTP_OUTGOING) Serial.println(F("connection failed"));
  }
  return false;
}

struct responseParts {
  bool isBmp;
  bool isHtml;
  bool isText;
  int contentType;
  int body;
};
struct responseParts parseResponse(const char* responsebuffer, unsigned int len) {
  uint32_t bodyoffset = 2;
  const char* skip2 = responsebuffer+2;
  char* startat = strstr(skip2 ,"\n\n"); //responsebuffer.indexOf("\n\n",2);

  if(startat) {
    bodyoffset = ((uint32_t)startat - (uint32_t)responsebuffer)+2;
  } else {
    startat = strstr(skip2 ,"\r\n\r\n");  //responsebuffer.indexOf("\r\n\r\n",2);
    if(startat)
        bodyoffset = ((uint32_t)startat - (uint32_t)responsebuffer) +4;
    else if(DEBUG_HTTP_OUTGOING)
      Serial.println(F("Cannot find blank line separating headers from content"));
  }

  char* content = strstr(skip2,"\nContent-Type:"); //responsebuffer.indexOf("\nContent-Type:");
  char* textat = strstr(skip2,"Content-Type: text/plain"); //responsebuffer.indexOf("Content-Type: text/plain");
  char* htmlat = strstr(skip2,"Content-Type: text/html"); //responsebuffer.indexOf("Content-Type: text/html");
  char* bmpat = strstr(skip2,"Content-Type: image/bmp"); //responsebuffer.indexOf("Content-Type: image/bmp");  
  char* bmpat2 = strstr(skip2,"Content-Type: image/x-ms-bmp"); 

  struct responseParts result = {
    bmpat ? (uint32_t)bmpat<(uint32_t)startat : bmpat2 ? (uint32_t)bmpat2<(uint32_t)startat : false, 
    htmlat ? (uint32_t)htmlat<(uint32_t)startat : false, 
    textat ? (uint32_t)textat<(uint32_t)startat : false, 
    content ? (int)((uint32_t)content - (uint32_t)responsebuffer) : -1, 
    bodyoffset
  };
  if(DEBUG_HTTP_OUTGOING) {
    Serial.print("found startat=");
    if(startat)
      Serial.println(trueToStr);
    else
      Serial.println(falseToStr);
    Serial.print("startat offset=");
    Serial.print((uint32_t)startat);
    
    Serial.print(F("found Content-Type: image/bmp="));
    Serial.println(bmpat ? trueToStr : falseToStr);
    Serial.print(F("found Content-Type: x-ms-bmp="));
    Serial.println(bmpat2 ? trueToStr : falseToStr);
    Serial.print("image/bmp offset=");
    Serial.print((uint32_t)bmpat);
    Serial.print("x-ms-bmp offset=");
    Serial.print((uint32_t)bmpat2);
    
    Serial.print(F("Detected bmp="));
    Serial.println(result.isBmp);
  }
  return result;
}

bool listenToSslClient(WiFiSSLClient client)
{ 
  unsigned int index = 0;
  const unsigned int BUFFER_SIZE = 1024*7;
  char responsebuffer[BUFFER_SIZE];
  unsigned long startTime = millis();
  bool received = false;

  if(!isNetworkConfigured) {
    if(DEBUG_HTTP_OUTGOING) Serial.println(null_client_detected_skipping_checking_for_response);
    return false;
  } else if(DEBUG_HTTP_OUTGOING) 
    Serial.println(isNetworkConfigured);

  //String response = "";
  bool flagged = false;
  while ((millis() - startTime < 10000) && !received) { //try to listen for 5 seconds
    if(DEBUG_HTTP_OUTGOING && !flagged) Serial.println(checking_for_https_response);
    while (client.available() && index < BUFFER_SIZE) {
      received = true;
      char c = client.read();
      if(DEBUG_HTTP_OUTGOING) Serial.write(c);
      //response.concat(c);
      responsebuffer[index++] = c;
      if(index>=BUFFER_SIZE) break;
    }
    if(DEBUG_HTTP_OUTGOING) Serial.print(' ');
    if(!flagged) flagged=true;
    if(index>=BUFFER_SIZE) break;
  }
  if(DEBUG_HTTP_OUTGOING) Serial.println(https_response_loop_exit);
  client.stop();
  if(DEBUG_HTTP_OUTGOING) Serial.println(received_);
  if(DEBUG_HTTP_OUTGOING) Serial.println(index);
  responsebuffer[index] = 0; //null terminate


  // Draw some text
  if(received) {
    if(!disableClear) {
      epd.clearBuffer();
      epd.setCursor(10, 100);
      epd.setTextColor(EPD_BLACK);
      String timestr = formattedlocal(rtcclock());
      String space = " ";
      String uptime = String(millis()) + space + timestr + space;
      timestr.~String();
      space.~String();
      epd.print(uptime);
    }
    //String mesg = String(responsebuffer);
    struct responseParts result = parseResponse(&responsebuffer[0], index); //parseResponse(mesg);
    if(result.isHtml || result.isText)    
      drawTextAsBlips(0,0, responsebuffer, result.body, index);
      //epd.print(mesg.substring(0,0,startat, mesg.length()));
    else if(result.isBmp)
      drawBmpOnEPD(125,0, responsebuffer, result.body, index-result.body);
    epd.setCursor(50, 110);
    epd.setTextColor(EPD_INVERSE);
    epd.print(__Web);
    epd.display();

    if(DEBUG_FREE_MEMORY) {
        Serial.print(End_of_HTTPS_request_response_parsing_Free_memory_);
        Serial.println(freeMemory());    
    }
  
    delay(15000);
  }
  return received;
}
bool listenToClient(WiFiClient client)
{
  unsigned int index = 0;
  const unsigned int BUFFER_SIZE = 1024*3;
  char responsebuffer[BUFFER_SIZE];
  unsigned long startTime = millis();
  bool received = false;

  if(!isNetworkConfigured) {
    if(DEBUG_HTTP_OUTGOING) Serial.println(null_client_detected_skipping_checking_for_response);
    return false;
  } else if(DEBUG_HTTP_OUTGOING) 
    Serial.println(isNetworkConfigured);

  //String response = "";
  bool flagged = false;
  while ((millis() - startTime < 10000) && !received) { //try to listen for 5 seconds
    if(DEBUG_HTTP_OUTGOING && !flagged) Serial.println(checking_for_http_response);
    while (client.available() && index < BUFFER_SIZE) {
      received = true;
      char c = client.read();
      if(DEBUG_HTTP_OUTGOING) Serial.write(c);
      //response.concat(c);
      responsebuffer[index++] = c;
      if(index>=BUFFER_SIZE) break;
    }
    if(DEBUG_HTTP_OUTGOING) Serial.print('.');
    if(!flagged) flagged=true;
    if(index>=BUFFER_SIZE) break;
  }
  if(DEBUG_HTTP_OUTGOING) Serial.println(http_response_loop_exit);
  client.stop();
  if(DEBUG_HTTP_OUTGOING) Serial.println(received_);
  if(DEBUG_HTTP_OUTGOING) Serial.println(index);
  responsebuffer[index] = 0; //null terminate


  // Draw some text
  if(received) {
    if(!disableClear) {
      epd.clearBuffer();
      epd.setCursor(10, 100);
      epd.setTextColor(EPD_BLACK);
      String timestr = formattedlocal(rtcclock());
      String space = " ";
      String uptime = String(millis()) + space + timestr + space;
      space.~String();
      timestr.~String();
      epd.print(uptime);
    }
    
    //String mesg = String(responsebuffer);
    struct responseParts result = parseResponse(&responsebuffer[0], index); //parseResponse(mesg);
    if(result.isHtml || result.isText)    
      drawTextAsBlips(0,0, responsebuffer, result.body, index);
      //drawTextAsBlips(0,0,mesg.substring(result.body, mesg.length()));
      //epd.print(mesg.substring(startat, mesg.length()));
    else if(result.isBmp)
      drawBmpOnEPD(125,0,responsebuffer, result.body, index-result.body);
    
    epd.setCursor(50, 110);
    epd.setTextColor(EPD_INVERSE);
    epd.print(__Web);
    epd.display();

    if(DEBUG_FREE_MEMORY) {
        Serial.print(End_of_HTTP_request_response_parsing_Free_memory_);
        Serial.println(freeMemory());    
    }

    delay(15000);
  }
  return received;
}



void print2digits(int number) {
  if (number < 10) {
    Serial.print(F("0"));
  }
  Serial.print(number);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {

  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp2390.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp2390.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  if(DEBUG_NTP) Serial.println(F("NTP UDP sent"));
  Udp2390.endPacket();
  //Serial.println("6");  
}


bool validclock(){
  if(datetimeAtNTP.epoch==0 && datetimeAtNTP.year==0 && datetimeAtNTP.month==0 && datetimeAtNTP.day==0 && datetimeAtNTP.hour==0 && datetimeAtNTP.minute==0 && datetimeAtNTP.second==0)
    return false; //never initiallized by NTP, despite RTC started

  return true;
}

unsigned long rtcexpiration = 60;
bool isclockstale(){
  if(!validclock())
    return true; //never initiallized by NTP, despite RTC started

  uint32_t estimated = rtc.getEpoch();
  uint32_t lastrefresh = datetimeAtNTP.epoch;
  unsigned long age = estimated - lastrefresh;
  if(DEBUG_NTP) {
    Serial.print(F("Clock age = "));
    Serial.println(age);
  }
  if(age > rtcexpiration) { 
    return true;
  }

  //https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=3&cad=rja&uact=8&ved=2ahUKEwiEy5Le-uXlAhUOHjQIHWcIB7gQFjACegQIBRAB&url=https%3A%2F%2Fwww.norwegiancreations.com%2F2018%2F10%2Farduino-tutorial-avoiding-the-overflow-issue-when-using-millis-and-micros%2F&usg=AOvVaw10JLBPozwAg86clziwN9sP
  //millis() and micros() overflow after around 50 days and 70 minutes, respectively.
  if(millis() < uptimeAtNTP) // let's make it easy to calculate time in milliseconds between events
    return true;

   return false;
}

void ntprequest() {  
  // log drift of RTC, differing from NTP
  unsigned long rtcepoch = rtcclock().epoch;
  unsigned long expecteddelta = rtcepoch - datetimeAtNTP.epoch;

  if(DEBUG_NTP) Serial.println(Sending_NTP_packet);
  unsigned long start = millis();
  Udp2390.flush();
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp2390.parsePacket()) {
    unsigned long end = millis();
    if(DEBUG_NTP) Serial.println(packet_received);
    
    // We've received a packet, read the data from it
    Udp2390.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    Udp2390.flush();

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    if(DEBUG_NTP) {
      Serial.print(Seconds_since_Jan_1_1900_);
      Serial.println(secsSince1900);
    }

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    rtc.setEpoch(epoch); //set internal clock as soon as possible to reduce lag-behind
    uptimeAtNTP = millis();  //only for reference, in case someone wants to know time by millseconds
    
    // print Unix time:
    if(DEBUG_NTP) {
      Serial.print(Unix_time_);
      Serial.println(epoch);
    }

    // submit data for checking for rtc clock drift
    //int drifthour = (byte)(expecteddelta/3600);
    // excludeEpochFromDrift is epoch of rtc updated w/o NTP.  so delta from that time would throw off the drift regression.
    if(datetimeAtNTP.epoch!=0 && datetimeAtNTP.epoch!=excludeEpochFromDrift) {
      long actualdelta = epoch - datetimeAtNTP.epoch;
      //int index = drifthour+1; //rechecking time one hour later, is stored in 0
      rtcdriftexpected[rtcdriftindex] = expecteddelta;
      rtcdriftactual[rtcdriftindex] = actualdelta;
      if(DEBUG_NTP) {
        Serial.print(Epoch_or_date_in_seconds_when_last_synced_with_NTP_);
        Serial.println(datetimeAtNTP.epoch);      
        Serial.print(Epoch_or_date_in_seconds_of_RTC_);
        Serial.println(rtcepoch);
        Serial.print(Seconds_passed_on_RTC_therefore_Expected_from_NTP_);
        Serial.println(expecteddelta);
        Serial.print(Epoch_or_date_in_seconds_of_NTP_response_);
        Serial.println(datetimeAtNTP.epoch);
        Serial.print(Seconds_passed_according_to_NTP_therefore_actual_);
        Serial.println(actualdelta);
        Serial.print(Data_bin_for_drift_calculation_);
        Serial.println(rtcdriftindex);
      }

      int mn, md, b, r;
      int code = linreg(48, rtcdriftexpected, rtcdriftactual, &mn, &md, &b, &r);
      if(code==0) {
        if(DEBUG_NTP) Serial.print(rtc_clock_drift_calculated);
        rtcdrift.Bn = mn;
        rtcdrift.Bd = md; //1; 
        rtcdrift.A = b;
      }

      rtcdriftindex++;
      if(rtcdriftindex>47)rtcdriftindex=0;
    }

    // print the hour, minute and second:
    unsigned long hr = rtc.getHours();
    unsigned long mi = rtc.getMinutes();
    unsigned long se = rtc.getSeconds();
    if(DEBUG_NTP) {
      Serial.print(The_UTC_time_is);       // UTC is the time at Greenwich Meridian (GMT)
      Serial.print(hr); // print the hour (86400 equals secs per day)
      Serial.print(':');
      if (mi < 10) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.print(mi); // print the minute (3600 equals secs per minute)
      Serial.print(':');
    
      if (se < 10) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.println(se); // print the second
    }
    

    //https://stackoverflow.com/questions/7960318/math-to-convert-seconds-since-1970-into-date-and-vice-versa
    //tm_sec 
    //+ tm_min*60 
    //+ tm_hour*3600 
    //+ tm_yday*86400 
    //+ (tm_year-70)*31536000 
    //+ ((tm_year-69)/4)*86400 
    //- ((tm_year-1)/100)*86400 
    //+ ((tm_year+299)/400)*86400
    struct utcdatetime utc = rtcclock();
    if(DEBUG_NTP) { 
      Serial.print(rtcclock_returned_year_month_day_hr_min_sec_);
      Serial.println(utc.year);
      Serial.println(utc.month);
      Serial.println(utc.day);
      Serial.println(utc.hour);
      Serial.println(utc.minute);
      Serial.println(utc.second);
    }
    
    // Everytime we successfully get a datetime with NTP, record the synctime with real UTC time
    //uptimeAtNTP = end > start ? (start+end)/2 : end;
    //memset( &aStruct, 0, sizeof(aStruct) );
    //memcpy(dst, src, sizeof(src[0])*len);
    memcpy(&datetimeAtNTP, &utc, sizeof(utcdatetime));

    //set the boot time
    if(datetimeAtConnect.epoch==0) {
      datetimeAtConnect = datetimeAtNTP;
    }

    // Compute time of next NTP refresh
    // https://www.wikihow.com/Change-the-Time-Synchronization-Interval-in-Windows
    // It specifies no more than 1/15min... the code below breaks that rule, but just for 18min.  Then it's on average, once a day.
    if(rtcexpiration<60*48) // re-sync 1 min, 2 min, 3 min, until is refreshed
      rtcexpiration+=60;
    else // re-sync 1 hr, 2 hr, 3 hr, until is refreshed
      rtcexpiration+=3600;
    //rtcexpiration += 3600;
    if(rtcexpiration>2592000)
      rtcexpiration = 3600;

    // Draw some text
    epd.clearBuffer();
    epd.setCursor(0, 10);
    epd.setTextSize(2);
    epd.setTextColor(EPD_BLACK);
    String uptime = String(millis());
    //epd.print(uptime + "\n" + String(hr) + ":" + String(mi) + ":" + String(se));
    String timestr = formattedlocal(utc);
    timestr.replace(" ","\n");
    String output = "Uptime:"+uptime + "ms\n" + timestr;
    timestr.~String();
    if(DEBUG_NTP) Serial.println(output); 
    epd.print(output);
    epd.setCursor(150, 70);
    epd.setTextSize(1);
    epd.setTextColor(EPD_RED);
    epd.print(__NTP);
    //epd.display();

    if(DEBUG_FREE_MEMORY) {
        Serial.print(End_of_NTPRequest_Free_memory_);
        Serial.println(freeMemory());    
    }

    //delay(15000);
    
    epdDisplayDeferredDelay(); // this needs to be here, bc this can be executed anytime the clock goes stale, even within 15sec of a refrsh

    //checks if the UDP buffer has more data, and throw away
    int limit = Udp2390.parsePacket();
    while(Udp2390.parsePacket() && (limit--)>0) {
      // the get the last packet of data, so that old data doesn't affect this
      Udp2390.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
      Udp2390.flush();
      delay(500);
    }
  }
  else {
    if(DEBUG_NTP)
      Serial.println(No_NTP_response_received);
      
    //if we can't update NTP, perhaps we can adjust the clock a little
    /*if(rtcdrift.Bn!=0 || rtcdrift.A!=0){
      unsigned long estimateddelta = rtcdrift.A + expecteddelta * rtcdrift.Bn / rtcdrift.Bd;
      if(estimateddelta!=expecteddelta) {
        rtc.setEpoch(datetimeAtNTP.epoch + estimateddelta);
        if(DEBUG_NTP)
          Serial.println("RTC adjusted for drift to NTP server: " + String(estimateddelta-expecteddelta) + "sec");
      }
    }*/
    
    /* 12/25, Don't adjust the RTC according to what you think the drift should be.
     *  THe below logic has a problem.  And the existing RTC drift is better than figuring out what is wrong w the algorithm below.
    unsigned long estimateddelta = computedrift(expecteddelta);
    if(estimateddelta!=expecteddelta) {
      unsigned long driftadjustedepoch = datetimeAtNTP.epoch + estimateddelta;
      rtc.setEpoch(driftadjustedepoch);
      excludeEpochFromDrift = driftadjustedepoch;

      if(DEBUG_NTP) {
        Serial.print(RTC_adjusted_for_drift_to_NTP_server_);
        Serial.print((long)estimateddelta-(long)expecteddelta);
        Serial.println(sec);
      }
    }*/
  }
}
long getdrift() {
  if(rtcdrift.Bn!=0 || rtcdrift.A!=0) {
    unsigned long rtcepoch = rtcclock().epoch;
    unsigned long expecteddelta = rtcepoch - datetimeAtNTP.epoch;
    unsigned long est = computedrift(expecteddelta);
    if(expecteddelta!=est)
      return est-expecteddelta;
  }
  return 0;
}
unsigned long computedrift(unsigned long expecteddelta) {
  if((rtcdrift.Bn!=0 || rtcdrift.A!=0) && rtcdrift.Bd!=0){
    unsigned long estimateddelta = rtcdrift.A + expecteddelta * rtcdrift.Bn / rtcdrift.Bd;
    unsigned long overflowmax = 2147483647 / rtcdrift.Bn; // this max starts at 3072 and gets smaller as the m numbers get bigger(more precise fractions)
    if(expecteddelta>overflowmax) { 
      double x = double(expecteddelta);
      double m = double(rtcdrift.Bn)/double(rtcdrift.Bd);
      double mx = x * m;
      estimateddelta = rtcdrift.A + (unsigned long)mx;
    }
    if(estimateddelta!=expecteddelta) {
      return estimateddelta;
    }
  }
  return expecteddelta;  
}

struct utcdatetime rtcclock() {
  uint32_t epoch = rtc.getEpoch();
  uint8_t se = rtc.getSeconds();
  uint8_t mi = rtc.getMinutes();
  uint8_t hr = rtc.getHours();
  uint8_t da = rtc.getDay();
  uint8_t mo = rtc.getMonth();
  uint16_t yr = rtc.getYear();
  if(yr<100) yr +=2000;

  //struct utcdatetime {
  //  unsigned long epoch;
  //  unsigned int year;
  //  unsigned int month;
  //  unsigned int day;
  //  unsigned int hour;
  //  unsigned int minute;
  //  unsigned int second;
  //};

  struct utcdatetime utc = { epoch, yr, mo, da, hr, mi, se };
  
  return utc;
}

/*
unsigned long ntpage() {
  if(validclock())
    return millis() - uptimeAtNTP;
  else
    return 0;
}*/

//https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=6&cad=rja&uact=8&ved=2ahUKEwj9n6Cf0NnlAhVfHjQIHek7CcUQFjAFegQIChAS&url=https%3A%2F%2Fen.m.wikipedia.org%2Fwiki%2FDaylight_saving_time_in_the_United_States&usg=AOvVaw0SnFENP-NPmk4qBZt2vYXF
String formattedlocal(struct utcdatetime dt) {
  if(!validclock())
    return String(0);
  
  long yr = (long)dt.year;
  long mo = (long)dt.month;
  long da = (long)dt.day;
  long hr = (long)dt.hour;
  long mi = (long)dt.minute;
  long se = (long)dt.second;

  //Serial.println("formatting");
  //Serial.println(yr);
  //Serial.println(mo);
  //Serial.println(da);
  //Serial.println(hr);
  //Serial.println(mi);
  //Serial.println(se);
  
  //Hard-code the Time-Zone
  long hr2 = (long)hr + tz; //i'm assuming two complement representation which positive values are the same for unsigned and signed... as long as there is no overflow... it's a hour and min, for christ's sake.  The long was just in case the mod operator returns long, I didnt have to do a cast into unsigned int.
  long mi2 = (long)mi + tz2; //apparently this is possible, though I can't imagine this affecting me in my gerry-rigged code.
  unsigned long numdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
  if(yr%4==0)
    numdays[2] = 29;
  if(mi2>=60){
    hr2++;
    mi2-=60;
  }
  if(mi2<0){
    hr2--;
    mi2+=60;
  }

  //The Uniform Time Act of 1966 established the system of uniform Daylight Saving Time throughout the US. 
  //In the U.S., daylight saving time starts on the second Sunday in March 
  //and ends on the first Sunday in November, with the time changes taking place at 2:00 a.m. local time.
  unsigned int secondsuninmar = dow(yr,3,1);
  if(secondsuninmar==0) //the exact date for 2nd sun in mar, of that year, once we know what day the 1st is
    secondsuninmar = 8;
  else
    secondsuninmar = (8-secondsuninmar) + 7;
  unsigned int firstsuninnov = dow(yr,11,1);
  if(firstsuninnov==0) //the exact date for 1st sun in nov, of that year, once we know what day the 1st is
    firstsuninnov = 1;
  else
    firstsuninnov = (8-secondsuninmar);
  if(mo>=3 && mo<=11) {
    //if(!((mo==3 && da<secondsuninmar) || (mo==3 && da==secondsuninmar && hr2<2) || (mo==11 && da>firstsuninnov) || (mo==11 && da==firstsuninnov && hr2>2) )) { //dunno if this works, the below seemed clearer to read
    unsigned long startdst = yr*1000000 + 30000 + secondsuninmar * 100 + 2; //2019030401 for 3/4/19@1a, and 32bit range is 0 through 4,294,967,295 (23^2 − 1)
    unsigned long enddst =  yr*1000000 + 110000 + firstsuninnov * 100 + 2;
    unsigned long today =  yr*1000000 + mo*10000 + da * 100 + hr;
    if(startdst<=today && today <=enddst) {
      hr2++;
      tzabbr.setCharAt(1,'D');
    }
  }

  // Everytime you do date math, you have to account for the artificial overflow conditions
  if(hr2>=24){
    hr2-=24;
    da++;
    if(da>numdays[mo]){
      mo++;
      if(mo>12){
        mo=1;
        yr++;
      }
      da=1;
    }
  }
  if(hr2<0){
    hr2+=24;
    da--;
    if(da<1){
      mo--;
      if(mo<1){
        mo=12;
        yr++;
      }
      da=numdays[mo];
    }
  }

  //Serial.println(" to ");
  //Serial.println(yr);
  //Serial.println(mo);
  //Serial.println(da);
  //Serial.println(hr2);
  //Serial.println(mi2);
  //Serial.println(se);

  
  //debug
  //unsigned int a = dow(2019,11,7); //should be thu
  //Serial.println("should be thu(4)");
  //unsigned int b = dow(2019,11,8); //should be fri
  //Serial.println("should be fri(5)");
  //unsigned int c = dow(1969,7,20); //should be sun
  //Serial.println("should be sun(0)");
  //unsigned int d = dow(2000,1,1); //should be sat
  //Serial.println("should be sat(6)");

  String weekdays[] = {F("Sun"),F("Mon"),F("Tue"),F("Wed"),F("Thu"),F("Fri"),F("Sat")};
  unsigned int dayinweek = dow(yr, mo, da);
  String weekday = dayinweek>=0 && dayinweek<=6 ? weekdays[dayinweek] : F("DoW_err");
  
  String formatted = weekday +spToStr+ String(mo)+"/"+String(da)+"/"+String(yr)+spToStr+String(hr2)+(mi2<10?":0":":")+String(mi2)+(se<10?":0":":")+String(se)+" "+tzabbr;
  // Not used yet bc I'm still using string everywhere, anyway.
  //char format[] = "DAY 0M/0D/YYYY 0H:0M:0S ZZZZZZZ   ";
  //updateCStr(format, 0, 4, weekday);
  //updateCStr(format, 5-mo/10, 3, String(mo));
  //updateCStr(format, 7-da<10?1:0, 3, String(da));
  //updateCStr(format, 10, 5, String(yr));
  //updateCStr(format, 15-hr2/10?1:0, 3, String(hr2));
  //updateCStr(format, 18-mi2/10?1:0, 3, String(mi2));
  //updateCStr(format, 21-se/10?1:0, 3, String(se));
  //updateCStr(format, 24, 8, tzabbr);
  //format[sizeof(format)-1] = 0;

  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("End of formattedlocal() Free memory="));
      Serial.println(freeMemory());    
  }

  return formatted;
}

//https://artofmemory.com/blog/how-to-calculate-the-day-of-the-week-4203.html
unsigned int dow(unsigned long yr, unsigned long mo, unsigned long da) {
  //Serial.println("Day of week for " + String(yr) + " " + String(mo) +" "+ String(da) + "...");
  unsigned int yy = (unsigned int)yr % 100;
  unsigned int ycode = (yy + (yy / 4)) % 7;
  unsigned int mcoder[] = {65535,0,3,3,6,1,4,6,2,5,0,3,5}; //month is 1-12, so zero index has to get filled
  unsigned mcode = mcoder[mo];

  //Serial.println(yy);
  //Serial.println(ycode);
  //Serial.println(mcode);

  unsigned int century = (unsigned int)yr/100;
  if(century==0) //requires a 4-year date
    return -1;
   
  unsigned int ccoder[] = { 4,2,0,6,4,2,0 };
  unsigned int ccode = ccoder[century-17];

  //doesn't account for century exclusion... ie. 2100 isn't a leap year
  unsigned int leapyearcode = yr%4==0 ? 1: 0;

  unsigned int result = (ycode + mcode + ccode + da - leapyearcode) % 7;
  //Serial.println("..." + String(result));
  return result;
}
String formatToDHMS(unsigned long totsec) {
  //Serial.println("formatToDHMS()");
  //Serial.println(totsec);
  unsigned long seconds = totsec;
  unsigned long minutes = seconds / 60;
  seconds = seconds % 60;
  unsigned long hours = minutes / 60;
  minutes = minutes % 60;
  unsigned long days = hours / 24;
  hours = hours % 24;
  //Serial.println(seconds);
  //Serial.println(minutes);
  //Serial.println(hours);
  //Serial.println(days);
  //if(seconds!=0) 
  //  Serial.println("if statement works correctly");
  if(days!=0) 
    return String(days) + "d:" + String(hours) + ":" + String(minutes); // + ":" + String(seconds);
  else if(hours!=0) 
    return String(hours) + "h:" + String(minutes) + ":" + String(seconds);
  else if(minutes!=0) 
    return String(minutes) + "m:" + String(seconds);
  else if(seconds!=0) 
    return String(seconds) + "sec(s)";
  else
    return "";
}

/*
// https://stackoverflow.com/questions/11188621/how-can-i-convert-seconds-since-the-epoch-to-hours-minutes-seconds-in-java/11197532#11197532
struct utcdatetime epochToDateTime(unsigned long seconds)
{
  //unsigned long sec;
  //unsigned long quadricentennials, centennials, quadrennials, annuals;
  //unsigned long year, leap;
  //unsigned long yday, hour, min;
  //unsigned long month, mday, wday;
  //static const unsigned long daysSinceJan1st[2][13]=
  //{
  //  {0,31,59,90,120,151,181,212,243,273,304,334,365}, // 365 days, non-leap
  //  {0,31,60,91,121,152,182,213,244,274,305,335,366}  // 366 days, leap
  //};

  //every 400 years, how many seconds starting from which year
  //every 100 years within above, how are seconds divided?
  //every 4years in above, how many seconds are divided
  //and WHY THE FUCK am I calculating for leap centuries?  Ignore the leap centuries.
  //31536000 seconds in a year.
  //31536000+24*60*60 seconds in a leap year.
  // every 4 years is 31536000*3 + 31536000+24*60*60 seconds
  //assuming 2000 is leap year, but normally 1700, 1800, and 1900 were not
  unsigned long day = 86400; //60*60*24;
  unsigned long hour = 3600; //60*60;
  unsigned long minute = 60; //60;
  unsigned long regyear = 31536000;
  unsigned long leapyear = 31622400;
  unsigned long fouryears = (3*regyear) + leapyear;
  //1970 is not a leap year!  I have no idea why I thought it was a leap year!
  //seconds is number of seconds since 1970.  
  //to make the math easier, let's start the number of seconds at 1969, and add 2 reg years of seconds
  //1969, 1970, 1971 are all reg years.  1972 is the leap year.  by putting the leap year at end of 4year cycle, the number of seconds into current year, is just always the remainder
  unsigned long since01011969 = seconds + regyear;
  unsigned long numberoffouryear = since01011969 / fouryears;
  unsigned long secondsintofouryear = since01011969 % fouryears;
  unsigned long whichfouryear = secondsintofouryear / regyear;
  unsigned long secondsintocurrentyear = secondsintofouryear % regyear;
  unsigned long UTCyear = 1969 + (4*numberoffouryear) + whichfouryear; //see the last comment
  unsigned long daysintocurrentyear = secondsintocurrentyear/day;
  //rather than iteratively, I've just hard-coded a number of days at end of month.  leap year is at end of 4year cycle starting from 1969 (epoch is 1970, so some math was needed).
  unsigned long jan = 31;
  unsigned long feb = 60;
  unsigned long mar = 91;
  unsigned long apr = 121;
  unsigned long may = 152;
  unsigned long jun = 182;
  unsigned long jul = 213;
  unsigned long aug = 244;
  unsigned long sep = 274;
  unsigned long oct = 305;
  unsigned long nov = 335;
  unsigned long dec = 366;
  if(whichfouryear!=3){ //{0,31,59,90,120,151,181,212,243,273,304,334,365}, // 365 days, non-leap
    feb = 59;
    mar = 90;
    apr = 120;
    may = 151;
    jun = 181;
    jul = 212;
    aug = 243;
    sep = 273;
    oct = 304;
    nov = 334;
    dec = 365;
  }
  //below is a hard-coded binary search, rather than iteratively search thru array.
  //Serial.print("Leap year is 0:"); 
  //Serial.println(whichfouryear); 
  //Serial.print("Days into year:"); 
  //Serial.println(daysintocurrentyear); 
  unsigned long UTCmonth = 0;
  unsigned long UTCday = 0;
  if(daysintocurrentyear < jun){
    //0,31,60,91,121,152,182,
    if(daysintocurrentyear < mar) {
      if(daysintocurrentyear < jan) {
        UTCmonth = 1;
        UTCday = daysintocurrentyear;
      } else if(daysintocurrentyear < feb) {
        UTCmonth = 2;
        UTCday = daysintocurrentyear - jan;
      } else {
        UTCmonth = 3;
        UTCday = daysintocurrentyear - feb;
      }
    }
    else {
      if(daysintocurrentyear < apr) {
        UTCmonth = 4;
        UTCday = daysintocurrentyear - mar;
      } if(daysintocurrentyear < may) {
        UTCmonth = 5;
        UTCday = daysintocurrentyear - apr;
      } else {
        UTCmonth = 6;
        UTCday = daysintocurrentyear - may;
      }
    }
  } else {
    //(182,)  213,244,274,305,335,366
    if(daysintocurrentyear < sep) {
      if(daysintocurrentyear < jul) {
        UTCmonth = 7;
        UTCday = daysintocurrentyear - jun;
      } else if(daysintocurrentyear < aug) {
        UTCmonth = 8;
        UTCday = daysintocurrentyear - jul;
      } else {
        UTCmonth = 9;
        UTCday = daysintocurrentyear - aug;
      }
    }
    else {
      if(daysintocurrentyear < oct) {
        UTCmonth = 10;
        UTCday = daysintocurrentyear - sep;
      } if(daysintocurrentyear < nov) {
        UTCmonth = 11;
        UTCday = daysintocurrentyear - oct;
      } else {
        UTCmonth = 12;
        UTCday = daysintocurrentyear - nov;
      }
    }
  }
  unsigned long secondsintocurrentday = secondsintocurrentyear%day;
  unsigned long UTChour = secondsintocurrentday/hour;
  unsigned long secondsintocurrrenthour = secondsintocurrentday%hour;
  unsigned long UTCmin = secondsintocurrrenthour/minute;
  unsigned long UTCsec = secondsintocurrrenthour%minute;

  struct utcdatetime utc = {seconds, (unsigned int)UTCyear, (unsigned int)UTCmonth, (unsigned int)UTCday+1, (unsigned int)UTChour, (unsigned int)UTCmin, (unsigned int)UTCsec};
  
  return utc;
}
*/

struct analogsamplesum getsample(unsigned int pin, int voltagecalibrate, struct linearconv *unitconverter) {
  if(DEBUG_SAMPLING) Serial.println("Sampling on " + String(pin));

  unsigned long reading = 0;
  unsigned long sum = 0;  
  unsigned long count = 0;
  //int fudge_factor = 0;
  //int fudge_drift = 1676-1706;

  analogReadResolution(12);
  unsigned long mn = 32767;
  unsigned long mx = 0;
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  while (count < sample_count) {
    reading = analogRead(pin);
    sum += reading;
    count++;
    if(reading<mn)
      mn = reading;
    if(reading>mx)
      mx = reading;
    if(sample_delay!=0)
      delay(sample_delay);
  }
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

  if(DEBUG_SAMPLING) {
    Serial.print(F("Sum="));
    Serial.println(sum);
    Serial.print(F("Count="));
    Serial.println(count);
  }

  struct analogsamplesum result; // = { millis(), pin, count, sum, mx, mn, sum/count, scaleN, scaleD, intercept, intercept + ((scaleN * (int)sum)/((int)count*scaleD)), units };
  /*struct analogsamplesum {
    unsigned long ms;
    unsigned int pin;
    unsigned int count;
    unsigned int sum;
    unsigned int mx;
    unsigned int mn;
    unsigned int avg;
    unsigned int tovolts;
    int scaleN;
    int scaleD=1024;
    int fudged=0;
    int converted;
    String units;
};*/
  
  result.ms = millis();
  result.pin = pin;
  result.count = (unsigned int)count;
  result.sum = (unsigned int)sum;
  result.mx = (unsigned int)mx;
  result.mn = (unsigned int)mn;
  //result.avg = (unsigned int)(sum/count);
  result.calibrate =  (int)voltagecalibrate;
  result.converter = unitconverter;

  if(DEBUG_SAMPLING) {
    Serial.println(F("struct confirm"));
    Serial.print(F("-->pin"));
    Serial.println(result.pin);
    Serial.print(F("-->sum"));
    Serial.println(result.sum);
    Serial.print(F("-->count"));
    Serial.println(result.count);
    Serial.print(F("-->calibrate"));
    Serial.println(result.calibrate);
    Serial.print(F("-->converter-->A"));
    Serial.println(result.converter->A);
    Serial.print(F("-->converter-->Bn"));
    Serial.println(result.converter->Bn);
    Serial.print(F("-->converter-->Bd"));
    Serial.println(result.converter->Bd);
  }
  
  //result.tovolts = ((unsigned int))((3300UL * sum)/(count*4095UL));
  //if(-voltagecalibrate>result.tovolts)
  //else
  //  result.tovolts+=voltagecalibrate;
  //result.converted = unitconverter.A + ((unitconverter.Bn * (long)result.tovolts)/(unitconverter.Bd));

  computesampleAvg(&result);
  computesampleVolts(&result);
  computesampleConversion(&result);
  
  if(DEBUG_SAMPLING) {
    Serial.print(F("Conversion starts at "));
    Serial.print(unitconverter->A);
    Serial.print(F(" and every millivolt translates to "));
    Serial.print(unitconverter->Bn);
    Serial.print(F("/"));
    Serial.print(unitconverter->Bd);
    Serial.print(F("= "));
    Serial.print(unitconverter->Bn/unitconverter->Bd);
    Serial.println(" milli" +unitconverter->units);
    
    Serial.println("Sample totaled=" + String(sum) 
                  + ", count=" + String(count)
                  + ", avg= "  + String(result.avg)
                  + " range of (" + String(mn)
                  + "-"  + String(mx)
                  + ","  + String(result.mx-result.mn)
                  + ") voltage of " + String(result.tovolts) 
                  + ", which translates to "  + String(result.converted)+unitconverter->units );
  }
  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("End of getsample() Free memory="));
      Serial.println(freeMemory());    
  }
  
  return result;
}
void computesampleAvg(struct analogsamplesum *record) {
  if(record->count==0)
    record->avg = 0;
  else
    record->avg =  (unsigned int)((unsigned long)record->sum/(unsigned long)record->count);
}
void computesampleVolts(struct analogsamplesum *record) {
  unsigned long work = ((3300UL * (unsigned long)record->sum)/((unsigned long)record->count*4095UL));
  long volts = (long)work; // the ratio arithmetic needs to be done in unsigned long b/c the adc converstion is unsigned, but the calibration is signed, so it needs to be in long
  long voltagecalibrate = record->calibrate;
  if(-voltagecalibrate>volts)
    volts = 0;
  else
    volts+=voltagecalibrate;
    
  record->tovolts = (unsigned int)volts; // after calibration, it's treated as a unsigned millivolt value again
}
void computesampleConversion(struct analogsamplesum *record) {
  struct linearconv *unitconverter = record->converter;
  //*unitconverter = record->converter;
  long A = unitconverter->A;
  long Bn = unitconverter->Bn;
  long Bd = unitconverter->Bd;
  long convertedvalue = A + ((Bn * (long)record->tovolts)/(Bd));

  record->converted  = (int)convertedvalue;
}

void autoadd(struct analogsamplesum* total, struct analogsamplesum addend) {
//struct analogsamplesum {
//    unsigned long ms;
//    unsigned int pin;
//    unsigned long count;
//    unsigned long sum;
//    unsigned long mx;
//    unsigned long mn;
//    unsigned long avg;
//    unsigned long calibrate;
//    unsigned long tovolts;
//    struct linearconv converter;
//    int converted;
//};
  total->ms += addend.ms;
  //total.pin
  total->count += addend.count;
  total->sum += addend.sum;
  total->mx = max(total->mx, addend.mx);
  total->mn = min(total->mn, addend.mn);
  
  //total->avg += addend.avg;
  //*total.calibrate;
  //total->tovolts = (3300UL * total->sum)/(total->count*4095UL);
  //if(-(total->calibrate) > (total->tovolts))
  //  total->tovolts = 0;
  //else
  //  total->tovolts+=total->calibrate;
  //*total.converter;
  //total->converted = (total->converter.A) + (( (total->converter.Bn) * (long)(total->tovolts))/(total->converter.Bd));

  computesampleAvg(total);
  computesampleVolts(total);
  computesampleConversion(total);
  
}

void autoadd2(struct aggregate* total, unsigned long sum) {
//struct analogsamplesum {
//    unsigned long ms;
//    unsigned int pin;
//    unsigned long count;
//    unsigned long sum;
//    unsigned long mx;
//    unsigned long mn;
//    unsigned long avg;
//    unsigned long calibrate;
//    unsigned long tovolts;
//    struct linearconv converter;
//    int converted;
//};
  if(total->count==0)
    total->sum = 0;
  total->count++;
  total->sum += sum;
  total->mx = max(total->mx, sum);
  total->mn = min(total->mn, sum);
  
  computeAggregateAvg(total);
}
void computeAggregateAvg(struct aggregate* total) {
  if(total->count==0) {
    total->avg=0;  
  } else {
    total->avg = total->sum/total->count;
  }
}

//re-level sets, sorounding errors may exist
void inclInAvg(struct analogsamplesum* total, struct analogsamplesum addend) {
//struct analogsamplesum {
//    unsigned long ms;
//    unsigned int pin;
//    unsigned long count;
//    unsigned long sum;
//    unsigned long mx;
//    unsigned long mn;
//    unsigned long avg;
//    unsigned long calibrate;
//    unsigned long tovolts;
//    struct linearconv converter;
//    int converted;
//};
  total->ms = millis();
  //total.pin
  total->count ++;
  total->sum += addend.avg;
  total->mx = max(total->mx, addend.avg);
  total->mn = min(total->mn, addend.avg);
  
  //total->avg += addend.avg;
  //*total.calibrate;
  //total->tovolts = (3300UL * total->sum)/(total->count*4095UL);
  //if(-(total->calibrate) > (total->tovolts))
  //  total->tovolts = 0;
  //else
  //  total->tovolts+=total->calibrate;
  //*total.converter;
  //total->converted = (total->converter.A) + (( (total->converter.Bn) * (long)(total->tovolts))/(total->converter.Bd));

  computesampleAvg(total);
  computesampleVolts(total);
  computesampleConversion(total);
  
}
void reduceToAvg(struct analogsamplesum* sample) {
  sample->count=sample->count/2;
  sample->sum = sample->sum/2;
}

void zeroOut(struct analogsamplesum* total) {
  total->ms = 0;
  total->count =0;
  total->sum =0;
  total->mx = 0;
  total->mn = 32000;
  total->avg = 0;
  total->tovolts =0;
  total->converted=0;     
}


char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}
String format0_0(long num, int decimalplaces) {
  if(decimalplaces>0) {
    long maxvalue = round(pow(10,abs(decimalplaces)+1));
    while((maxvalue<=abs(num)) && (decimalplaces>0)) {
      //Serial.print("Max value that can be presumably displayed=");
      //Serial.println(maxvalue);
      //Serial.print("Currently formatting=");
      //Serial.println(num);
      //Serial.print("assuming decimal places=");
      //Serial.println(decimalplaces);
      num=num/10;
      decimalplaces--;
    }
    maxvalue = round(pow(10,abs(decimalplaces)));
    long places = pow(10,decimalplaces);
    long whole = num / places;
    long decimal = abs(num % places);
    String result = (num<0 && whole==0 ? "-" : "") + String(whole) + ".";
    if(decimal==0)
      result +="0";
    else {
      long decimal2 = decimal*10;
      //Serial.print("number=");
      //Serial.println(num);
      //Serial.print("decimal places=");
      //Serial.println(decimalplaces);
      //Serial.print("decimal to be displayed=");
      //Serial.println(decimal);
      //Serial.print("is not decimal, if bigger than=");
      //Serial.println(maxvalue);
      while(decimal2<maxvalue) {
        //Serial.print("Adding zero bc next number is still smaller than 1=");
        //Serial.println(decimal2);
        result +="0";
        decimal2*=10;
      }
    }
    result +=String(decimal);
    result.replace("0","O"); //damn 0, looks like an 8.
    return result;
  } else if(decimalplaces<0) {
    long places = pow(10,decimalplaces);
    long whole = num * places;
    return String(whole);
  }
}
String formatHHMM(struct utcdatetime utc) {
  if(utc.epoch==0)
    return ""; // not a timestamp derived value
  
  String formatted = formattedlocal(utc);
  unsigned int len = formatted.length();
  //between second and 3rd space
  unsigned int count=0;
  unsigned int start=0;
  unsigned int i=0;
  while(count<3 && i<50) {
    if(formatted.charAt(i)==' ')
    {
      count++;
      if(count==2)
        start = i;
    }
    i++;
  }
  if(start!=0 && count==3) {
    String trimmed = formatted.substring(start+1, i-4);
    formatted.~String();
    return trimmed;
  }else{
    formatted.~String();
    return "";
  }
}

void hdashRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  epd.fillRect(x, y, w, h, color);
  if(abs(h)>1) {
    int arrow = h>1 ? 1 : -1;
    int len=abs(h)/2 +1;
    for(unsigned int i=1; i<len; i++)
      epd.drawFastHLine(x, y+(i*2*arrow)-arrow, w, EPD_INVERSE);
  }
}


void addToGraph(struct analogsamplesum amperes, struct analogsamplesum voltage, struct analogsamplesum solarA, struct analogsamplesum solarV) {
  //get size of labels
  const String string = "90.000";
  int16_t x=0;
  int16_t y=0;
  int16_t l=0;
  int16_t t=0;
  uint16_t w=0;
  uint16_t h=0;
  epd.getTextBounds(string2char(string), 0, 0, &l, &t, &w, &h);
  if(DEBUG_UI) {
    Serial.print(string);
    Serial.print(F(" size= ("));
    Serial.print(w);
    Serial.print(F("px x "));
    Serial.print(h);
    Serial.println(F("px)"));
  }
  
  // Draw date
  epd.clearBuffer();
  epd.setCursor(0, 0);
  epd.setTextColor(EPD_BLACK);
  String timstr = formattedlocal(rtcclock());
  epd.print(timstr);
  timstr.~String();


  //output new data
  if(DEBUG_UI) { 
    Serial.print(F("Ampere Sample was averaged to "));
    Serial.print(amperes.avg);
    Serial.print(F(" which converted to volts is "));
    Serial.print(format0_0(amperes.tovolts,3));
    Serial.print(F(", converted & formatted to amps to "));
    Serial.println(format0_0(amperes.converted,3));
  
    Serial.print(F("Voltage Sample was averaged to "));
    Serial.print(voltage.avg);
    Serial.print(F(" which converted to signal volts is "));
    Serial.print(format0_0(voltage.tovolts,3));
    Serial.print(F(", converted & formatted to measured voltage to "));
    Serial.println(format0_0(voltage.converted,3));
  }
  /*
  epd.setCursor(10, 12);
  epd.print(String(amperes.avg) + " for " +String(amperes.tovolts)+ "v is " + String(amperes.converted) + amperes.converter.units);

  epd.setCursor(10, 110);
  epd.print(String(voltage.avg) + " for " +String(voltage.tovolts)+ "v is " + String(voltage.converted) + voltage.converter.units);
  */
  // process new data points
  //graphactualmah[graphdatahead] = amperes.converted;
  readings[graphdatahead].utc = rtcclock();
  readings[graphdatahead].batterymA = amperes.converted;
  readings[graphdatahead].batterymV = voltage.converted;
  readings[graphdatahead].solarmA = solarA.converted;
  readings[graphdatahead].solarmV = solarV.converted;
  
  if(DEBUG_UI) Serial.println("Date added to data=" + formatHHMM(readings[graphdatahead].utc)); //datatime[graphdatahead]));
  int battpx = 6*amperes.converted / 1000; // 6px/amp 
  if(DEBUG_UI) { 
    Serial.print(F("amp sample is "));
    Serial.print(battpx);
    Serial.println(F("px high"));
  }
  int moonpx = 3*voltage.converted / 1000; // 2px/v 
  if(DEBUG_UI) { 
    Serial.print(F("volt sample is "));
    Serial.print(moonpx);
    Serial.println(F("px high"));
  }
  batteryamppxgraph[graphdatahead] = battpx;
  batteryvoltpxgraph[graphdatahead] = moonpx;

  int panelpx = 6*solarA.converted / 1000; // 2px/v 
  if(DEBUG_UI) { 
    Serial.print(F("solar ampere sample is "));
    Serial.print(panelpx);
    Serial.println(F("px high"));
  }
  int solpx = 3*solarV.converted / 1000; // 2px/v 
  if(DEBUG_UI) { 
    Serial.print(F("solar voltage sample is "));
    Serial.print(solpx);
    Serial.println(F("px high"));
  }
  solaramppxgraph[graphdatahead] = panelpx;
  solarvoltpxgraph[graphdatahead] = solpx;
  graphdatahead++;
  if(graphdatahead>=DATA_COUNT)
    graphdatahead = 0;

  // convert hourly log of battery charge/discharge to graph px
  int hourlyAmpGraph[DATA_COUNT];
  for(int i=0; i<DATA_COUNT;i++)
    hourlyAmpGraph[i] = 6*hourlyAmpSum[readings[(graphdatahead+i) % DATA_COUNT].utc.hour].converted/1000;

  // int hourlyAmpGraph[24] = {7,7,7,6,4,3,1,1,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,0,1};
  //for(int i=0; i<24;i++)
  //  hourlyAmpGraph[i] = 6*hourlyAmpGraph[i];


  // calculate size and position of graph
  unsigned int x1 = w+2;
  unsigned int y1 = 24;
  unsigned int w1 = 250-x1*2;
  unsigned int h1 = 122-36;
  unsigned int x2 = x1+w1;
  unsigned int y2 = y1+h1;
  int xaxisoffset = 20;

  struct graphproperties pasthourlygraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, hourlyAmpGraph, graphdatahead, 24, steps6, drawAvgA);
  
  struct graphproperties voltgraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, batteryvoltpxgraph, graphdatahead, DATA_COUNT, bigdot, offsetBlip);
  struct graphproperties solarVgraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, solarvoltpxgraph, graphdatahead, DATA_COUNT, bigdash, offsetSun);
  
  struct graphproperties ampgraph  = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, batteryamppxgraph,  graphdatahead, DATA_COUNT, column, drawRightTick);
  struct graphproperties solarAgraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, solaramppxgraph, graphdatahead, DATA_COUNT, dotted, drawNothing);


  //drawHist(x1+164, pasthourlygraph.lastDataY, EPD_BLACK);



  // create (2) x-axis tick labels of time
  String labelX1 = formatHHMM(readings[graphdatahead].utc);
  if(labelX1.length()!=0) {
    epd.setCursor(x1, y2+3);
    epd.print(labelX1);
    uparrow(x1+4, y2-2);
  } else {
    epd.setCursor(x1, y2+3);
    epd.print(String(DATA_COUNT * MIN_PER_GRAPH_PX) + "min");
    uparrow(x1+4, y2-2);
  }

  unsigned int middle = (graphdatahead+DATA_COUNT/2)%DATA_COUNT;
  String labelX2 = formatHHMM(readings[middle].utc);
  if(labelX2.length()!=0) {
    epd.setCursor(x1+DATA_COUNT/2, y2+3);
    epd.print(labelX2);
    uparrow(x1+DATA_COUNT/2, y2-2);
  } else {
    epd.setCursor(x1+DATA_COUNT/2, y2+3);
    epd.print(String(DATA_COUNT * MIN_PER_GRAPH_PX/2) + "min");
    uparrow(x1+DATA_COUNT/2, y2-2);
  }

  epd.setCursor(x1+DATA_COUNT-10, y2+3);
  epd.print("now");
  uparrow(x1+DATA_COUNT, y2-2);

  
  // show current wattage consumed / charging
  /*
  int watts = (int)((long)amperes.converted * (long)voltage.converted/1000L);
  epd.setCursor(170, y2+3);
  epd.print(format0_0(watts/10,2)+ " watts");
  */
  

  /*
   * I checked the implementation for drawPixel() in github
   * https://github.com/adafruit/Adafruit_EPD/blob/master/Adafruit_EPD.cpp
   * And the damn thing doesn't have a if-else for the color EPD_WHITE.  
   * the pBuf is initialized as null (I assume in c-world, this is always all zero bits, and why c afficienados bitch about check for null by just if(null), which is if(0000000), which is if(false))...which almost every language imitates logically in some fashion.  if(null) always seems to result to false.
   * in fact, a lot of languages, when you combine null + something else, if they dont give you an error, it's usually null again.  except strings.
   *
   * ANYWAY.  Im saying there doesn't seem to be code to overwrite a pixel with white.
   *
  epd.setColorBuffer(0,true);
  epd.fillRect(0, 50, 100, 20, EPD_BLACK);
//  epd.fillRect(x+8, y, w, h, EPD_WHITE);
  epd.drawLine(x,y+h,x+w+8,y+h, EPD_BLACK);
  for(unsigned int j=100; j<150; j++)
    for(unsigned int i=0; i<150; i++)
      epd.drawPixel(i,j,EPD_BLACK);
  epd.setColorBuffer(0,false);
  */

  //show bar graph of current power status, with labels and tick markers
  //...unit labels
  epd.setCursor(250-w, y1);
  epd.print(amperes.converter->units);
  epd.setCursor(0, y1);
  epd.print(voltage.converter->units);

  //...now data labels and bars
  //        x-axis, y-axis,         +Y range        -Y range      -widthofbar, n/a  x, y-data,   label on bar,                                     label width, label height
  bool usvertleft = moonpx < solpx;
  bool usvertright = battpx < panelpx;
  bargraph2(250-w,  y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, battpx,   format0_0(amperes.converted,3),                    w,           h,          usvertright);
  bargraph2(0,      y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, moonpx,    format0_0(voltage.converted,3),                   w,           h,          usvertleft);
  
  bargraph2(250-w/2,y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, panelpx,  usvertright ? format0_0(solarA.converted,3) : "",  w,           h,          usvertright);
  bargraph2(w/2,    y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, solpx,  usvertleft ? format0_0(solarV.converted,3) : "",     w,           h,          usvertleft);

  drawBlip(w/4-2,y2-h, EPD_BLACK);
  drawSun(3*w/4-2,  y2-h, EPD_BLACK);
  epd.fillRect(3*w/4-3, y2-h+8-1, 3,3, EPD_BLACK);
  epd.fillRect(3*w/4-3, y2-h+12-1, 3,3, EPD_BLACK);
  epd.fillRect(3*w/4-3, y2-h+16-1, 3,3, EPD_BLACK);
  epd.fillCircle(w/4-2, y2-h+8, 1, EPD_BLACK);  
  epd.fillCircle(w/4-2, y2-h+12, 1, EPD_BLACK);  
  epd.fillCircle(w/4-2, y2-h+16, 1, EPD_BLACK);  

  // follow the axes or height or negative amp bars
  int yamplabel = max(y2-xaxisoffset-battpx+(battpx<0 ? h:0), y2-xaxisoffset)+h;
  drawBattery(250-3*w/4-2, yamplabel, EPD_BLACK);
  drawSolPanel(250-w/4-2,  yamplabel, EPD_BLACK);


  // net amp-hour summary bar, with duration
  long mn = 2147483647;
  long mx = -2147483648;
  long mah = 0;
  long actualcount = 0;
  for(int i=0; i<DATA_COUNT; i++) {
    long timevalue = readings[i].utc.epoch; //datatime[i].epoch;
    if(timevalue!=0) { //valid value
      mah += (long)readings[i].batterymA;  //graphactualmah[i];
      actualcount++;
      if(timevalue > mx)
        mx = timevalue;
      if(timevalue < mn)
        mn = timevalue;
    }
  }
  epd.fillRect(0,10,240,12,EPD_BLACK);
  epd.setTextColor(EPD_INVERSE);
  epd.setCursor(64,12);
  if(DEBUG_UI) { 
    Serial.println(F("Timespan(sec)="));
    Serial.println(mx);
    Serial.println(mn);
    Serial.println(mx-mn);
    Serial.println(F("mah="));
    Serial.println(mah);
    Serial.println(F("Above is value shown on graph"));
  }
  epd.print(format0_0((mx-mn)*mah/(actualcount*3600),3) + " Ah (graph)");

  epd.setCursor(165,12);
  long netAh = getUptimeNetBatteryAh();
  if(DEBUG_UI) { 
    Serial.println(format0_0(netAh,3));
    Serial.println(F("Above is ongoing aggregate Ah"));
  }
  epd.print("/ " + format0_0(netAh,3) + " Ah");

  if(totalAhDatetime.epoch!=0) {
    long timespan = rtcclock().epoch - totalAhDatetime.epoch;
    epd.setCursor(3,12);
    epd.print(formatToDHMS(timespan) + ",");
  }


  drawBatterystateofcharge(x1+3,y1-1, 35+17,13+3);
  

  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("end of addToGraph()/E-ink UI, Free memory="));
      Serial.println(freeMemory());    
  }

  epdDisplayDeferredDelay();
}
void refreshGraph() {
  //get size of labels
  const String string = "90.000";
  int16_t x=0;
  int16_t y=0;
  int16_t l=0;
  int16_t t=0;
  uint16_t w=0;
  uint16_t h=0;
  epd.getTextBounds(string2char(string), 0, 0, &l, &t, &w, &h);
  if(DEBUG_UI) {
    Serial.print(string);
    Serial.print(F(" size= ("));
    Serial.print(w);
    Serial.print(F("px x "));
    Serial.print(h);
    Serial.println(F("px)"));
  }

  unsigned int latest = (graphdatahead+DATA_COUNT-1) % DATA_COUNT;
  struct downloadable_history data = readings[latest];

  // Draw date
  epd.clearBuffer();
  epd.setCursor(0, 0);
  epd.setTextColor(EPD_BLACK);
  String timstr = formattedlocal(data.utc.epoch!=0? data.utc : rtcclock());
  epd.print(timstr);
  timstr.~String();

  //draw copy marker
  epd.drawRect(235,0,10,10,EPD_BLACK);
  epd.drawRect(239,4,10,10,EPD_INVERSE);

  
  // convert hourly log of battery charge/discharge to graph px
  int hourlyAmpGraph[DATA_COUNT];
  for(int i=0; i<DATA_COUNT;i++)
    hourlyAmpGraph[i] = 6*hourlyAmpSum[readings[graphdatahead+i % DATA_COUNT].utc.hour].converted/1000;


  // calculate size and position of graph
  unsigned int x1 = w+2;
  unsigned int y1 = 24;
  unsigned int w1 = 250-x1*2;
  unsigned int h1 = 122-36;
  unsigned int x2 = x1+w1;
  unsigned int y2 = y1+h1;
  int xaxisoffset = 20;

  struct graphproperties pasthourlygraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, hourlyAmpGraph, graphdatahead, 24, steps6, drawAvgA);
  
  struct graphproperties voltgraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, batteryvoltpxgraph, graphdatahead, DATA_COUNT, bigdot, offsetBlip);
  struct graphproperties solarVgraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, solarvoltpxgraph, graphdatahead, DATA_COUNT, bigdash, offsetSun);
  
  struct graphproperties ampgraph  = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, batteryamppxgraph,  graphdatahead, DATA_COUNT, column, drawRightTick);
  struct graphproperties solarAgraph = drawGraph(x1, y1, w1, h1, SHOW_XY_AXIS, xaxisoffset, solaramppxgraph, graphdatahead, DATA_COUNT, dotted, drawNothing);



  // create (2) x-axis tick labels of time
  String labelX1 = formatHHMM(readings[graphdatahead].utc);
  if(labelX1.length()!=0) {
    epd.setCursor(x1, y2+3);
    epd.print(labelX1);
    uparrow(x1+4, y2-2);
  } else {
    epd.setCursor(x1, y2+3);
    epd.print(String(DATA_COUNT * MIN_PER_GRAPH_PX) + "min");
    uparrow(x1+4, y2-2);
  }

  unsigned int middle = (graphdatahead+DATA_COUNT/2)%DATA_COUNT;
  String labelX2 = formatHHMM(readings[middle].utc);
  if(labelX2.length()!=0) {
    epd.setCursor(x1+DATA_COUNT/2, y2+3);
    epd.print(labelX2);
    uparrow(x1+DATA_COUNT/2, y2-2);
  } else {
    epd.setCursor(x1+DATA_COUNT/2, y2+3);
    epd.print(String(DATA_COUNT * MIN_PER_GRAPH_PX/2) + "min");
    uparrow(x1+DATA_COUNT/2, y2-2);
  }

  epd.setCursor(x1+DATA_COUNT-10, y2+3);
  epd.print("now");
  uparrow(x1+DATA_COUNT, y2-2);


  //show bar graph of current power status, with labels and tick markers
  //...unit labels
  epd.setCursor(250-w, y1);
  epd.print("amps"); // amperes.converter->units);
  epd.setCursor(0, y1);
  epd.print("volts"); // voltage.converter->units);


  // bar charts of latest
  int battpx = batteryamppxgraph[latest]; // 6px/amp 
  int moonpx = batteryvoltpxgraph[latest]; // 2px/v 
  int panelpx = solaramppxgraph[latest]; // 2px/v 
  int solpx = solarvoltpxgraph[latest]; // 2px/v 
  if(data.utc.epoch==0){
    battpx = 0;
    moonpx = 0;
    panelpx = 0;
    solpx = 0;   
  }
  
  //...now data labels and bars
  //        x-axis, y-axis,         +Y range        -Y range      -widthofbar, n/a  x, y-data,   label on bar,                                     label width, label height
  bool usvertleft = moonpx < solpx;
  bool usvertright = battpx < panelpx;
  bargraph2(250-w,  y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, battpx,   format0_0(data.batterymA,3),                    w,           h,          usvertright);
  bargraph2(0,      y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, moonpx,    format0_0(data.batterymV,3),                   w,           h,          usvertleft);
  
  bargraph2(250-w/2,y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, panelpx,  usvertright ? format0_0(data.solarmA,3) : "",  w,           h,          usvertright);
  bargraph2(w/2,    y2-xaxisoffset, h1-xaxisoffset, -xaxisoffset, -w/2+1,      0,   0, solpx,  usvertleft ? format0_0(data.solarmV,3) : "",     w,           h,          usvertleft);

  drawBlip(w/4-2,y2-h, EPD_BLACK);
  drawSun(3*w/4-2,  y2-h, EPD_BLACK);
  epd.fillRect(3*w/4-3, y2-h+8-1, 3,3, EPD_BLACK);
  epd.fillRect(3*w/4-3, y2-h+12-1, 3,3, EPD_BLACK);
  epd.fillRect(3*w/4-3, y2-h+16-1, 3,3, EPD_BLACK);
  epd.fillCircle(w/4-2, y2-h+8, 1, EPD_BLACK);  
  epd.fillCircle(w/4-2, y2-h+12, 1, EPD_BLACK);  
  epd.fillCircle(w/4-2, y2-h+16, 1, EPD_BLACK);  

  // follow the axes or height or negative amp bars
  int yamplabel = max(y2-xaxisoffset-battpx+(battpx<0 ? h:0), y2-xaxisoffset)+h;
  drawBattery(250-3*w/4-2, yamplabel, EPD_BLACK);
  drawSolPanel(250-w/4-2,  yamplabel, EPD_BLACK);


  // net amp-hour summary bar, with duration
  long mn = 2147483647;
  long mx = -2147483648;
  long mah = 0;
  long actualcount = 0;
  for(int i=0; i<DATA_COUNT; i++) {
    long timevalue = readings[i].utc.epoch; //datatime[i].epoch;
    if(timevalue!=0) { //valid value
      mah += (long)readings[i].batterymA;  //graphactualmah[i];
      actualcount++;
      if(timevalue > mx)
        mx = timevalue;
      if(timevalue < mn)
        mn = timevalue;
    }
  }
  epd.fillRect(0,10,240,12,EPD_BLACK);
  epd.setTextColor(EPD_INVERSE);
  epd.setCursor(64,12);
  if(DEBUG_UI) { 
    Serial.println(F("Timespan(sec)="));
    Serial.println(mx);
    Serial.println(mn);
    Serial.println(mx-mn);
    Serial.println(F("mah="));
    Serial.println(mah);
    Serial.println(F("Above is value shown on graph"));
  }
  epd.print(format0_0((mx-mn)*mah/(actualcount*3600),3) + " Ah (graph)");

  epd.setCursor(165,12);
  long netAh = getUptimeNetBatteryAh();
  if(DEBUG_UI) { 
    Serial.println(format0_0(netAh,3));
    Serial.println(F("Above is ongoing aggregate Ah"));
  }
  epd.print("/ " + format0_0(netAh,3) + " Ah");

  if(totalAhDatetime.epoch!=0) {
    long timespan = rtcclock().epoch - totalAhDatetime.epoch;
    epd.setCursor(3,12);
    epd.print(formatToDHMS(timespan) + ",");
  }


  drawBatterystateofcharge(x1+3,y1-1, 35+17,13+3);

  if(!isWifiConnected()) {
    epd.drawLine(0,0,250,122,EPD_BLACK);
    epd.drawLine(0,122,250,0,EPD_BLACK);
  }

  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("end of addToGraph()/E-ink UI, Free memory="));
      Serial.println(freeMemory());    
  }

  epdDisplayDeferredDelay();
}
void uparrow(int x, int y) {
  epd.drawLine(x, y, x+2, y+2, EPD_BLACK);
  epd.drawLine(x, y, x-2, y+2, EPD_BLACK);
  epd.drawLine(x-2, y+2, x+2, y+2, EPD_BLACK);
  //epd.drawPixel(x, y, EPD_BLACK);
  //epd.drawPixel(x, y+1, EPD_BLACK);
  //epd.drawPixel(x-1, y+1, EPD_BLACK);
  //epd.drawPixel(x+1, y+1, EPD_BLACK);
}

//void column(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1)
void bargraph(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1) {
  int pboundsY = (int)zeroY - rangePY;
  int nboundsY = (int)zeroY - rangeNY;
  unsigned int width = x1-x0-1;
  int x = (int)zeroX+x1;
  int y = (int)zeroY-y1-1;
  if(y<pboundsY) y=pboundsY;
  if(y>nboundsY) y=nboundsY;
  int dy =  y - (int)zeroY;


  hdashRect(x, zeroY, width, dy+1, EPD_BLACK);

  /*
  epd.drawCircle(zeroX,      zeroY, y, EPD_BLACK);
  epd.drawCircle(zeroX,      zeroY, rangePY, EPD_BLACK);
  epd.drawCircle(zeroX,      zeroY, -rangeNY, EPD_BLACK);
  epd.fillCircle(zeroX,      pboundsY, 2, EPD_BLACK);
  epd.fillCircle(zeroX,      nboundsY, 2, EPD_BLACK);
  epd.drawCircle(x,      y, 4, EPD_BLACK);
  Serial.println("data is at ");
  Serial.println(x1);
  Serial.println(y);*/
}
void bargraph2(unsigned int zeroX, unsigned int zeroY, int rangePY, int rangeNY, int x0, int y0, int x1, int y1, String label, unsigned int w, unsigned int h, bool vert) {
  if(label.length()!=0) {
    rangePY -= h;
    rangeNY += h;  
    bargraph(zeroX, zeroY, rangePY, rangeNY, x0, y0, x1, y1);
    if(!vert) {
      int pboundsY = (int)zeroY - (int)rangePY;
      int nboundsY = (int)zeroY - (int)rangeNY;
      int x = (int)zeroX+x1;
      int y = (int)zeroY-y1;
      if(y<pboundsY) y=pboundsY;
      if(y>nboundsY) y=nboundsY;
      if(y1>=0){  //uses the unrounded value, to keep negative sign
        epd.fillRect(x-1, y-h-2, w+2,h+2, EPD_BLACK);
        epd.fillRect(x-1, y-h-2, w+2,h+2, EPD_INVERSE);
        epd.setCursor(x, y-h-1);
      }else{
        epd.fillRect(x-1, y+2, w+2,h+2, EPD_BLACK);
        epd.fillRect(x-1, y+2, w+2,h+2, EPD_INVERSE);
        epd.setCursor(x,y+3);
      }
      epd.print(label);
    } else {
      int16_t xf=0;
      int16_t yf=0;
      uint16_t wf=0;
      uint16_t hf=0;
      epd.getTextBounds(string2char(label), 0, 0, &xf, &xf, &wf, &hf);
      
      #define LANDSCAPE_NORMAL 0
      #define LANDSCAPE_UPSIDEDOWN 2
      #define PORTRAIT_LEFT_OF_NORMAL 3
      #define PORTRAIT_RIGHT_OF_NORMAL 1
      epd.setRotation(PORTRAIT_LEFT_OF_NORMAL);
      int xr = 122-zeroY+8;
      int yr = zeroX;
      epd.fillRect(xr, yr, wf+2, hf+2, EPD_BLACK);
      epd.fillRect(xr, yr, wf+2, hf+2, EPD_INVERSE);
      epd.setCursor(xr+1, yr+1);
      epd.print(label);
      
      epd.setRotation(LANDSCAPE_NORMAL);
    }
  }
  else
    bargraph(zeroX, zeroY, rangePY, rangeNY, x0, y0, x1, y1);
}

long getUptimeNetBatteryAh() {
  long timespan = rtcclock().epoch - totalAhDatetime.epoch;
  if(DEBUG_FUNCTIONS) { 
    Serial.println(F("Timespan(sec)="));
    Serial.println(timespan);
    Serial.println(F("total since Ah="));
    Serial.println(format0_0(totalAh,3));
    Serial.println(F("+"));
    Serial.println(format0_0(nowAh,3));
    Serial.println(F("*(("));
    Serial.println(millis());
    Serial.println(F("-"));
    Serial.println(nowAhMillis);
    Serial.println(F(")/1000)*"));
  }
  long nowspan = (millis()-nowAhMillis)/1000;
  if(DEBUG_FUNCTIONS) { 
    Serial.println(String(nowspan) + "/(3600"); // the initial nowspan will never hit an hour, but the 3600000 never changes.  nowspan=40min left in hour, it will never be an hour.  therefore the average will never be for an hour, but partial hour.
    Serial.println(F("x"));
    Serial.println(nowAhCount);
    Serial.println(F(")="));
    Serial.println(format0_0(nowAhCount==0 ? 0 : totalAh + nowAh*nowspan/(nowAhCount*3600),3));
    Serial.println(F("Above is ongoing aggregate Ah"));
  }
  long ah = nowAhCount==0 ? 0 : totalAh + nowAh*nowspan/(nowAhCount*3600);
  return ah;
}

void column(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1)
{
  int x = (int)zeroX+x1+2;
  if(y1>rangePY)
    epd.drawFastVLine(x, zeroY-rangePY, rangePY+2, EPD_INVERSE);
  else if(y1<rangeNY)
    epd.drawFastVLine(x, zeroY+2, -rangeNY-2, EPD_INVERSE);
  else if(y1>1)
    epd.drawFastVLine(x, zeroY-2, -y1+1, EPD_INVERSE);
  else if(y1<-1) 
    epd.drawFastVLine(x, zeroY+2, (-y1)-1, EPD_INVERSE);

  if(y1>rangePY && y1<rangeNY && abs(y1)>1)
    epd.drawPixel(x, zeroY-y1, EPD_BLACK);

  if(DEBUG_UI) 
    Serial.println("data is col=" + String(x1) +", h=" + String(y1));
}
void line(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1)
{
  int yy0 = y0;
  int yy1 = y1;
  if(y1>rangePY)
    yy1 = rangePY;
  if(y0>rangePY)
    yy0 = rangePY;

  epd.drawLine(zeroX+x0+2, zeroY-yy0, zeroX+x1+2, zeroY-yy1, EPD_BLACK);

  if(DEBUG_UI) 
    Serial.println("data is col=" + String(x1) +", h=" + String(y1));
}
void dotted(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1)
{
  int yy1 = y1;
  if(y1>rangePY)
    yy1 = rangePY;
  else if(y1<rangeNY)
    yy1 = rangeNY;

  if((x1&1)==0)
    epd.drawPixel(zeroX+x1+2, zeroY-yy1, EPD_BLACK);  

  int yy0 = y0;
  if(y0>rangePY)
    yy0 = rangePY;
  else if(y0<rangeNY)
    yy0 = rangeNY;

  if(abs(y0-y1)>3)
    dottedbresenham(zeroX+x0+2, zeroY-yy0, zeroX+x1+2, zeroY-yy1, EPD_BLACK);

  if(DEBUG_UI) 
    Serial.println("data is dotted=" + String(x1) +", h=" + String(y1));
}
void bigdot(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1)
{
  int yy1 = y1;
  if(y1>rangePY)
    yy1 = rangePY;
  else if(y1<rangeNY)
    yy1 = rangeNY;

  if((x1&3)==0)
    epd.fillCircle(zeroX+x1+2, zeroY-yy1, 1, EPD_BLACK);  

  int yy0 = y0;
  if(y0>rangePY)
    yy0 = rangePY;
  else if(y0<rangeNY)
    yy0 = rangeNY;

  if(abs(y0-y1)>3)
    circlebresenham(zeroX+x0+2, zeroY-yy0, zeroX+x1+2, zeroY-yy1, EPD_BLACK);
   
  if(DEBUG_UI) 
    Serial.println("data is dotted=" + String(x1) +", h=" + String(y1));
}
void bigdash(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1)
{
  int yy1 = y1;
  if(y1>rangePY)
    yy1 = rangePY;
  else if(y1<rangeNY)
    yy1 = rangeNY;

  if((x1&3)==0)
    epd.fillRect(zeroX+x1+2-1, zeroY-yy1-1, 3,3, EPD_BLACK);  

  int yy0 = y0;
  if(y0>rangePY)
    yy0 = rangePY;
  else if(y0<rangeNY)
    yy0 = rangeNY;

  if(abs(y0-y1)>3)
    sqrbresenham(zeroX+x0+2, zeroY-yy0, zeroX+x1+2, zeroY-yy1, EPD_BLACK);
   
  if(DEBUG_UI) 
    Serial.println("data is dotted=" + String(x1) +", h=" + String(y1));
}

void steps6(const unsigned int zeroX, const unsigned int zeroY, const int rangePY, const int rangeNY, const int x0, const int y0, const int x1, const int y1)
{
  //breakhere
  //int step = 6;
  //String labelX1 = formatHHMM(datatime[graphdatahead]);
  //String labelX2 = formatHHMM(datatime[(graphdatahead+DATA_COUNT)]);
  
  int xx1 = zeroX+x1+2;
  int yy1 = y1;
  if(y1>rangePY) {
      yy1 = zeroY-rangePY;
      epd.drawFastHLine(xx1, yy1, 6, EPD_BLACK);
      epd.drawFastHLine(xx1, yy1-1, 6, EPD_BLACK);
  } else if(y1<rangeNY) {
      yy1 = zeroY-rangeNY;
      epd.drawFastHLine(xx1, yy1, 6, EPD_BLACK);
      epd.drawFastHLine(xx1, yy1+1, 6, EPD_BLACK);
  } else {
    yy1 = zeroY-yy1;
    epd.drawFastHLine(xx1, yy1, 6, EPD_BLACK);
    //epd.drawRect(zeroX+x1*6+2, zeroY-yy1, 6, yy1, EPD_BLACK);
  }
  
  int xx0 = xx1-1;
  int yy0 = zeroY-y0;
  if(y0>rangePY) {
      yy0 = zeroY-rangePY;
      epd.drawLine(xx1, yy1, xx0, yy0, EPD_BLACK);
  } else if(y0<rangeNY) {
      yy0 = zeroY-rangeNY;
      epd.drawLine(xx1, yy1, xx0, yy0, EPD_BLACK);
  } else 
    epd.drawLine(xx1, yy1, xx0, yy0, EPD_BLACK);
    
  if(DEBUG_UI) 
    Serial.println("data is 6px wide steps=" + String(x1) +", h=" + String(y1));
}

//250x122
struct graphproperties drawGraph(unsigned int x, unsigned int y, unsigned int w, unsigned int h, byte hasborder, int xaxisoffset, int data[], byte datastart, unsigned int len, DataPoint pt, AxesTick tick) {
  if(hasborder==1){
    epd.drawFastVLine(x, y, h, EPD_BLACK);
    epd.drawFastHLine(x, y+h-xaxisoffset, w, EPD_BLACK);
  }
  if(hasborder==2){
    epd.drawFastVLine(x+w, y, h, EPD_BLACK);
    epd.drawFastHLine(x, y, w, EPD_BLACK);
    epd.drawFastHLine(x, y+h, w, EPD_BLACK);
  }

  if(DEBUG_UI) Serial.println("drawgraph() datapoints=" + String(len));
  
  unsigned int zeroY = y+h - xaxisoffset;
  int rangeY1 = h-2-xaxisoffset;
  int rangeY2 = -xaxisoffset;  
  if(DEBUG_UI) {
    Serial.println("Zero is y=" + String(zeroY));
    Serial.println("Top range of Y=" + String(rangeY1));
    Serial.println("Bottom range of Y=" + String(rangeY2));
  }
  int prev = data[datastart];
  for (unsigned int i=0; i < len; i++) {
    unsigned int start = (i+datastart)%len;
    if(DEBUG_UI) Serial.print(start);
    int hh = data[start];
    
    pt(x, zeroY, rangeY1, rangeY2, i-1, prev, i, hh);

    if(DEBUG_UI) {
      Serial.print(F("] graphing..."));
      Serial.print(i);
      Serial.print(F(","));
      Serial.println(hh);
      Serial.println(F("--------"));
    }
    prev = hh;
  }
  //epd.drawFastVLine(, top, rangeY1, EPD_BLACK);
  if(len=DATA_COUNT) // stupid hack b/c I don't want to add a flag for adding this, which is stupid for the 24hr graph
  if(rangeY1>=prev && rangeY2<=prev)
    //epd.fillCircle(x + len+2, (int)zeroY-prev, 2, EPD_BLACK); // put a point on the circle 
    tick(x + len+2, (int)zeroY-prev, EPD_BLACK);
  
  //epd.display();
  //delay(15000); //always delay 15seconds after display.  It's not worth the risk to the epaper.
  if(DEBUG_UI) Serial.println(F("graphed"));
  
  //struct graphproperties {
  //    unsigned int zeroX;
  //    unsigned int zeroY;
      
  //    unsigned int rangePlusY;
  //    unsigned int rangeNegY;
  //    unsigned int rangePlusX;
  //    unsigned int rangeNegX;
  
  //    unsigned int lastDataX;
  //    unsigned int lastDataY;
  //};
  struct graphproperties properties = {x+2, y+h-xaxisoffset, rangeY1, rangeY2, 0, (int)w-2, x+len-2, zeroY-prev };

  return properties;
}

void offsetSun(const int x, const int y, int color)
{
  drawSun(x+4, y, color);
}
void offsetBlip(const int x, const int y, int color)
{
  drawBlip(x+4, y, color);
}
void drawBlip(const int x, const int y, int color)
{
  epd.fillCircle(x, y, 2, color);
}
void drawAvgA(const int x, const int y, int color)
{
  epd.drawFastHLine(x, y, 12, color);
  drawRightArrow(x+14, y, color);
  
  int x1 = x+18;
  int y1 = y-2;
  epd.drawFastHLine(x1-1, y1-2, 7, color);
  epd.setTextColor(color);
  epd.setCursor(x1, y1);
  epd.print("A");
  epd.setCursor(x1+1, y1);
  epd.print("A");
}
void drawRightArrow(const int x, const int y, int color)
{
  epd.drawLine(x+2, y, x-2, y-2, color);
  epd.drawLine(x+2, y, x-2, y+2, color);
  epd.drawFastVLine(x-2, y-2, 5, color);
}
void drawLeftArrow(const int x, const int y, int color)
{
  epd.drawLine(x-2, y, x+2, y-2, color);
  epd.drawLine(x-2, y, x+2, y+2, color);
  epd.drawFastVLine(x+2, y-2, 5, color);
}
void drawSun(const int x, const int y, int color)
{
  epd.drawCircle(x, y, 2, color);

  epd.drawFastVLine(x,y+4, 2, color);
  epd.drawFastVLine(x,y-4, -1, color);
  epd.drawFastHLine(x+4, y, 2, color);
  epd.drawFastHLine(x-4, y, -1, color);

  epd.drawLine(x+3, y+3, x+4, y+4, color);
  epd.drawLine(x+3, y-3, x+4, y-4, color);
  epd.drawLine(x-3, y-3, x-4, y-4, color);
  epd.drawLine(x-3, y+3, x-4, y+4, color);
}
void drawBattery(const int x, const int y, int color)
{
  epd.fillRect(x-6, y-3, 13,8, color);
  epd.drawPixel(x-7, y-2, color);
  epd.drawPixel(x+7, y-2, color);
  
  epd.drawPixel(x-5, y-4, color);
  epd.drawPixel(x-4, y-4, color);
  epd.drawPixel(x+5, y-4, color);
  epd.drawPixel(x+4, y-4, color);
  epd.drawPixel(x-5, y-5, color);
  epd.drawPixel(x-4, y-5, color);
  epd.drawPixel(x+5, y-5, color);
  epd.drawPixel(x+4, y-5, color);

  epd.drawPixel(x-2, y, EPD_INVERSE);
  epd.drawPixel(x-2, y+2, EPD_INVERSE);
  epd.drawFastHLine(x-3, y+1, 3, EPD_INVERSE);
  epd.drawFastHLine(x+1, y+1, 3, EPD_INVERSE);
}
void drawSolPanel(const int x, const int y, int color)
{
  epd.drawCircle(x, y, 2, color);

  epd.drawFastVLine(x,y+4, 2, color);
  epd.drawFastVLine(x,y-4, -1, color);
  epd.drawFastHLine(x+4, y, 2, color);
  epd.drawFastHLine(x-4, y, -1, color);

  epd.drawLine(x+3, y+3, x+4, y+4, color);
  epd.drawLine(x+3, y-3, x+4, y-4, color);
  epd.drawLine(x-3, y-3, x-4, y-4, color);
  epd.drawLine(x-3, y+3, x-4, y+4, color);

  epd.fillRect(x-5, y+2, 11, 5, color);
  epd.fillRect(x-5, y+2, 11, 5, EPD_INVERSE);
  //epd.drawRect(x-5, y+2, 11, 5, color);
  epd.drawFastHLine(x-4, y+2, 9, color);
  epd.drawFastHLine(x-5, y+6, 11, color);
  epd.drawLine(x-4, y+2,x-5, y+6,color);
  epd.drawLine(x+4, y+2,x+5, y+6,color);
  //epd.drawFastHLine(x-5, y, 11, EPD_INVERSE);
  //epd.drawFastHLine(x-5, y+4, 11, EPD_INVERSE);
  epd.drawFastVLine(x, y+2, 4, color);
  //epd.drawFastVLine(x, y+2, 5, EPD_INVERSE);
}
void drawHist(const int x, const int y, int color)
{
  epd.drawCircle(x, y, 5, color);

  epd.drawPixel(x-5, y, EPD_INVERSE);
  epd.drawPixel(x-5, y-1, EPD_INVERSE);
  epd.drawPixel(x-5, y-2, EPD_INVERSE);
  epd.drawPixel(x-4, y-3, EPD_INVERSE);
  
  epd.drawPixel(x-4, y-3, color);
  epd.drawPixel(x-4, y-4, color);
  epd.drawPixel(x-4, y-5, color);
  epd.drawPixel(x-4, y-6, color);
  
  epd.drawPixel(x-4, y-3, color);
  epd.drawPixel(x-3, y-3, color);
  epd.drawPixel(x-2, y-3, color);
  epd.drawPixel(x-1, y-3, color);

  epd.drawPixel(x, y, color);
  epd.drawPixel(x, y+1, color);
  epd.drawPixel(x, y+2, color);
  epd.drawPixel(x-1, y, color);
  epd.drawPixel(x-2, y, color);
  epd.drawPixel(x-3, y, color);

  //epd.setTextColor(color);
  //epd.setCursor(x-2, y-4);
  //epd.print("A");
}
void drawSunA(const int x, const int y, int color)
{
  epd.setTextColor(color);
  epd.setCursor(x-2, y-4);
  epd.print("a");

  epd.drawFastVLine(x,y+4, 2, color);
  epd.drawFastVLine(x,y-4, -1, color);
  epd.drawFastHLine(x+4, y, 2, color);
  epd.drawFastHLine(x-4, y, -1, color);

  epd.drawLine(x+3, y+3, x+4, y+4, color);
  epd.drawLine(x+3, y-3, x+4, y-4, color);
  epd.drawLine(x-3, y-3, x-4, y-4, color);
  epd.drawLine(x-3, y+3, x-4, y+4, color);
}
void drawSunV(const int x, const int y, int color)
{
  epd.setTextColor(color);
  epd.setCursor(x-2, y-4);
  epd.print("v");

  epd.drawFastVLine(x,y+4, 2, color);
  epd.drawFastVLine(x,y-4, -1, color);
  epd.drawFastHLine(x+4, y, 2, color);
  epd.drawFastHLine(x-4, y, -1, color);

  epd.drawLine(x+3, y+3, x+4, y+4, color);
  epd.drawLine(x+3, y-3, x+4, y-4, color);
  epd.drawLine(x-3, y-3, x-4, y-4, color);
  epd.drawLine(x-3, y+3, x-4, y+4, color);
}
void drawRightTick(const int x, const int y, int color)
{
  for(int i=x; i<230; i++)
    if((i&1)==0) {
      epd.drawPixel(i,y,color);
    }
}
void drawNothing(const int x, const int y, int color) { }

// function for line generation 
//https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm
void dottedbresenham(int x0, int y0, int x1, int y1, int color) 
{ 
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;

  byte count=0;
  for(;;){
    //if((count++ & 1)==0)
    //  epd.drawPixel(x0,y0,color);
    if(((int)sqrt(x0*x0+y0*y0) & 1)==0)
      epd.drawPixel(x0,y0,color);
      
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
} 
void circlebresenham(int x0, int y0, int x1, int y1, int color) 
{ 
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;

  byte count=0;
  for(;;){
    //if((count++ & 1)==0)
    //  epd.drawPixel(x0,y0,color);
    if(((int)sqrt(x0*x0+y0*y0) & 3)==0)
      epd.fillCircle(x0,y0,1,color);
      
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
} 
void sqrbresenham(int x0, int y0, int x1, int y1, int color) 
{ 
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;

  byte count=0;
  for(;;){
    //if((count++ & 1)==0)
    //  epd.drawPixel(x0,y0,color);
    if(((int)sqrt(x0*x0+y0*y0) & 3)==0)
      epd.fillRect(x0-1,y0-1,3,3,color);
      
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
} 

void drawBatterystateofcharge(int x,int y, int w,int h) {
  epd.fillRect(x,y,w,h,EPD_BLACK);
  epd.fillRect(x,y,w,h,EPD_INVERSE);
  epd.drawFastHLine(x+1,y+1,  w-2,EPD_BLACK);
  epd.drawFastHLine(x+1,y+h-2,w-2,EPD_BLACK);
  epd.drawFastVLine(x+1,y+1,h-2,EPD_BLACK);

  //lostCapacityAh = 0;
  //  dischargedAh = stateofcharge * batteryCapacityAh/100;
  
  long trueCapacityAh = batteryCapacityAh - lostCapacityAh;
  long availCapacityAh = trueCapacityAh+dischargedAh;

  unsigned int xx = x+3;
  unsigned int yy = y+3;
  unsigned int hh = h-6;
  unsigned int ww = (abs(w)-3) * availCapacityAh/(trueCapacityAh);
//  Serial.println("=====battery symbol====");
//  Serial.print("batteryCapacityAh=");
//  Serial.println(batteryCapacityAh);
//  Serial.print("lostCapacityAh=");
//  Serial.println(lostCapacityAh);
//  Serial.print("trueCapacityAh=");
//  Serial.println(trueCapacityAh);
//  Serial.print("dischargedAh=");
//  Serial.println(dischargedAh);
//  Serial.print("ww=");
//  Serial.println(ww);
//  Serial.print("/w=");
//  Serial.println(w);
  epd.fillRect(xx,yy,ww,hh,EPD_BLACK);
  
  epd.setCursor(x+4,y+4);
  epd.setTextColor(EPD_INVERSE);
  epd.print(String(availCapacityAh/1000) + "/" + String(trueCapacityAh/1000));
  epd.setTextColor(EPD_BLACK);
}



//y = 0.1898x2 - 3.8741x + 19.627
//dod = f(voltage)
int estimateStateOfCharge(int voltage, int batteryamperage) {
  int soc = 0;

  int capacity = batteryCapacityAh;
  int dischargeRate = 1000*batteryamperage / capacity;

  // +/- (C/100) or (1% of capacity)/hr
  if(dischargeRate >= -10 && dischargeRate<10 && voltage>11200) {
    float estatrest = (1.9D*sq((float)voltage)/100000.0D) - 0.3874D * ((float)voltage) + 1962.7D;
    soc = (int)(estatrest > 100.0 ? 100.0D : estatrest);
    return soc;
  }


  if(dischargeRate < 0) {
      //https://forums.sailboatowners.com/threads/battery-voltage-vs-state-of-charge.176854/
      //      100%  90%   80%   70.0% 60.0% 50.0% 40.0% 30.0% 20.0% 10.0%
      //discharge 
      //rate 
      //as 
      //% 
      //of 
      //capacity 
      //measured 
      //voltage                  
      //0.01  12.7  12.7  12.65 12.55 12.5  12.4  12.25 12.1  11.95 11.7
      //0.05  12.6  12.6  12.55 12.45 12.3  12.2  12.1  11.9  11.7  11.5
      //0.1   12.5  12.4  12.3  12.15 12    11.85 11.7  11.5  11.25 11
      //0.2   12.1  12    11.9  11.6  11.5  11.4  11.2  10.95 10.6  10.25
      //0.333 11.75 11.7  11.6  11.3  11.25 11    10.75 10.4  10    9.5
      int discharge01[10] = {12700,  12700,  12650,  12550, 12500,  12400,  12250,  12100,  11950, 11700};
      int discharge05[10] = {12600,  12600,  12550,  12450, 12300,  12200,  12100,  11900,  11700, 11500};
      int discharge10[10] = {12500,  12400,  12300,  12150, 12000,  11850,  11700,  11500,  11250, 11000};
      int discharge20[10] = {12100,  12000,  11900,  11600, 11500,  11400,  11200,  10950,  10600, 10250};
      int discharge33[10] = {11750,  11700,  11600,  11300, 11250,  11000,  10750,  10400,  10000, 9500};
      int *a;
      int *b;
      long m=0;
      long n=0;
      long discharge[10];
    
      //      100%  90%   80%   70.0% 60.0% 50.0% 40.0% 30.0% 20.0% 10.0%
      //discharge rate as % of capacity measured voltage                  
      if(dischargeRate>-0010) {
        a = &discharge01[0];
        b = &discharge01[0];
        m=100;
        n=0;
      }
      else if(dischargeRate>-0050) {
        a = &discharge01[0];
        b = &discharge05[0];
        n = (50 - dischargeRate) * 100 / (50-10);
        m = (dischargeRate - 10)* 100 / (50-10);
      }
      else if(dischargeRate>-0100) {
        a = &discharge05[0];
        b = &discharge10[0];
        n = (100 - dischargeRate) * 100 / (100-50);
        m = (dischargeRate - 50)* 100 / (100-50);
      }
      else if(dischargeRate>-0200) {
        a = &discharge10[0];
        b = &discharge20[0];
        n = (200 - dischargeRate) * 100 / (200-100);
        m = (dischargeRate - 100)* 100 / (200-100);
      }
      else if(dischargeRate>-0333) {
        a = &discharge20[0];
        b = &discharge33[0];
        n = (333 - dischargeRate) * 100 / (333-200);
        m = (dischargeRate - 200)* 100 / (333-200);
    
      } else {
        a = &discharge33[0];
        b = &discharge33[0];
        m=100;
        n=0;
      }
      for(int i=0; i<10; i++)
        discharge[i] = (m*(long)a[i] + n*(long)b[i])/100;
      if(voltage>discharge[0])
        soc=100;
      else if(voltage<discharge[9])
        soc = 0;
      else
        for(int i=0; i<9; i++)
          if(voltage <= discharge[i] && voltage>=discharge[i+1]) {
            long difference = discharge[i] -discharge[i+1];
            long result = ((10-i)*10L*(discharge[i]-voltage) + (9-i)*10L*(voltage-discharge[i+1]))/difference;
//Serial.print(",voltage under charge=");
//Serial.print(voltage);
//Serial.print(" between =");
//Serial.print(discharge[i]);
//Serial.print(" and ");
//Serial.print(discharge[i+1]);
//Serial.print(" puts it at ");
//Serial.print((10-i)*10);
//Serial.print("% and ");
//Serial.print((9-i)*10);
//Serial.print("%,");

//Serial.print("weighted avg A=");
//Serial.print(discharge[i]-voltage);
//Serial.print("weighted avg B=");
//Serial.print(voltage-discharge[i+1]);
//Serial.print("/");
//Serial.print(difference);
//Serial.print(" ");
            soc = (int)result;
            break;
          }
       return soc;
  } else  if(dischargeRate > 0) {
      //https://forums.sailboatowners.com/threads/battery-voltage-vs-state-of-charge.176854/
      // charge rate  100%  90.0% 80.0% 70.0% 60.0% 50.0% 40.0% 30.0% 20.0% 10.0%
      // 0.2          14    14    14    13.7  13.5  13.4  13.2  13    12.75 12.6
      // 0.1          14    14    13.65 13.4  13.3  13.2  13.05 12.84 12.6  12.4
      // 0.05         14    13.55 13    13.2  13.1  13    12.9  12.68 12.4  12.1
      // 0.025        13.5  13.2  12.9  12.95 12.9  12.8  12.7  12.5  12.25 11.8

      // Note: the high amperage into battery during low state of charge numbers don't seem correct.  It seems too high for a 20% battery receiving a 15amp/(C/5) charge, to read a 12.4v.  But that is what the table says.
      // WHILE: the high amperage into battery during low state of charge numbers ARE IRRELEVANT BC THEY ARE IMPOSSIBLE.  10Amp charge into a 90% charged battery is impossible.
      // BUT it's not worth fixing right now, bc the chances of that occurring during initialization, is miniscule
      int charge20[10] = { 14000, 14000, 14000, 13700, 13500, 13400, 13200, 13000, 12750, 12600 };
      int charge10[10] = { 14000, 14000, 13650, 13400, 13300, 13200, 13050, 12840, 12600, 12400 };
      int charge05[10] = { 14000, 13550, 13000, 13200, 13100, 13000, 12900, 12680, 12400, 12100 };
      int charge02[10] = { 13500, 13200, 12900, 12950, 12900, 12800, 12700, 12500, 12250, 11800 };
      int *a;
      int *b;
      int m=0;
      int n=0;
      int charge[10];
    
      //      100%  90%   80%   70.0% 60.0% 50.0% 40.0% 30.0% 20.0% 10.0%
      //discharge rate as % of capacity measured voltage                  
      if(dischargeRate<0025) {
        a = &charge02[0];
        b = &charge02[0];
        m=100;
        n=0;
      }
      else if(dischargeRate<0050) {
        a = &charge02[0];
        b = &charge05[0];
        n = (50 + dischargeRate) * 100 / (50-25);
        m = (dischargeRate + 025)* 100 / (50-25);
      }
      else if(dischargeRate<0100) {
        a = &charge05[0];
        b = &charge10[0];
        n = (100 + dischargeRate) * 100 / (100-50);
        m = (dischargeRate + 50)* 100 / (100-50);
      }
      else if(dischargeRate<0200) {
        a = &charge10[0];
        b = &charge20[0];
        n = (200 + dischargeRate) * 100 / (200-100);
        m = (dischargeRate+- 100)* 100 / (200-100);
      } else {
        a = &charge20[0];
        b = &charge20[0];
        m=100;
        n=0;
      }
      for(int i=0; i<10; i++)
        charge[i] = (m*(long)a[i] + n*(long)b[i])/100;
      if(voltage>charge[0])
        soc=100;
      else if(voltage<charge[9])
        soc = charge[9]; // here we simply make the lowest 
      else
        for(int i=0; i<9; i++)
          if(voltage <= charge[i] && voltage>=charge[i+1]) {
            long difference = charge[i] - charge[i+1];
            long result = ((10-i)*10L*(charge[i]-voltage) + (9-i)*10L*(voltage-charge[i+1]))/difference;
//Serial.print(",voltage under charge=");
//Serial.print(voltage);
//Serial.print(" between =");
//Serial.print(charge[i]);
//Serial.print(" and ");
//Serial.print(charge[i+1]);
//Serial.print(" puts it at ");
//Serial.print((10-i)*10);
//Serial.print("% and ");
//Serial.print((9-i)*10);
//Serial.print("%,");

//Serial.print("weighted avg A=");
//Serial.print(charge[i]-voltage);
//Serial.print("weighted avg A=");
//Serial.print(voltage-charge[i+1]);
//Serial.print("/");
//Serial.print(difference);
//Serial.print(" ");
            soc = (int)result;
            break;
          }
       return soc;
  }

}
void updatestateofcharge(int ah, int milliamps, int millivolts) {
  if(isdischargeestimate){
    lostCapacityAh = 0;
    int stateofcharge = estimateStateOfCharge(millivolts, milliamps);
//    Serial.print("millivolts=");
//    Serial.println(millivolts);
//    Serial.print("milliamps=");
//    Serial.println(milliamps);
//    Serial.print("stateofcharge=");
//    Serial.println(stateofcharge);
    dischargedAh = (stateofcharge * batteryCapacityAh/100)-batteryCapacityAh;
    isdischargeestimate = false;
  } else {
    return; //disable for now
    
    if(millivolts>13000 && milliamps<1000) { 
      // if it's not charging at 13v, then it is full despite what the estimate says.  Adjust capacity.
      // if it's full
      lostCapacityAh = max(lostCapacityAh, max(0,-dischargedAh));
      dischargedAh = 0;
      Serial.println("battery is full");
    } else {
      dischargedAh+=ah;
      // I doubt the system will ever underestimate the depth of discharge, but if it does, just ignore it on the "fuel gauge" and it'll show true discharge once it starts to discharge.
      if(dischargedAh>0) {
        dischargedAh = 0;
        Serial.println("battery was less full than it thought previously it was, and it's re-adjusting it's point to start counting down from full");
      }
    }
  }
  Serial.print("Battry discharge is=");
  Serial.println(dischargedAh);
}

// linear regression, NOT draw line!
// https://stackoverflow.com/questions/5083465/fast-efficient-least-squares-fit-algorithm-in-c
// had to change to long array b/c the seconds in the regression can span 2 days, but int only can support 1/3 of a day
int linreg(int n, const long x[], const long y[], int* mN, int* mD, int* b, int* r){
    long   overflowlimit = sqrt(2147483647);
    long   sumx = 0.0;                      /* sum of x     */
    long   sumx2 = 0.0;                     /* sum of x**2  */
    long   sumxy = 0.0;                     /* sum of x * y */
    long   sumy = 0.0;                      /* sum of y     */
    long   sumy2 = 0.0;                     /* sum of y**2  */

    int count = 0;
    int minN = 6;
    for (int i=0;i<n;i++){ 
        if(x[i]!=0 && y[i]!=0) { //all data fields have to be filled in... only for NTP, BY 11/18
          sumx  += x[i];       
          sumx2 += x[i]*x[i]; //sqr(x[i]);  
          sumxy += x[i] * y[i];
          sumy  += y[i];      
          sumy2 += y[i]*y[i]; //sqr(y[i]);

          //one of these is going to be multiplied by the other.  So it's going to overflow.  
          //fall back to floating point sums, and deal with some slowness and loss of precision
          if(sumx>overflowlimit || sumx>overflowlimit || sumx2>overflowlimit || sumxy>overflowlimit || sumy>overflowlimit)
            return linregF(n, x, y, mN, mD, b, r);

          count++;
        } 
    } 

    // there has to be at least 12 data items to do an regression
    if(count<minN)
      return count;
    n = count;

    long denom = (n * sumx2 - sumx*sumx);
    if (denom == 0) {
        // singular matrix. can't solve the problem.
        *mN = 0;
        *mD = 0;
        *b = 0;
        if (r) *r = 0;
            return 1;
    }

    // debugging?
    //Serial.println("linreg()");
    //Serial.println(n * sumxy );
    //Serial.println(sumx * sumy);
    //Serial.println(((long)n * sumxy  -  sumx * sumy));
    //Serial.println(denom);
    
    *mN = ((long)n * sumxy  -  sumx * sumy);
    *mD = denom;
    *b = (sumy * sumx2  -  sumx * sumxy) / denom;
    if (r!=NULL) {
        *r = 100*(sumxy - sumx * sumy / n) /    /* compute correlation coeff */
              sqrt((sumx2 - (sumx * sumx)/n) *
              (sumy2 - (sumy*sumy)/n));
    }

    if(DEBUG_FREE_MEMORY) {
        Serial.print(F("End of linereg(), Free memory="));
        Serial.println(freeMemory());    
    }

    return 0; 
}
int linregF(int n, const long x[], const long y[], int* mN, int* mD, int* b, int* r){
    double   sumx = 0.0;                      /* sum of x     */
    double   sumx2 = 0.0;                     /* sum of x**2  */
    double   sumxy = 0.0;                     /* sum of x * y */
    double   sumy = 0.0;                      /* sum of y     */
    double   sumy2 = 0.0;                     /* sum of y**2  */

    int orign = n;
    int count = 0;
    int minN = 6;
    for (int i=0;i<n;i++){ 
        if(x[i]!=0 && y[i]!=0) { //all data fields have to be filled in... only for NTP, BY 11/18
          sumx  += (double)x[i];       
          sumx2 += sq((double)x[i]); //sqr(x[i]);  
          sumxy += (double)x[i] * (double)y[i];
          sumy  += (double)y[i];      
          sumy2 += sq((double)y[i]); //sqr(y[i]);

          count++;
        } 
    } 

    // there has to be at least 12 data items to do an regression
    if(count<minN)
      return count;
    n = count;

    double denom = ((double)n * sumx2 - sumx*sumx);
    if (denom == 0) {
        // singular matrix. can't solve the problem.
        *mN = 0;
        *mD = 0;
        *b = 0;
        if (r) *r = 0;
            return 1;
    }

    // debugging?
    //Serial.println("linreg()");
    //Serial.println(n * sumxy );
    //Serial.println(sumx * sumy);
    //Serial.println(((long)n * sumxy  -  sumx * sumy));
    //Serial.println(denom);
    double m = ((double)n * sumxy  -  sumx * sumy)/denom;
    double multiple = min(32000/m,32000);
    *mN = (int)(m*multiple); //make the frsction have as much significant digits as possible to fit in int
    *mD =   (int)multiple;
    double intercept = (sumy * sumx2  -  sumx * sumxy) / denom;
    *b = (int)intercept;
    if (r!=NULL) {
        double err = 100*(sumxy - sumx * sumy / n) /    /* compute correlation coeff */
              sqrt((sumx2 - (sumx * sumx)/n) *
              (sumy2 - (sumy*sumy)/n));
        *r = (int)err;
    }

    if(DEBUG_FREE_MEMORY) {
        Serial.print(F("End of linereg(), Free memory="));
        Serial.println(freeMemory());    
    }

    return 0; 
}



//https://arduino.stackexchange.com/questions/19795/how-to-read-bitmap-image-on-arduino
// any mapping to 32bit data structures advances 5 bytes.  and doesn't even copy correctly.
struct bmp_file_header_t {
  uint16_t signature;
  uint16_t file_size1;
  uint16_t file_size2;
  uint16_t reserved[2];
  uint16_t image_offset1;
  uint16_t image_offset2;
};

struct bmp_image_header_t {
  uint16_t header_size1;
  uint16_t header_size2;
  uint16_t image_width1;
  uint16_t image_width2;
  uint16_t image_height1;
  uint16_t image_height2;
  
  uint16_t color_planes;
  uint16_t bits_per_pixel;
  uint16_t compression_method1;
  uint16_t compression_method2;
  uint16_t image_size1;
  uint16_t image_size2; 
  uint32_t horizontal_resolution; //shouldn't map correctly
  uint32_t vertical_resolution; //shouldn't map correctly
  uint32_t colors_in_palette; //shouldn't map correctly
  uint32_t important_colors; //shouldn't map correctly
};

// BMP compress
#define BI_RGB            0
#define BI_RLE8           1
#define BI_RLE4           2
#define BI_BITFIELDS      3
#define BI_JPEG           4
#define BI_PNG            5
#define BI_ALPHABITFIELDS 6
#define BI_CMYK           11
#define BI_CMYKRLE8       12
#define BI_CMYKRLE4       13

//https://raw.githubusercontent.com/adafruit/Adafruit_Learning_System_Guides/master/EInk_Autostereograms/images/adafruit.bmp
int drawBmpOnEPD(int left, int top, char* databuffer, unsigned int start, unsigned int count){
  //  bmpImage = SD.open("Circle.bmp", FILE_READ);
  //...
  // Read the file header

  // this is correct, to map to struct : Serial.println((unsigned long)&databuffer[start]);
  // this is not : Serial.println((unsigned long)&databuffer);
  
  bmp_file_header_t fileHeader;
  //fileHeader= (bmp_file_header_t*)&databuffer[start];
  //fileHeader = (bmp_file_header_t*)&databuffer; //c0ff
  memcpy(&fileHeader, databuffer+start, sizeof(fileHeader));

  if(fileHeader.signature == 0x4D42) { // || fileHeader.signature == 0x424D){

    //bmpImage.read(&fileHeader, sizeof(fileHeader));
    //...
    // Check signature
    //...
    // Read the image header
    
    bmp_image_header_t imageHeader; // = (bmp_image_header_t*)(&databuffer + start + sizeof(fileHeader));
    memcpy(&imageHeader, databuffer + start + sizeof(fileHeader), sizeof(imageHeader));
    
    //bmpImage.read(&imageHeader, sizeof(imageHeader));
    //...
    // Check image size and format
    //...
    // Locate the pixels
    //bmpImage.seek(fileHeader.image_offset);

    unsigned long pixels = (unsigned long)start + (unsigned long)fileHeader.image_offset1 + ((unsigned long)fileHeader.image_offset2 << 16);
    unsigned long w = (unsigned long)imageHeader.image_width1 + ((unsigned long)imageHeader.image_width2 << 16);
    unsigned long h = (unsigned long)imageHeader.image_height1 + ((unsigned long)imageHeader.image_height2 << 16);
    unsigned int bpp = (unsigned long)imageHeader.bits_per_pixel;
    unsigned long compression = (unsigned long)imageHeader.compression_method1 + ((unsigned long)imageHeader.compression_method2 << 16);
    unsigned long imagesize = (unsigned long)imageHeader.image_size1 + ((unsigned long)imageHeader.image_size2 << 16);
    int fit = max(w/250, h/122)+1;
    if(DEBUG_BMP) {
      Serial.println("image header width 2 bytes + height bytes as unsigned");
      Serial.println(imageHeader.image_width1);
      Serial.println(imageHeader.image_width2);
      Serial.println(imageHeader.image_height1);
      Serial.println(imageHeader.image_height2);
      Serial.println("BMP size =" + String(w) + "x" + String(h));
      Serial.println("bpp=" + String(bpp));
      Serial.println("compression code=" + String(compression));
      Serial.println("Image Size=" + String(imagesize));
      Serial.println("Resize factor to fit on display=1/" + String(fit));
    }

//    bool endreached = false;
    int x=0;
    int y=0;
    if(bpp==1 && compression==BI_RGB) {
      // w=36*8;
      if(DEBUG_BMP) Serial.println(F("1BPP Monochrome, No compression"));
      for(int j=0;j<count;j++ ) {
        byte color = databuffer[pixels++];
        byte mask=128;
        if(color!=0) {
          
          for(int k=0; k<8; k++) {
            if((color & mask) !=0) {
              epd.drawPixel(x/fit+left,h/fit+1-y/fit+top,EPD_BLACK); // bmp's are stored upside down... that's why h-...
            }
            x++;
            mask = mask >> 1;
          }
          //Serial.print("1");
          //epd.drawPixel(x,y,EPD_BLACK);
          //x++;
        } else {
          x+=8;
        }
        if(x>=w) {
          if((x/8)%4!=0)
            pixels += (4-((x/8)%4));
          x=0;
          y++;
          
        }
        if(y>=h)
          break;
      }
    }
    else if(bpp==1 && compression==BI_RLE8) {
      if(DEBUG_BMP) Serial.println(F("Run length encoding detected"));
      
      for(int j=start;j<count;j++ ) {
        byte instr = databuffer[pixels++];
        if(instr==0){
          byte instr2 = databuffer[pixels++];
          if(instr==1) { // end of file
            if(DEBUG_BMP) Serial.println(F("end of file"));
            break;
          } else if(instr==0) {  // end of line
            if(DEBUG_BMP) Serial.println(F("new line"));
            x=0;
            y++;
          } else if(instr==2) { // next 2 bytes indicate delta coordinate change
            byte dx = databuffer[pixels++];
            byte dy = databuffer[pixels++];  
            if(DEBUG_BMP) Serial.println("delta " + String(x) + " " + String(y));
            x+=dx;
            y+=dy;
          } else { // instr 3 to 255, means next few bytees copy exactly
            if(DEBUG_BMP) Serial.println("Copy next " + String(instr2) + " to " + String(x) + " and " + String(x+instr2-1));
            for(unsigned int i=0; i<instr2; i++) {
              byte color = databuffer[pixels++];  
              if(color!=0)
                epd.drawPixel(x++,y,EPD_BLACK);
            }
          }
        } else {
          byte color = databuffer[pixels++];
          if(DEBUG_BMP) Serial.println("Copy color " + String(color) + " to " + String(x) + " and " + String(x+instr-1));
          if(color!=0)
            epd.drawFastHLine(x,y,instr, EPD_BLACK);
          x+=instr;
        }
        //  int padding = pixels%4; //https://en.wikipedia.org/wiki/BMP_file_format#Pixel_array_(bitmap_data)
      }
      return 0;
    } else {
      if(DEBUG_BMP) Serial.println(F("Does not currently support compression, or BPP"));
      return -1;
    }
  }
  else {
    if(DEBUG_BMP) Serial.println(F("Does not recognize as BMP file.  Wrong sig bytes"));
    return -2;
  }
}




// Fake NTP server, primarily to sychronize Cameras on network to one time.
// Either they will go directly to Pi GPS for NTP,
// or this arduino will get time from NTP on Pi GPS, 
// and synchronize it's clocks and account for drift
// but cameras will connect with the this less accurate arduino but planned to be more available
/*
https://docstore.mik.ua/orelly/networking/firewall/ch08_13.htm
NTP is a UDP -based service. 
NTP servers use well-known port 123 to talk to each other and to NTP clients. 
NTP clients use random ports above 1023. 
As with DNS , you can tell the difference between: 
- An NTP client-to-server query - source port above 1023, destination port 123. 
- NTP server-to-client response - source port 123, destination port above 1023. 
- NTP server-to-server query or response - source and destination ports both 123. 
*/
// http://lloydm.net/Demos/NTP/NTP-GPS_Server.ino.txt
// https://docstore.mik.ua/orelly/networking/firewall/ch08_13.htm

////////////////////////////////////////
#define LISTENING_NTP_PORT 123

// An Ethernet UDP instance 
WiFiUDP Udp123;

void startServerNTP() {
  Udp123.begin(LISTENING_NTP_PORT);
}


String byte1_ = "byte1="; 
String byte2_ = "byte2="; 
String byte3_ = "byte3="; 
String byte4_ = "byte4="; 
void handleNTPOn123() {
  if(DEBUG_NTPS) Serial.println(Checking_if_NTP_request_received_on_port_123___);
  
  // if there's data available, read a packet
  int packetSize = Udp123.parsePacket();
  if(packetSize)
  {
    Udp123.read(packetBuffer2,NTP_PACKET_SIZE);
    IPAddress Remote = Udp123.remoteIP();
    int PortNum = Udp123.remotePort();
    Udp123.flush();

    if(DEBUG_NTPS) {
      Serial.println();
      Serial.print(Received_UDP_packet_size_);
      Serial.println(packetSize);
      Serial.print(From_);
    
      for (int i =0; i < 4; i++)
      {
        Serial.print(Remote[i], DEC);
        if (i < 3)
        {
          Serial.print('.');
        }
      }
      Serial.print(__port_);
      Serial.println(PortNum);
    }

    byte LIVNMODE = packetBuffer2[0];
    if(DEBUG_NTPS) Serial.print(LI_Vers_Mode_);
    if(DEBUG_NTPS) Serial.println(packetBuffer2[0],HEX);

    byte STRATUM = packetBuffer2[1];
    if(DEBUG_NTPS) Serial.print(Stratum_);
    if(DEBUG_NTPS) Serial.println(packetBuffer2[1],HEX);

    byte POLLING = packetBuffer2[2];
    if(DEBUG_NTPS) Serial.print(Polling_);
    if(DEBUG_NTPS) Serial.println(packetBuffer2[2],HEX);

    byte PRECISION = packetBuffer2[3];
    if(DEBUG_NTPS) {
      Serial.print(Precision_);
      Serial.println(packetBuffer2[3],HEX);

      //full dump
      Serial.println(packet_dump_);
      for (int z = 0; z < NTPS_PACKET_SIZE; z++) {
        Serial.print(packetBuffer2[z],HEX);
        if (((z+1) % 4) == 0) {
          Serial.println();
        }
      }
      Serial.println();
    }

    packetBuffer2[0] = 0b00100100;   // LI, Version, Mode
    packetBuffer2[1] = 1 ;   // stratum
    packetBuffer2[2] = 6 ;   // polling minimum
    packetBuffer2[3] = 0xFA; // precision

    packetBuffer2[7] = 0; // root delay
    packetBuffer2[8] = 0;
    packetBuffer2[9] = 8;
    packetBuffer2[10] = 0;

    packetBuffer2[11] = 0; // root dispersion
    packetBuffer2[12] = 0;
    packetBuffer2[13] = 0xC;
    packetBuffer2[14] = 0;

    //gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
// commented by bob to compile temporarily    crack(sUTD, sUTC);

// int year;
// byte month, day, hour, minute, second, hundredths;
// unsigned long date, time, age;
// uint32_t tempval;
// unsigned long    timestamp = numberOfSecondsSince1900Epoch(year,month,day,hour,minute,second);

    struct utcdatetime now = rtcclock();
    //unsigned long timestamp = numberOfSecondsSince1900Epoch(now.year,now.month,now.day,now.hour,now.minute,now.second);
    unsigned long epoch = now.epoch;
    long drift = getdrift(); //a little inefficient, but beats re-calculating the elapsed rtc sec since last sync
    if(drift!=0)
      epoch = (unsigned long)((long)epoch + drift);
      
    if(DEBUG_NTPS) {
      Serial.print(Local_time_is_);
      String timestr = formattedlocal(now);
      Serial.println(timestr); 
      timestr.~String();
      Serial.print(UTC_Time_in_seconds_); 
      Serial.println(now.epoch); 
      Serial.print(Drift_of_clock_calculated_is_); 
      Serial.println(drift); 
      Serial.print(UTC_Time_sent_); 
      Serial.println(epoch); 
    }

    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    //unsigned long epoch = secsSince1900 - seventyYears;
    unsigned long timestamp = epoch + seventyYears;
    unsigned long tempval = timestamp;
    byte byt1 = (tempval >> 24) & 0XFF;
    byte byt2 = (tempval >> 16) & 0xFF;
    byte byt3 = (tempval >> 8) & 0xFF;
    byte byt4 = (tempval) & 0xFF;

    if(DEBUG_NTPS) {
      Serial.print(Time_sent_is_supposed_to_be_);
      Serial.println(timestamp); 
      Serial.print(byte1_); 
      Serial.println(byt1,HEX); 
      Serial.print(byte1_); 
      Serial.println(byt2,HEX); 
      Serial.print(byte1_); 
      Serial.println(byt3,HEX); 
      Serial.print(byte1_); 
      Serial.println(byt4,HEX); 
    }

    packetBuffer2[12] = 71; //"G";
    packetBuffer2[13] = 80; //"P";
    packetBuffer2[14] = 83; //"S";
    packetBuffer2[15] = 0;  //"0";

    // reference timestamp
    packetBuffer2[16] = byt1; //(tempval >> 24) & 0XFF;
    //tempval = timestamp;
    packetBuffer2[17] = byt2; //(tempval >> 16) & 0xFF;
    //tempval = timestamp;
    packetBuffer2[18] = byt3; //(tempval >> 8) & 0xFF;
    //tempval = timestamp;
    packetBuffer2[19] = byt4; //(tempval) & 0xFF;

    packetBuffer2[20] = 0;
    packetBuffer2[21] = 0;
    packetBuffer2[22] = 0;
    packetBuffer2[23] = 0;


    //copy originate timestamp from incoming UDP transmit timestamp
    packetBuffer2[24] = packetBuffer2[40];
    packetBuffer2[25] = packetBuffer2[41];
    packetBuffer2[26] = packetBuffer2[42];
    packetBuffer2[27] = packetBuffer2[43];
    packetBuffer2[28] = packetBuffer2[44];
    packetBuffer2[29] = packetBuffer2[45];
    packetBuffer2[30] = packetBuffer2[46];
    packetBuffer2[31] = packetBuffer2[47];

    //receive timestamp
    packetBuffer2[32] = byt1; //(tempval >> 24) & 0XFF;
    //tempval = timestamp;
    packetBuffer2[33] = byt2; //(tempval >> 16) & 0xFF;
    //tempval = timestamp;
    packetBuffer2[34] = byt3; //(tempval >> 8) & 0xFF;
    //tempval = timestamp;
    packetBuffer2[35] = byt4; //(tempval) & 0xFF;

    packetBuffer2[36] = 0;
    packetBuffer2[37] = 0;
    packetBuffer2[38] = 0;
    packetBuffer2[39] = 0;

    //transmitt timestamp
    packetBuffer2[40] = byt1; //(tempval >> 24) & 0XFF;
    //tempval = timestamp;
    packetBuffer2[41] = byt2; //(tempval >> 16) & 0xFF;
    //tempval = timestamp;
    packetBuffer2[42] = byt3; //(tempval >> 8) & 0xFF;
    //tempval = timestamp;
    packetBuffer2[43] = byt4; //(tempval) & 0xFF;

    packetBuffer2[44] = 0;
    packetBuffer2[45] = 0;
    packetBuffer2[46] = 0;
    packetBuffer2[47] = 0;


    // Reply to the IP address and port that sent the NTP request

    Udp123.beginPacket(Remote, PortNum);
    Udp123.write(packetBuffer2,NTP_PACKET_SIZE);
    Udp123.endPacket();

    // store for website referece
    lastremoteip[0] = Remote[0];
    lastremoteip[1] = Remote[1];
    lastremoteip[2] = Remote[2];
    lastremoteip[3] = Remote[3];
    lastremoteepoch = epoch;
    ntpscount++;
  }
  else if(DEBUG_NTPS) 
    Serial.println(No_NTP_request_received);
}

const uint8_t daysInMonth [] PROGMEM = { 
  31,28,31,30,31,30,31,31,30,31,30,31 }; //const or compiler complains

const unsigned long seventyYears = 2208988800UL; // to convert unix time to epoch

// NTP since 1900/01/01
unsigned long int numberOfSecondsSince1900Epoch(uint16_t y, uint8_t m, uint8_t d, uint8_t h, uint8_t mm, uint8_t s) {
  int leapAdjustment = 0;
  if (y >= 1970) {
    y -= 1970;
    leapAdjustment = 2;                         // LM: 1970 was NOT a leap year!
  }

  uint16_t days = d;
  
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
/*
  if (m > 2 && y % 4 == 0)
    ++days;
*/
  if (m > 2 && (y + leapAdjustment) % 4 == 0)   // LM: Weak but okay for the present
    ++days;
    
  days += 365 * y + (y + 3) / 4 - 1;
  return days*24L*3600L + h*3600L + mm*60L + s + seventyYears;
}



bool startBluetooth() {
  if (DEBUG_BLE)  Serial.println(F("Trying Start Bluetooth LE!"));
    
  // begin initialization
  if (BLE.begin()) {
    if (DEBUG_BLE)  Serial.println(F("Bluetooth LE detected!"));
  
    // set advertised local name and service UUID:
    BLE.setLocalName("Van Battery Status");
    BLE.setAdvertisedService(btService);
  
    // add the characteristic to the service
    btService.addCharacteristic(systemVoltageCharacteristic);
    btService.addCharacteristic(solarVoltageCharacteristic);
    btService.addCharacteristic(batterymAmpsCharacteristic);
    btService.addCharacteristic(solarmAmpsCharacteristic);
    btService.addCharacteristic(batteryNetmAhCharacteristic);
    btService.addCharacteristic(batteryTopDischargemAhCharacteristic);
  
    // add service
    BLE.addService(btService);
  
    // start advertising
    BLE.advertise();
  
    // BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central  
    if (DEBUG_BLE) Serial.println(F("BLE startup finished"));
  } else if (DEBUG_BLE) {
    Serial.println(F("starting BLE failed!"));
  }
  
  if(DEBUG_FREE_MEMORY) {
      Serial.print(F("end of startBluetooth(), Free memory="));
      Serial.println(freeMemory());    
  }
}

bool handleBluetoothInput() {
    // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    if (DEBUG_BLE) {
      Serial.print(F("Connected to central: "));
      Serial.println(central.address());
    }
    // while the central is still connected to peripheral:
    if (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      //if (switchCharacteristic.written()) {
      //  switchCharacteristic.value();
      //}
      if(DEBUG_FREE_MEMORY) {
        Serial.print(F("end of startBluetooth(), Free memory="));
        Serial.println(freeMemory());    
      }
    }
    return true;
  }
  else
    return false;
}

bool updateBluetoothService() {
    // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    if (DEBUG_BLE) {
      Serial.print(F("There is a central connected:"));
      Serial.println(central.address());
    }
    // while the central is still connected to peripheral:
    if (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      
      systemVoltageCharacteristic.writeValue(0);
      solarVoltageCharacteristic.writeValue(0);
      batterymAmpsCharacteristic.writeValue(0);
      solarmAmpsCharacteristic.writeValue(0);
      batteryNetmAhCharacteristic.writeValue(0);
      batteryTopDischargemAhCharacteristic.writeValue(0);


      if(DEBUG_FREE_MEMORY) {
          Serial.print(F("end of updateBluetoothService(), Free memory="));
          Serial.println(freeMemory());    
      }
    }
    return true;
  }
  else
    return false;
}
