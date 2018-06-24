#ifndef ADE7880REGISTERNAMES_H
    #include "ADE7880RegisterNames.h"
#endif
#define READ 0x01
#define WRITE 0x00

#include <HttpClient.h>
#include "Applications.h"
//#include <application.h>
//#include <stdint.h>


String hostname = "liandon-meetdata.nl";
http_request_t request;
http_response_t response;
HttpClient http;
char HTTPHeaderAuthValue[]="Basic MTo2M2Z4VjVkVTlCSWk1ZmV5RmRQWVZqUmFUUzJrUnA1RA==";
http_header_t headers[] = {
      { "Content-Type", "application/json" },
      { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { "Authorization" , (const char *)&HTTPHeaderAuthValue},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};
/*

//settings for voltage and freqency measurement
//Initialize WTHR, VARTHR, VATHR, VLEVEL and VNOM registers based Equation 26, Equation 37, Equation 44, Equation 22, and Equation 42, respectively

//Eq.22: VLEVEL = Ufs x 4 x10e6 / Un = 6148755 Met U=230, Ufs=353.5534--> VLEVEL = 0x5DD293 (signed register, checked that it's a positive number)
//first bit is a 0 of the 16-bit pad with four times this zero, followed by four fixed additional zero's

char dummydata[7] = {READ, VLEVEL>>8, VLEVEL&0xFF,0x00,0x5D,0xD2,0x93};
bcm2835_spi_transfern (dummydata, 7);

//Eq.42: VNOM = 3766572 x U / Ufs. Met U=230, Ufs=353.5534 --> VNOM = 0x25637B  //Similar to the register presented in Figure 57, the VNOM 24-bit signed register is accessed as a 32-bit register with the eight MSBs padded with 0s??????

char dummydata[7] = {READ, VNOM>>8, VNOM&0xFF,0x00,0x25,0x63,0x7B};
bcm2835_spi_transfern (dummydata, 7);

//Enable the data memory RAM protection, by writing 0xAD to an internal 8-bit register located at Address 0xE7FE, followed by a write of 0x80 to an internal 8-bit register located at Address 0xE7E3.

char dummydata[4] = {WRITE, RAMPROTECTENABLE>>8, RAMPROTECTENABLE&0xFF,0xAD};
bcm2835_spi_transfern (dummydata, 4);
char dummydata[4] = {WRITE, RAMPROTECTVAL>>8, RAMPROTECTVAL&0xFF,0x80};
bcm2835_spi_transfern (dummydata, 4);

//Start the DSP by setting Run = 1

char dummydata[5] = {WRITE, RUN>>8, RUN&0xFF,0x00,0x01};
bcm2835_spi_transfern (dummydata, 5); //Write 0x0001 to the Run register

//frequency measurement for phase A
char dummydata[3] = {READ, APERIOD>>8, APERIOD&0xFF};
char receivebuffer[5];
bcm2835_spi_transfernb (dummydata, receivebuffer, 5);
//3.90625 us/LSB so 50Hz equals 5120(0x1400) in the receivedata buffer
printf("freqeuncy register 0x %x %x",receivebuffer[3],receivebuffer[4] );

//voltage measurement
char dummydata[3] = {READ, AVRMS>>8, AVRMS&0xFF};
char receivebuffer[7];
bcm2835_spi_transfernb (dummydata, receivebuffer, 7);
printf("voltage register 0x %x %x %x %x",receivebuffer[3],receivebuffer[4],receivebuffer[5],receivebuffer[6] );
//Vmeasured=(receivebuffer/0d3766572)*0.5Volt*.5*sqrt(2)*1000 (attenuation network)
//0x2D8131/0d3766572=2982193/3766572 -->Vmeasured=0.283V*1000


*/


uint8_t RegisterTypeArr[] = {3, 3, 3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 3, 3, 0, 0, 0, 0, 0, 3, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 8, 8, 8, 5, 5, 5, 2, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 6, 6, 6, 6, 0, 0, 0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 9, 9, 9, 9, 0, 0, 0, 0, 9, 0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 11, 11, 11, 11, 11, 11, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 11, 11, 11, 11, 11, 0, 0, 0, 11, 11, 11, 11, 11, 11};

//uint8_t  RegisterTypeArr[1680];


#define REG_TYPE_32_32_SIGNED 1
#define REG_TYPE_32_32_UNSIGNED 2
#define REG_TYPE_24_32ZPSE_SIGNED 3
#define REG_TYPE_24_32ZP_SIGNED 4
#define REG_TYPE_24_32ZP_UNSIGNED 5
#define REG_TYPE_24_32SE_SIGNED 6
#define REG_TYPE_24_32_SIGNED 7
#define REG_TYPE_20_32ZP_UNSIGNED 8
#define REG_TYPE_16_16_UNSIGNED 9
#define REG_TYPE_10_16ZP_UNSIGNED 10
#define REG_TYPE_8_8_UNSIGNED 11
#define REG_TYPE_BITWISE 12

uint8_t giveRegType(uint16_t RegisterAdress)
{
    //Returns the type/size of the register, so that one knows which SPI read/write function to call
    //there are 11 types noted as (bit length of register/Bit Length During Communication/signed or unsigned):
    //  32/32/S
    //  32/32/U
    //  24/32 ZPSE/S
    //  24/32 ZP/S
    //  24/32 ZP/U
    //  24/32 SE/S
    //  24/32/S
    //  20/32ZP/U
    //  16/16/U
    //  10/16 ZP/U
    //  8/8/U
    //
    //we can do this with a switch, as it is now: uses more CPU and less memory
    //or by array/look-up-table: uses ca 2kB of memory, but less CPU
    //RegisterAdresses go from 0x4389-0x43FF and 0xE400-0xEA0A. 0xEA0B-0xEBFE is reserved(not used). But 0xEBFF, 0xEC00, 0xEC01 are also used.
    //these are 119 adresses + 1547 adresses + 501 reserved/not used + 3 adresses
    
    //maybe we should return a pointer to the correct SPI-function in stead of returning the size 
    //to call the function belonging to the pointer test: int (*test)(int a, int b);
    
    
    uint32_t RegisterRange = (RegisterAdress & 0xFF00);
    uint32_t IndexOffset;
    if (RegisterRange > 0xE300 && RegisterRange < 0xEB00)
    {
        //we subtract IndexOffset from the RegisterAdress so that 0xE400-0xEA0A maps on index 130...1676 of the look up table
        IndexOffset = 0xE37E;
    }
    else
    {
        if (RegisterRange == 0x4300)
        {
            IndexOffset = 0x4380;
            //This is for  0x4380-0x43FF mapped on index 0...127 of the look up table
        }
        else
        {
            if (RegisterRange == 0xE200)
            {
                //and 0xE203 and E228 is for 128...129
                //only two cases without intermediate zero's in de look-up-table
                //IndexOffset = 0x4380;
                if (RegisterAdress == 0xE203)
                {
                    return RegisterTypeArr[128];
                }
                if (RegisterAdress == 0xE228)
                {
                    return RegisterTypeArr[129];
                }
            }
            else
            //This is for 0xEBFF, 0xEC00, 0xEC01 mapped onto 1677...1679
            IndexOffset = 0xE572;
        }
        
    }
    //Serial1.print("giveRegType>\t");
    //Serial1.print(RegisterAdress,DEC);Serial1.print(" ");
    //Serial1.print(RegisterRange,DEC);Serial1.print(" ");
    //Serial1.print(IndexOffset,DEC);Serial1.print(" ");
    //Serial1.println(RegisterTypeArr[RegisterAdress-IndexOffset],DEC);
    return RegisterTypeArr[RegisterAdress-IndexOffset];
   // return 3;
    
}



int PostHttp(String body,String url)
{
//    //Serial1.println();
//    //Serial1.println("PostHttp>\tStart");
//    request.hostname = hostname;
//    request.port = 80;
//    request.path = url;
//    request.body = body;
//    //Serial1.print(request.hostname);Serial1.print(" ");
//    //Serial1.print(request.path);Serial1.print(":");
//    //Serial1.println(request.port);
//
//    //Serial1.println(request.body);
//    //Serial1.println(request.body);
//    noInterrupts();
//    http.post(request, response, headers);
//    interrupts();
//    //Serial1.print("PostHttp>\tResponse status: ");
//    //Serial1.println(response.status);
//    Serial1.print("PostHttp>\tHTTP Response Body: ");
//    Serial1.println(response.body);
//
//    return response.status;
//    //return 200;
}
/*
int PostHttpp(String body,String url)
{
    PostHttp(body,url);
}
*/



// Cloud functions must return int and take one String
int getSettings(String input) {
//    extern ApplicationManager application;
//
//    noInterrupts();
//    Serial1.println("getSettings>\t called with input("+input+") at time "); Serial1.println(millis());
//    //request.path = "/Kisas/Options.php";
//    int status = PostHttp("{\"Src\":1}","/Kisas/Options.php");
//    if ( status == 200)
//    {
//        //Serial1.println(millis());
//        String NewSettings = response.body;  //"1;2;3;4;5;6;7\r\n";
//        //Serial1.print(NewSettings.length(),DEC);
//        //Serial1.print(">");
//        for (int CharPointer=0;CharPointer<NewSettings.length();)
//        {
//            int EndOfLine = NewSettings.indexOf('\r\n', CharPointer);
//            //Serial1.println(EndOfLine,DEC);
//            //String OneLine = NewSettings.substring(CharPointer, EndOfLine+2);
//            //Serial1.print(OneLine);
//            //Serial1.println(NewSettings.substring(CharPointer, EndOfLine+2));
//            //signed long SettingsArr[8]; // Src,Ptp,Pid,Fmt,Key,Value,DateTime
//            float SettingsArr[8]; // Src,Ptp,Pid,Fmt,Key,Value,DateTime
//            for(int i=0;(CharPointer<EndOfLine+1)&&i<8;i++)
//            {
//                //Serial1.print(i,DEC);
//                //Serial1.print("\t");
//                int EndOfSubstr = NewSettings.indexOf(';', CharPointer);
//                if (EndOfSubstr>EndOfLine || EndOfSubstr == -1)
//                {
//                    EndOfSubstr = EndOfLine-1;
//                }
//
//                //Serial1.print(EndOfSubstr,DEC);Serial1.print("\t");
//                //Serial1.print(value);
//                //Serial1.print(NewSettings.substring(CharPointer, EndOfSubstr));
//                //Serial1.print("\t");
//                SettingsArr[i] = (NewSettings.substring(CharPointer, EndOfSubstr)).toFloat();
//               // Serial1.println(SettingsArr[i],DEC);
//                CharPointer = EndOfSubstr+1;
//            }
//            //Serial1.println(EndOfLine,DEC);
//            CharPointer = EndOfLine+1;
//            //Serial1.println(CharPointer,DEC);
//            //Serial1.print(NewSettings.charAt(CharPointer-3));
//            //Serial1.print(NewSettings.charAt(CharPointer-2));
//            //Serial1.print(NewSettings.charAt(CharPointer-1));
//            //Serial1.print(NewSettings.charAt(CharPointer-0));
//            //Serial1.print(NewSettings.charAt(CharPointer+1));
//            //Serial1.println(NewSettings.charAt(CharPointer+2));
//            Serial1.println("goto Application.parseSetting");
//            application.parseSetting(SettingsArr,sizeof(SettingsArr)/sizeof(SettingsArr[0]));
//        }
//    }
//    else
//    {
//        Serial1.print("getSettings>\t failed -");
//        Serial1.println(status,DEC);
//    }
//    //request.path = "/Kisas/Post.php";
//    Serial1.println(millis());
//    Serial1.println(request.path);
//    Serial1.print("getSettings>\t exit");
//    interrupts();

    return 1;
}


