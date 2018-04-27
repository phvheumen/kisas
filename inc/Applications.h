#include <application.h>
#include <HttpClient.h>
#include "math.h"

class ApplicationManager; //forward declare

class StandardAppFunctions
{
public:
    StandardAppFunctions();
    virtual ~StandardAppFunctions();
    TCPClient client;

    //bool sendMessagePack(String Message);
    //HttpClient http;
    // Headers currently need to be set at init, useful for API keys etc.

    //char HTTPHeaderAuthValue[]="Basic user:password0user:password4567890user:password4567890user:password4567890user:password4567890user:password4567890user:password4567890123";
    //char HTTPHeaderAuthValue[143];
    //http_header_t Headers[] = {{"Content-Type","application/json"},{"Authorization",(const char *)&HTTPHeaderAuthValue}};
    //http_header_t Headerss[] = {{"Content-Type","application/json"},{"Authorization",{"BASIC"}};

    //1:63fxV5dU9BIi5feyFdPYVjRaTS2kRp5D
    //MTo2M2Z4VjVkVTlCSWk1ZmV5RmRQWVZqUmFUUzJrUnA1RA==
    //char HTTPHeaderAuthValue[55];//="Basic MTo2M2Z4VjVkVTlCSWk1ZmV5RmRQWVZqUmFUUzJrUnA1RA==";
    //http_header_t headers[4];
    //http_header_t headers[] = {
    //    { "Content-Type", "application/json" },
    //    { "Accept" , "*/*"},
    //    { "Authorization" , (const char *)&HTTPHeaderAuthValue},
    //    { NULL, NULL } // NOTE: Always terminate headers will NULL
    //};
    //http_request_t request;
    //http_response_t response;

    // allow for interface destription language such as Rx, with different message formats:
    //0=JSON with abbreviated variable names.
    bool GeneralMessageFormat=0;
    //uint32_t DeviceIdentifier=277;
    unsigned long DeviceIdentifier=277;
    int PostHttp(String body);
    HttpClient http;
    http_request_t request;
    http_response_t response;
    //char HTTPHeaderAuthValue[55]="Basic MTo2M2Z4VjVkVTlCSWk1ZmV5RmRQWVZqUmFUUzJrUnA1RA==";

private:

};

//Time-averaging Application
class TmAverage : public StandardAppFunctions
{
  public:
  TmAverage(); //constructor
  //TmAverage(ApplicationManager* AppManPtr); //constructor
  virtual ~TmAverage(); //destructor
  void reset(void);
  void update(void);
  void addToVarList(uint16_t RegisterAdress);
  void removeFromVarList(uint16_t RegisterAdress);
  void SendMessage(void);
  void parseSetting(float * SettingsArr, int Length);
 // void init(ApplicationManager& AppManPtr);
  void init();
  Timer updateTimer;

  private:

  Timer sendTimer;
  uint16_t VarList[101]={0x43C3,0xE906,0};//{0}; //does this work or do we need to make a function out of this
      //Initialize the varlist to zero --> allready done in intializer
      // for(int i=0:i<size(varlist):i++)
      // {
      //     varlist(i)=0;
      // }
  uint32_t NumSamplesList[101];
  int32_t NewMeasurementList[101];
  bool InNummeratorList[101]={1,0,0};
  float ScalingList[101]={0.0031288697042764067,256000,0}; //{0.0001877321822565844,256000,0};
  float AverageList[101]={0,0,0};//;
  ApplicationManager* Manager;
  //uint8_t PresentPowerMode;
  //bool CommunicationMode;
};

//Simple Scope, using variables of Particle Cloud
//the scope is currently blokking (for 37 miliseconds)
//would be nice make it non-blocking
class SmplScope : public StandardAppFunctions
{
  public:
  SmplScope();  //constructor ////:varlist(){}
  virtual ~SmplScope(); //destructor
  void init(void);
  void update(void);
  void parseSetting(float * SettingsArr, int Length);
  //void timerstart();
  //void tmrCallback(void);
  //void onTimeout();
  //Timer timer(1000, SmplScope::update());
  //Timer timer(1000, &SmplScope::update);

  private:
  //uint8_t PresentPowerMode;
  Timer updateTimer;
  bool AppEnable;
  String ossil;
  unsigned long Dtime;
  unsigned long Curtime;
  unsigned long Newtime;
  float SampleBlock[1000];
  int StartIndex, StopIndex;
  //unsigned int Interval=1000;
};

//Application manager
class ApplicationManager
{
public:
//ApplicationManager& func (void)
//{
   // Some processing
//   return *this;
//}
  TmAverage TimeAverage;
  //SmplScope SimpleScope;


  void parseSetting(float * SettingsArr, int Length);

   // void test(void){Serial1.println("ApplicationManager.test>\t ok");}
    /* Reference to the calling object can be returned */

private:

  //Timer te(1000, &SmplScope::update, ApplicationManager.SimpleScope);
  /*void writeCommand(uint8_t cmd[], size_t cmdLen);
  bool readReply(uint8_t replyBuf[], size_t replyBufLen);
  uint8_t _mbSlaveId;
  uint8_t _i2cSlaveAddr;
  uint8_t _retryCount;*/
  //uint8_t PresentPowerMode;
  bool CommunicationMode;

};
