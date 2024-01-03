
#define SPI_CS_PIN 5
#define CAN_INT_PIN 21
#define MAX_DATA_SIZE 8

void IRAM_ATTR NewDataReadyISR();

#define POT_PIN 36

class RotationSensor {
  public:
    static int newValue;
    static int oldValue;
};

class WindSensor {
  public:
    static double windSpeedKnots;
    static int windAngleDegrees;
};

typedef struct {
  unsigned long PGN;
  void (*Handler)(const tN2kMsg &N2kMsg); 
} tNMEA2000Handler;

void WindSpeed(const tN2kMsg &N2kMsg);

extern tNMEA2000Handler NMEA2000Handlers[];

int readAnalogRotationValue();
void ParseWindCAN();
double ReadWindAngle(int);
double ReadWindSpeed();
int readWindAngleInput();
void SendN2kWind(int);

#define WindUpdatePeriod 500

