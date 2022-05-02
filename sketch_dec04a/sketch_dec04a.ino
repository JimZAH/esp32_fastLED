#include <WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>

#define BRIGHTNESS  150
#define COOLING  55
#define DATA_PIN 23
#define FRAMES_PER_SECOND 60
#define NUM_LEDS 50
#define SPARKING 120
#define UPDATES_PER_SECOND 100
#define STATUS_LED 12

bool gReverseDirection = false;
bool led_state = false;


CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

const char* ssid = "";
const char* password = "";
const char* mqtt_broker = "10.145.0.4";

const char *topic = "tele/ring";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

int cmd = 0;
int brightness = BRIGHTNESS;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(STATUS_LED,OUTPUT);
 FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
 FastLED.setBrightness( BRIGHTNESS );
 // Set software serial baud to 115200;
 Serial.begin(115200);
 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
         digitalWrite(STATUS_LED,HIGH);
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  cmd = payload[0];
  brightness = payload[1];
}

void chase(){
  for (int i = 0; i < 50; i++){
    leds[i] = CRGB::Red;
    leds[i+5] = CRGB::Green;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
    leds[i+5] = CRGB::Black;
    FastLED.show();
  }
  for (int j = 50; j > 0; j--){
    leds[j] = CRGB::Green;
    leds[j+5] = CRGB::Blue;
    FastLED.show();
    delay(100);
    leds[j] = CRGB::Black;
    leds[j+5] = CRGB::Black;
    FastLED.show();
    }
    for (int i = 0; i < 50; i++){
    leds[i] = CRGB::Blue;
    leds[i+5] = CRGB::Red;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
    leds[i+5] = CRGB::Black;
    FastLED.show();
    }
}

void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = BRIGHTNESS;
    
    for( int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; ++i) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

void greenred(){
  for (int i = 0; i < 50; i++){
    leds[i-6] = CRGB::Black;
    leds[i-9] = CRGB::Black;
    leds[i] = CRGB::Red;
    leds[i+5] = CRGB::Green;
    FastLED.show();
    delay(250);
    if (i > 7){
      leds[i-5] = CRGB::Green;
      leds[i-8] = CRGB::Red;
    }
    leds[i] = CRGB::Black;
    leds[i+5] = CRGB::Black;
    FastLED.show();
  }
}

void grey(){
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Gray;
  }
  FastLED.show();
}





void loop() {

  if (WiFi.status() != WL_CONNECTED) {
     digitalWrite(STATUS_LED,LOW);
     WiFi.disconnect();
     delay(3000);
     WiFi.begin(ssid, password);
     delay(3000);
     digitalWrite(STATUS_LED,HIGH);
 }
 
 client.loop();
 
 switch (cmd){
  case 48:
  if (led_state){
    FastLED.clear();
    FastLED.show();
    led_state = false; 
  }
  break;
  case 49:
  if (!led_state){
    grey();
    led_state = true; 
  }
  break;
  case 64:
  if (FastLED.getBrightness() != brightness){
    if (brightness < 1 || brightness > BRIGHTNESS) {
      brightness = BRIGHTNESS;
      }
    FastLED.setBrightness( brightness );
  }
  break;
  case 65:
  led_state = true; 
  chase();
  break;
  case 66:
  led_state = true;
  Fire2012();
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
  break;
  case 67:
  led_state = true;
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  ChangePalettePeriodically();
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */ 
  FillLEDsFromPaletteColors( startIndex); 
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
  break;
  case 68:
  led_state = true;
  greenred();
  break;
 }
}
