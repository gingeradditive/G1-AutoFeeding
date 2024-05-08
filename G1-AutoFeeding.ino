#define relayPin 2
#define ledPin 3
#define buzzerPin 12
#define sensorPin 4
#define feederPin 9

const bool invertSensorReading = false; 

const int maxFeedingCicle = 3;
const int maxPauseCicle = 3;
const int feedingTime = 1000;
const int silentPauseTime = 5000;
const int alarmTime = 3000;
const int alarmPauseTime = 17000;
const int startingDelay = 10000;

unsigned long debounceDelayFeeder = 1000;
unsigned long debounceDelaySensor = 1000;
unsigned int debounceSample = 10;

int feedingCicle = 0;
int pauseCicle = 0;

bool lastFeederStatus = LOW;
bool lastSensorStatus = LOW;

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

    //Serial.begin(9600);
    //Serial.print("Starting Auto Feeding System");
}

bool debouncedFeederStatus(){
    bool currentFeederStatus = digitalRead(feederPin);
    if (currentFeederStatus != lastFeederStatus) {
        //Serial.print("Start Feed Debounce ");
        //Serial.print("| Waiting: ");
        for(int i = 0; i < debounceSample; i++){
            delay(debounceDelayFeeder/debounceSample);
            if(currentFeederStatus != digitalRead(feederPin)) {
                //Serial.println("\n| Debounce Failed");
                return lastFeederStatus;
            }
            //Serial.print(" . ");
        }
        //Serial.println("\n| Debounce Success");
        //update lastFeederStatus only after X consistent readings
        lastFeederStatus = currentFeederStatus;
    }
    return lastFeederStatus;
}

bool debouncedSensorStatus(){
    bool currentSensorStatus = invertSensorReading ? !digitalRead(sensorPin) : digitalRead(sensorPin);
    if (currentSensorStatus != lastSensorStatus) {
        //Serial.print("Start Sensor Debounce ");
        //Serial.print("| Waiting: ");
        for(int i = 0; i < debounceSample; i++){
            delay(debounceDelaySensor/debounceSample);
            if(currentSensorStatus != invertSensorReading ? !digitalRead(sensorPin) : digitalRead(sensorPin)) {
                //Serial.println("\n| Debounce Failed");
                return lastSensorStatus;
            }
            //Serial.print(" . ");
        }
        //Serial.println("\n| Debounce Success");
        //update lastSensorStatus only after X consistent readings
        lastSensorStatus = currentSensorStatus;
    }
    return lastSensorStatus;
}

void loop(){
    
    if(debouncedFeederStatus() == false || debouncedSensorStatus() == true) {
        digitalWrite(relayPin, LOW);
        digitalWrite(ledPin, LOW);
        //Serial.println("Feeder OFF");
        feedingCicle = 0; 
        pauseCicle = 0; 
    }
    else {
        if(feedingCicle >= maxFeedingCicle) {
            feedingCicle = 0;
            digitalWrite(relayPin, LOW);
            digitalWrite(ledPin, LOW);
            //Serial.println("Feeder OFF - Alarm");
            if(pauseCicle >= maxPauseCicle) {
                pauseCicle = 0;
                digitalWrite(buzzerPin, HIGH);
                delay(alarmTime);
                digitalWrite(buzzerPin, LOW);
                delay(alarmPauseTime);
                //Serial.println("Buzzer Alarm");
            }
            else {
                delay(silentPauseTime);
                pauseCicle ++;
            }
        }
        else {
            digitalWrite(relayPin, HIGH);
            digitalWrite(ledPin, HIGH);
            //Serial.println("Feeder ON");
            delay(feedingTime);
            feedingCicle ++; 
        }
    }
}
