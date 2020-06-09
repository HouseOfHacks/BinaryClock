// Sketch to display time in a BCD format.

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

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() 
{
  auto currentTime = GetTime();

  Serial.print("Hours: "); Serial.print(currentTime.hours);
  Serial.print("  Minutes: "); Serial.print(currentTime.minutes);
  Serial.print("  Seconds: "); Serial.print(currentTime.seconds);
  
  auto binaryTime = ConvertTimeToBinary(currentTime);

  Serial.print("  binary time: "); Serial.print(binaryTime, BIN);
  Serial.print("  Binary: ");

  DisplayTime(binaryTime);
}

const long msPerSecond = 1000;
const long msPerMinute = 60 * msPerSecond;
const long msPerHour = 60 * msPerMinute;

// Gets the current time to display on the clock
timeInfo GetTime()
{
  delay(980);
  
  timeInfo result;
  auto msSinceStart = millis();
  result.hours = msSinceStart / msPerHour;
  
  msSinceStart %= msPerHour;
  result.minutes = msSinceStart / msPerMinute;
  
  msSinceStart %= msPerMinute;
  result.seconds = msSinceStart / msPerSecond;
  
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
