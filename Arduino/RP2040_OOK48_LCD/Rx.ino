//Receive routines for an OOK48 message

void RxInit(void)
{
  sampleRate = SAMPLERATE;                  //samples per second.
  rxTone = TONE;                              //tone in bins. 
  toneTolerance = TONETOLERANCE;            //Tone tolerance in bins. 
  cacheSize = CACHESIZE;                    // tone decode samples.
  hzPerBin = HZPERBIN;
  snBins = SNBINS;
  calcLegend();
  dma_init();                       //Initialise and start ADC conversions and DMA transfers. 
  dma_handler();                    //call the interrupt handler once to start transfers
  dmaReady = false;                 //reset the transfer ready flag
  cachePoint = 0;                   //zero the data received cache
  sigNoise = -100.00;
}

void RxTick(void)
{
  uint8_t tn;
   if(dmaReady)                                                 //Do we have a complete buffer of ADC samples ready?
    {
      calcSpectrum();                                           //Perform the FFT of the data
      rp2040.fifo.push(GENPLOT);                                //Ask Core 1 to generate data for the Displays from the FFT results.  
      rp2040.fifo.push(DRAWSPECTRUM);                           //Ask core 1 to draw the Spectrum Display
      rp2040.fifo.push(DRAWWATERFALL);                          //Ask core 1 to draw the Waterfall Display      
      toneCache[cachePoint] = toneDetect();                     //Add the Tone magnitude to the tone cache. 
      if(toneCache[cachePoint++] > threshold)
       {
         rp2040.fifo.push(SHOWTONE1);     //Ask Core 1 to highlight the Tone Green. 
       }
       else 
       {
         rp2040.fifo.push(SHOWTONE0);     //Ask Core 1 to highlight the Tone Orange.
       }

      if(cachePoint == cacheSize)                               //If the Cache is full (8 bits of data)
        {
          cachePoint =0;                                        //Reset ready for the next period
          if(PPSActive)                                         //decodes are only valid if the PPS Pulse is present
          { 
           if(decodeCache())                                     //Try to extract the character
             {
               rp2040.fifo.push(MESSAGE);                         //Successfull decode. Ask Core 1 to display it 
             } 
            else 
             {
                rp2040.fifo.push(ERROR);                         //Unsuccessfull decode. Ask Core 1 to display error char 
             } 
          }
        }                                  
      dmaReady = false;                                         //Clear the flag ready for next time     
    }
}

//Search the Tone Cache to try to decode the character. 
bool decodeCache(void)
{

//calculate the threshold as the mean of the received tone magnitudes. 
  threshold=0;
  for(int i = 0; i < cacheSize ; i++)
    {
      threshold=threshold + toneCache[i];
    }
  threshold = threshold / cacheSize;

// now take each received symbol and determine if it is a 1 or a zero
  uint8_t dec = 0;
  for(int i = 0; i < cacheSize; i++)
    {
      if(toneCache[i] > threshold) 
        {
          dec = dec | (0x80 >> i);        //add a one bit if detected. 
        }
    }


  decoded = decode4from8[dec];           //use the decode array to recover the original Character

  if(decoded > 0)
    {
       return 1;                     // return 1 if it is a valid 4 from 8 character
    }
  else 
    {
        return 0;                         //invalid character
    }
}
