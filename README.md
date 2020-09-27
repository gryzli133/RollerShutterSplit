# RollerShutterSplit

Roller Shutter code - multiple Files

This library helps to add multiple smart Blinds to one Arduino board (preferably Arduino MEGA) by just adding one line of code per Blind. It is based on MySensors library. Tested with Domoticz, but also worked for Home Assistant (feedback from community).


v27:
- new logic for counting the position
- new feature Toggle Button (UP/STOP/DOWN/STOP/UP/...)
  - you can choose UP and DOWN buttons or Toggle button -> or if needed all of them ;-)
  - if you don't need a button then use MP_PIN_NONE instead
  
v27.2:
 - issue with timers longer than 32s is now solved. The bilnds can now run up to 255s in each direction.


To Use the code:
- download newest .ino file and all other files to one folder - same name as newest .ino file
- change the declaration in .ino file, for example:



{1, 21, 41, 61, 22, 23, MP_PIN_NONE, 38, 39, 21, 20, 21, 50, 0, "Roleta w kuchni"},

{2, 22, 42, 62, 24, 25, MP_PIN_NONE, 40, 41, 30, 29, 30, 50, 0, "Roleta w kuchni w oknie otwieranym"},

{3, 23, 43, 63, 26, 27, MP_PIN_NONE, 42, 43, 30, 29, 30, 50, 0, "Roleta HS lewa"},

{4, 24, 44, 64, 28, 29, MP_PIN_NONE, 44, 45, 30, 29, 30, 50, 0, "Roleta HS prawa"},

{5, 25, 45, 65, 30, 31, MP_PIN_NONE, 46, 47, 31, 30, 31, 50, 0, "Roleta FIX"},

{6, 26, 46, 66, 32, 33, MP_PIN_NONE, 48, 49, 20, 19, 20, 50, 0, "Roleta gabinet"},

{7, 27, 47, 67, 34, 35, MP_PIN_NONE, 50, 51, 17, 16, 17, 50, 0, "Roleta WC parter"},

{8, 28, 48, 68, 36, 37, MP_PIN_NONE, 52, 53, 17, 16, 17, 50, 0, "Roleta w łazince na dole"},

{9, 29, 49, 69, A0, A1, MP_PIN_NONE, A8, A9, 19, 18, 19, 50, 0, "Roleta garaż lewa"},

{10, 30, 50, 70, A2, A3, MP_PIN_NONE, A10, A11, 19, 18, 19, 50, 0, "Roleta garaż prawa"}





The values are representing:
1. Child ID of your Blind/Roller Shutter (Controller like Domoticz, Home Assistant will show this as device number)
2. Child ID of the timer device UP (same as above, but using this object you can correct the time the Blinds need to go full UP)
3. Child ID of the timer device DOWN (same as above, but using this object you can correct the time the Blinds need to go full DOWN)
4. Child ID - it will be not used by the controller, but is needed to keep the times during Arduino update - KEEP ALL Child ID UNIQUE !!!
5. Pin number of the Button UP
6. Pin number of the Button DOWN
7. Pin number of the Button TOGGLE
8. Pin number of the Relay UP
9. Pin number of the Relay DOWN
10. Time in second for Blind to go full UP from bottom position
11. Time in second for Blind to go full DOWN from top position
12. Calibration time - this time is defining how long the relay is going to be ON after the Blind reach the UP or DOWN position - there are some situation, when the Blinds cannot reach the end position (f.e.: Time is set too short, Power supply is missing while Arduino is working etc). In most cases the best possibility is to set it to te bigger value of UP/DOWN time preset.
13. Debounce time - if you don't know what it is, just set it to 50 ms
14. It defines what state of the output the relays needs to switch ON. High Level Trigger = 0; Low level Trigger = 1
15. Description - your blind will show this description to your controller
