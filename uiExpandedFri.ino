#include "MenuBackend.h"
#include "pins.h"
#include <math.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//this controls the menu backend and the event generation
    MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent);

    //////////////////////
    // Global variables //
    //////////////////////    
    int initTime;
    
    int denat; 
    int denatTime;
    int anneal; 
    int annealTime;
    int elong; 
    int elongTime;
    int cycles;
    
    int hold; 
    int holdTime;
    int heaterGoal;
    int peltGoal;
    
    float heaterTemperature;
    float peltTemperature;
     
    // interrupt service routine vars
    boolean A_set;              
    boolean B_set;
    
    unsigned long startTime;
    
    volatile unsigned int row;
    volatile byte run;
    volatile byte holding;
    volatile byte numberSelect;
    volatile int * numberPointer;

////////////////
// Menu Items //
////////////////
MenuItem pcr         = MenuItem(menu, "PCR            ", 1);
MenuItem pcrRun      = MenuItem(menu, "Run PCR ", 2);
MenuItem pcrStop     = MenuItem(menu, "Stop PCR", 2);
MenuItem pcrHold     = MenuItem(menu, "Hold PCR", 2);
MenuItem pcrSettings = MenuItem(menu, "Settings", 2);
  MenuItem pcrInitTime   = MenuItem(menu,    "Init Time  ", 3);
  MenuItem pcrDenatTemp  = MenuItem(menu,    "Denat Temp ", 3);
  MenuItem pcrDenatTime  = MenuItem(menu,    "Denat Time ", 3);
  MenuItem pcrAnnealTemp = MenuItem(menu,    "Anneal Temp", 3);
  MenuItem pcrAnnealTime = MenuItem(menu,    "Anneal Time", 3);
  MenuItem pcrElongTemp  = MenuItem(menu,    "Elong Temp ", 3);
  MenuItem pcrElongTime  = MenuItem(menu,    "Elong Time ", 3);
  MenuItem pcrHoldTemp     = MenuItem(menu,  "Hold Temp  ", 3);
  MenuItem pcrHoldTime     = MenuItem(menu,  "Hold Time  ", 3);
  MenuItem pcrCycles       = MenuItem(menu,  "Cycles     ", 3);
  MenuItem pcrLidTemp      = MenuItem(menu,  "Lid Temp   ", 3);

  MenuItem pcrSettingsOK   = MenuItem(menu,  "OK         ", 2);
MenuItem pcrBack     = MenuItem(menu, "Back    ", 2);


MenuItem cent         = MenuItem(menu, "Centrifuge     ", 1);
MenuItem centRun      = MenuItem(menu, "Run Centrifuge ", 2);
MenuItem centStop     = MenuItem(menu, "Stop Centrifuge", 2);
MenuItem centBack     = MenuItem(menu, "Back           ", 2);

MenuItem ep           = MenuItem(menu, "Electrophoresis", 1);
MenuItem epRun        = MenuItem(menu, "Run EP     ", 2);
MenuItem epStop       = MenuItem(menu, "Stop EP    ", 2);
MenuItem ledToggle    = MenuItem(menu, "Toggle LEDs", 2);
MenuItem epBack       = MenuItem(menu, "Back       ", 2);



////////////////
// Build Menu //
////////////////
void menuSetup()
{
      menu.getRoot().add(pcr); 
      //pcr.addBefore(ep);
      //setup the settings menu item
      pcr.addBefore(cent); 
      cent.addBefore(ep); 
      ep.addBefore(pcr);
    
      pcr.addRight(pcrRun);     //chain settings to  pin on right
      pcrRun.addBefore(pcrStop);     //chain settings to  pin on right
      pcrStop.addBefore(pcrHold);
      pcrHold.addBefore(pcrSettings);
      pcrSettings.addBefore(pcrBack);
      pcrBack.addBefore(pcrRun);
      pcrBack.addLeft(pcr);
      
          pcrSettings.addRight(pcrInitTime);
          pcrInitTime.addBefore(pcrDenatTemp);
          pcrDenatTemp.addBefore(pcrDenatTime);
          pcrDenatTime.addBefore(pcrAnnealTemp);
          pcrAnnealTemp.addBefore(pcrAnnealTime);
          pcrAnnealTime.addBefore(pcrElongTemp);
          pcrElongTemp.addBefore(pcrElongTime);
          pcrElongTime.addBefore(pcrCycles);
          pcrCycles.addBefore(pcrHoldTemp);
          pcrHoldTemp.addBefore(pcrHoldTime);
          pcrHoldTime.addBefore(pcrLidTemp);
          pcrLidTemp.addBefore(pcrSettingsOK);
          pcrSettingsOK.addBefore(pcrInitTime);
          pcrSettingsOK.addLeft(pcrSettings);
             
      cent.addRight(centRun);     //chain settings to  pin on right
      centRun.addBefore(centStop);
      centStop.addBefore(centBack);
      centBack.addBefore(centRun);
      centBack.addLeft(cent);
    
      ep.addRight(epRun);     //chain settings to  pin on right
      epRun.addBefore(epStop);
      epStop.addBefore(ledToggle);
      ledToggle.addBefore(epBack);
      epBack.addBefore(epRun);
      epBack.addLeft(ep);
}

/////////////////////////
// Menu item behaviour //
/////////////////////////
void menuUseEvent(MenuUseEvent used)
{
        if (used.item.isEqual(pcr)){row = 2;menu.moveRight();}  
        if (used.item.isEqual(ep)){row = 2;menu.moveRight();}  
        if (used.item.isEqual(cent)){row = 2;menu.moveRight();}
      
        /////////////
        // PCR Menu//
        /////////////        
        if (used.item.isEqual(pcrRun)) //START PCR
        {
            run = 1; holding = 0;
        }
        
        if (used.item.isEqual(pcrStop)) // STOP PCR
        {
            run = 0;
            holding = 0;
            peltOff();
            heaterOff();
            line(3);
            Serial.write("                 ");
            line(4);
            Serial.write("               ");
        }

        if (used.item.isEqual(pcrHold)) //START PCR HOLD
        {
            holding = 1; run = 0;
        }
      
        if (used.item.isEqual(pcrSettings)) // SETTINGS (STOPS CURRENT RUN)
        {
            run = 0;
            holding = 0;
            peltOff();
            heaterOff();
            line(3);
            Serial.write("                 ");
            line(4);
            Serial.write("               ");
            row = 3;
            menu.moveRight();
         }    

            if (used.item.isEqual(pcrInitTime)) 
            {
                numberSelect = 1;
                numberPointer = &initTime;
                line(4);
                Serial.print(*numberPointer);         
            }

            if (used.item.isEqual(pcrDenatTemp)) 
            {
                numberSelect = 1;
                numberPointer = &denat;
                line(4);
                Serial.print(*numberPointer);         
            }            
            
            if (used.item.isEqual(pcrDenatTime)) 
            {
                numberSelect = 1;
                numberPointer = &denatTime;
                line(4);
                Serial.print(*numberPointer);         
            }                
            
            if (used.item.isEqual(pcrAnnealTemp)) 
            {
                numberSelect = 1;
                numberPointer = &anneal;
                line(4);
                Serial.print(*numberPointer);         
            }    
            
            if (used.item.isEqual(pcrAnnealTime)) 
            {
                numberSelect = 1;
                numberPointer = &annealTime;
                line(4);
                Serial.print(*numberPointer);         
            } 
         
            if (used.item.isEqual(pcrElongTemp))
            {
                numberSelect = 1;
                numberPointer = &elong;
                line(4);
                Serial.print(*numberPointer);         
            }              
            
            if (used.item.isEqual(pcrElongTime)) 
            {
                numberSelect = 1;
                numberPointer = &elongTime;
                line(4);
                Serial.print(*numberPointer);         
            }              

            if (used.item.isEqual(pcrCycles)) 
            {
                numberSelect = 1;
                numberPointer = &cycles;
                line(4);
                Serial.print(*numberPointer);         
            } 

            if (used.item.isEqual(pcrHoldTemp)) 
            {
                numberSelect = 1;
                numberPointer = &hold;
                line(4);
                Serial.print(*numberPointer);         
            }  

            if (used.item.isEqual(pcrHoldTime)) 
            {
                numberSelect = 1;
                numberPointer = &holdTime;
                line(4);
                Serial.print(*numberPointer);         
            }  

            if (used.item.isEqual(pcrLidTemp)) 
            {
                numberSelect = 1;
                numberPointer = &heaterGoal;
                line(4);
                Serial.print(*numberPointer);         
            }  
            
            if (used.item.isEqual(pcrSettingsOK)) // BACK TO PCR
            {
                row = 2;
                line(3);
                Serial.write("          ");
                menu.moveLeft();
            }

        ////////////////////
        // Centrifuge Menu//
        ////////////////////
        if (used.item.isEqual(centRun)) //START CENTRIFUGE
        {
            centOn();
        }
        
        if (used.item.isEqual(centStop)) // STOP CENTRIFUGE
        {
            centOff();
        }
        
    
        /////////////////////////
        // Electrophoresis Menu//
        /////////////////////////
        if (used.item.isEqual(ledToggle)) // TOGGLE LEDs
        {
            ledsToggle();
        }

        
        // BACK TO MAIN
        if (used.item.isEqual(pcrBack) || used.item.isEqual(centBack) || used.item.isEqual(epBack)) 
        {
            row = 1;
            line(2);
            Serial.write("        ");
            menu.moveLeft();
        } 
}
// Menu item is changed
void menuChangeEvent(MenuChangeEvent changed)
{
        line(row);
        Serial.print(changed.to.getName());
}


//////////////////////
// Encoder Readings //
//////////////////////
void doEncoderA()
{
      delayMicroseconds(1000); 
      if( digitalRead(encoderA_pin) != A_set ) 
      {  
          A_set = !A_set;  
          // adjust counter + if A leads B
          if ( A_set && !B_set )
          {
          if(numberSelect){line(4);*numberPointer-=1;Serial.print(*numberPointer);}
          else{menu.moveDown();}
          }
      }
}
// Interrupt on B changing state, same as A above
void doEncoderB()
{
      delayMicroseconds(1000);
      if( digitalRead(encoderB_pin) != B_set ) 
      {
          B_set = !B_set;
          //  adjust counter - 1 if B leads A
          if( B_set && !A_set )
          {
          if(numberSelect){line(4);*numberPointer+=1;Serial.print(*numberPointer);}
          else{menu.moveUp();}
          }
      }
}

/////////////////////////////
// LCD Character positions //
/////////////////////////////
void line(int _x)
{ 
    if(_x==1){Serial.write(254);Serial.write(128);}  
    if(_x==2){Serial.write(254);Serial.write(194);}  
    if(_x==3){Serial.write(254);Serial.write(151);}
    if(_x==4){Serial.write(254);Serial.write(217);}
}
void screenClear(){Serial.write(254);Serial.write(1);}


////////////////////////
// HARDWARE FUCNTIONS //
////////////////////////
void peltHeat(){digitalWrite(peltOne_pin, HIGH);digitalWrite(peltTwo_pin, LOW);}

void peltCool(){digitalWrite(peltOne_pin, LOW);digitalWrite(peltTwo_pin, HIGH);}
void peltOff(){digitalWrite(peltOne_pin, LOW);digitalWrite(peltTwo_pin, LOW);}

void ledsToggle(){digitalWrite(leds_pin, !digitalRead(leds_pin));}

void centOn(){digitalWrite(motor_pin, HIGH);}
void centOff(){digitalWrite(motor_pin, LOW);}

void heaterOn(){digitalWrite(heater_pin, HIGH);}
void heaterOff(){digitalWrite(heater_pin, LOW);}


//////////////////////
// READ THERMISTORS //
//////////////////////
float peltRead()
{
    float _temp;
    
    int _reading = analogRead(peltSense_pin);
    
    float _R = 100000*(_reading/1023.)/(1 - (_reading/1023.));
    _temp = 4036. / log(_R/0.13214111618);
    return _temp - 273.15;
}

float heaterRead()
{
    float _temp;
    
    int _reading = analogRead(heaterSense_pin);
    
    float _R = 100000*(_reading/1023.)/(1 - (_reading/1023.));
    _temp = 4036 / log(_R/0.13214111618);
    return _temp - 273.15;
}


/////////////////////////////////////////////////////
// Timer interrupt for PCR and enter button at 5Hz //
/////////////////////////////////////////////////////
ISR(TIMER1_COMPA_vect)
{
    if(run || holding)
    {
      
        heaterTemperature = heaterRead();
        peltTemperature = peltRead();
        
        // Heated lid control
        if (heaterTemperature < heaterGoal - 0.5){heaterOn();}
        else{heaterOff();}
        
        // PCR control
        if (peltTemperature < peltGoal - 0.5){peltHeat();}
        else if (peltTemperature > peltGoal + 0.5){peltCool();}
        else {peltOff();}    

    }
    else
    {heaterOff();peltOff();}
  
    if (digitalRead(enter_pin) == LOW )  
    {
        if(numberSelect)
        {numberSelect=0;line(4);Serial.print("         ");}
        else{menu.use();}
    }
}

//////////////////////
// Boot Screen text //
//////////////////////
void sayHello()
{
      line(1);
      Serial.print("beta v0.02          ");
      line(2);
      Serial.print("Darwin says -");
      line(3);
      Serial.print("'Hello World!'");
      delay(1000);
      screenClear();
}

void printTemps()
{
        Serial.write(254);Serial.write(155);
        Serial.print(heaterTemperature);
        Serial.write(254);Serial.write(162);
        Serial.print(heaterGoal);
        Serial.write(254);Serial.write(219);
        Serial.print(peltTemperature);
        Serial.write(254);Serial.write(226);
        Serial.print(peltGoal);
}


///////////
// SETUP //
///////////
void setup()
{
      
          ////////////////////////////////
          // 5Hz intterupt timer set up //
          ////////////////////////////////    
          cli();//stop interrupts    
          //set timer1 interrupt at 5Hz
          TCCR1A = 0;// set entire TCCR1A register to 0
          TCCR1B = 0;// same for TCCR1B
          TCNT1  = 0;//initialize counter value to 0
          // set compare match register for 5hz increments
          OCR1A = 3124;// = (16*10^6) / (5*1024) - 1 (must be <65536)
          // turn on CTC mode
          TCCR1B |= (1 << WGM12);
          // Set CS10 and CS12 bits for 1024 prescaler
          TCCR1B |= (1 << CS12) | (1 << CS10);  
          // enable timer compare interrupt
          TIMSK1 |= (1 << OCIE1A);    
          sei();//allow interrupts
    
    
          //////////////////////////
          // Pin mode assignments //
          //////////////////////////
          pinMode(peltOne_pin, OUTPUT);
          pinMode(peltTwo_pin, OUTPUT);
          pinMode(leds_pin,OUTPUT);
          pinMode(heater_pin, OUTPUT);
          pinMode(motor_pin, OUTPUT);
          pinMode(peltSense_pin, INPUT);
          pinMode(heaterSense_pin, INPUT);
    
          pinMode(encoderA_pin, INPUT); 
          pinMode(encoderB_pin, INPUT); 
          pinMode(enter_pin, INPUT);

          digitalWrite(encoderA_pin, HIGH);
          digitalWrite(encoderB_pin, HIGH);
          digitalWrite(enter_pin, HIGH);


          Serial.begin(9600);
   
          screenClear();
          sayHello();
          line(1);
          menuSetup();

          // encoder pin on interrupt 0 (pin 2)
          attachInterrupt(0, doEncoderA, CHANGE);
          // encoder pin on interrupt 1 (pin 3)
          attachInterrupt(1, doEncoderB, CHANGE);
    

          /////////////////////////////
          // Initial variable values //
          /////////////////////////////
          numberSelect = 0;
          run = 0;
          holding = 0;
          startTime = 0;
          row = 1;
          A_set = false;              
          B_set = false;
    
          initTime = 300;
          denat = 94; 
          denatTime = 30;
          anneal = 50; 
          annealTime = 30;
          elong = 72; 
          elongTime = 120;
          cycles = 30;
          hold = 72; 
          holdTime = 300;
          
          heaterGoal = 105;
          peltGoal = denat;
          
          heaterTemperature = 0.;
          peltTemperature = 0.;
     
}

/////////////////
//  Main PCR Loop  //
/////////////////
void loop()
{    
    if(holding)
    {
      while(holding){peltGoal = hold;printTemps();delay(200);}      
    }
    
    if(run)
    {      
        peltGoal = denat;

        // Do nothing while the device is warming up
        while( run && ( (abs(peltTemperature-peltGoal)>1) || (abs(heaterTemperature-heaterGoal)>1) ) ){printTemps();delay(200);}
        
        startTime = millis();
        while ( run && ( millis() - startTime < initTime*1000 ) ){printTemps();delay(200);} // Initialisation/melting
        
        for(int i=0;i<cycles; i++)    // Cycling
        {
          peltGoal = denat;
          while(run && ( abs(peltTemperature-peltGoal)>1 ) ){printTemps();delay(200);}
          startTime = millis();
          while ( run && ( millis() - startTime < denatTime*1000 ) ){printTemps();delay(200);} // Denaturation
        
          peltGoal = anneal;
          while(run && ( abs(peltTemperature-peltGoal)>1 ) ){printTemps();delay(200);}
          startTime = millis();
          while ( run && ( millis() - startTime < annealTime*1000 ) ){printTemps();delay(200);} // Annealing
        
          peltGoal = elong;
          while(run && ( abs(peltTemperature-peltGoal)>1 ) ){printTemps();delay(200);}
          startTime = millis();
          while ( run && ( millis() - startTime < elongTime*1000 ) ){printTemps();delay(200);} // Elongation
        }    
        
        peltGoal = hold;
        while(run && ( abs(peltTemperature-peltGoal)>1 ) ){printTemps();delay(200);}
        startTime = millis();
        while ( run && ( millis() - startTime < holdTime*1000 ) ){printTemps();delay(200);} // Holding    
    }
    
    run = 0;
}
