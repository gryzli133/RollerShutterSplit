enum CoverState 
{
  STOP,
  UP,                             // Window covering. Up.
  DOWN,                           // Window covering. Down.
};
#define DIRECTION_UP 1
#define DIRECTION_DOWN 0
#define STATE_UP 100              // 100 is opened - up
#define STATE_DOWN 0              // 0 is closed - down
#define MIN_SYNC 2000        // Send data to controller not often than every 2s
#define MAX_SYNC 1800000     // Refresh data to controller at least once every 30min

bool IS_ACK = false; //is to acknowlage

class RollerShutter
{ 
  uint8_t CHILD_ID_COVER;
  uint8_t CHILD_ID_SET_UP;
  uint8_t CHILD_ID_SET_DOWN;
  uint8_t CHILD_ID_INITIALIZATION;
  int buttonPinUp;
  int buttonPinDown;
  int relayPinUp;
  int relayPinDown;
  uint8_t initialTimeUp;
  uint8_t initialTimeDown;
  bool relayON;
  bool relayOFF;
  const char * relayDescription;
  uint32_t lastSync;
  bool requestSync;

  bool setupMode = false;   // true = setup mode activated -> pass-through mode, Button write directly to relay.

  static bool initial_state_sent;
  uint8_t checksum_initialization;      //store the checksum for the initialization
  int value = 0;
  int oldValueUp = 0;
  int oldValueDown = 0;

  Bounce debouncerUp = Bounce();  
  Bounce debouncerDown = Bounce();
  Bounce debouncerStop = Bounce();

  
  int rollTimeUp;
  int rollTimeDown;
  
  float timeOneLevelUp;
  float timeOneLevelDown;
  float requestedShutterLevel;
  float currentShutterLevel;
  bool currentShutterLevelChanged = false;
  unsigned long lastLevelTime = 0;
  bool isMoving = false;
  int directionUpDown;
  bool calibrateDown;
  bool calibrateUp;
  unsigned long calibrationStartTime;
  float calibrationTime;
  bool calibratedDown;
  bool calibratedUp;
  int coverState = STOP;
  unsigned long relayUpOffTime = 0;
  unsigned long relayDownOffTime = 0;
  bool serviceMode = 0;
  
  public:
  RollerShutter(int childId, int setIdUp, int setIdDown, int initId,
                int buttonUp, int buttonDown, int relayUp, int relayDown,   
                uint8_t initTimeUp, uint8_t initTimeDown, uint8_t initCalibrationTime,
                int debaunceTime, bool invertedRelay, const char *descr) : 
                                           msgUp(childId, V_UP),msgDown(childId, V_DOWN),
                                           msgStop(childId, V_STOP), msgPercentage(childId, V_PERCENTAGE), 
                                           msgSetpointUp(setIdUp, V_HVAC_SETPOINT_HEAT), msgSetpointDown(setIdDown, V_HVAC_SETPOINT_COOL),
                                           msgActualUp(setIdUp, V_TEMP), msgActualDown(setIdDown, V_TEMP)
  {
                                          // constructor - like "setup" part of standard program
    CHILD_ID_COVER = childId;
    CHILD_ID_SET_UP = setIdUp;
    CHILD_ID_SET_DOWN = setIdDown;
    CHILD_ID_INITIALIZATION = initId;
    buttonPinUp = buttonUp;
    buttonPinDown = buttonDown;
    relayPinUp = relayUp;
    relayPinDown = relayDown;    
    relayON = !invertedRelay;
    relayOFF = invertedRelay;
    initialTimeUp = initTimeUp;
    initialTimeDown = initTimeDown;
    calibrationTime = initCalibrationTime;
    relayDescription = descr;     
    pinMode(buttonPinUp, INPUT_PULLUP);     // Setup the button and Activate internal pull-up
    pinMode(buttonPinDown, INPUT_PULLUP);   // Setup the button and Activate internal pull-up
    debouncerUp.attach(buttonPinUp);        // After setting up the button, setup debouncer
    debouncerDown.attach(buttonPinDown);    // After setting up the button, setup debouncer
    debouncerUp.interval(debaunceTime);     // After setting up the button, setup debouncer
    debouncerDown.interval(debaunceTime);   // After setting up the button, setup debouncer 
    digitalWrite(relayPinUp, relayOFF);     // Make sure relays are off when starting up
    digitalWrite(relayPinDown, relayOFF);   // Make sure relays are off when starting up
    pinMode(relayPinUp, OUTPUT);            // Then set relay pins in output mode
    pinMode(relayPinDown, OUTPUT);          // Then set relay pins in output mode              

    checksum_initialization = CHILD_ID_COVER + CHILD_ID_SET_UP + CHILD_ID_SET_DOWN + CHILD_ID_INITIALIZATION 
                              + initialTimeUp + initialTimeDown + calibrationTime;

    if ( checksum_initialization != loadState(CHILD_ID_INITIALIZATION))    
    {
        saveState(CHILD_ID_SET_UP, initialTimeUp);
        saveState(CHILD_ID_SET_DOWN, initialTimeDown);
        saveState(CHILD_ID_INITIALIZATION, checksum_initialization);                  
    }
    
    currentShutterLevel = loadState(CHILD_ID_COVER);
    requestedShutterLevel = currentShutterLevel;

    rollTimeUp = loadState(CHILD_ID_SET_UP);
    rollTimeDown = loadState(CHILD_ID_SET_DOWN);
    timeOneLevelUp = rollTimeUp / 100.0;
    timeOneLevelDown = rollTimeDown / 100.0;
  }  

  MyMessage msgUp;
  MyMessage msgDown;
  MyMessage msgStop;
  MyMessage msgPercentage;
  MyMessage msgSetpointUp;
  MyMessage msgActualUp;  
  MyMessage msgSetpointDown;
  MyMessage msgActualDown;  
  
  void sendState()                          // Send current state and status to gateway.
  {
    if(serviceMode == 0)
    {
      if(currentShutterLevel==100)
      {
        send(msgUp.set(1));
      }
      else 
      {
        if(currentShutterLevel==0)
        {
          send(msgDown.set(1));
        }
        else
        {
          send(msgStop.set(1));
        }
      }
      send(msgPercentage.set((int)currentShutterLevel));
      send(msgSetpointUp.set((int)rollTimeUp));
      send(msgActualUp.set((int)rollTimeUp));
      send(msgSetpointDown.set((int)rollTimeDown));
      send(msgActualDown.set((int)rollTimeDown));    
      requestSync = false;
      lastSync = millis();
    }
  }
  
  void shuttersUp(void) 
  {
    if(serviceMode == 0)
    {
      #ifdef MY_DEBUG
        Serial.println("Shutters going up");
      #endif
      if (digitalRead(relayPinDown) == relayON) {
        digitalWrite(relayPinDown, relayOFF);
        wait(50);                             // NEED TO BE CHANGED !!!!!!!!!!!!
      }
      digitalWrite(relayPinUp, relayON);
    
      directionUpDown = DIRECTION_UP;
      isMoving = true;
      coverState = UP;
      requestSync = true;
    }
  }

  void shuttersDown(void) 
  {
    if(serviceMode == 0)
    {    
      #ifdef MY_DEBUG
        Serial.println("Shutters going down");
      #endif
      if (digitalRead(relayPinUp) == relayON) {
        digitalWrite(relayPinUp, relayOFF);
        wait(50);                             // NEED TO BE CHANGED !!!!!!!!!!!!
      }
      digitalWrite(relayPinDown, relayON);
    
      directionUpDown = DIRECTION_DOWN;
      isMoving = true;
      coverState = DOWN;
      requestSync = true;
    }
  }    

  void shuttersHalt(void) 
  {
    if(serviceMode == 0)
    {        
      #ifdef MY_DEBUG
        Serial.println("Shutters halted");
      #endif
        digitalWrite(relayPinUp, relayOFF);
        digitalWrite(relayPinDown, relayOFF);
      
        isMoving = false;
        requestedShutterLevel = currentShutterLevel;
      #ifdef MY_DEBUG
        Serial.println("saving state to: ");
        Serial.println(String(currentShutterLevel));
      #endif
        saveState(CHILD_ID_COVER, (int)currentShutterLevel);
        coverState = STOP;
        requestSync = true;
    }
  }
    
  void changeShuttersLevel(int level) 
  {
      calibrateUp = false;
      calibratedUp = false;
      calibrateDown = false;
      calibratedDown = false;      
      int dir = (level > currentShutterLevel) ? DIRECTION_UP : DIRECTION_DOWN;
      if (isMoving && dir != directionUpDown) {
        shuttersHalt();
      }
      requestedShutterLevel = level;
  }

  void enterServiceMode()
  {
      serviceMode = 1;
  }

  void Update()
  {
    if(serviceMode == 0)
    {
      debouncerUp.update();
      value = debouncerUp.read();
      if (value == 0 && value != oldValueUp) {
        if(isMoving){
          shuttersHalt();
        }  
        else{
        calibrateUp = false;
        calibratedUp = false;
        changeShuttersLevel(STATE_UP);
        }
        //state = UP;
        //sendState();
      }
      oldValueUp = value;
    
      debouncerDown.update();
      value = debouncerDown.read();
      if (value == 0 && value != oldValueDown) {
        if(isMoving){
          shuttersHalt();
        }  
        else{
        calibrateDown = false;
        calibratedDown = false;
        changeShuttersLevel(STATE_DOWN);
        }    
        //state = DOWN;
        //sendState();
      }
      oldValueDown = value;
    
      if(currentShutterLevel != 100)
      {
        calibrateUp = false;
        calibratedUp = false;
      }
      if(currentShutterLevel != 0)
      {
        calibrateDown = false;
        calibratedDown = false;
      }
      
      if (isMoving) 
      {
        unsigned long _now = millis();
        if (_now - lastLevelTime >= timeOneLevelUp * 1000 && directionUpDown == DIRECTION_UP) 
        {
            currentShutterLevel += 1;
            currentShutterLevelChanged = true;
        } 
        else
        {
          if (_now - lastLevelTime >= timeOneLevelDown * 1000 && directionUpDown == DIRECTION_DOWN)  
          {
            currentShutterLevel -= 1;
            currentShutterLevelChanged = true;
          }
        }
        if(currentShutterLevelChanged)
        {
          currentShutterLevelChanged = false;
          currentShutterLevel = constrain(currentShutterLevel, 0, 100);
          #ifdef MY_DEBUG
          Serial.println(String(requestedShutterLevel));
          Serial.println(String(currentShutterLevel));
          #endif
          lastLevelTime = millis();
          requestSync = true;
          //send(msgPercentage.set((int)currentShutterLevel));
        }
        if (currentShutterLevel == requestedShutterLevel) 
        {
          if(currentShutterLevel == 0 && !calibratedDown)
          {
            if(calibrateDown == false)
            {
              calibrateDown = true;
              calibratedDown = false;
              calibrationStartTime = _now;
            }
            else 
            {
              if(calibratedDown == false)
              {
                if (_now - calibrationStartTime >= calibrationTime * 1000)
                {
                 calibratedDown = true;
                }
              }
            }
          }
          else if (currentShutterLevel == 100 && !calibratedUp)
          {
            if(calibrateUp == false)
            {
              calibrateUp = true;
              calibratedUp = false;
              calibrationStartTime = _now;
            }
            else 
            {
              if(calibratedUp == false)
              {
                if (_now - calibrationStartTime >= calibrationTime * 1000)
                {
                 calibratedUp = true;
                }
              }
            }
          }
          else
          {
            shuttersHalt();
          }
        }
      } 
      else 
      {
        if (requestedShutterLevel != currentShutterLevel) 
        {
          if (requestedShutterLevel > currentShutterLevel) {
            shuttersUp();
          }
          else {
            shuttersDown();
          }
          lastLevelTime = millis();
        }
      }
      if((requestSync && millis() - lastSync > MIN_SYNC) || millis() - lastSync > MAX_SYNC)
      {
        sendState();
      }
    }
    else // Service Mode Aktive = input signal connected directly to output !!!
    {
      digitalWrite(relayPinUp, !digitalRead(buttonPinUp));
      digitalWrite(relayPinDown, !digitalRead(buttonPinDown));
    }
  }         
  

  void SyncController()
  {
    requestSync = true;
  }
  
  void SetupMode(bool active)
  {
    setupMode = active;
  }  
  
  void Present()
  {
    // Register all sensors to gw (they will be created as child devices)
     present(CHILD_ID_COVER, S_COVER, relayDescription, IS_ACK);
     present(CHILD_ID_SET_UP, S_HVAC, "TIME UP");
     present(CHILD_ID_SET_DOWN, S_HVAC, "TIME DOWN");
  }

  void Receive(const MyMessage &message)
  {
    #ifdef MY_DEBUG
      Serial.println("recieved incomming message");
      Serial.println("Recieved message for sensor: ");
      Serial.println(String(message.sensor));
      Serial.println("Recieved message with type: ");
      Serial.println(String(message.type));
    #endif
    
     if (message.sensor == CHILD_ID_COVER) {
        int per;
        switch (message.type) {
          case V_UP:
            changeShuttersLevel(STATE_UP);
            break;
    
          case V_DOWN:
            changeShuttersLevel(STATE_DOWN);
            break;
    
          case V_STOP:
            shuttersHalt();
            break;

          case V_STATUS:
            message.getBool()?per=100:per=0;
            changeShuttersLevel(per);
            break;
    
          case V_PERCENTAGE:
            per = message.getInt();
            if (per > 100) {
              per = 100;
            }
            else if (per<0) {
              per = 0;
            }
            changeShuttersLevel(per);
            break;
        }
      } 
    else if (message.sensor == CHILD_ID_SET_UP) 
    {
        if (message.type == V_HVAC_SETPOINT_COOL || message.type == V_HVAC_SETPOINT_HEAT) 
        {
          #ifdef MY_DEBUG
          Serial.println(", New status: V_HVAC_SETPOINT_COOL, with payload: ");
          #endif      
          String strRollTimeUp = message.getString();
          rollTimeUp = strRollTimeUp.toFloat();
          #ifdef MY_DEBUG
          Serial.println("rolltime Up value: ");
          Serial.println(String(rollTimeUp));
          #endif
          saveState(CHILD_ID_SET_UP, rollTimeUp);
          requestSync = true;
          timeOneLevelUp = rollTimeUp / 100.0;
        }
    }
    else if (message.sensor == CHILD_ID_SET_DOWN) 
    {
        if (message.type == V_HVAC_SETPOINT_COOL || message.type == V_HVAC_SETPOINT_HEAT) 
        {
          #ifdef MY_DEBUG
          Serial.println(", New status: V_HVAC_SETPOINT_COOL, with payload: ");
          #endif      
          String strRollTimeDown = message.getString();
          rollTimeDown = strRollTimeDown.toFloat();
          #ifdef MY_DEBUG
          Serial.println("rolltime Down value: ");
          Serial.println(String(rollTimeDown));
          #endif
          saveState(CHILD_ID_SET_DOWN, rollTimeDown);
          requestSync = true;
          timeOneLevelDown = rollTimeDown / 100.0;
        }
      }      
    #ifdef MY_DEBUG
      Serial.println("exiting incoming message");
    #endif
      return;
    }     
};
