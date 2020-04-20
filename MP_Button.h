#ifndef MP_Button_h
#define MP_Button_h
class MP_Button
{
  uint8_t buttonPin; 
  uint8_t buttonReading = 0;
  uint8_t lastButtonReading = 0;
  uint8_t buttonState = 0;
  uint8_t lastButtonState = 0;
  uint8_t buttonStatus = 0;
  uint8_t lastButtonStatus = 0;
  uint8_t previousStatus = 0;
  uint8_t templastButtonStatus = 0;
  bool ON;
  bool OFF;
  bool statusChanged = 0;
  uint32_t timeDebaunce = 20;
  uint32_t timeShortClick = 300;
  uint32_t timeWait = 300;
  uint32_t timeLongClick = 800;
  uint32_t timeLongerClick = 2000;
  uint32_t timeLongestClick = 4000;

  uint8_t clickCount = 0;
  
  uint32_t timeNow;
  uint32_t lastDebounceTime;
  uint32_t lastChangeTime;

  public:
  MP_Button(uint8_t _buttonPin) : MP_Button(_buttonPin, 1, timeDebaunce, timeShortClick, timeWait,
            timeLongClick, timeLongerClick, timeLongestClick)
  {
    
  }
  
  MP_Button(uint8_t _buttonPin, bool _inverted) : MP_Button(_buttonPin, _inverted, timeDebaunce, timeShortClick, timeWait,
            timeLongClick, timeLongerClick, timeLongestClick)
  {
    
  }
  
  MP_Button(uint8_t _buttonPin, bool _inverted, uint32_t _timeDebaunce, uint32_t _timeShortClick, uint32_t _timeWait,
            uint32_t _timeLongClick, uint32_t _timeLongerClick, uint32_t _timeLongestClick)
  {
    buttonPin = _buttonPin;
    ON = !_inverted;
    OFF = _inverted;
    timeDebaunce = _timeDebaunce;
    timeShortClick = _timeShortClick;
    timeWait = _timeWait;
    timeLongClick = _timeLongClick;
    timeLongerClick = _timeLongerClick;
    timeLongestClick = _timeLongestClick;    
  }

  void Init()
  {
    if(ON) // Button ON = 1/VCC
    {
      pinMode(buttonPin, INPUT);
    }
    else  // Button ON = 0/GND
    {
      pinMode(buttonPin, INPUT_PULLUP);
    }
    buttonReading = OFF;
    lastButtonReading = OFF;
    buttonState = OFF;
	#if defined(MP_DEBUG_BUTTON)
	Serial.print("Buton Initialized; ");
	Serial.print("buttonReading: ");
	Serial.print(buttonReading);
	Serial.print(", lastButtonReading: ");
	Serial.print(lastButtonReading);
	Serial.print(", buttonState: ");
	Serial.print(buttonState);
	Serial.print(", button: ");
	if(ON) Serial.println(" buttonPin, INPUT");
	else Serial.println(" buttonPin, INPUT_PULLUP");
	#endif
  }

  void Update()
  {
    timeNow = millis();
    buttonReading = digitalRead(buttonPin);    
    statusChanged = 0;

    // debauncing part
    if(buttonReading != lastButtonReading){
      lastDebounceTime = timeNow;
      lastButtonReading = buttonReading;
    }
    if((timeNow - lastDebounceTime) > timeDebaunce) 
    {
      if (buttonReading != buttonState) 
      {
        buttonState = buttonReading;
        lastChangeTime = timeNow;
      }    
    } 
    // debauncing finished, now we need to find the status
    previousStatus = lastButtonStatus;
    
    switch(buttonStatus) {
      case 0: // switch is off
        if(buttonState == ON){
          buttonStatus = 1;
        }
        break;
      case 1: // schwitch is pressed
        if(buttonState == OFF){
          buttonStatus = 2;
        }
        else if((timeNow - lastChangeTime) > timeShortClick){
          buttonStatus = 4;
        }
        break;
      case 2: // schwitch is released
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        else{
          if((timeNow - lastChangeTime) > timeWait){
            buttonStatus = 3;
          }
        }
        break;
      case 3: // schwitch is released for longer time
        clickCount = 0;
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        break; 
      case 4: // schwitch is clicked for short time
        if(buttonState == OFF){
          buttonStatus = 5;
        }
        else if((timeNow - lastChangeTime) > timeLongClick){
          buttonStatus = 7;
        }
        break;
      case 5: // schwitch is released after short click
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        else{
          if((timeNow - lastChangeTime) > timeWait){
            buttonStatus = 6;
          }
        }
        break;
      case 6: // schwitch is released for longer time after short click
        clickCount = 0;
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        break;       
      case 7: // schwitch is clicked for long time
        if(buttonState == OFF){
          buttonStatus = 8;
        }
        else if((timeNow - lastChangeTime) > timeLongerClick){
          buttonStatus = 10;
        }
        break;
      case 8: // schwitch is released after long click
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        else{
          if((timeNow - lastChangeTime) > timeWait){
            buttonStatus = 9;
          }
        }
        break;  
      case 9: // schwitch is released for longer time after long click
        clickCount = 0;
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        break;     
      case 10: // schwitch is clicked for longer time
        if(buttonState == OFF){
          buttonStatus = 11;
        }
        else if((timeNow - lastChangeTime) > timeLongestClick){
          buttonStatus = 13;
        }
        break;
      case 11: // schwitch is released after longer click
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        else{
          if((timeNow - lastChangeTime) > timeWait){
            buttonStatus = 12;
          }
        }
        break;  
      case 12: // schwitch is released for longer time after longer click
        clickCount = 0;
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        break;    
      case 13: // schwitch is clicked for longest time
        if(buttonState == OFF){
          buttonStatus = 14;
        }
        break;         
      case 14: // schwitch is released after longest click
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        else{
          if((timeNow - lastChangeTime) > timeWait){
            buttonStatus = 15;
          }
        }
        break;  
      case 15: // schwitch is released for longer time after longest click
        clickCount = 0;
        if(buttonState == ON){
          lastButtonStatus = buttonStatus;
          buttonStatus = 1;
        }
        break;  
    }  
        
    if(templastButtonStatus != buttonStatus){
      statusChanged = 1;
    }
        
    templastButtonStatus = buttonStatus;

  }

  uint8_t StatusPuls()
  {
    if(statusChanged)
    {
      return buttonStatus;
    }
    return 0;
  }

  uint8_t Status()
  {
    return buttonStatus;
  }

  bool SingleClick()
  {
    if(buttonStatus == 3 && previousStatus != 2 && statusChanged)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool PreSingleClick()
  {
    if(buttonStatus == 1 && previousStatus != 2 && statusChanged)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  
  bool DoubleClick()
  {
    if(buttonStatus == 3 && previousStatus == 2 && statusChanged)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool DoublePress()
  {
    if((buttonStatus == 4 ||buttonStatus == 7 || buttonStatus == 10 || buttonStatus == 13) && previousStatus == 2)
    {
      return true;
    }
    else
    {
      return false;
    }
  }  

  bool PressAndHold()
  {
    if((buttonStatus == 4 ||buttonStatus == 7 || buttonStatus == 10 || buttonStatus == 13) && previousStatus != 2)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool isPressed()
  {
    if(buttonState == ON)
    {
      return true;
    }
    else
    {
      return false;
    }
  }  

};
#endif
