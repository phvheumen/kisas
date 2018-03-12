#include "Applications.h"

#include "ADE7880.h"
#include "ADE7880RegisterNames.h"

#include <application.h>


extern uint8_t RegisterTypeArr[];
extern uint8_t giveRegType(uint16_t RegisterAdress);

/*
void setTimer(unsigned int interval, ApplicationManager appMan, class koe, void * function)
{
    //Timer timer(interval, function);
    //Timer te(1000, &SmplScope::update, appMan.SimpleScope);
    //Timer te(1000, &function, appMan.SimpleScope);
    //te.start();
}
*/

//------Time averageing-----------
TmAverage::TmAverage() : timer(1000, &TmAverage::update, *this) {
}

TmAverage::~TmAverage() {
}

//void TmAverage::reset()
//{
//
//    for(unsigned int i=0;i<sizeof(VarList);i++)
//    {
//        NumSamplesList[i]=0;
//        AverageList[i]=0;
//    }
//}
//
//void TmAverage::addToVarList(uint16_t RegisterAdress)
//{
//    //Find the place where nextvar is zero
//    unsigned int i;
//    for(i=0;i<sizeof(VarList);i++)
//    {
//        if (VarList[i]==0)
//        {
//            break;
//        }
//    }
//    //check if varlist is not full (or incorrectly initialized), and add to the the list
//    if (i != sizeof(VarList)-1)
//    {
//        VarList[i]=RegisterAdress;
//        VarList[i+1]=0;
//    }
//}
//
//void TmAverage::removeFromVarList(uint16_t RegisterAdress)
//{
//    //find index of registeradress to remove from the list
//    unsigned int i;
//    for(i=0;i<sizeof(VarList);i++)
//    {
//        if (VarList[i]==RegisterAdress)
//        {
//            break;
//        }
//    }
//    if (i == sizeof(VarList))
//    {
//        //register not found, return without doing anything
//        return;
//    }
//    else
//    {
//        //shift all trailing registers to fill the gap of the removed register
//        for(;i<(sizeof(VarList)-1);i++)
//        {
//
//            VarList[i]=VarList[i+1];
//            if (VarList[i]==0)
//            {
//                break;
//            }
//        }
//    }
//    return;
//}
//
void TmAverage::update()
{
    //loop throught the list and request the variables from he ADE7880;
//    extern ADE7880 Measurement;

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
//        NewMeasurementList[i] = Measurement.SPIReadAutoRegsize(VarList[i],giveRegType(VarList[i]));
        //do we need different scaling for all regsiters? assume we do not
        //...
        //then recalculate all the averages and check if measurement went ok.
        if ( NewMeasurementList[i] != 0 )
        {
            NumSamplesList[i]++;
            AverageList[i] = ((NumSamplesList[i]-1)/NumSamplesList[i]) * AverageList[i] + (1/NumSamplesList[i])*((float)NewMeasurementList[i]);
        }
    }
}

//------Simple Scope-----------
SmplScope::SmplScope() : timer(1000, &SmplScope::update, *this) {
}

SmplScope::~SmplScope() {
}

void SmplScope::init()
{
    #if defined (__arm__)
    Particle.variable("getossil", ossil);
    timer.start();
    //Timer timer(1000, update);
    //timer.start();
    #endif

}

void SmplScope::update()
{
    Curtime = millis();
    //take 30 samples
    for ( int j=0; j<60; j++ )
    {
//      extern ADE7880 Measurement;
//      SampleBlock[j] =  Measurement.SPIRead24S((uint16_t)VCWV); // TODO: Change read function
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
    Serial.println("updated result: ");
    Serial.println(ossil);
}
