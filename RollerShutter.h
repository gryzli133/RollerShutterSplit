// 100% is opened - up
// 0% is closed - down
#define MP_MIN_SYNC 2000        // Send data to controller not often than every 2s
#define MP_MAX_SYNC 1800000     // Refresh data to controller at least once every 30min
#define MP_PIN_NONE 255         // if there is no need for PIN, then it will be used
#define MP_WAIT_DIRECTION 100   // waiting time before changing direction - to protect the motor 

//#include <MP_Button.h>

bool IS_ACK = false; //is to acknowlage

class RollerShutter
{ 
  uint8_t CHILD_ID_COVER;
  uint8_t CHILD_ID_SET_UP;
  uint8_t CHILD_ID_SET_DOWN;
  uint8_t CHILD_ID_INITIALIZATION;
  uint8_t buttonPinUp;
  uint8_t buttonPinDown;
  uint8_t buttonPinToggle;
  uint8_t relayPinUp;
  uint8_t relayPinDown;
  bool relayON;
  bool relayOFF;
  bool directionUp;
  const char * relayDescription;
  uint32_t lastSync;
  uint8_t requestSync;

  bool setupMode = false;   // true = setup mode activated -> pass-through mode, Button write directly to relay.

  bool value = 0; // debauncer helper
  
  uint8_t rollTimeUp;
  uint8_t rollTimeDown;
  
  uint8_t requestShutterLevel;
  uint8_t currentShutterLevel;
  uint32_t currentMsUp = 0;
  uint32_t currentMsDown = 0;
  int32_t currentMs = 0;
  uint32_t timeRelayOff;
  uint32_t timeRelayOn;
  uint8_t calibrationTime;
  uint8_t relayState = 0; // 0= request relay off; 1= request relay up; 2= request relay down;
  uint8_t requestRelayState = 0; // 0= relay off; 1= relay up is on; 2= relay down is on;
  bool serviceMode = 0;

  Bounce debouncerUp = Bounce();
  Bounce debouncerDown = Bounce();
  Bounce debouncerToggle = Bounce();

  bool oldValueUp = 0;
  bool oldValueDown = 0;
  bool oldValueToggle = 0;
  
  public:
  RollerShutter(uint8_t childId, uint8_t setIdUp, uint8_t setIdDown, uint8_t initId,
                uint8_t buttonUp, uint8_t buttonDown, uint8_t buttonToggle, uint8_t relayUp, uint8_t relayDown,   
                uint8_t initTimeUp, uint8_t initTimeDown, uint8_t initCalibrationTime,
                int debaunceTime, bool invertedRelay, const char *descr) : 
                                           msgUp(childId, V_UP),msgDown(childId, V_DOWN),
                                           msgStop(childId, V_STOP), msgPercentage(childId, V_PERCENTAGE) 
                                           #ifdef MP_BLINDS_TIME_THERMOSTAT
                                           , msgSetpointUp(setIdUp, V_HVAC_SETPOINT_HEAT), msgSetpointDown(setIdDown, V_HVAC_SETPOINT_COOL),
                                           msgActualUp(setIdUp, V_TEMP), msgActualDown(setIdDown, V_TEMP)
                                           #endif
  {
                                          // constructor - like "setup" part of standard program
    CHILD_ID_COVER = childId;
    CHILD_ID_SET_UP = setIdUp;
    CHILD_ID_SET_DOWN = setIdDown;
    CHILD_ID_INITIALIZATION = initId;
    buttonPinUp = buttonUp;
    buttonPinDown = buttonDown;
    buttonPinToggle = buttonToggle;
    relayPinUp = relayUp;
    relayPinDown = relayDown;    
    relayON = !invertedRelay;
    relayOFF = invertedRelay;
    initTimeUp = initTimeUp;
    initTimeDown = initTimeDown;
    calibrationTime = initCalibrationTime;
    relayDescription = descr;
    
    if(buttonPinUp != MP_PIN_NONE) 
    {
      pinMode(buttonPinUp, INPUT_PULLUP);     // Setup the button and Activate internal pull-up
      debouncerUp.attach(buttonPinUp);        // After setting up the button, setup debouncer
      debouncerUp.interval(debaunceTime);     // After setting up the button, setup debouncer
    }
    if(buttonPinDown != MP_PIN_NONE) 
    {    
      pinMode(buttonPinDown, INPUT_PULLUP);     // Setup the button and Activate internal pull-up
      debouncerDown.attach(buttonPinDown);      // After setting up the button, setup debouncer
      debouncerDown.interval(debaunceTime);     // After setting up the button, setup debouncer
    }
    if(buttonPinToggle != MP_PIN_NONE) 
    {
      pinMode(buttonPinToggle, INPUT_PULLUP);     // Setup the button and Activate internal pull-up
      debouncerToggle.attach(buttonPinToggle);    // After setting up the button, setup debouncer
      debouncerToggle.interval(debaunceTime);     // After setting up the button, setup debouncer
    }

    digitalWrite(relayPinUp, relayOFF);     // Make sure relays are off when starting up
    digitalWrite(relayPinDown, relayOFF);   // Make sure relays are off when starting up
    pinMode(relayPinUp, OUTPUT);            // Then set relay pins in output mode
    pinMode(relayPinDown, OUTPUT);          // Then set relay pins in output mode              

    if (CHILD_ID_COVER + CHILD_ID_SET_UP + CHILD_ID_SET_DOWN + CHILD_ID_INITIALIZATION + initTimeUp + initTimeDown + calibrationTime != loadState(CHILD_ID_INITIALIZATION))    
    {
        saveState(CHILD_ID_SET_UP, initTimeUp);
        saveState(CHILD_ID_SET_DOWN, initTimeDown);
        saveState(CHILD_ID_INITIALIZATION, CHILD_ID_COVER + CHILD_ID_SET_UP + CHILD_ID_SET_DOWN + CHILD_ID_INITIALIZATION + initTimeUp + initTimeDown + calibrationTime);                  
    }
    
    rollTimeUp = loadState(CHILD_ID_SET_UP);
    rollTimeDown = loadState(CHILD_ID_SET_DOWN);
    
    currentShutterLevel = loadState(CHILD_ID_COVER);
    requestShutterLevel = currentShutterLevel;
    currentMsUp = 10 * currentShutterLevel * rollTimeUp;
  }  

  MyMessage msgUp;
  MyMessage msgDown;
  MyMessage msgStop;
  MyMessage msgPercentage;
  #ifdef MP_BLINDS_TIME_THERMOSTAT
  MyMessage msgSetpointUp;
  MyMessage msgActualUp;  
  MyMessage msgSetpointDown;
  MyMessage msgActualDown;  
  #endif
  
  void sendState()                          // Send current state and status to gateway.
  {
    if(serviceMode == 0)
    {
      if(currentShutterLevel == 100 && requestRelayState != 2)
      {
        send(msgUp.set(1));
      }
      else 
      {
        if(currentShutterLevel == 0 && requestRelayState != 1)
        {
          send(msgDown.set(1));
        }
        else
        {
          if(requestRelayState == 0)
          {
            send(msgStop.set(1));
          }
        }
      }
      send(msgPercentage.set(currentShutterLevel));
      requestSync ++;
      lastSync = millis();
    }
  }
  #ifdef MP_BLINDS_TIME_THERMOSTAT
  void sendTimeState()
  {
      send(msgSetpointUp.set((int)rollTimeUp));
      send(msgActualUp.set((int)rollTimeUp));
      send(msgSetpointDown.set((int)rollTimeDown));
      send(msgActualDown.set((int)rollTimeDown));        
  }
  #endif
  
  void shuttersUp(void) 
  {
    #ifdef MP_DEBUG_SHUTTER
    Serial.println("shuttersUp");
    #endif
    if(serviceMode == 0)
    {
      requestShutterLevel = 100;
    }
  }

  void shuttersDown(void) 
  {
    #ifdef MP_DEBUG_SHUTTER
    Serial.println("shuttersDown");
    #endif
    if(serviceMode == 0)
    {    
      requestShutterLevel = 0;
    }
  }    

  void shuttersHalt(void) 
  {
    #ifdef MP_DEBUG_SHUTTER
    Serial.println("shuttersHalt");
    #endif
    if(serviceMode == 0)
    {        
      requestShutterLevel = 255;
    }
  }
    
  void changeShuttersLevel(int level) 
  {
    #ifdef MP_DEBUG_SHUTTER
    Serial.println("changeShuttersLevel");
    #endif
    if(level == 255)
    {
      requestShutterLevel = 255;
    }
    else
    {
      requestShutterLevel = constrain(level, 0, 100);
    }
  }

  void enterServiceMode()
  {
      serviceMode = 1;
  }

  void exitServiceMode()
  {
    serviceMode = 0;
  }

  void Update()
  {
    if(serviceMode == 0)
    {
      if(buttonPinUp != MP_PIN_NONE) 
      {
        debouncerUp.update();
        value = debouncerUp.read();
        if (value == 0 && value != oldValueUp) {
          if(requestRelayState != 0){
            requestShutterLevel = 255; // request stop
          }  
          else{
          requestShutterLevel = 100;
          }
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("Button Up : ");
          Serial.println(requestShutterLevel);
          #endif
        }
        oldValueUp = value;
      }
      if(buttonPinDown != MP_PIN_NONE) 
      {
        debouncerDown.update();
        value = debouncerDown.read();
        if (value == 0 && value != oldValueDown) {
          if(requestRelayState != 0){
            requestShutterLevel = 255; // request stop
          }  
          else{
          requestShutterLevel = 0;
          } 
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("Button Down : ");
          Serial.println(requestShutterLevel);
          #endif   
        }
        oldValueDown = value;
      }
      if(buttonPinToggle != MP_PIN_NONE)
      {
        
        debouncerToggle.update();
        value = debouncerToggle.read();
        if (value == 0 && value != oldValueToggle) 
        {
          if(requestRelayState != 0)
          {
            requestShutterLevel = 255; // request stop
          }  
          else
          {
            if(directionUp) // last direction was UP -> change to DOWN
            {
              requestShutterLevel = 0;
            }
            else // last direction was DOWN -> change to UP
            {
              requestShutterLevel = 100;
            }
          }
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("Button Toggle : ");
          Serial.println(requestShutterLevel);
          #endif
        }
        oldValueToggle = value;
      }

      
      
      switch(relayState)
      {
        case 0:
          currentMs = currentMsUp / rollTimeUp - currentMsDown / rollTimeDown;
          break;
        case 1:
          currentMs = (currentMsUp + millis() - timeRelayOn) / rollTimeUp - currentMsDown / rollTimeDown;
          break;
        case 2:
          currentMs = currentMsUp / rollTimeUp - (currentMsDown + millis() - timeRelayOn) / rollTimeDown;
          break;
      }
      
      uint16_t currentShutterLevelTemp = constrain(currentMs, 0, 1000);
      currentShutterLevel = currentShutterLevelTemp / 10; // convert to percentage
      #ifdef MP_DEBUG_SHUTTER
      Serial.print("currentShutterLevel / requestShutterLevel / currentMs / currentMsUp / currentMsDown : ");
      Serial.print(currentShutterLevel);
      Serial.print(" / ");
      Serial.print(requestShutterLevel);
      Serial.print(" / ");
      Serial.print(currentMs);
      Serial.print(" / ");
      Serial.print(currentMsUp);
      Serial.print(" / ");
      Serial.println(currentMsDown);
      #endif

      // REQUEST LEVEL CODE //
      if(requestShutterLevel == 255)
      {
        requestRelayState = 0; 
        if(currentMs <= 0)
        {
          currentMsUp = 0;
          currentMsDown = 0;
        }
        else if(currentMs >= 1000)
        {
          currentMsUp = 1000 * rollTimeUp;
          currentMsDown = 0;
        }
        #ifdef MP_DEBUG_SHUTTER
      Serial.print("requestShutterLevel == 255 : ");
      Serial.print(currentShutterLevel);
      Serial.print(" / ");
      Serial.print(requestShutterLevel);
      Serial.print(" / ");
      Serial.print(currentMsUp);
      Serial.print(" / ");
      Serial.println(currentMsDown);
      #endif    
      }
      else if(requestShutterLevel == currentShutterLevel)
      {
        if(requestShutterLevel == 0)
        {
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("requestShutterLevel == 0 : ");
          Serial.print(currentShutterLevel);
          Serial.print(" / ");
          Serial.print(requestShutterLevel);
          Serial.print(" / ");
          Serial.print(currentMsUp);
          Serial.print(" / ");
          Serial.println(currentMsDown);
          #endif
          if(currentMs <= 0 - (1000*calibrationTime/rollTimeDown))
          {
            requestRelayState = 0;
            currentMsUp = 0;
            currentMsDown = 0;
          }
        }
        else if(requestShutterLevel == 100)
        {
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("requestShutterLevel == 100 : ");
          Serial.print(currentShutterLevel);
          Serial.print(" / ");
          Serial.print(requestShutterLevel);
          Serial.print(" / ");
          Serial.print(currentMsUp);
          Serial.print(" / ");
          Serial.println(currentMsDown);
          #endif
          if(currentMs >= 1000 + (1000*calibrationTime/rollTimeUp))
          {
            requestRelayState = 0; 
            currentMsUp = 1000 * rollTimeUp;
            currentMsDown = 0; 
          }
        }
        else
        {
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("requestShutterLevel == currentShutterLevel : ");
          Serial.print(currentShutterLevel);
          Serial.print(" / ");
          Serial.print(requestShutterLevel);
          Serial.print(" / ");
          Serial.print(currentMsUp);
          Serial.print(" / ");
          Serial.println(currentMsDown);
          #endif
          requestRelayState = 0;
        }
      }
      else
      {
        requestSync = 1;
        if(requestShutterLevel > currentShutterLevel)
        {
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("requestShutterLevel > currentShutterLevel : ");
          Serial.print(currentShutterLevel);
          Serial.print(" / ");
          Serial.print(requestShutterLevel);
          Serial.print(" / ");
          Serial.print(currentMsUp);
          Serial.print(" / ");
          Serial.println(currentMsDown);
          #endif
          requestRelayState = 1;
        }
        else
        {
          #ifdef MP_DEBUG_SHUTTER
          Serial.print("requestShutterLevel < currentShutterLevel : ");
          Serial.print(currentShutterLevel);
          Serial.print(" / ");
          Serial.print(requestShutterLevel);
          Serial.print(" / ");
          Serial.print(currentMsUp);
          Serial.print(" / ");
          Serial.println(currentMsDown);
          #endif
          requestRelayState = 2;
        }
      }
      //**********************

      if(relayState != requestRelayState)
      {
        requestSync = 1;
        #ifdef MP_DEBUG_SHUTTER
        Serial.print("relayState / requestRelayState; ");
        Serial.print(relayState);
        Serial.print(" / ");
        Serial.println(requestRelayState);
        #endif
        if(relayState != 0)
        {
          digitalWrite(relayPinUp, relayOFF);
          digitalWrite(relayPinDown, relayOFF);
          timeRelayOff = millis();
          
          if(relayState == 1) // UP
          {
            currentMsUp += timeRelayOff - timeRelayOn;
          } 
          else                // DOWN
          {
            currentMsDown += timeRelayOff - timeRelayOn;
          }
          saveState(CHILD_ID_COVER, currentShutterLevel);
          relayState = 0;
        }
        else
        {
          if(millis() - timeRelayOff > MP_WAIT_DIRECTION)
          {
            if(requestRelayState == 1)        //Request UP
            {
              digitalWrite(relayPinDown, relayOFF);
              digitalWrite(relayPinUp, relayON);
              relayState = 1;
              directionUp = true;
              timeRelayOn = millis();
            }
            else if(requestRelayState == 2)   //Request DOWN
            {
              digitalWrite(relayPinUp, relayOFF);
              digitalWrite(relayPinDown, relayON);
              relayState = 2;
              directionUp = false;
              timeRelayOn = millis();
            }
          }
        }
      }

      if((requestSync == 1 && millis() - lastSync > MP_MIN_SYNC) ||              // sync first time
         (requestSync == 2 && millis() - lastSync > (MP_MIN_SYNC+MP_MAX_SYNC)/4) || // resync after quarter time
         (requestSync == 3 && millis() - lastSync > (MP_MIN_SYNC+MP_MAX_SYNC)/2) || // resync after half time
         millis() - lastSync > MP_MAX_SYNC)                                      // resync every MP_MAX_SYNC time
      {
        sendState();
      }        
    }
    else // Service Mode Aktive = input signal connected directly to output !!!
    {
      if(buttonPinUp != MP_PIN_NONE) digitalWrite(relayPinUp, !digitalRead(buttonPinUp));
      if(buttonPinDown != MP_PIN_NONE) digitalWrite(relayPinDown, !digitalRead(buttonPinDown));
    }
  }         
  

  void SyncController()
  {
    requestSync = 1;
  }
  
  void SetupMode(bool active)
  {
    setupMode = active;
  }  
  
  void Present()
  {
    // Register all sensors to gw (they will be created as child devices)
     present(CHILD_ID_COVER, S_COVER, relayDescription, IS_ACK);
     #ifdef MP_BLINDS_TIME_THERMOSTAT
     present(CHILD_ID_SET_UP, S_HVAC, "TIME UP");
     present(CHILD_ID_SET_DOWN, S_HVAC, "TIME DOWN");
     #endif
  }

  void Receive(const MyMessage &message)
  {
    #ifdef MP_DEBUG_SHUTTER
      Serial.println("recieved incomming message");
      Serial.println("Recieved message for sensor: ");
      Serial.println(String(message.sensor));
      Serial.println("Recieved message with type: ");
      Serial.println(String(message.type));
    #endif
    
     if (message.sensor == CHILD_ID_COVER) 
     {
        lastSync = millis();
        int per;
        switch (message.type) {
          case V_UP:
            requestShutterLevel = 100;
            break;
    
          case V_DOWN:
            requestShutterLevel = 0;
            break;
    
          case V_STOP:
            requestShutterLevel = 255;
            break;

          case V_STATUS:
            message.getBool()?requestShutterLevel = 100 : requestShutterLevel = 0;
            break;
    
          case V_PERCENTAGE:
            changeShuttersLevel(message.getInt());
            break;
        }
      } 

    
    #ifdef MP_BLINDS_TIME_THERMOSTAT
    else if (message.sensor == CHILD_ID_SET_UP) 
    {
        if (message.type == V_HVAC_SETPOINT_COOL || message.type == V_HVAC_SETPOINT_HEAT) 
        {
          #ifdef MP_DEBUG_SHUTTER
          Serial.println(", New status: V_HVAC_SETPOINT_COOL, with payload: ");
          #endif
          rollTimeUp = message.getInt();
          #ifdef MP_DEBUG_SHUTTER
          Serial.println("rolltime Up value: ");
          Serial.println(rollTimeUp);
          #endif
          saveState(CHILD_ID_SET_UP, rollTimeUp);
          requestSync = true;
        }
        sendTimeState();
    }
    else if (message.sensor == CHILD_ID_SET_DOWN) 
    {
        if (message.type == V_HVAC_SETPOINT_COOL || message.type == V_HVAC_SETPOINT_HEAT) 
        {
          #ifdef MP_DEBUG_SHUTTER
          Serial.println(", New status: V_HVAC_SETPOINT_COOL, with payload: ");
          #endif      
          rollTimeDown = message.getInt();
          #ifdef MP_DEBUG_SHUTTER
          Serial.println("rolltime Down value: ");
          Serial.println(rollTimeDown);
          #endif
          saveState(CHILD_ID_SET_DOWN, rollTimeDown);
          requestSync = true;
        }
        sendTimeState();
      }      
    #endif
    #ifdef MP_DEBUG_SHUTTER
      Serial.println("exiting incoming message");
    #endif
      return;
    }     
};
