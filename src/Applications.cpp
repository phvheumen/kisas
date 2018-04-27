#include "Applications.h" //ADE7880 Applications
#include "ADE7880.h"
#include "ADE7880RegisterNames.h"

#include "spark_wiring_tcpclient.h"
#include "math.h"
#include <application.h>
#include <HttpClient.h>

extern uint8_t RegisterTypeArr[];
extern uint8_t giveRegType(uint16_t RegisterAdress);
extern String PostBuffer;
//extern HttpClient http;
//extern http_request_t request;
//extern http_response_t response;
//extern http_header_t headers[];
/*
void setTimer(unsigned int interval, ApplicationManager appMan, class koe, void * function)
{
    //Timer timer(interval, function);
    //Timer te(1000, &SmplScope::update, appMan.SimpleScope);
    //Timer te(1000, &function, appMan.SimpleScope);
    //te.start();
}
*/

//-------------------------------------------------Time averageing-------------------------------------------

//TmAverage::TmAverage(ApplicationManager * _Manager)  : timer(1000, &TmAverage::update, *this), Manager(_Manager){
//
//}
TmAverage::TmAverage()  : updateTimer(1000, &TmAverage::update, *this), sendTimer(900000, &TmAverage::SendMessage, *this) {
//Manager=_Manager;
//Manager = AppManPtr;
//ApplicationManager* AppManPtr
}

TmAverage::~TmAverage() {
}


void TmAverage::init()
{
    //ApplicationManager& AppManPtr
    //Manager = &AppManPtr;

	updateTimer.start();
	sendTimer.start();

    //delay(1000);
    //AppManPtr.test();

}

void TmAverage::reset()
{

    for(unsigned int i=0;i<sizeof(VarList);i++)
    {
        NumSamplesList[i]=0;
        AverageList[i]=0;
    }
}

void TmAverage::SendMessage(void)
{


    //extern http_request_t request;
    Serial1.print("TmAverage.SendMessage>\t : ");
    //Serial1.println(request.path);
    //Serial1.print(millis(),DEC);
    //Serial1.print(", ");

    //prepare messagestructure
    //{"Src":1234,"Ptp":2,"Pid":10,"Fmt":0,"DTm":23,"MV":[{"Adr":60415,"Val":4.12345},{"Adr":60398,"Val":3.12245}]}
    int Source=1;
	String MessageHeader="{\"Src\":"+String(Source)+",\"Ptp\":2,\"Pid\":10,\"Fmt\":0,\"DTm\":"+String(Time.now())+",\"MV\":[";
	String MessageBodyVars="";
	String MessageBody="";
	unsigned int i=0;
	for(;i<(sizeof(VarList)/sizeof(VarList[0]));i++)
    {
        //NumSamplesList[i]=0;
        //AverageList[i]=0;
        if (VarList[i]==0){
            break;}
        //Serial1.print(i,DEC); Serial1.print(" ");Serial1.println(VarList[i],HEX);
        //Serial1.println(MessageBodyVars);
        if (i !=0){
            MessageBodyVars+=",";}
            //Serial1.println(MessageBodyVars);
        //MessageBodyVars+=(unsigned char)0x82;
        MessageBodyVars+="{\"Adr\":";//(unsigned char)0xCD; //int16 itentifier
        //Serial1.println(MessageBodyVars);
        MessageBodyVars+=String(VarList[i]);
        MessageBodyVars+=",\"Val\":";

        //---------scale with a scaling list-------------------
        float scaled=AverageList[i];
        if (VarList[i]==0x43C3)
        {
             scaled = 2*((AverageList[i])/100000) *9.38660911282921877506715;
             Serial1.println(scaled,5);


        }
        if (VarList[i]==0xE906)
        {
             scaled = (256000.0/AverageList[i]);
             Serial1.println(scaled,5);
             Serial1.println(AverageList[i],5);
             Serial1.println(ScalingList[i],5);
             Serial1.println(InNummeratorList[i],5);
        }

        if(InNummeratorList[i])
        {
            scaled = AverageList[i]*ScalingList[i];
        }
        else
        {
            scaled = ScalingList[i]/AverageList[i];
        }
        Serial1.println(scaled,5);

         //---------end of scale with a scaling list-------------------


        MessageBodyVars+=String(scaled);
        MessageBodyVars+="}";

        MessageBody+=MessageBodyVars;
        MessageBodyVars="";
        //MessageBodyValues+=(unsigned char)0xCA; //float32 itentifier
        //unsigned char MeasuredValue[]={0,0,0,0};
        //memcpy(MeasuredValue, (unsigned char*) (&AverageList[i]), 4);
        //MessageBodyValues+=MeasuredValue[3];
        //MessageBodyValues+=MeasuredValue[2];
        //MessageBodyValues+=MeasuredValue[1];
        //MessageBodyValues+=MeasuredValue[0];
    }
    Serial1.print(MessageHeader);Serial1.print(MessageBody);Serial1.println("]}");
    String total = MessageHeader+MessageBody+"]}";
    Serial1.println(total);
    delay(100);
    uint32_t freemem = System.freeMemory();
Serial1.print("free memory: ");
Serial1.println(freemem);

//updateTimer.stopFromISR();
    //this->PostHttp(total);
    PostBuffer = total;
//updateTimer.startFromISR();
    //PostHttpp(MessageHeader+MessageBody+"]}","/Kisas/Post.php");

    NumSamplesList[0]=0;
    NumSamplesList[1]=0;
    NumSamplesList[2]=0;

    for(unsigned int i=0;i<sizeof(VarList);i++)
    {
        if (VarList[i]==0)
        {
            break;
        }
        NumSamplesList[i]=0;
    }
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
    uint32_t freemem = System.freeMemory();

    //loop throught the list and request the variables from he ADE7880;
    extern ADE7880 Measurement;
    Serial1.print("TmAverage.update>\t t=");
    Serial1.print(((float)millis())/1000,DEC);
    Serial1.print("s, ");
    //Serial1.print("free memory: ");
    //Serial1.print(freemem);
    for(unsigned int i=0;i<sizeof(VarList);i++)
    {
        if (VarList[i]==0)
        {
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
        //Serial1.print("TmAverage.update>\t New measurement: ");
        //Serial1.print(millis(), DEC);
        //Serial1.print(i, DEC);Serial1.print(", ");
        Serial1.print(VarList[i], DEC);Serial1.print("->");
        Serial1.print(NewMeasurementList[i], DEC); Serial1.print(" ");//Serial1.print(", ");
        //Serial1.print(tempvar, DEC);Serial1.print(", ");
        //Serial1.println(NewMeasurementList[i], DEC);
        //do we need different scaling for all regsiters? assume we do not
        //...
        //then recalculate all the averages and check if measurement went ok.
        if ( NewMeasurementList[i] != 0 )
        {
            Serial1.print(AverageList[i],4);Serial1.print(" ");
            //Serial1.print(((NumSamplesList[i]-1)/NumSamplesList[i]),DEC);Serial1.print(" ");
            //Serial1.print((1/NumSamplesList[i]),DEC);Serial1.print(" ");
            NumSamplesList[i]++;
            AverageList[i] = (((float)(NumSamplesList[i]-1))/((float)NumSamplesList[i])) * AverageList[i] + (1/((float)NumSamplesList[i]))*(NewMeasurementList[i]);
            Serial1.print(NumSamplesList[i],DEC); Serial1.print(" ");
            Serial1.print(AverageList[i],4); Serial1.print(" ");
        }
        else
        {
            Serial1.println("TmAverage.update>\t can't add empty measurement");
        }
    }
    Serial1.println("");
}

void TmAverage::parseSetting(float * SettingsArr, int Length)
{
	//signed long SettingsArr[8]; // Src,Ptp,Pid,Fmt,Key,Value,DateTime
    //if(length < 7){return;} // not enough data received, unknown format.
    Serial1.print("TmAverage.parseSetting ");
	Serial1.print(SettingsArr[4],DEC);Serial1.print(" ");
	Serial1.print(SettingsArr[5],DEC);Serial1.print(" ");
	Serial1.println(SettingsArr[6],DEC);

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
		Serial1.print(i,DEC);Serial1.print(" ");
		Serial1.println(RegisterValueExists,DEC);
		//if it did not break out of the varlist before it's size, the varlist is full
		if (i == sizeof(VarList))
		{
			Serial1.println("TmAverage>\t Could not add to varlist, list is full.");
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
		Serial1.println("updateTimer.changePeriod");
	} else if(SettingsArr[4] == 0 && SettingsArr[5] == 2)
	{
		// set this.sendTimer to SettingsArr[6]
		this->sendTimer.changePeriod(SettingsArr[6]);
			Serial1.println("sendTimer.changePeriod");
	}
	else if((SettingsArr[4] == 0 || SettingsArr[4] == 1) && SettingsArr[5] == 3)
	{
		// set In Nummerator when scaling
		Serial1.print("In Nummerator Set=0/Reset=1 ");
		Serial1.println(SettingsArr[4],DEC);

		unsigned int i;
		for(i=0;i<sizeof(this->VarList);i++)
		{

			if (this->VarList[i] == (uint16_t) SettingsArr[6])
			{
			    Serial1.print(i,DEC);Serial1.print(" ");
			    Serial1.print(InNummeratorList[i],DEC);Serial1.print(" ");
				InNummeratorList[i] = !((bool)SettingsArr[4]);
				Serial1.println(InNummeratorList[i],DEC);
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
		Serial1.println("Set ScalingFactor pair");
		unsigned int i;
		for(i=0;i<sizeof(this->VarList);i++)
		{
			if (this->VarList[i] == (uint16_t) SettingsArr[5])
			{
			    Serial1.println(SettingsArr[6],DEC);
			    Serial1.println(((float)SettingsArr[6]),6);
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
      Serial1.println("SmplScope.SmplScope>\t constructor");
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
    Serial1.println("SmplScope.init>\t ok");

}

void SmplScope::update()
{
    Serial1.print("SmplScope.update>\t ");
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
    Serial1.println(ossil);
}

void SmplScope::parseSetting(float * SettingsArr, int Length)
{
}

//--------------------------------------------------------Standard App functions---------------------------------------

/*
bool StandardAppFunctions::sendMessagePack(String Message){
//Messagepack is not supported yet, only type 0/JSON is implemented now
}*/

int StandardAppFunctions::PostHttp(String body)
{
    noInterrupts();
        http_header_t headers[] = {
      { "Content-Type", "application/json" },
      { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { "Authorization" , "Basic MTo2M2Z4VjVkVTlCSWk1ZmV5RmRQWVZqUmFUUzJrUnA1RA=="},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};
Serial1.println(response.body);
    /*noInterrupts();
    Serial1.println();
    Serial1.println("PostHttp>\tStart of PostHttp.");
    this->request.hostname = "liandon-meetdata.nl";
    this->request.port = 80;
    this->request.path = "/Kisas/Post.php";
    this->request.body = body;
    //Serial1.println(request.body);
    this->http.post(request, response, headers);
    Serial1.print("PostHttp>\tResponse status: ");
    Serial1.println(this->response.status);
    Serial1.print("PostHttp>\tHTTP Response Body: ");
    Serial1.println(this->response.body);
    Serial1.println("PostHttp>\tEnd of PostHttp.");
    interrupts();
    */
    //return response.status;
    Serial1.println(String(body.length()) );
    String HeaderCombined = "POST /Kisas/Post.php HTTP/1.0\r\nConnection: close\r\nHOST: liandon-meetdata.nl\r\nContent-Length: " + String(body.length()) + "\r\n";
    HeaderCombined += "Authorization: Basic MTo2M2Z4VjVkVTlCSWk1ZmV5RmRQWVZqUmFUUzJrUnA1RA==\r\n";

    Serial1.println(HeaderCombined);
   /// delay(1000);
    Serial1.println(body);
    //Serial1.flush();
   // delay(1000);
 //   uint32_t freemem = System.freeMemory();
//Serial1.print("free memory: ");
//Serial1.println(freemem);
//Serial1.flush();
String server = "liandon-meetdata.nl";
    bool connected = false;
    connected = client.connect(server.c_str(), 80 );
    Serial1.println("connected: ");
    Serial1.println(connected,DEC);
//     freemem = System.freeMemory();
//Serial1.print("free memory: ");
//Serial1.println(freemem);
//Serial1.flush();
    client.println(HeaderCombined);
    client.flush();
 //    freemem = System.freeMemory();
//Serial1.print("free memory: ");
//Serial1.println(freemem);
//Serial1.flush();
    client.println(body);
    client.flush();
 //    freemem = System.freeMemory();
//Serial1.print("free memory: ");
//Serial1.println(freemem);
//Serial1.flush();
    unsigned long timeout=millis()+10000;
    while (millis()<timeout && client.connected())
    {
        if (client.available())
        {
            char c = client.read();
        }
    }
    client.stop();
     Serial1.print(">>>>");
    Serial1.println(response.body);
       Serial1.print("<<<<");
    interrupts();
    return 200;
}

//Constructor, used for initialization
StandardAppFunctions::StandardAppFunctions()
{

    Serial1.println("StandardAppFunctions>\t Constructor completed...");
}

StandardAppFunctions::~StandardAppFunctions() {
}

void ApplicationManager::parseSetting(float * SettingsArr, int Length)
{
    //signed long SettingsArr[8]; // Src,Ptp,Pid,Fmt,Key,Value,DateTime
    Serial1.print("ApplicationManager.parseSetting entered with:");
    Serial1.print(SettingsArr[2],DEC);
    Serial1.print(" and ");
    Serial1.println(SettingsArr[3],DEC);
    if(SettingsArr[2] == 2 && SettingsArr[3] == 10) // Application: TimeAverage
	{
	    //this->TimeAverage.parseSetting(SettingsArr,Length);
	    TimeAverage.parseSetting(SettingsArr,Length);
	    Serial1.println("ApplicationManager.parseSetting>\t to TimeAverage <-- broken");
	}
	if(SettingsArr[2] == 2 && SettingsArr[3] == 11) // Application: SimpleScope
	{
	    //this->SimpleScope.parseSetting(SettingsArr,Length);
	    Serial1.println("ApplicationManager.parseSetting>\t to SimpleScope");
	}
	 Serial1.println("ApplicationManager.parseSetting return");
}
