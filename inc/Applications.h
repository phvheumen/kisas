#include <application.h>
#include <HttpClient.h>
#include "math.h"

class ApplicationManager; //forward declare

class StandardAppFunctions
{
public:
    StandardAppFunctions();
    ~StandardAppFunctions();



    //TCPClient client;

    bool GeneralMessageFormat=0;
    unsigned long DeviceIdentifier=277;
    int PostHttp(String body);
    HttpClient http;
    http_request_t request;
    http_response_t response;

    //char HTTPHeaderAuthValue[55]="Basic MTo2M2Z4VjVkVTlCSWk1ZmV5RmRQWVZqUmFUUzJrUnA1RA==";
private:

};

/* ---- Time-averaging Application ---- */
class TmAverage : public StandardAppFunctions
{
  public:
  TmAverage();
  TmAverage(ApplicationManager* AppManPtr);

  ~TmAverage();

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
  uint16_t VarList[101]={0x43C3,0xE906,0};
  uint32_t NumSamplesList[101];
  int32_t NewMeasurementList[101];
  bool InNummeratorList[101]={1,0,0};
  float ScalingList[101]={0.0031288697042764067,256000,0};
  float AverageList[101]={0,0,0};//;
  ApplicationManager* Manager;

  uint8_t PresentPowerMode;
  bool CommunicationMode;
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

