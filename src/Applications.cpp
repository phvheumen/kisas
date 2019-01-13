#include "Applications.h" //ADE7880 Applications
#include "ADE7880.h"
#include "ADE7880RegisterNames.h"

#include "math.h"
#include <application.h>
#include <HttpClient.h>
#include <string.h>

//
// CLASS APPLICATION
//
Application::Application() {
	this->systemID = String(0);
	this->applicationID = String(0);
	this->messageFormatID = String(0);
	this->appManager = nullptr;
	this->httpClientService = nullptr;

	APP_MSG("Application> Warning: Constructor with no arguments called")
	this->printApplicationInstance();
}

Application::Application(String systemID, String applicationID, String messageFormatID) {
	this->systemID = systemID;
	this->applicationID = applicationID;
	this->messageFormatID = messageFormatID;
	this->appManager = nullptr;
	this->httpClientService = nullptr;

	this->printApplicationInstance();
}

Application::Application(unsigned int systemID, unsigned int applicationID, unsigned int messageFormatID) {
	this->systemID = String(systemID);
	this->applicationID = String(applicationID);
	this->messageFormatID = String(messageFormatID);
	this->appManager = nullptr;
	this->httpClientService = nullptr;

	this->printApplicationInstance();
}

Application::Application(unsigned int systemID, unsigned int applicationID, unsigned int messageFormatID,  ApplicationManager* AppManPtr) {
	this->systemID = String(systemID);
	this->applicationID = String(applicationID);
	this->messageFormatID = String(messageFormatID);
	this->appManager = AppManPtr;
	this->httpClientService = nullptr;

	this->printApplicationInstance();
}

Application::Application(String systemID, String applicationID, String messageFormatID, ApplicationManager* AppManPtr) {
	this->systemID = String(systemID);
	this->applicationID = String(applicationID);
	this->messageFormatID = String(messageFormatID);
	this->appManager = AppManPtr;
	this->httpClientService = nullptr;

	this->printApplicationInstance();
}

Application::Application(unsigned int systemID, unsigned int applicationID, unsigned int messageFormatID,  ApplicationManager* AppManPtr, HttpClient* httpClientService) {
	this->systemID = String(systemID);
	this->applicationID = String(applicationID);
	this->messageFormatID = String(messageFormatID);
	this->appManager = AppManPtr;
	this->httpClientService = httpClientService;

	this->printApplicationInstance();
}

Application::Application(String systemID, String applicationID, String messageFormatID, ApplicationManager* AppManPtr,  HttpClient* httpClientService) {
	this->systemID = String(systemID);
	this->applicationID = String(applicationID);
	this->messageFormatID = String(messageFormatID);
	this->appManager = AppManPtr;
	this->httpClientService = httpClientService;

	this->printApplicationInstance();
}

void Application::printApplicationInstance(void) {
	APP_MSG_FORMATTED("Application> systemID: \"%s\"", this->systemID.c_str());
	APP_MSG_FORMATTED("Application> applicationID: \"%s\"", this->applicationID.c_str());
	APP_MSG_FORMATTED("Application> messageFormatID: \"%s\"", this->messageFormatID.c_str());
}

Application::~Application() {

}

//
// CLASS APPLICATIONMANAGER
//
ApplicationManager::ApplicationManager() :
//		TimeAverage(this),
		PerCall(this)
{
	APP_MSG("ApplicationManger> Started");
}

ApplicationManager::ApplicationManager(HttpClient* _httpClientService) :
		PerCall(this, _httpClientService)
{

}

ApplicationManager::~ApplicationManager() {

}

//
// CLASS PERIODICCALL
//
PeriodicCall::PeriodicCall(ApplicationManager * _Manager):
	Application("Particle", "PeriodicCall", String(0), _Manager),
	periodicCallTimer(5000, &PeriodicCall::run, *this)
{

}

PeriodicCall::PeriodicCall(ApplicationManager * _Manager, HttpClient* _httpClientService) :
		PeriodicCall(_Manager)
{
		httpClientService = _httpClientService;
}

PeriodicCall::~PeriodicCall() {

}

void PeriodicCall::init(void) {
	this->periodicCallTimer.start();
}
void PeriodicCall::run(void) {
	APP_MSG("PeriodicCall::run> Entry");

	http_header_field_T header_fields[] = {
			{"User-Agent",  "Mozilla/5.0"},
			{nullptr, nullptr}
	};

	HTTP_Request http_req = HTTP_Request("requestbin.net", 80, HTTP_METHOD_POST, "/r/13amhyo1", header_fields, "Hello, world!");
//	http_req.addCallback(&PeriodicCall::httpRequestCallback, *this);
	httpClientService->pushRequest(http_req);

	APP_MSG("PeriodicCall::run> Exit");
}

void PeriodicCall::httpRequestCallback(HTTP_Request& r) {
	APP_MSG("PeriodicCall::httpRequestCallback> Entry");

	APP_MSG("PeriodicCall::httpRequestCallback> Exit");
}




extern uint8_t RegisterTypeArr[];
extern uint8_t giveRegType(uint16_t RegisterAdress);
extern String PostBuffer;

//-------------------------------------------------Time averageing-------------------------------------------

TmAverage::TmAverage(ApplicationManager * _Manager) :
		Application("Particle", "TmAverage", String(0)),
		updateTimer(1000, &TmAverage::update, *this),
		sendTimer(900000, &TmAverage::SendMessage, *this)
{
	Manager = _Manager;
}

TmAverage::~TmAverage() {

}


void TmAverage::init() {
	APP_MSG("TmAverage> Initialise")
	updateTimer.start();
	sendTimer.start();
}

void TmAverage::reset()
{
	memset(NumSamplesList, 0, sizeof(NumSamplesList));
	memset(AverageList, 0, sizeof(AverageList));
}

void TmAverage::SendMessage(void)
{
	APP_MSG("TmAverage> Send Message");
//    //extern http_request_t request;
//    Serial.print("TmAverage.SendMessage>");
//    //Serial.println(request.path);
//    //Serial.print(millis(),DEC);
//    //Serial.print(", ");
//
//    //prepare messagestructure
//    //{"Src":1234,"Ptp":2,"Pid":10,"Fmt":0,"DTm":23,"MV":[{"Adr":60415,"Val":4.12345},{"Adr":60398,"Val":3.12245}]}
//    int Source=1;
//	String MessageHeader="{\"Src\":"+String(Source)+",\"Ptp\":2,\"Pid\":10,\"Fmt\":0,\"DTm\":"+String(Time.now())+",\"MV\":[";
//	String MessageBodyVars="";
//	String MessageBody="";
//	unsigned int i=0;
//	for(;i<(sizeof(VarList)/sizeof(VarList[0]));i++)
//    {
//        //NumSamplesList[i]=0;
//        //AverageList[i]=0;
//        if (VarList[i]==0){
//            break;}
//        //Serial.print(i,DEC); Serial.print(" ");Serial.println(VarList[i],HEX);
//        //Serial.println(MessageBodyVars);
//        if (i !=0){
//            MessageBodyVars+=",";}
//            //Serial.println(MessageBodyVars);
//        //MessageBodyVars+=(unsigned char)0x82;
//        MessageBodyVars+="{\"Adr\":";//(unsigned char)0xCD; //int16 itentifier
//        //Serial.println(MessageBodyVars);
//        MessageBodyVars+=String(VarList[i]);
//        MessageBodyVars+=",\"Val\":";
//
//        //---------scale with a scaling list-------------------
//        float scaled=AverageList[i];
//        if (VarList[i]==0x43C3)
//        {
//             scaled = 2*((AverageList[i])/100000) *9.38660911282921877506715;
//             Serial.println(scaled,5);
//
//
//        }
//        if (VarList[i]==0xE906)
//        {
//             scaled = (256000.0/AverageList[i]);
//             Serial.println(scaled,5);
//             Serial.println(AverageList[i],5);
//             Serial.println(ScalingList[i],5);
//             Serial.println(InNummeratorList[i],5);
//        }
//
//        if(InNummeratorList[i])
//        {
//            scaled = AverageList[i]*ScalingList[i];
//        }
//        else
//        {
//            scaled = ScalingList[i]/AverageList[i];
//        }
//        Serial.println(scaled,5);
//
//         //---------end of scale with a scaling list-------------------
//
//
//        MessageBodyVars+=String(scaled);
//        MessageBodyVars+="}";
//
//        MessageBody+=MessageBodyVars;
//        MessageBodyVars="";
//        //MessageBodyValues+=(unsigned char)0xCA; //float32 itentifier
//        //unsigned char MeasuredValue[]={0,0,0,0};
//        //memcpy(MeasuredValue, (unsigned char*) (&AverageList[i]), 4);
//        //MessageBodyValues+=MeasuredValue[3];
//        //MessageBodyValues+=MeasuredValue[2];
//        //MessageBodyValues+=MeasuredValue[1];
//        //MessageBodyValues+=MeasuredValue[0];
//    }
//    Serial.print(MessageHeader);Serial.print(MessageBody);Serial.println("]}");
//    String total = MessageHeader+MessageBody+"]}";
//    Serial.println(total);
//    delay(100);
//    uint32_t freemem = System.freeMemory();
//Serial.print("free memory: ");
//Serial.println(freemem);
//
////updateTimer.stopFromISR();
//    //this->httpPostRequest(total);
//    PostBuffer = total;
////updateTimer.startFromISR();
//    //PostHttpp(MessageHeader+MessageBody+"]}","/Kisas/Post.php");
//
//    NumSamplesList[0]=0;
//    NumSamplesList[1]=0;
//    NumSamplesList[2]=0;
//
//    for(unsigned int i=0;i<sizeof(VarList);i++)
//    {
//        if (VarList[i]==0)
//        {
//            break;
//        }
//        NumSamplesList[i]=0;
//    }
}

void TmAverage::addToVarList(uint16_t RegisterAdress)
{
    //Find the place where nextvar is zero
    unsigned int i;
    for(i=0;i<sizeof(VarList);i++)
    {
        if (VarList[i]==0)
        {
            break;
        }
    }
    //check if varlist is not full (or incorrectly initialized), and add to the the list
    if (i != sizeof(VarList)-1)
    {
        VarList[i]=RegisterAdress;
        VarList[i+1]=0;
    }
}

void TmAverage::removeFromVarList(uint16_t RegisterAdress)
{
    //find index of registeradress to remove from the list
    unsigned int i;
    for(i=0;i<sizeof(VarList);i++)
    {
        if (VarList[i]==RegisterAdress)
        {
            break;
        }
    }
    if (i == sizeof(VarList))
    {
        //register not found, return without doing anything
        return;
    }
    else
    {
        //shift all trailing registers to fill the gap of the removed register
        for(;i<(sizeof(VarList)-1);i++)
        {

            VarList[i]=VarList[i+1];
            if (VarList[i]==0)
            {
                break;
            }
        }
    }
    return;
}

void TmAverage::update()
{
    //loop through the list and request the variables from he ADE7880;
    extern ADE7880 Measurement;
    Serial.printlnf("TmAverage.update> t = %.5fs", ( (float)millis() )/1000);

    for(unsigned int i=0; i < sizeof(VarList); i++)
    {
        if (VarList[i]==0) {
            break;
        }

        //VarList[i]
        // This method does not work yet for 32bit unsigned registers:
        //IPEAK, VPEAK, STATUS0, STATUS1, MASK0, MASK1, CHECKSUM, LAST_RWDATA32
        //(0xE500, 0xE501, 0xE502,0xE503,0xE50A, 0xE50B, 0xE51F, 0xE5FF).
        //Casting unsigned n to int preserves the value for 0 <= n <= (2^bit)-1 (e.g. 32767 for 16bit) and yields ZERO otherwise

        // TODO: Make equivalent of this deprecated function
//        int32_t tempvar = Measurement.SPIReadAutoRegsize(VarList[i],giveRegType(VarList[i]),0, &NewMeasurementList[i]);
        int32_t tempvar = 0; // Temporarely set to zero
        NewMeasurementList[i] = tempvar;

        Serial.printlnf("TmAverage.update> New measurement: 0x%04X -> %10d", VarList[i], NewMeasurementList[i]);

        //then recalculate all the averages and check if measurement went ok.
        if ( NewMeasurementList[i] != 0 )
        {
            Serial.print(AverageList[i],4);Serial.print(" ");
            //Serial.print(((NumSamplesList[i]-1)/NumSamplesList[i]),DEC);Serial.print(" ");
            //Serial.print((1/NumSamplesList[i]),DEC);Serial.print(" ");
            NumSamplesList[i]++;
            AverageList[i] = (((float)(NumSamplesList[i]-1))/((float)NumSamplesList[i])) * AverageList[i] + (1/((float)NumSamplesList[i]))*(NewMeasurementList[i]);
            Serial.print(NumSamplesList[i],DEC); Serial.print(" ");
            Serial.print(AverageList[i],4); Serial.print(" ");
        }
        else
        {
            Serial.println("TmAverage.update> Can not add empty measurement");
        }
    }
    Serial.println("");
}

void TmAverage::parseSetting(float * SettingsArr, int Length)
{
	//signed long SettingsArr[8]; // Src,Ptp,Pid,Fmt,Key,Value,DateTime
    //if(length < 7){return;} // not enough data received, unknown format.
    Serial.print("TmAverage.parseSetting ");
	Serial.print(SettingsArr[4],DEC);Serial.print(" ");
	Serial.print(SettingsArr[5],DEC);Serial.print(" ");
	Serial.println(SettingsArr[6],DEC);

	//in the format attribute, the action is stored (add, remove or )
	if(SettingsArr[4] == 0 && SettingsArr[5] == 0) // set setting, first check if settting is applied
	{
		unsigned int i;
		bool RegisterValueExists = false;
		//search if the variable is in the varlist
		for(i=0;i<sizeof(this->VarList);i++)
		{
			if (this->VarList[i] == (uint16_t) SettingsArr[6])
			{
				RegisterValueExists = true;
				break;
			}
			if (this->VarList[i]==0)
			{
				break;
			}
		}
		Serial.print(i,DEC);Serial.print(" ");
		Serial.println(RegisterValueExists,DEC);
		//if it did not break out of the varlist before it's size, the varlist is full
		if (i == sizeof(VarList))
		{
			Serial.println("TmAverage>\t Could not add to varlist, list is full.");
		} else if (RegisterValueExists == false) //there is space for an additional varaiable and it doesnot exist yet, add it to the list.
		{
			if (i != sizeof(this->VarList))
			{
				this->VarList[i] = (uint16_t) SettingsArr[6];
				this->VarList[i+1] = 0;
			}

		}

	} else if (SettingsArr[4] == 3 && SettingsArr[5] == 0) // remove settting
	{
		this->removeFromVarList((uint16_t) SettingsArr[6]);
	} else if(SettingsArr[4] == 2 && SettingsArr[5] == 0) // add settting
	{  // this is still without check if it actually is in the list, need to merge this with the other case (SettingsArr[4] == 0)

		this->addToVarList((uint16_t) SettingsArr[6]);
	} else if (SettingsArr[4] == 0 && SettingsArr[5] == 1)
	{
		//change ?
		// set this->timer to SettingsArr[6]
		this->updateTimer.changePeriod(SettingsArr[6]);
		Serial.println("updateTimer.changePeriod");
	} else if(SettingsArr[4] == 0 && SettingsArr[5] == 2)
	{
		// set this.sendTimer to SettingsArr[6]
		this->sendTimer.changePeriod(SettingsArr[6]);
			Serial.println("sendTimer.changePeriod");
	}
	else if((SettingsArr[4] == 0 || SettingsArr[4] == 1) && SettingsArr[5] == 3)
	{
		// set In Nummerator when scaling
		Serial.print("In Nummerator Set=0/Reset=1 ");
		Serial.println(SettingsArr[4],DEC);

		unsigned int i;
		for(i=0;i<sizeof(this->VarList);i++)
		{

			if (this->VarList[i] == (uint16_t) SettingsArr[6])
			{
			    Serial.print(i,DEC);Serial.print(" ");
			    Serial.print(InNummeratorList[i],DEC);Serial.print(" ");
				InNummeratorList[i] = !((bool)SettingsArr[4]);
				Serial.println(InNummeratorList[i],DEC);
				break;
			}
			if (this->VarList[i]==0)
			{
				break;
			}
		}

	}
	else if(SettingsArr[4] == 11)
	{
		// set Set ScalingFactor pair
		Serial.println("Set ScalingFactor pair");
		unsigned int i;
		for(i=0;i<sizeof(this->VarList);i++)
		{
			if (this->VarList[i] == (uint16_t) SettingsArr[5])
			{
			    Serial.println(SettingsArr[6],DEC);
			    Serial.println(((float)SettingsArr[6]),6);
				ScalingList[i] = ((float)SettingsArr[6]);
				break;
			}
			if (this->VarList[i]==0)
			{
				break;
			}
		}
	}

}

//--------------------------------------------------Simple Scope---------------------------------------------------


SmplScope::SmplScope() : updateTimer(1000, &SmplScope::update, *this) {
      Serial.println("SmplScope.SmplScope>\t constructor");
}

SmplScope::~SmplScope() {
}

void SmplScope::init()
{
    #if defined (__arm__)
    Particle.variable("getossil", ossil);
    updateTimer.start();
    //Timer timer(1000, update);
    //timer.start();
    #endif
    Serial.println("SmplScope.init>\t ok");

}

void SmplScope::update()
{
    Serial.print("SmplScope.update>\t ");
    Curtime = millis();
    //take 30 samples
    for ( int j=0; j<60; j++ )
    {
      extern ADE7880 Measurement;
//      SampleBlock[j] =  Measurement.SPIRead24S((uint16_t)VCWV);
      int32_t reg_raw;
      Measurement.regRead24S((uint16_t)VCWV, reg_raw);
      SampleBlock[j] = (float) reg_raw;
      delayMicroseconds(610); //666 us - 56us communication time@1MHz
    }
    Newtime = millis();
    Dtime = Newtime - Curtime;
    for ( int k=0; k<60; k++ )
    {
        if (SampleBlock[k] <0 && SampleBlock[k+1] >0)
        {
            StartIndex = k;
            break;
        }
    }
    /*
    for ( int l=999; l>0; l-- )
    {
        if (SampleBlock[l] <0 && SampleBlock[l+1] >0)
        {
        StopIndex = l;
        break;
        }
    }
    */
    StopIndex=StartIndex+31;
    ossil = "";
    for ( int m=StartIndex+1; m<StopIndex; m++ )
    {
        ossil += String((((float)SampleBlock[m])/100000) *9.38660911282921877506715,0);
        ossil += 'a';
    }
    Serial.println(ossil);
}

void SmplScope::parseSetting(float * SettingsArr, int Length)
{
}

void ApplicationManager::parseSetting(float * SettingsArr, int Length)
{
    //signed long SettingsArr[8]; // Src,Ptp,Pid,Fmt,Key,Value,DateTime
    Serial.print("ApplicationManager.parseSetting entered with:");
    Serial.print(SettingsArr[2],DEC);
    Serial.print(" and ");
    Serial.println(SettingsArr[3],DEC);
    if(SettingsArr[2] == 2 && SettingsArr[3] == 10) // Application: TimeAverage
	{
	    //this->TimeAverage.parseSetting(SettingsArr,Length);
//	    TimeAverage.parseSetting(SettingsArr,Length);
	    Serial.println("ApplicationManager.parseSetting>\t to TimeAverage <-- broken");
	}
	if(SettingsArr[2] == 2 && SettingsArr[3] == 11) // Application: SimpleScope
	{
	    //this->SimpleScope.parseSetting(SettingsArr,Length);
	    Serial.println("ApplicationManager.parseSetting>\t to SimpleScope");
	}
	 Serial.println("ApplicationManager.parseSetting return");
}
