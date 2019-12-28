/**
   Calculates the Heart Rates given the 'pleth' waveform from a PPG sensor (IR light + photoreceptor).
   Oxigenated and Deoxigenated blood allow different wavelengths of light, creating the waveform.
   Because this sensor is only designed to detect HR, it only has one light. Conventional sensors would
   include an IR light alongside with a red light.

   This uses a Sliding Discrete Fourier Transform to extract the fundamental (frequency) of the wave. This
   represents the Heart Rate.

   Created with the help, knowledge and guidance of Mr. Roller. Thanks!

   Heart Rate Monitor
   Author: Patrick Song
   Course: TEJ3M
   Date: 2019-06-18
*/
#include <LiquidCrystal.h>
#include <EEPROM.h>

/* conjugate-symmetric */
static const PROGMEM boolean CS = true;
/* samples per second */
static const PROGMEM float SAMPLE_RATE = 20.0f;
/* number of samples */
static const PROGMEM uint16_t N = 550;
/* the amount that must be calculated given whether the input is conjugate-symmetric */
static const PROGMEM uint16_t BOUND = CS ? N / 2 : N;
static const PROGMEM float M_2PI = M_PI * 2.0f;
/* analogue pin for sensor */
static const PROGMEM uint16_t PIN = 0;
/* the amount of bits in the mantissa to mask out */
static const PROGMEM uint32_t BITS = 16;
/* the mantissa bit mask */
static const PROGMEM uint32_t MASK = -(1ul << BITS);
/* smoothing factor */
static const PROGMEM uint16_t FACTOR = 2;

// rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2
static LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/* previous input */
static float rold, iold;

/* initial reading */
static uint16_t initial;
/* the deltas between inputs */
static int8_t input[N - 1];
/* fourier transform output */
static uint16_t out[BOUND * 2];

/**
   Truncates a float to 16 bits by trimming off the mantissa.
*/
uint16_t truncate(float x)
{
  // mask bits of float and shift to 16 bits
  uint32_t bits = *((uint32_t*) &x);
  bits &= MASK;
  bits >>= BITS;

  uint16_t result = (uint16_t) bits;
  return result;
}

/**
   Returns the value of a truncated float.
*/
float getValue(uint16_t x) {
  float result;

  uint32_t shifted = (uint32_t) x << BITS;
  result = *((float*) &shifted);

  return result;
}

/**
   Reads and stores value from the HR sensor in a memory efficient manner.
   Must store the entire N samples, even if conjugate-symmetric. This is because
   maintaining the entire signal is still essential to the Fourier Transform.
*/
float readSensor()
{
  float space = 1.0f / SAMPLE_RATE;

  uint64_t start = millis();

  /*
     Populate initial input and the first value of the input array
  */
  initial = analogRead(PIN);
  // delay to satisfy sample rate
  delay((uint64_t) (space * 1000));

  lcd.print(F("HR Sampling..."));

  uint16_t previous = analogRead(PIN);
  input[0] = (int8_t) (previous - initial);
  // delay to satisfy sample rate
  delay((uint64_t) (space * 1000));

  /*
     Populate remaining values of the input array
  */
  for (uint16_t i = 1; i < N - 1; i++) {
    uint16_t current = analogRead(PIN);

    // calculating the delta
    input[i] = (int8_t) (current - previous);
    previous = current;

    /*
       Print percentage completion to LCD
    */
    lcd.setCursor(0, 1);
    lcd.print((int) round(100 * i * space / ((N - 1) * space)));
    lcd.print(F(" %     "));

    // delay to satisfy sample rate
    delay((uint64_t) (space * 1000));
  }

  // return the actual sample rate given a start and end time
  return (1000.0f * N) / (millis() - start);
}

/**
   Calculate HR once.
*/
void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);

  // get previous heart rate from EEPROM
  float previous = readFloat();
  // sample values
  float sampleRate = readSensor();

  // value is used to smooth the input
  float value = initial;
  sdft(0, value, 0);

  // print the input
  Serial.print(F("0"));
  Serial.print(F("\t"));
  Serial.println(value);

  lcd.clear();
  lcd.print(F("Analysing "));

  float bpm = 0;
  float top = 0;
  for (uint16_t i = 1; i < N; i++) {
    // smooth the input
    value += ((initial += input[i - 1]) - value) / FACTOR;
    sdft(i, value, 0);

    // print the input
    Serial.print(i / sampleRate);
    Serial.print(F("\t"));
    Serial.println(value);

    // print the percentage completion
    lcd.setCursor(10, 0);
    lcd.print((int) round(100 * i * sampleRate / ((N - 1) * sampleRate)));
    lcd.print(F(" %     "));

    if (i % 2 == 0) {
      // record the peak
      for (uint16_t i = 0; i < BOUND; i++) {
        // get frequency in beats per minute by multiplying by 60
        float frequency = 60 * i * sampleRate / N;
        // magnitude of the complex output returns the amplitude
        float amplitude = 2.0 * sqrt(pow(getValue(out[i]), 2) + pow(getValue(out[BOUND + i]), 2)) / N;

        if (amplitude >= top) {
          top = amplitude;
          bpm = frequency;
        }
      }

      lcd.setCursor(0, 1);
      lcd.print(bpm);
      lcd.print(F(" bpm     "));
    }
  }

  lcd.setCursor(0, 0);
  lcd.print(F("Complete        "));

  for (uint16_t i = 0; i < BOUND; i++) {
    float frequency = 60 * i * sampleRate / N;
    float amplitude = 2.0 * sqrt(pow(getValue(out[i]), 2) + pow(getValue(out[BOUND + i]), 2)) / N;

    // print sdft result
    Serial.print(frequency);
    Serial.print(F("\t"));
    Serial.println(amplitude);
  }

  // record hr to EEPROM
  writeFloat(bpm);

  Serial.flush();
  Serial.end();

  lcd.setCursor(0, 0);
  lcd.print(F("Previous "));
  lcd.print(previous);
}

/**
   Writes float to EEPROM.
*/
void writeFloat(float x)
{
  // get bits from float
  uint32_t bits = *((uint32_t*) &x);

  /* Write the 4 bits in the float to different bytes in EEPROM */
  EEPROM.update(0, (byte) (bits & 0xff));
  EEPROM.update(1, (byte) ((bits >> 8) & 0xff));
  EEPROM.update(2, (byte) ((bits >> 16) & 0xff));
  EEPROM.update(3, (byte) ((bits >> 24) & 0xff));
}

/**
   Reads float from EEPROM.
*/
float readFloat()
{
  float result;
  uint32_t bits;

  /* Reading the float values and shifting them to make up a float */
  bits = (uint32_t) ((uint32_t) EEPROM.read(0) + ((uint32_t) EEPROM.read(1) << 8) + ((uint32_t) EEPROM.read(2) << 16) + ((uint32_t) EEPROM.read(3) << 24));

  result = *((float*) &bits);
  return result;
}

/**
   Performs a Sliding Discrete Fourier Transform on data.
*/
void sdft(const unsigned short index, const float rin, const float iin)
{
  float multiplier = hanning(index);

  // calculating the delta and applying the hann window
  float rdelta = multiplier * rin - multiplier * rold;
  float idelta = multiplier * iin - multiplier * iold;

  // caching the input
  rold = rin;
  iold = iin;

  // only need to loop through the first half if conjugate-symmetric
  for (uint16_t i = 0; i < BOUND; i++) {
    /*
       The circular buffer trick for SDFT is replaced with some index manipulation,
       specifically by looping an index around N incrementing by the current data index.
    */
    float angle = i * index % N * -M_2PI / N;

    // real and imaginary components
    float wmr = cos(angle);
    float wmi = sin(angle);

    // while half-precision floating-point addition could be used, dealing with different exponents would be inefficient due to hardware
    out[i] = truncate(getValue(out[i]) + rdelta * wmr - idelta * wmi);
    out[BOUND + i] = truncate(getValue(out[BOUND + i]) + rdelta * wmi + idelta * wmr);
  }
}

/**
   Calculates the value to multiply for the Hann window.
*/
float hanning(uint16_t index)
{
  return 0.5f * (1 - cos(M_2PI * index / (N - 1)));
}

/**
   The loop method is not used, as doing the above calculations in realtime would be far too intensive.
   A design decision was made to only run the HR measurement once.
*/
void loop()
{
  // put your main code here, to run repeatedly:
}
