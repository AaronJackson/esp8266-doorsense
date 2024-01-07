#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

#define SENSOR 0

#define DOOR_OPEN 1
#define DOOR_CLOSED 0

#define MESSAGE "Metalworking Firedoor state changed to "
#define PUBLISH_TOPIC "nh/irc/tx"

#define STATUS_TOPIC "nh/status"

#define NAME "MetalworkingFiredoor"

static int last_state = DOOR_OPEN;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = NAME;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(STATUS_TOPIC "/req");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, STATUS_TOPIC "/req") == 0 && memcmp(payload, "STATUS", length) == 0) {
    client.publish(STATUS_TOPIC "/res", "Running: " NAME);
  }
}

void setup() {
  pinMode(SENSOR, INPUT_PULLUP);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  last_state = digitalRead(SENSOR);
}

void publish_state(int state) {
  const char *strstate = (state == DOOR_OPEN) ? MESSAGE "OPEN" : MESSAGE "CLOSED";
  Serial.print("Publish message: ");
  Serial.println(strstate);
  client.publish(PUBLISH_TOPIC, strstate);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  const int state = digitalRead(SENSOR);
  if (state != last_state) {
    publish_state(state);
    delay(100);
    last_state = state;
  }
}
