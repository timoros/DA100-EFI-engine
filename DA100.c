/*******************************
* Project name:
	DA100
* Description:
	This code demostrates how to use DA100 EFI engine
* Programmer:
	Pye Sone Aung Myint Htay (Myanmar)
* Version:
	1.0.0
* MCU:
	PIC16F887
********************************/

// LCD module connections
sbit LCD_RS at RB4_bit;
sbit LCD_EN at RB5_bit;
sbit LCD_D4 at RB0_bit;
sbit LCD_D5 at RB1_bit;
sbit LCD_D6 at RB2_bit;
sbit LCD_D7 at RB3_bit;

sbit LCD_RS_Direction at TRISB4_bit;
sbit LCD_EN_Direction at TRISB5_bit;
sbit LCD_D4_Direction at TRISB0_bit;
sbit LCD_D5_Direction at TRISB1_bit;
sbit LCD_D6_Direction at TRISB2_bit;
sbit LCD_D7_Direction at TRISB3_bit;
// End LCD module connections
char data_buff;
char mode=0,data_mode=0;            //data case
char SYNC_0=0xA5;
char SYNC_1=0x5A;
char Serial_Number_1=0x04;
char Serial_Number_0=0x77;
char data_array[26];    //Data array based on Message length
unsigned int Serial_Number=0;  // serial number
unsigned short Packet_Type=0xFF,Size=0,ThrottlePos=0,ThrottlePos_O=0, RPMCmd=0,RPMCmd_O=0;
unsigned int ThrottlePulse=0,ThrottlePulse_O=0,RPM=0,RPM_O=0;
unsigned short ThrottlePulseSource=0,ThrottlePulseSource_O=0;
unsigned short CHTA=0,CHTAO=0;
unsigned short Ttatus=0,Ttatus_O=0;
unsigned int Baro=0, Baro_O=0;
unsigned short MAP_Ratio=0,MAP_Ratio_O=0;
short IAT=0,IAT_O=0;
unsigned int FuelPress=0,FuelPress_O=0;
unsigned long FuelUsed=0, FuelUsed_O=0,EngineTime=0, EngineTime_O=0;
unsigned short InputVoltage=0, InputVoltage_O=0;
unsigned int CustomerID=0,CustomerID_O=0,CS=0,CS_C=0;
unsigned short c0=0,c1=0;
unsigned int count=0;             // data array shifting counter
bit data_true;
char txt[4];                      // for lcd display

void fletcher_encode(unsigned short buff);

void UART() iv 0x0004 ics ICS_AUTO {
             data_buff=UART1_read();

             switch (mode)
             {
                   case 0:                 // 0xA5
                    {
                        CS=0;
                        count = 0;
                        if(data_buff==SYNC_0)
                        {
                           mode=1;
                           fletcher_encode(data_buff);
                        }
                        else
                        {
                             mode=0;
                        }
                        break;
                    }
                   case 1:                 // 0x5A
                   {
                        if(data_buff==SYNC_1)
                        {
                           mode=2;
                           fletcher_encode(data_buff);
                        }
                        else
                        {
                             mode=0;
                        }
                        break;
                   }
                   case 2:                  //Serial_Number
                   {
                         if(data_buff==Serial_Number_1)
                        {
                           mode=3;
                           fletcher_encode(data_buff);
                        }
                        else
                        {
                             mode=0;
                        }
                        break;
                   }

                   case 3:                  //Serial_Number
                   {
                         if(data_buff==Serial_Number_0)
                        {
                           mode=4;
                           fletcher_encode(data_buff);
                        }
                        else
                        {
                             mode=0;
                        }
                        break;
                   }

                   case 4:                  //Packet_type
                   {
                        fletcher_encode(data_buff);
                        Packet_Type=data_buff;
                        mode=5;
                        break;
                   }
                   case 5:
                   {

                        fletcher_encode(data_buff);
                        Size=data_buff;
                        mode=6;
                        break;
                   }
                   case 6:              //enter with count=6
                   {
                            data_array[count]=data_buff;
                            count++;
                            if(count<=24)
                              fletcher_encode(data_buff);

                            switch(count)
                            {
                                         case 1:
                                         {
                                          ThrottlePos=data_array[count-1]/2.55;
                                          break;
                                         }
                                         case 2:
                                         {
                                          RPMCmd=data_array[count-1]*50;
                                          break;
                                         }
                                         case 4:
                                         {

                                          ThrottlePulseSource=data_array[count-2]>>4;
                                          ThrottlePulse=data_array[count-2]& 0x0F;
                                          ThrottlePulse=ThrottlePulse<<8;
                                          ThrottlePulse=ThrottlePulse+data_array[count-1];
                                          break;
                                         }
                                         case 6:
                                         {
                                          RPM=data_array[count-2]<<8;
                                          RPM=(RPM+data_array[count-1])*50;
                                          break;
                                         }
                                         case 7:
                                         {
                                          CHTA=data_array[count-1]-10;
                                          break;
                                         }
                                         case 8:
                                         {
                                          Status=data_array[count-1];
                                          break;
                                         }
                                         case 10:
                                         {
                                          Baro=data_array[count-2]<<8;
                                          Baro=(Baro+data_array[count-1])*2;
                                          break;
                                         }
                                         case 11:
                                         {
                                          MAP_Ratio=data_array[count-1];
                                          break;
                                         }
                                         case 12:
                                         {
                                          IAT=data_array[count-1];
                                          break;
                                         }
                                         case 14:
                                         {
                                          FuelPress=data_array[count-2]<<8;
                                          FuelPress=(FuelPress+data_array[count-1])*20;
                                          break;
                                         }
                                         case 18:
                                         {
                                          FuelUsed=data_array[count-4]<<24;
                                          FuelUsed=FuelUsed+(data_array[count-3]<<16);
                                          FuelUsed=FuelUsed+(data_array[count-2]<<8);
                                          FuelUsed=FuelUsed+data_array[count-1];
                                          break;
                                         }
                                         case 21:
                                         {
                                          EngineTime=(data_array[count-3]<<16);
                                          EngineTime=EngineTime+(data_array[count-2]<<8);
                                          EngineTime=(EngineTime+data_array[count-1])*60;
                                          break;
                                         }
                                         case 22:
                                         {
                                          InputVoltage=(data_array[count-1]);
                                          break;
                                         }
                                         case 24:
                                         {
                                          CustomerID=(data_array[count-2]<<8);
                                          CustomerID=CustomerID+data_array[count-1];
                                          CS_C=(unsigned short)(c0-c1);
                                          CS_C=CS_C<<8;
                                          CS_C|=(unsigned short)(c1-2*c0);
                                          break;
                                         }
                                         case 26:
                                         {
                                          CS=(data_array[count-2]<<8);
                                          CS=CS+data_array[count-1];
                                          if(CS==CS_C)
                                          {
                                            PORTB.F6=~PORTB.F6;
                                            ThrottlePos_O=ThrottlePOS;
                                            RPMCmd_O=RPMCmd;
                                            ThrottlePulse_O=ThrottlePulse;
                                            RPM_O=RPM;
                                            CHTAO=CHTA;
                                            Ttatus_O=Ttatus;
                                            Baro_O=Baro;
                                            MAP_Ratio_O=MAP_Ratio;
                                            IAT_O=IAT;
                                            FuelPress_O=FuelPress;
                                            FuelUsed_O=FuelUsed;
                                            EngineTime_O=EngineTime;
                                            InputVoltage_O=InputVoltage;
                                            CustomerID_O=CustomerID;
                                          }
                                          count=0;
                                          mode=0;
                                          break;
                                         }
                            }
                            break;
                   }

             }
             PIR1.RCIF=0;
}

void main(){
  ANSEL  = 0;                        // Configure AN pins as digital I/O
  ANSELH = 0;
  C1ON_bit = 0;                      // Disable comparators
  C2ON_bit = 0;
  OSCCON=0b01110001;
  INTCON.F7=1;
  INTCON.F6=1;
  PIE1.F5=1;
  PIR1.RCIF=0;
  TRISB.F6=0;
  PORTB.F6=0;
  UART1_Init(56000);
  Lcd_Init();                        // Initialize LCD
  Lcd_Cmd(_LCD_CLEAR);               // Clear display
  Lcd_Cmd(_LCD_CURSOR_OFF);          // Cursor off*/
  while(1)
  {

   //Lcd_Out(2,1,"TESTING");
   ByteToStr(MAP_Ratio_O,txt);
   Lcd_Out(1,1,txt);                 // Write text in first row
   ByteToStr(InputVoltage_O,txt);
   Lcd_Out(2,1,txt);                 // Write text in second row
   Delay_ms(2000);

  }
}

void fletcher_encode(unsigned short buff)
{
        c0=(unsigned short)(c0+ buff);
        c1=(unsigned short)(c1+c0);
}//fletcher _encode
