#ifndef APPLICATIONS_H
#define APPLICATIONS_H

#include <application.h>
#include "math.h"


//Time-averaging Application
class TmAverage
{
  public:
  TmAverage(); //constructor
  virtual ~TmAverage(); //destructor
  void reset(void);
  void update(void);
  void addToVarList(uint16_t RegisterAdress);
  void removeFromVarList(uint16_t RegisterAdress);

  private:
  Timer timer;
  uint16_t VarList[101]={0}; //does this work or do we need to make a function out of this
      //Initialize the varlist to zero --> allready done in intializer
      // for(int i=0:i<size(varlist):i++)
      // {
      //     varlist(i)=0;
      // }
  uint32_t NumSamplesList[101];
  int32_t NewMeasurementList[101];
  float AverageList[101];
  //uint8_t PresentPowerMode;
  //bool CommunicationMode;
};

// Simple Scope, using variables of Particle Cloud
// the scope is currently blocking (for 37 miliseconds)
// would be nice make it non-blocking
class SmplScope
{
  public:
  SmplScope();  //constructor ////:varlist(){}
  virtual ~SmplScope(); //destructor
  void init(void);
  void update(void);
  //void timerstart();
  //void tmrCallback(void);
  //void onTimeout();
  //Timer timer(1000, SmplScope::update());
  //Timer timer(1000, &SmplScope::update);

  private:
  //uint8_t PresentPowerMode;
  Timer timer;
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
  TmAverage TimeAverage;
  SmplScope SimpleScope;

private:
//  Timer te(1000, &SmplScope::update, ApplicationManager.SimpleScope);
  /*void writeCommand(uint8_t cmd[], size_t cmdLen);
  bool readReply(uint8_t replyBuf[], size_t replyBufLen);
  uint8_t _mbSlaveId;
  uint8_t _i2cSlaveAddr;
  uint8_t _retryCount;*/
  //uint8_t PresentPowerMode;
  bool CommunicationMode;

};

#endif /* APPLICATIONS_H */
