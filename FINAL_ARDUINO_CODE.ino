#define BLYNK_PRINT SwSerial

#include <SoftwareSerial.h>
SoftwareSerial SwSerial(10, 11); // RX, TX

#include <BlynkSimpleStream.h>

char auth[] = "<AUTHENTICATION TOKEN>";

//CONTROL SYSTEM
int t=0;
int P=0;

int h=75;        //Analog HIGH
int l=0;         //Analog LOW

//ARDUINO
#define POT A0
#define GA A1   //GRID SUPPLY
#define IA A2   //INVERTER SUPPLY
#define BA A3   //BATTERY CHARGING

#define Gr 13
#define Gg 10
#define Ir 9
#define Ig 8
#define Br 7
#define Bg 6
#define Tr 5
#define Tg 4
#define Lr 3
#define Lg 2

//APP

int peak_start = 18;
int peak_stop = 22;
int offpeak_start = 22;
int offpeak_stop = 06;

#define BL V3               //BATTERY LEVEL
#define PERIOD_INPUT V10    //PERIOD DATA
#define TIME_INPUT V11      //TIME DATA

WidgetLED ledG(V0);         //GRID SUPPLY 
WidgetLED ledI(V1);         //INVERTER SUPPLY
WidgetLED ledB(V2);         //BATTERY CHARGING
WidgetLED ledP(V6);         //TIME OF DAY : PEAK
WidgetLED ledS(V7);         //TIME OF DAY : SHOULDER
WidgetLED ledO(V8);         //TIME OF DAY : OFF PEAK
BlynkTimer timer;

void mode1()//I -> H
{
    //RELAY
    digitalWrite(GA,HIGH);  //OFF
    digitalWrite(IA,LOW);   //ON
    digitalWrite(BA,HIGH);  //OFF
    
    //LED
    analogWrite(Gr,h);
    analogWrite(Gg,l);
    analogWrite(Ir,0);
    digitalWrite(Ig, HIGH);
    analogWrite(Br,h);
    analogWrite(Bg,l);
    analogWrite(Tr,h);       //TIME OF DAY : PEAK
    analogWrite(Tg,l);
    analogWrite(Lr,0);      //LOAD LOW
    digitalWrite(Lg, HIGH);
}

void mode2()//G&I -> H
{
    //RELAY
    digitalWrite(GA,LOW);   //ON
    digitalWrite(IA,LOW);   //ON
    digitalWrite(BA,HIGH);  //OFF
    
    //LED
    analogWrite(Gr,0);
    analogWrite(Gg,h);
    analogWrite(Ir,0);
    digitalWrite(Ig, HIGH);
    analogWrite(Br,h);
    analogWrite(Bg,l);
    analogWrite(Tr,h);    //TIME OF DAY : PEAK
    analogWrite(Tg,l);
    analogWrite(Lr,h);    //LOAD HIGH
    analogWrite(Lg,l);
}

void mode3()//G -> B & H
{
    //RELAY
    digitalWrite(GA,LOW);   //ON
    digitalWrite(IA,HIGH);  //OFF
    digitalWrite(BA,LOW);   //ON
    
    //LED
    analogWrite(Gr,0);
    analogWrite(Gg,h);
    analogWrite(Ir,h);
    analogWrite(Ig,l);
    analogWrite(Br,0);
    analogWrite(Bg,h);
    analogWrite(Tr,0);      //TIME OF DAY : OFF PEAK
    digitalWrite(Tg, HIGH);
    analogWrite(Lr,h);      //LOAD NA
    digitalWrite(Lg, HIGH);
}

void mode4()//G -> H
{
    //RELAY
    digitalWrite(GA,LOW);   //ON
    digitalWrite(IA,HIGH);  //OFF
    digitalWrite(BA,HIGH);  //OFF
    
    //LED
    analogWrite(Gr,0);
    analogWrite(Gg,h);
    analogWrite(Ir,h);
    analogWrite(Ig,l);
    analogWrite(Br,h);
    analogWrite(Bg,l);
    analogWrite(Tr,h);     //TIME OF DAY : SHOULDER
    digitalWrite(Tg, HIGH);
    analogWrite(Lr,h);     //LOAD NA
    digitalWrite(Lg, HIGH);
}

// DATA INPUT FROM APP FOR PEAK AND OFFPEAK TIME
// BLYNK_WRITE(PERIOD_INPUT)
// {
//     int period = param.asInt();
//     switch(period)
//     {
//         case 1:
//         BLYNK_WRITE(TIME_INPUT)
//         {
//             TimeInputParam t(param);
//             peak_start = t.getStartHour()+;    //18
//             peak_stop = t.getStopHour();       //22
//         }

//         case 2:
//         BLYNK_WRITE(TIME_INPUT)
//         {
//             TimeInputParam t(param);
//             offpeak_start = t.getStartHour(); //22
//             offpeak_stop = t.getStopHour();   //06
//         }
//     }
// }

void myTimerEvent()
{
    P = map(analogRead(POT), 0, 1024, 3000, 0); //LOAD VALUE

    if(t<offpeak_stop){                 //OFF PEAK (6)
        mode3();

        Blynk.virtualWrite(BL,i);
        ledG.setValue(255);
        ledI.setValue(0);
        ledB.setValue(255);
        ledP.off();
        ledS.off();
        ledO.on();
    }
    else if(t<peak_start){             //SHOULDER (18)
        mode4();
        ledG.setValue(255);
        ledI.setValue(0);
        ledB.setValue(0);
        ledP.off();
        ledS.on();
        ledO.off();
    }
    else if(t<peak_stop){             //PEAK (22)

        Blynk.virtualWrite(BL,i);
        ledP.on();
        ledS.off();
        ledO.off();

        if(P <2000)                  //LOW LOAD
        {
            mode1();
            ledG.setValue(0);
            ledI.setValue(255);
            ledB.setValue(0);
        }
        else                        //HIGH LOAD
        {
            mode2();
            ledG.setValue(255);
            ledI.setValue(255);
            ledB.setValue(0);
        }
    }
    else{                           //OFF PEAK
        mode3();
        
        Blynk.virtualWrite(BL,i);
        ledG.setValue(255);
        ledI.setValue(0);
        ledB.setValue(255);
        ledP.off();
        ledS.off();
        ledO.on();
        if(t==24)   //RESET t at 24
        {
            t=0;
        }
    }     
    t=t+1;
    Blynk.virtualWrite(V5,P);   //upload LOAD
}

void setup()
{
    SwSerial.begin(9600);
    pinMode(POT, INPUT);
    pinMode(IA, OUTPUT);
    pinMode(BA, OUTPUT);
    pinMode(GA, OUTPUT);

    pinMode(Gr, OUTPUT);  pinMode(Gg, OUTPUT);
    pinMode(Ir, OUTPUT);  pinMode(Ig, OUTPUT);
    pinMode(Br, OUTPUT);  pinMode(Bg, OUTPUT);

    Serial.begin(9600);
    Blynk.begin(Serial, auth);

    timer.setInterval(1000L, myTimerEvent);   //TIMER INTERVEAL
}

void loop()
{
  Blynk.run();
  timer.run(); // Initiates Timer
}
