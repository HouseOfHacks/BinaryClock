// Sketch to display time in a BCD format.
// With encoder to set values. (encoder code adapted from https://bildr.org/2012/08/rotary-encoder-arduino/)

struct timeInfo
{
  long hours;
  long minutes;
  long seconds;
};

struct bcdValues 
{
  byte tens;
  byte units;
};

enum displayMode { clock, hrInput, minInput, secInput };

//these pins can not be changed 2/3 are special pins
const int encoderPin1 = 2;
const int encoderPin2 = 3;
const int encoderSwitchPin = 4; //push button switch

volatile byte lastEncoded = 0;
volatile long encoderValue = 0;

bool lastButtonState = false;
long lastShownValue;
long lastBinaryTime;

displayMode mode = clock;

int hours;
int minutes;
int seconds;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  pinMode(encoderSwitchPin, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
  digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on

  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);
}

void loop() 
{
  if (digitalRead(encoderSwitchPin))
  {
    //button is not being pushed
    if (lastButtonState)
    {
      lastButtonState = false;
    }
  } else {
    //button is being pushed
    if (!lastButtonState)
    {
      lastButtonState = true;
      switch(mode)
      {
        case clock:
          mode = hrInput;
          Serial.println("Hour adjust");
          break;
        case hrInput:
          mode = minInput;
          Serial.println("Minute adjust");
          break;
        case minInput:
          mode = secInput;
          Serial.println("Second adjust");
          break;
        case secInput:
          mode = clock;
          Serial.println("Clock display");
          break;
      }
    }
  }

  if (mode == clock)
  {
    auto currentTime = GetTime();
    auto binaryTime = ConvertTimeToBinary(currentTime);

    if (binaryTime != lastBinaryTime)
    {
      lastBinaryTime = binaryTime;
    
      Serial.print("Hours: "); Serial.print(currentTime.hours);
      Serial.print("  Minutes: "); Serial.print(currentTime.minutes);
      Serial.print("  Seconds: "); Serial.print(currentTime.seconds);

      Serial.print("  binary time: "); Serial.print(binaryTime, BIN);
      Serial.print("  Binary: ");

      DisplayTime(binaryTime);
    }
  } else {
      if (encoderValue != 0)
      {
    switch(mode)
    {
      case hrInput:
        hours += encoderValue;
        if (hours > 23)
          hours = 0;
        else if (hours < 0)
          hours = 23;
        Serial.println(hours);
        break;
      case minInput:
        minutes += encoderValue;
        if (minutes > 59)
          minutes = 0;
        else if (minutes < 0)
          minutes = 59;
        Serial.println(minutes);
        break;
      case secInput:
        seconds += encoderValue;
        if (seconds > 59)
          seconds = 0;
        else if (seconds < 0)
          seconds = 59;
        Serial.println(seconds);
        break;
    }
    encoderValue = 0;
      }
  }
}

const long msPerSecond = 1000;
const long msPerMinute = 60 * msPerSecond;
const long msPerHour = 60 * msPerMinute;

long lastUpdate;

// Gets the current time to display on the clock
timeInfo GetTime()
{
  auto msSinceStart = millis();
  if ((msSinceStart - lastUpdate) >= 1000)
  {
    lastUpdate = msSinceStart;
    seconds += 1;
    if (seconds > 59)
    {
      minutes += 1;
      seconds = 0;

      if (minutes > 59)
      {
        hours += 1;
        minutes = 0;

        if (hours > 23)
        {
          hours = 0;
        }
      }
    }
  }

  timeInfo result;
  result.hours = hours;
  result.minutes = minutes;
  result.seconds = seconds;
  
  return result;
}

// Packs timeInfo struct into a long int (32 bits)
// hour -> 2 bits for 10s, 4 bits for 1s
// minute -> 3 bits for 10s, 4 bits for 1s
// seconds -> 3 bits for 10s, 4 bits for 1s
// 20 bits used. upper MSB not used
// unused----------------------------- hr 2- hr 1------- min 2--- min 1------ sec 2--- sec 1------
// 31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0
long ConvertTimeToBinary(timeInfo currentTime)
{
  auto hrDigits = SplitDigits(currentTime.hours);
  auto minDigits = SplitDigits(currentTime.minutes);
  auto secDigits = SplitDigits(currentTime.seconds);

  return (long)hrDigits.tens << 18
    | hrDigits.units << 14
    | minDigits.tens << 11
    | minDigits.units << 7
    | secDigits.tens << 4
    | secDigits.units;
}

bcdValues SplitDigits(int valueToSplit)
{
  bcdValues result;
  result.tens = (byte)(valueToSplit / 10);
  result.units = (byte)(valueToSplit % 10);
  return result;
}

// Display the binaryTimeInfo on the hardware
// unused----------------------------- hr 2- hr 1------- min 2--- min 1------ sec 2--- sec 1------
// 31 30 29 28 27 26 25 24|23 22 21 20 19 18 17 16|15 14 13 12 11 10  9  8| 7  6  5  4  3  2  1  0

void DisplayTime(long binaryTime)
{
  for(auto bitMask = 0b100000000000000000000; bitMask != 0; bitMask = bitMask>>1)
  {
    Serial.print((binaryTime & bitMask) == bitMask ? "1 " : "0 ");
    if (bitMask == 0b1000000000000000000 // hr 2/hr 1
      || bitMask == 0b100000000000000    // hr 1/min 2
      || bitMask == 0b100000000000       // min 2/min 1
      || bitMask == 0b10000000           // min 1/sec 2
      || bitMask == 0b10000)             // sec 2/sec 1
      Serial.print("  ");
  }
  Serial.println();
}

void updateEncoder()
{
  int MSB = digitalRead(encoderPin1); // MSB = most significant bit
  int LSB = digitalRead(encoderPin2); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB; // converting the 2 pin value to single number
  int sum = (lastEncoded << 2) | encoded; // adding it to the previous encoded value
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) 
    encoderValue ++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) 
    encoderValue --; 
  lastEncoded = encoded; //store this value for next time 
}
