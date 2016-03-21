// Ver 1.00
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUDP.h>
#include <Wire.h>
#include <Ticker.h>



#define  LOW    (0)
#define  HI     (!LOW)

#define  OACTIVE_LED  (13)
#define  IUDPSEL      (14)
#define  ONONEL       (12)
#define  ONONEL2      (16)

/* Set these to your desired credentials. */
const char APName[] = "********";
const char APPass[] = "12345678";      // 8 charctor
const unsigned int UDP_PORT = 65002;
#define  MAX_CONNECT     2
boolean UDPTCPMode;
boolean alreadyConnected[MAX_CONNECT] ;
uint8_t PacketBuff[256];
uint8_t Display[32];
uint16_t SendTemp;
uint16_t NowTemp;
boolean  TenFlag;
boolean  LedFlag;
uint16_t TenSec;
char     TempC[5];
uint16_t DispClearTimer;
boolean  DispSendReq;

WiFiUDP     Udp;
WiFiServer server(23);
WiFiClient client[MAX_CONNECT];
Ticker Timer10Ms;

const uint8_t INITIAL_TCP_DISP[] = {"WiFi TCP Server"};
const uint8_t INITIAL_UDP_DISP[] = {"WiFi UDP Server"};


////////////////////////////////////////////////////
uint8_t ToAscii(uint8 Nibble)
{
  switch ( Nibble & 0x0f)
  {
    case  0: return ('0');
    case  1: return ('1');
    case  2: return ('2');
    case  3: return ('3');
    case  4: return ('4');
    case  5: return ('5');
    case  6: return ('6');
    case  7: return ('7');
    case  8: return ('8');
    case  9: return ('9');
    case  0xa: return ('A');
    case  0xb: return ('B');
    case  0xc: return ('C');
    case  0xd: return ('D');
    case  0xe: return ('E');
    case  0xf: return ('F');
  }
}
//////////////////////////////////////////////////////////////
void DispHex( uint8_t *Buffer, uint8_t Hex)
{
  Buffer[0] = ToAscii(Hex >> 4);
  Buffer[1] = ToAscii(Hex );
}

//////////////////////////////////////////////////////////////
uint8_t I2cWrite(uint8 I2CAddr, uint8_t *Data, uint8_t Length)
{
  uint8_t i, result;
  Wire.beginTransmission(I2CAddr);
  for ( i = 0 ; i < Length ; i ++)
  {
    Wire.write(*Data);
    Data++;
  }
  result = Wire.endTransmission(false);
  delayMicroseconds(50);
  return result;
}

//////////////////////////////////////////////////////////////
uint8_t LcdCommand(uint8_t Cmd1)
{
  uint8_t Buffer[2];
  Buffer[0] = 0X00;
  Buffer[1] = Cmd1;
  I2cWrite( 0x3c  , Buffer, sizeof(Buffer));
}
//////////////////////////////////////////////////////////////
uint8_t LcdFunction(uint8_t Cmd1 , uint8_t Cmd2)
{
  uint8_t Buffer[2];
  Buffer[0] = Cmd1;
  Buffer[1] = Cmd2;
  I2cWrite( 0x3c , Buffer, sizeof(Buffer));
}
//////////////////////////////////////////////////////////////
void DisplayOut(void)
{
  uint8_t LcdBuffer[17];
  LcdCommand(0x80);

  memcpy(&LcdBuffer[1], &Display[0], 16);
  LcdBuffer[0] = 0x40;
  I2cWrite( 0x3c, LcdBuffer, sizeof(LcdBuffer));
  delay(20);

  LcdCommand(0xA0);
  memcpy(&LcdBuffer[1], &Display[16], 16);
  LcdBuffer[0] = 0x40;
  I2cWrite( 0x3c, LcdBuffer, sizeof(LcdBuffer));
  delay(20);

}
//////////////////////////////////////////////////////////////
void InitialElLcd(void)
{

  LcdCommand(0x2a);  // RE 1;
  LcdCommand(0x71);  // 内部レギュレータ停止
  LcdCommand(0x00);
  LcdCommand(0x28);  // RE 0;

  LcdCommand(0x08);  // Display OffSet

  LcdCommand(0x2a);  // RE 1;
  LcdCommand(0x79);  // SD 1;
  LcdCommand(0xD5);
  LcdCommand(0x70);
  LcdCommand(0x78);  // SD 1;
  LcdCommand(0x28);  // RE 0;

  LcdCommand( 0x01);   // Clear Display!
  LcdCommand( 0x02);   // Return Home
  LcdCommand( 0x0c);   // Display On
  LcdCommand( 0x01);   // Set Displaye

  delay(100);
}
/////// 10ms Base Timer //////////////////
/////////////////////////////////////////
void Int10ms( void )
{
  if (TenSec)
  {
    TenSec--;
  }
  else
  {
    TenSec = (10 * 100) - 1;
    TenFlag = true;
  }

  if(DispClearTimer )
  {
    if(--DispClearTimer == 0)
    {
         DispSendReq = true;
     }
  }
}
////////////////TEMP//////////////////////////////////////////
//////////////////////////////////////////////////////////////
void InitialTemp(void)
{
  uint8_t TEMP_CONFIG[2] = { 0X03 , 0X80 };  // EVENT STOP 10BIT;
  uint8_t TEMP_SAMPLE[2] = { 0X04 , 0X02 };  // 250ms
  I2cWrite( 0x70 , TEMP_CONFIG, sizeof(TEMP_CONFIG));
  I2cWrite( 0x70 , TEMP_SAMPLE, sizeof(TEMP_SAMPLE));
}
////////////////TEMP//////////////////////////////////////////
//////////////////////////////////////////////////////////////
uint8_t Nibble2Ascii ( uint8_t nible )
{
  return ( '0' + nible );
}
////////////////TEMP//////////////////////////////////////////
//////////////////////////////////////////////////////////////
#define TEMPI2CADR  ( 0X38 )
void TempRead(void)
{
  uint8_t Temp, dTemp;
  uint8_t TEMP_HIREAD   =  0X00 ;  // READ HI;
  uint8_t TEMP_LOWREAD  =  0X02 ;  // READ LOW

  I2cWrite( TEMPI2CADR, &TEMP_HIREAD , sizeof(TEMP_HIREAD  ));
  Wire.requestFrom( TEMPI2CADR, 1);
  Temp = Wire.read();    // HIGH.

  if (Temp & 0x80)  Display[16] = '-';
  else             Display[16] = '+';
  Temp &= 0x7f;
  Display[17] = TempC[0] = Nibble2Ascii((Temp / 10) % 10);
  Display[18] = TempC[1] = Nibble2Ascii(Temp % 10);
  Display[19] = TempC[2] = '.';

  I2cWrite( TEMPI2CADR , &TEMP_LOWREAD , sizeof(TEMP_LOWREAD  ));
  Wire.requestFrom(TEMPI2CADR, 1);
  Temp = Wire.read();  // LOW.
  dTemp = 0;
  if ( Temp & 0x080 ) dTemp += 50;
  if ( Temp & 0x040 ) dTemp += 25;
  if ( Temp & 0x020 ) dTemp += 12;
  if ( Temp & 0x010 ) dTemp +=  6;

  Display[20] = TempC[3] = Nibble2Ascii((dTemp / 10 ) % 10);
  Display[21] = TempC[4] = Nibble2Ascii(dTemp % 10);
  Display[22] = '\'';
  Display[23] = 'C';
  DisplayOut();
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
void setup()
{
  pinMode(IUDPSEL       , INPUT);
  pinMode(OACTIVE_LED   , OUTPUT);
  pinMode(ONONEL        , OUTPUT);
  pinMode(ONONEL2        , OUTPUT);
  Wire.begin(4, 5);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Program Start");

  WiFi.softAPConfig(IPAddress(192, 168, 0, 1),              // ip
                    IPAddress(192, 168, 0, 1),        // gateway
                    IPAddress(255, 255, 255, 0)  );  //sub net mask

  WiFi.softAP((char *)&APName[0] , APPass );
  InitialElLcd();
  InitialTemp();

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  delay(10);
  if (digitalRead(IUDPSEL))
  {
    UDPTCPMode = true;
    if (Udp.begin(UDP_PORT))
    {
      Serial.println("Udp OPen Success");

    }
    else
    {
      Serial.println("Udp Error");
    }

    Serial.print("Udp Port:");
    Serial.println(Udp.remotePort());

    memset(&Display, ' ', sizeof(Display));
    memcpy(Display, INITIAL_UDP_DISP, sizeof(INITIAL_UDP_DISP) - 1);
    DisplayOut();
  }
  else
  {
    UDPTCPMode = false;

    server.begin();
    Serial.println("\nServer Started.");
    memset(&Display, ' ', sizeof(Display));
    memcpy(Display, INITIAL_TCP_DISP, sizeof(INITIAL_TCP_DISP) - 1);
    DisplayOut();
  }
  Timer10Ms.attach(0.010f, Int10ms);
}
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
void TcpLoop(void)
{
  unsigned int i;
  boolean Acitve = false;
  for (i = 0; i < MAX_CONNECT; i++)
  {
    // wait for a new client:
    if (!client[i])
    {
      client[i] = server.available();
    }
    else
    {
      if (client[i].status() == CLOSED)
      {
        client[i].stop();
        Serial.print("\nConnection closed on client: "  );
        Serial.println(i);
        alreadyConnected[i] = false;
      }

      else
      {
        Acitve = true;
        if (!alreadyConnected[i]) // when the client sends the first byte, say hello:
        {
          Serial.print("Find Client:");
          Serial.println(i);
          client[i].println("Hello, client !");
          alreadyConnected[i] = true;
        }

        if (client[i].available())
        {

          String Str = client[i].readString();
          client[i].print(Str);
          Serial.println(Str);
          memset(&Display, ' ', sizeof(Display));
          uint8_t StrLength = Str.length();
          if ( StrLength > sizeof(Display)) StrLength = sizeof(Display);
          memcpy(Display, &Str[0], StrLength);
          DisplayOut();
        }
      }
    }
  }
  if (  Acitve  )  digitalWrite(OACTIVE_LED, HIGH);
  else              digitalWrite(OACTIVE_LED, LOW );

}
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
void UdpLoop(void)
{
  uint8_t i,ReadCount;
  if(DispSendReq)
  {
    DispSendReq = false;
    Display[25] = Display[26] = Display[27] = Display[28] = Display[29] = Display[30] = Display[31] =    ' ';
    DisplayOut();
  }
  
  ReadCount = Udp.parsePacket();
  if ( ReadCount  )
  {
    Udp.read(PacketBuff, ReadCount); // read the packet into the buffer
    Serial.print("IN:");
    Display[25] = Display[26] = Display[27] = Display[28] = Display[29] = Display[30] = Display[31] =    ' ';
    
    for( i = 0 ; i < ReadCount ; i ++ )
    {
        if( i < sizeof(Display) ) 
        {
          Display[25+i] = PacketBuff[i];
        }
        Serial.print( PacketBuff[i] );
    }
    DispClearTimer = 1000/10 ;
    DisplayOut();
    Serial.print("\n");
  }
  else
  {
    TempRead();
    if (TenFlag)
    {
      TenFlag = false;
      LedFlag = ~LedFlag;
      digitalWrite(OACTIVE_LED, LedFlag);
      Udp.beginPacket(IPAddress(192, 168, 0, 255), UDP_PORT);
      Udp.write(TempC);
      Udp.endPacket();
    }
  }
}
//////////////////////////////////////////////////////////////
void loop(void)
{
  if ( UDPTCPMode )  UdpLoop();
  else              TcpLoop();
}

