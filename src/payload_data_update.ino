#include <Wire.h>
#include <MPU9250_asukiaaa.h>  // Library for MPU9250
#include <Adafruit_BMP280.h>   // Library for BMP280

#define SDA_PIN 4  // GP4
#define SCL_PIN 5  // GP5

MPU9250_asukiaaa mpu9250;
Adafruit_BMP280 bmp;  // BMP280 sensor object

unsigned long previousLogTime = 0;
const unsigned long logInterval = 1000; // Log data every 1 second

struct PayloadState {
    bool receivedSignal;
    bool armDeployed;
    bool camerasPoweredUp;
    bool descentStable;
    bool criteriaMet;
};

PayloadState payload = {false, false, false, false, false};

void receiveDeploymentSignal() {
    payload.receivedSignal = true;
    Serial.println("Deployment signal received.");
}

void activatePayloadEjectionMechanism() {
    if (payload.receivedSignal) {
        Serial.println("Payload ejection mechanism activated.");
    } else {
        Serial.println("Error: Deployment signal not received.");
    }
}

void ejectPayload() {
    if (payload.receivedSignal) {
        Serial.println("Payload ejected using mechanical actuator.");
    } else {
        Serial.println("Error: Cannot eject payload before receiving signal.");
    }
}

void deployArms() {
    payload.armDeployed = true;
    Serial.println("Arms deployed using spring and lock solenoid.");
}

void powerUpCameras() {
    payload.camerasPoweredUp = true;
    Serial.println("Cameras powered up. Configured to capture at 30 FPS.");
}

void checkDescentConditions() {
    payload.descentStable = true; // Placeholder logic
    Serial.println("Descent conditions checked. Stable descent achieved.");
}

void evaluateCriteria() {
    payload.criteriaMet = payload.receivedSignal && payload.armDeployed && payload.camerasPoweredUp && payload.descentStable;
    if (payload.criteriaMet) {
        Serial.println("All criteria met. Payload operation is complete.");
    } else {
        Serial.println("Criteria not met.");
    }
}

void logData() {
  // Read MPU9250 Data (Accel, Gyro, Mag)
  mpu9250.accelUpdate();
  mpu9250.gyroUpdate(); 
  mpu9250.magUpdate();   

  Serial.print("Accel (m/s²): X = "); Serial.print(mpu9250.accelX() * 9.81, 2);  
  Serial.print(" | Y = "); Serial.print(mpu9250.accelY() * 9.81, 2);  
  Serial.print(" | Z = "); Serial.println(mpu9250.accelZ() * 9.81, 2);

  Serial.print("Gyro (°/s): X = "); Serial.print(mpu9250.gyroX(), 2);  
  Serial.print(" | Y = "); Serial.print(mpu9250.gyroY(), 2);  
  Serial.print(" | Z = "); Serial.println(mpu9250.gyroZ(), 2);

  Serial.print("Mag (µT): X = "); Serial.print(mpu9250.magX(), 2);  
  Serial.print(" | Y = "); Serial.print(mpu9250.magY(), 2);  
  Serial.print(" | Z = "); Serial.println(mpu9250.magZ(), 2);

  // Read BMP280 Data (Pressure & Temperature)
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;  // Convert to hPa

  Serial.print("Temperature (°C): "); Serial.println(temperature, 2);
  Serial.print("Pressure (hPa): "); Serial.println(pressure, 2);

  Serial.println("---------------------------------------------------");
  delay(1000);
}

void realTimeLogData() {
    unsigned long currentTime = millis();
    if (currentTime - previousLogTime >= logInterval) {
        previousLogTime = currentTime;
        logData();
    }
}

void delayWithLogging(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        realTimeLogData();
    }
}

void performPayloadOperation() {
    receiveDeploymentSignal();
    activatePayloadEjectionMechanism();

    while (!payload.descentStable) {
        ejectPayload();
        realTimeLogData();
        delayWithLogging(5000);
        checkDescentConditions();
    }

    deployArms();
    delayWithLogging(5000);
    powerUpCameras();
    evaluateCriteria();

    while (!payload.criteriaMet) {
        realTimeLogData();
        checkDescentConditions();

        if (!payload.descentStable) {
            while (!payload.descentStable) {
                realTimeLogData();
                ejectPayload();
                delayWithLogging(5000);
                checkDescentConditions();
            }
        }

        if (!payload.armDeployed) {
            deployArms();
            delayWithLogging(5000);
        }

        if (!payload.camerasPoweredUp) {
            powerUpCameras();
        }

        evaluateCriteria();
    }
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  // Manually set I2C pins for Raspberry Pi Pico
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  // Initialize MPU9250
  mpu9250.setWire(&Wire);
  mpu9250.beginAccel();  // Initialize Accelerometer
  mpu9250.beginGyro();   // Initialize Gyroscope 
  mpu9250.beginMag();    // Initialize Magnetometer 

  Serial.println("MPU9250 Initialized!");

  // Initialize BMP280
  if (!bmp.begin(0x76)) { // BMP280 I2C address is 0x76
    Serial.println("BMP280 Initialization Failed!");
  } else {
    Serial.println("BMP280 Initialized!");
  }
    while (!Serial);
    performPayloadOperation();
}

void loop() {
  realTimeLogData();
}
