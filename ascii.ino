
enum Acmds { A_NONE, A_ST, A_RGB };
enum A_SM { SM_WAITING, SM_READING };

WiFiClient Aclient;

void modeAscii(void)
{
static char ALine[300];
static int idx = 0, SM_State=SM_WAITING;
int cmd = 0, rly, state;
uint R, G, B;
char *p;

  switch(SM_State) {
    case SM_WAITING:
      Aclient = tcpServer.available();   // listen for incoming clients
      if (Aclient) {
        SM_State = SM_READING;              // if you get a client
        Serial.println("New Ascii Client.");           // print a message out the serial port
      }
      break;
      
    case SM_READING:
      if(Aclient.connected()) {
        if (Aclient.available()) {             // if there's bytes to read from the client,
          char c = Aclient.read();             // read a byte, then
          ALine[idx++] = toupper(c);
          if (c == '\n') {                    // if the byte is a newline character
            ALine[idx] = 0;
  
            if(ALine[0] == 'S') {
              if(ALine[1] == 'T') cmd = A_ST;       // Status command
            }
            else if(ALine[0] == 'R') {
              if(ALine[1] == 'G') {
                if(ALine[2] == 'B') cmd = A_RGB;    // Led command
              }
            }

            switch(cmd) {
              case A_ST:
                Aclient.println();
                Aclient.println("Status:");
                Aclient.print("Firmware Version: ");
                Aclient.println(ver);
                Aclient.print("RSSI: ");
                Aclient.print(WiFi.RSSI());
                Aclient.println();
                idx = 0;
                break;
              case A_RGB:
                p = &ALine[4];
                p = skipWhite(p);
                R = getNumber(p);
                p = skipDigits(p);
                p = skipWhite(p);
                G = getNumber(p);
                p = skipDigits(p);
                p = skipWhite(p);
                B = getNumber(p);
                ledcWrite(ledChannelR, (R*256)/100);
                ledcWrite(ledChannelG, (G*256)/100);
                ledcWrite(ledChannelB, (B*256)/100);
                idx = 0;
                break;
              default:
                idx = 0;
                break;
            }
            cmd = 0;                            // don't leave old command in buffer.
        } // if newline
      }   // if client.available
    }     // if client.connected
    else {
      Aclient.stop();                              // close the connection:
      Serial.println("Client Disconnected.");   
      SM_State = SM_WAITING;
      return;    
    }
  }     // switch
}       // function


bool setRelay(char rly, char state) 
{
  return false;
}

char getRelay(char rly)
{
  return '?';
}
