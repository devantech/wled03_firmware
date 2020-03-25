
#include <Preferences.h>
#include <WiFi.h>

const char ver[] = {"1.0"};

#define BUFSIZE 50
#define Led 23

Preferences nvm;
WiFiServer server(80);
WiFiServer tcpServer(0);

IPAddress local_IP(192, 168, 0, 121);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

char buffer[BUFSIZE];
char ssid[BUFSIZE];
char password[BUFSIZE];
char password2[BUFSIZE];
uint AsciiPort;


const int ledPinR = 5;
const int ledPinB = 16;  // 16 corresponds to GPIO16
const int ledPinG = 17;
const int freq = 5000;
const int ledChannelR = 0;
const int ledChannelG = 1;
const int ledChannelB = 2;
const int resolution = 8;

void setup() {

  ledcSetup(ledChannelR, freq, resolution);
  ledcSetup(ledChannelG, freq, resolution);
  ledcSetup(ledChannelB, freq, resolution);
  ledcAttachPin(ledPinR, ledChannelR);
  ledcAttachPin(ledPinG, ledChannelG);
  ledcAttachPin(ledPinB, ledChannelB);
  ledcWrite(ledChannelR, 0);
  ledcWrite(ledChannelG, 0);
  ledcWrite(ledChannelB, 0);

  pinMode(Led, OUTPUT);
  digitalWrite(Led, HIGH);

  Serial.begin(115200);
  delay(10);

  wifi_connect();
}

void loop() {
//  modeHttp();
  modeAscii();
  serialMonitor();
  if(WiFi.status() != WL_CONNECTED) wifi_connect();

}

void wifi_connect(void)
{
  unsigned int x;

  digitalWrite(Led, HIGH);
  Serial.println("");
  nvm.begin("devantech", false);    // Note: Namespace name is limited to 15 chars
  local_IP = nvm.getUInt("IPAddress", 0);
  gateway = nvm.getUInt("GateWay", 0);
  subnet = nvm.getUInt("SubNet", 0);
  primaryDNS = nvm.getUInt("primaryDNS", 0);
  secondaryDNS = nvm.getUInt("secondaryDNS", 0);
  nvm.getString("ssid", ssid, sizeof(ssid) - 1);
  nvm.getString("password", password, sizeof(password) - 1);
  strcpy(password2, "********");
  AsciiPort = nvm.getUInt("AsciiPort", 17123);

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println("...");

  if (local_IP != 0) {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }
  }
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid, password);
    for (x = 0; x < 300; x++) {
      delay(10);
      serialMonitor();
    }
  }
  digitalWrite(Led, LOW);
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  tcpServer.close();
  tcpServer = WiFiServer(AsciiPort);
  tcpServer.begin();
}
