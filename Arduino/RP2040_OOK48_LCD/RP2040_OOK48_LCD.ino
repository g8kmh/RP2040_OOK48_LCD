
// OOK48 Encoder and Decoder LCD version
// Colin Durbridge G4EML 2025


#include <hardware/dma.h>
#include <hardware/adc.h>
#include "hardware/irq.h"
#include "arduinoFFT.h"
#include <EEPROM.h>
#include <TFT_eSPI.h>                 // Hardware-specific library. Must be pre-configured for this display and touchscreen
#include "DEFINES.h"                  //include the defines for this project
#include "globals.h"                  //global variables
#include "float.h"
#include <TinyGPS++.h>

TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
TinyGPSPlus gps;
 
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, NUMBEROFBINS, SAMPLERATE);         //Declare FFT function

struct repeating_timer TxIntervalTimer;                   //repeating timer for Tx bit interval



//Run once on power up. Core 0 does the time critical work. Core 1 handles the GUI.  
void setup() 
{
    Serial.begin();                   //enable the debug serial port
    EEPROM.begin(1024);
    pinMode(PPSINPUT,INPUT);
    pinMode(KEYPIN,OUTPUT);
    digitalWrite(KEYPIN,0);
    pinMode(TXPIN,OUTPUT);
    digitalWrite(TXPIN,0);
    mode = RX;  
    RxInit();
    loadMsg();
    loadBaud();
    TxMessNo = 0;
    TxInit();
    attachInterrupt(PPSINPUT,ppsISR,RISING);
}

//Interrupt called every symbol time to update the Key output. 
bool TxIntervalInterrupt(struct repeating_timer *t)
{
  TxSymbol();
  return true;
}


//interrupt routine for 1 Pulse per second input
void ppsISR(void)
{
  PPSActive = 3;              //reset 3 (was 5) second timeout for PPS signal

  if(mode == RX)
    {
      dma_stop();
      dma_handler();        //call dma handler to reset the DMA timing and restart the transfers
      dmaReady = 0;
      if((halfRate == false ) || (halfRate & (gpsSec & 0x01) )) {
        cachePoint = 0;
      }
      else
      {
        cachePoint = 8;
      }
             //Reset ready for the first symbol
    } 
  else 
    {
      cancel_repeating_timer(&TxIntervalTimer);                           //Stop the symbol timer if it is running. 
      add_repeating_timer_us(-TXINTERVAL,TxIntervalInterrupt,NULL,&TxIntervalTimer);    // re-start the Symbol timer
      TxSymbol();                       //send the first symbol
    }
 
}

//core 1 handles the GUI
void setup1()
{
  Serial2.setRX(GPSRXPin);              //Configure the GPIO pins for the GPS module
  Serial2.setTX(GPSTXPin);

  while(gpsBaud == 0)                   //wait for core zero to initialise the baud rate for GPS. 
   {
    delay(1);
   }
  Serial2.begin(gpsBaud);    


  gpsPointer = 0;
  waterRow = 0;
  initGUI();                        //initialise the GUI screen
}


//Main Loop Core 0. Runs forever. Does most of the work.
void loop() 
{
  if(mode == RX)
   {
    RxTick();
   }
   else 
    {
     TxTick();
    }
}


//Core 1 handles the GUI. Including synchronising to GPS if available
void loop1()
{
  uint32_t command;
  char m[64];
  unsigned long inc;
 
    if((gpsSec != lastSec) | (millis() > lastTimeUpdate + 2000))
    {         
      showTime();                                   //display the time
      if(PPSActive >0) PPSActive--;                 //decrement the PPS active timeout. (rest by the next PPS pulse)
      lastSec = gpsSec;
      lastTimeUpdate = millis();
    }


  if(rp2040.fifo.pop_nb(&command))          //have we got something to process from core 0?
    {
      switch(command)
      {
        case GENPLOT:
        generatePlotData();
        break;
        case DRAWSPECTRUM:
        drawSpectrum();
        break;
        case DRAWWATERFALL:
        drawWaterfall();
        break;
        case REDLINE:
        markWaterfall(TFT_RED);
        break;
        case CYANLINE:
        markWaterfall(TFT_CYAN);
        break;
        case SHOWTONE0:
        showTone(0);
        break;
        case SHOWTONE1:
        showTone(1);
        break;
        case MESSAGE:
        textPrintChar(decoded,TFT_BLUE);                                 
        break;
        case TMESSAGE:
        textPrintChar(TxCharSent,TFT_RED);                               
        break;
        case ERROR:
        textPrintChar(decoded,TFT_ORANGE);                                           
        break;
      }
    }


  if(screenTouched())
    {
      processTouch();
    } 

  if(Serial2.available() > 0)           //data received from GPS module
      {
      while (Serial2.available()) {
      uint8_t gpsn = Serial2.read();

      if (gps.encode(gpsn)) {
        gpsActive=true;
        if (gps.time.isUpdated()) {
          gpsSec = gps.time.second();
          gpsMin = gps.time.minute();
          gpsHr = gps.time.hour();
        }
        if (mode == RX){ 
            if (gps.location.isUpdated()){ // don't update in TX 
              lon = gps.location.lng();
              lat = gps.location.lat();

              // convert longitude to Maidenhead

              d = 180.0 + lon;
              d = 0.5 * d;
              int ii = (int)(0.1 * d);
              qthLocator[0] = char(ii + 65);
              float rj = d - 10.0 * (float)ii;
              int j = (int)rj;
              qthLocator[2] = char(j + 48);
              float fpd = rj - (float)j;
              float rk = 24.0 * fpd;
              int k = (int)rk;
              qthLocator[4] = char(k + 65);
              fpd = rk - (float)(k);
              float rl = 10.0 * fpd;
              int l = (int)(rl);
              qthLocator[6] = char(l + 48);
              fpd = rl - (float)(l);
              float rm = 24.0 * fpd;
              int mm = (int)(rm);
              qthLocator[8] = char(mm + 65);
              //  convert latitude to Maidenhead
              d = 90.0 + lat;
              ii = (int)(0.1 * d);
              qthLocator[1] = char(ii + 65);
              rj = d - 10. * (float)ii;
              j = (int)rj;
              qthLocator[3] = char(j + 48);
              fpd = rj - (float)j;
              rk = 24.0 * fpd;
              k = (int)rk;
              qthLocator[5] = char(k + 65);
              fpd = rk - (float)(k);
              rl = 10.0 * fpd;
              l = int(rl);
              qthLocator[7] = char(l + 48);
              fpd = rl - (float)(l);
              rm = 24.0 * fpd;
              mm = (int)(rm);
              qthLocator[9] = char(mm + 65);
              qthLocator[locatorLength] = '\0'; // Shorten Locator string
//              Serial.println(qthLocator);
             
            }
          } 
      }
    else
      gpsActive = false;
      }
  } // while date available

}      


/***********************
void processNMEA(void)
{
  float gpsTime;

 gpsActive = true;
 if(strstr(gpsBuffer , "RMC") != NULL)                         //is this the RMC sentence?
  {
    if(strstr(gpsBuffer , "A") != NULL)                       // is the data valid?
      {
       int p=strcspn(gpsBuffer , ",");                         // find the first comma
       gpsTime = strtof(gpsBuffer+p+1 , NULL);                 //copy the time to a floating point number
       gpsSec = int(gpsTime) % 100;
       gpsTime = gpsTime / 100;
       gpsMin = int(gpsTime) % 100; 
       gpsTime = gpsTime / 100;
       gpsHr = int(gpsTime) % 100;         
      }
    else
     {
       gpsSec = -1;                                            //GPS time not valid
       gpsMin = -1;
       gpsHr = -1;
     }
  }
}
***********************************************************************/



void clearEEPROM(void)
{
  for(int i=0;i<1024;i++)
   {
    EEPROM.write(i,0);
    EEPROM.commit();
   }
}

void loadBaud(void)
{
  if(EEPROM.read(EEBAUDVALID) == 42)
   {
     EEPROM.get(EEBAUD,gpsBaud);
   }
  else 
   {
     if(autoBaud(9600))
      {
        gpsBaud = 9600;
        saveBaud();
      }
    else if(autoBaud(38400))
      {
        gpsBaud = 9600;
        saveBaud();
      } 
      else if(autoBaud(57600))
      {
        gpsBaud = 57600;
        saveBaud();
      }
      else Serial.println("No baud rate");
   }
}

void clearBaud(void)
{
  EEPROM.put(EEBAUD,0);
  EEPROM.write(EEBAUDVALID,0);
  EEPROM.commit();
}

void saveBaud(void)
{
  EEPROM.put(EEBAUD,gpsBaud);
  EEPROM.write(EEBAUDVALID,42);
  EEPROM.commit();
}


void loadMsg(void)
{
  if(EEPROM.read(EEMSGVALID) == 173)
   {
     EEPROM.get(EEMSG,TxMessage);
   }
  else 
   {
     clearMsg();
   }
}

void clearMsg(void)
  {
    for(int i=0;i<10;i++)
     {
      strcpy(TxMessage[i] , "Empty\r"); 
     }  
  }

void saveMsg(void)
{
  EEPROM.put(EEMSG,TxMessage);
  EEPROM.write(EEMSGVALID,173);
  EEPROM.commit();
}

bool autoBaud(int rate)
{
  long baudTimer;
  char test[3];
  bool gotit;

  Serial2.begin(rate);                  //start GPS port comms
  baudTimer = millis();                    //make a note of the time
  gotit = false;
  while((millis() < baudTimer+2000) & (gotit == false))       //try 38400 for two seconds
    {
      if(Serial2.available())
       {
         test[0] = test[1];             //shift the previous chars up one
         test[1] = test[2];
         test[2]=Serial2.read();        //get the next char
         if((test[0] == 'R') & (test[1] == 'M') & (test[2] == 'C'))    //have we found the string 'RMC'?
          {
            gotit = true;
          }
       }
    }     
   Serial2.end();     
   return gotit;
}
char * replace(
    char const * const original, 
    char const * const pattern, 
    char const * const replacement
) {
  size_t const replen = strlen(replacement);
  size_t const patlen = strlen(pattern);
  size_t const orilen = strlen(original);

  size_t patcnt = 0;
  const char * oriptr;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
  {
    patcnt++;
  }

  {
    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string, 
      // replacing all the instances of the pattern
      char * retptr = returned;
      for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
      {
        size_t const skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement 
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}
