#include <application.h>
#include <HttpClient.h>
#include "http_request.h"
#include "math.h"

/* Defines */
#define ENABLE_APP_MSG

/* MACROS */
/**
	The application message macro's should only be used inside the Application class and its children
	Define ENABLE_APP_MSG to enable messages from the applications.
 */
#ifdef ENABLE_APP_MSG
	#define APP_MSG(x) {\
	Serial.printf("[%8.3f] ", ((float) millis() )/1000);\
	Serial.printlnf(x);\
	}
#else
	#define APP_MSG(x)
#endif

#ifdef ENABLE_APP_MSG
	#define APP_MSG_FORMATTED(x, y) {\
	Serial.printf("[%8.3f] ", ((float) millis() )/1000);\
	Serial.printlnf(x, y);\
	}
#else
	#define APP_MSG_FORMATTED(x, y)
#endif

class ApplicationManager; // Forward declare

class Application
{
public:
	Application(unsigned int systemID, unsigned int applicationID, unsigned int messageFormatID,  ApplicationManager* AppManPtr);
    Application(String systemID, String applicationID, String messageFormatID, ApplicationManager* AppManPtr);
    Application(unsigned int systemID, unsigned int applicationID, unsigned int messageFormatID,  ApplicationManager* AppManPtr, HttpClient* httpClientService);
    Application(String systemID, String applicationID, String messageFormatID, ApplicationManager* AppManPtr,  HttpClient* httpClientService);


    // The following constructors will be removed in the near future
    Application();
    Application(String systemID, String applicationID, String messageFormatID);
    Application(unsigned int systemID, unsigned int applicationID, unsigned int messageFormatID);


    ~Application();

    void printApplicationInstance(void);

protected:
    String systemID;		// Unique identifier for each Particle/KISAS sensor
	String applicationID;	// Unique identifier for each application
	String messageFormatID; // Identify the message format. In this way the overhead of sending field names can be avoided

	ApplicationManager* appManager; // Pointer to application manager, useful for inter-application communication
	HttpClient* httpClientService;
private:

};

/* ---- Dummy Periodic Application ----*/
class PeriodicCall: public Application
{
public:
	PeriodicCall(ApplicationManager* _Manager);
	PeriodicCall(ApplicationManager* _Manager, HttpClient* _httpClientService);
	~PeriodicCall();

	void init(void);
	void run(void);
	void httpRequestCallback(HTTP_Request& r);
private:
	Timer periodicCallTimer;
};

/* ---- Time-averaging Application ---- */
class TmAverage : public Application
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
  ApplicationManager* Manager;



  Timer sendTimer;
  uint16_t VarList[101]={0x43C3,0xE906,0};
  uint32_t NumSamplesList[101];
  int32_t NewMeasurementList[101];
  bool InNummeratorList[101]={1,0,0};
  float ScalingList[101]={0.0031288697042764067,256000,0};
  float AverageList[101]={0,0,0};//;


};

//Simple Scope, using variables of Particle Cloud
//the scope is currently blokking (for 37 miliseconds)
//would be nice make it non-blocking
class SmplScope : public Application
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
	ApplicationManager();
	ApplicationManager(HttpClient* _httpClientService);
	~ApplicationManager();


//	TmAverage TimeAverage;
	PeriodicCall PerCall;
//	SmplScope SimpleScope;

	void parseSetting(float * SettingsArr, int Length);
private:

};

