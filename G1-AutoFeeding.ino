const bool invertSensorReading = false; 

const int relayPin = 2;
const int ledPin = 3;
const int buzzerPin = 12;
const int sensorPin = 4;
const int feederPin = 9;

const int maxFeedingCicle = 3;
const int maxPauseCicle = 3;
const int feedingTime = 1000;
const int silentPauseTime = 5000;
const int alarmTime = 3000;
const int alarmPauseTime = 17000;
const int startingDelay = 10000;

unsigned long debounceDelay = 1000;
unsigned int debounceSample = 10;

int feedingCicle = 0;
int pauseCicle = 0;

bool lastFeederStatus = LOW;
bool lastSensorStatus = LOW;
unsigned long lastDebounceTimeFeeder = 0;
unsigned long lastDebounceTimeSensor = 0;

void setup(){
    pinMode(relayPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(sensorPin, INPUT);
    pinMode(feederPin, INPUT);

    digitalWrite(relayPin, LOW);
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    
    delay(startingDelay);
}

bool debouncedFeederStatus(){
    bool currentFeederStatus = digitalRead(feederPin);
    if (currentFeederStatus != lastFeederStatus) {
        lastDebounceTimeFeeder = millis();
        for(int i = 0; i < debounceSample; i++){
            delay(debounceDelay/debounceSample);
            if(currentFeederStatus != digitalRead(feederPin)) 
                return lastFeederStatus;
        }
        //update lastFeederStatus only after X consistent readings
        lastFeederStatus = currentFeederStatus;
    }
    return lastFeederStatus;
}

bool debouncedSensorStatus(){
    bool currentSensorStatus = invertSensorReading ? !digitalRead(sensorPin) : digitalRead(sensorPin);
    if (currentSensorStatus != lastSensorStatus) {
        lastDebounceTimeSensor = millis();
        for(int i = 0; i < debounceSample; i++){
            delay(debounceDelay/debounceSample);
            if(currentSensorStatus != invertSensorReading ? !digitalRead(sensorPin) : digitalRead(sensorPin)) 
                return lastSensorStatus;
        }
        //update lastSensorStatus only after X consistent readings
        lastSensorStatus = currentSensorStatus;
    }
    return lastSensorStatus;
}

void loop(){
    
    if(debouncedFeederStatus() == false || debouncedSensorStatus() == true) {
        digitalWrite(relayPin, LOW);
        digitalWrite(ledPin, LOW);
        feedingCicle = 0; 
        pauseCicle = 0; 
    }
    else {
        if(feedingCicle >= maxFeedingCicle) {
            feedingCicle = 0;
            digitalWrite(relayPin, LOW);
            digitalWrite(ledPin, LOW);
            if(pauseCicle >= maxPauseCicle) {
                pauseCicle = 0;
                digitalWrite(buzzerPin, HIGH);
                delay(alarmTime);
                digitalWrite(buzzerPin, LOW);
                delay(alarmPauseTime);
            }
            else {
                delay(silentPauseTime);
                pauseCicle ++;
            }
        }
        else {
            digitalWrite(relayPin, HIGH);
            digitalWrite(ledPin, HIGH);
            delay(feedingTime);
            feedingCicle ++; 
        }
    }
}