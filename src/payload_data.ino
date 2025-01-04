#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;
unsigned long previousLogTime = 0;
const unsigned long logInterval = 1000; // Log data every 1 second

struct PayloadState {
    bool receivedSignal;
    bool armDeployed;
    bool camerasPoweredUp;
    bool descentStable;
    bool criteriaMet;
};

// Initialize the payload states
PayloadState payload = {false, false, false, false, false};

// Signal Function
void receiveDeploymentSignal() {

    //code more
  
    payload.receivedSignal = true;
    Serial.println("Deployment signal received.");
}

// Ejection Mechanism Function
void activatePayloadEjectionMechanism() {
  
    //code more
  
    if (payload.receivedSignal) {
        Serial.println("Payload ejection mechanism activated.");
    } else {
        Serial.println("Error: Deployment signal not received.");
    }
}

// Mechanical Actuator Function
void ejectPayload() {
  
    //code more
  
    if (payload.receivedSignal) {
        Serial.println("Payload ejected using mechanical actuator.");
    } else {
        Serial.println("Error: Cannot eject payload before receiving signal.");
    }
}

// Payload Arms Function
void deployArms() {
  
    //code more
  
    payload.armDeployed = true;
    Serial.println("Arms deployed using spring and lock solenoid.");
}

// Camera Function
void powerUpCameras() {
  
    //code more
  
    payload.camerasPoweredUp = true;
    Serial.println("Cameras powered up. Configured to capture at 30 FPS.");
}

// Stable Function
void checkDescentConditions() {
  
    //code more
  
    payload.descentStable = true; // Placeholder logic
    Serial.println("Descent conditions checked. Stable descent achieved.");
}

// Criteria Function
void evaluateCriteria() {
    payload.criteriaMet = payload.receivedSignal && payload.armDeployed && payload.camerasPoweredUp && payload.descentStable;
    if (payload.criteriaMet) {
        Serial.println("All criteria met. Payload operation is complete.");
    } else {
        Serial.println("Criteria not met.");
    }
}

// Log Function
void logData() {
    sensors_event_t a, g, temp;
    if (mpu.getEvent(&a, &g, &temp)) {
        Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.println(" m/s^2");
        Serial.print("Accel Y: "); Serial.print(a.acceleration.y); Serial.println(" m/s^2");
        Serial.print("Accel Z: "); Serial.print(a.acceleration.z); Serial.println(" m/s^2");

        Serial.print("Gyro X: "); Serial.print(g.gyro.x); Serial.println(" rad/s");
        Serial.print("Gyro Y: "); Serial.print(g.gyro.y); Serial.println(" rad/s");
        Serial.print("Gyro Z: "); Serial.print(g.gyro.z); Serial.println(" rad/s");

        Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" °C");
        Serial.println("Logging data from IMU.");
    } else {
        Serial.println("Error reading data from MPU6050.");
    }
}

// Hardware Initialization
void initializeHardware() {
    Wire.setSDA(0);
    Wire.setSCL(1);
    Wire.begin();

    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip. Check connections.");
        while (1); // Halt if sensor is not initialized
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("MPU6050 initialized successfully!");
}

// log for one time
void realTimeLogData() {
    unsigned long currentTime = millis();
    if (currentTime - previousLogTime >= logInterval) {
        previousLogTime = currentTime;
        logData();
    }
}

// continue log while delay fpr next operation
void delayWithLogging(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        realTimeLogData();
    }
}

// Main Operation
void performPayloadOperation() {
    receiveDeploymentSignal(); 
    activatePayloadEjectionMechanism();

    while (!payload.descentStable) {
        ejectPayload();
        realTimeLogData(); // Continuous logging
        delayWithLogging(5000); // 5 seconds for stabilization with logging
        checkDescentConditions();
    }

    deployArms();
    delayWithLogging(5000); // 5 seconds for arm extension with logging
    powerUpCameras();
    evaluateCriteria();

    while (!payload.criteriaMet) {
        realTimeLogData(); // Continuous logging
        checkDescentConditions();

        if (!payload.descentStable) {
            while (!payload.descentStable) {
                realTimeLogData(); // Continuous logging
                ejectPayload();
                delayWithLogging(5000); // 5 seconds for stabilization with logging
                checkDescentConditions();
            }
        }

        if (!payload.armDeployed) {
            deployArms();
            delayWithLogging(5000); // 5 seconds for arm extension with logging
        }

        if (!payload.camerasPoweredUp) {
            powerUpCameras();
        }

        evaluateCriteria();
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial);
    
    initializeHardware();
    performPayloadOperation();
}

void loop() {
    realTimeLogData(); // Continue logging 
}
