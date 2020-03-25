
enum cmds { CMD_NONE, CMD_ST, CMD_RB, CMD_IP, CMD_SB, CMD_GW, CMD_PD, CMD_SD, CMD_SS, CMD_PW, CMD_PA, 
            CMD_RGB
          };

uint idx;

void serialMonitor(void)
{
IPAddress IP, IPZ;
char *p;
uint port;
uint R, G, B;

  if(getMLine()) {
    int cmd = getCommand();
    switch(cmd) {
      case CMD_NONE:
        Serial.println("Unknown command:");
        Serial.println(buffer);
        break;
      case CMD_ST:
        Serial.println();
        Serial.println("Status:");
        Serial.print("Firmware Version: ");
        Serial.println(ver);
        Serial.print("IP: ");
        IP = nvm.getUInt("IPAddress", 0);
        Serial.print(IP.toString());
        IPZ.fromString("0.0.0.0");
        if( IP==IPZ ) {
          Serial.print(" (");
          Serial.print(WiFi.localIP());
          Serial.print(")");
        }
        Serial.println();
        Serial.print("Subnet: ");
        IP = nvm.getUInt("SubNet", 0);
        Serial.println(IP.toString());
        Serial.print("Gateway: ");
        IP = nvm.getUInt("GateWay", 0);
        Serial.println(IP.toString());
        Serial.print("Primary DNS: ");
        IP = nvm.getUInt("primaryDNS", 0);
        Serial.println(IP.toString());
        Serial.print("Secondary DNS: ");
        IP = nvm.getUInt("secondaryDNS", 0);
        Serial.println(IP.toString());
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(password2);
        Serial.print("ASCII TCP Port: ");
        Serial.println(AsciiPort);
        Serial.print("RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println();
        break;
      case CMD_RB:
        Serial.println("Re-Booting.. .");
        delay(500);
        esp_restart();
        break;       
      case CMD_IP:
        p = getStrPtr(&buffer[3]);
        if(p) {
          IP = getIP(p);
          nvm.putUInt("IPAddress", IP);
          Serial.print(" OK. Saved IP Address: "); 
          Serial.println(IP.toString());
        }
        break;
      case CMD_SB:
        p = getStrPtr(&buffer[3]);
        if(p) {
          IP = getIP(p);
          nvm.putUInt("SubNet", IP);
          Serial.print("OK. Saved Subnet Mask: "); 
          Serial.println(IP.toString());
        }
        break;
      case CMD_GW:
        p = getStrPtr(&buffer[3]);
        if(p) {
          IP = getIP(p);
          nvm.putUInt("GateWay", IP);
          Serial.print("OK. Saved Gateway Address: "); 
          Serial.println(IP.toString());
        }
        break;
      case CMD_PD:
        p = getStrPtr(&buffer[3]);
        if(p) {
          IP = getIP(p);
          nvm.putUInt("primaryDNS", IP);
          Serial.print("OK. Saved Primary DNS: "); 
          Serial.println(IP.toString());
        }
        break;
      case CMD_SD:
        p = getStrPtr(&buffer[3]);
        if(p) {
          IP = getIP(p);
          nvm.putUInt("secondaryDNS", IP);
          Serial.print("OK. Saved Secondary DNS: "); 
          Serial.println(IP.toString());
        }
        break;
      case CMD_SS:
        p = getStrPtr(&buffer[3]);
        if(p) {
          nvm.putString("ssid", p);
          nvm.getString("ssid", ssid, sizeof(ssid)-1);
          Serial.print("OK. Saved SSID: "); 
          Serial.println(ssid);    
          WiFi.disconnect();
        }
        else Serial.println("SSID string not found");    
        break;
      case CMD_PW:
        p = getStrPtr(&buffer[3]);
        if(p) {
          nvm.putString("password", p);
          nvm.getString("password", password, sizeof(password)-1);
          strcpy(password2, password);
          Serial.print("OK. Saved Password: "); 
          Serial.println(password2);    
          WiFi.disconnect();
        }
        else Serial.println("Password string not found");    
        break;
      case CMD_PA:
//        AsciiPort = getNumber(&buffer[3]);
        nvm.putUInt("AsciiPort", AsciiPort);
        Serial.print("OK. Saved ASCII port number: "); 
        Serial.println(AsciiPort);  
        tcpServer.close();
        tcpServer = WiFiServer(AsciiPort);
        tcpServer.begin();  
        break;
      case CMD_RGB:
        char *p = &buffer[4];
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
        break;
    }
  }
}


int getIP(char* p)
{
char *p1;
IPAddress IP;

   p = skipWhite(p);
   p1 = p;
   while( (*p >= '0' && *p <= '9') || (*p == '.') ) ++p;
   *p = 0;
   IP.fromString(p1);
   return IP;
}

int getNumber(char *p)
{
  p = skipWhite(p);
  return atoi(p);
}

char* skipWhite(char *p)
{
  while(isspace(*p)) ++p;  
  return p;
}

char* skipDigits(char *p)
{
  while(isdigit(*p)) ++p;  
  return p;
}

char * getStrPtr(char *p)
{
char *p1;

  p = skipWhite(p);
  if(*p!='"') return 0;
  p1 = ++p; 
  while(*p && (*p!='"') ) ++p;
  if(*p=='"') {
    *p = 0;
    return p1;
  }
  return 0;
}

bool getMLine(void)
{
static int idx = 0;
char c;

  if(Serial.available()) {
    c = Serial.read();
    Serial.write(c);
    if( (c == '\n') || (c == '\r') )
    {
      if(idx == 0) return false;    // ignore empty lines
    }
    buffer[idx++] = c;
    if(idx == sizeof(buffer)-1) {
      idx = 0;                    // we overflowed the buffer, just start over.
      return false;
    }
    if( (c == '\n') || (c == '\r') )
    {
      buffer[idx] = '\0';
      idx = 0;
      return true;
    }
  }
  return false;
}

int getCommand()
{
  if(toupper(buffer[0]) == 'S') {
    if(toupper(buffer[1]) == 'T') return CMD_ST; 
    if(toupper(buffer[1]) == 'B') return CMD_SB; 
    if(toupper(buffer[1]) == 'D') return CMD_SD; 
    if(toupper(buffer[1]) == 'S') return CMD_SS; 
  }
  else if(toupper(buffer[0]) == 'I') {
    if(toupper(buffer[1]) == 'P') return CMD_IP; 
  }
  else if(toupper(buffer[0]) == 'G') {
    if(toupper(buffer[1]) == 'W') return CMD_GW; 
  }
  else if(toupper(buffer[0]) == 'P') {
    if(toupper(buffer[1]) == 'D') return CMD_PD; 
    if(toupper(buffer[1]) == 'W') return CMD_PW; 
    if(toupper(buffer[1]) == 'A') return CMD_PA; 
  }  
  else if(toupper(buffer[0]) == 'R') {
    if(toupper(buffer[1]) == 'B') return CMD_RB; // ReBoot
    if(toupper(buffer[1]) == 'G') {
       if(toupper(buffer[2]) == 'B') return CMD_RGB;
    }
  }   
  return CMD_NONE;
}
