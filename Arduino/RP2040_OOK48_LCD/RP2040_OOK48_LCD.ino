
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

TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
 
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
    mode = RX;  
    RxInit();
    loadMsg();
    TxMessNo = 0;
    TxInit();
    loopTimer = millis() + 1000;
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
  PPSActive = 5;              //reset 5 second timeout for PPS signal
  if(mode == RX)
    {
      dma_stop();
      dma_handler();        //call dma handler to reset the DMA timing and restart the transfers
      dmaReady = 0;
      cachePoint =0;        //Reset ready for the first symbol
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
  Serial2.begin(38400);                  //start GPS port comms
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
 
  if(millis() >= loopTimer)
    {         
      loopTimer = loopTimer + 1000;
      seconds++;
      if(seconds == 60)
        {
          minutes++;
          seconds = 0;
          if(minutes == 60)
            {
              hours++;
              minutes = 0;
              if(hours == 24)
                {
                  hours = 0;
                }
            }
        } 
      showTime();                                   //display the time
      if(PPSActive >0) PPSActive--;                 //decrement the PPS active timeout. (rest by the next PPS pulse)
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
        textPrintChar('?',TFT_ORANGE);                                 
        break;
      }
    }


  if(screenTouched())
    {
      processTouch();
    } 
//synchronise the local clock to the GPS clock if available
  int secErr = abs(seconds - gpsSec);
  if((gpsSec != -1) && (gpsActive) && (secErr > 1) && (secErr < 58))
         {
          Serial.println("Adjusting Clock");
          loopTimer = millis() + 1000;
          seconds = gpsSec;
          minutes = gpsMin;
          hours = gpsHr;
          gpsActive = false;
         }

  if(Serial2.available() > 0)           //data received from GPS module
      {
        while(Serial2.available() >0)
          {
            gpsCh=Serial2.read();
            if(gpsCh > 31) gpsBuffer[gpsPointer++] = gpsCh;
            if((gpsCh == 13) || (gpsPointer > 255))
              {
                gpsBuffer[gpsPointer] = 0;
                processNMEA();
                gpsPointer = 0;
              }
          }

      }
}


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

void loadMsg(void)
{
  if(EEPROM.read(99) == 173)
   {
     EEPROM.get(100,TxMessage);
   }
  else 
   {
    for(int i=0;i<10;i++)
     {
      strcpy(TxMessage[i] , "Empty\r"); 
     }
   }
}


void saveMsg(void)
{
  EEPROM.put(100,TxMessage);
  EEPROM.write(99,173);
  EEPROM.commit();
}
