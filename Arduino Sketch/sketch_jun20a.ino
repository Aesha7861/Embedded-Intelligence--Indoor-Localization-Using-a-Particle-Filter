#include <Arduino_BMI270_BMM150.h>
#include <ArduinoBLE.h>
#include <MadgwickAHRS.h>

// === Sensor + Filter ===
Madgwick filter;
float ax, ay, az;
float gx, gy, gz;

const unsigned int SENSOR_UPDATE_FREQUENCY = 50; // Hz
const unsigned long SENSOR_UPDATE_INTERVAL_MS = 1000 / SENSOR_UPDATE_FREQUENCY;
unsigned long lastUpdateTime = 0;

// === BLE ===
String nearestDeviceName = "Unknown";
String nearestDeviceAddress = "00:00:00:00:00:00";
int nearestRSSI = -999;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // === IMU Initialization ===
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  filter.begin(SENSOR_UPDATE_FREQUENCY);

  // Warm-up IMU readings to stabilize filter
  for (int i = 0; i < 100; i++) {
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);
    filter.updateIMU(gx, gy, gz, ax, ay, az);
    delay(10);
  }

  // === BLE Initialization ===
  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }
  BLE.scan(); // Start scanning
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= SENSOR_UPDATE_INTERVAL_MS) {
    lastUpdateTime = currentTime;

    // === Read IMU ===
    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    // Get raw yaw (no offset)
    float yaw = filter.getYaw();

    // Wrap yaw to 0–360
    yaw = fmod(yaw, 360.0);
    if (yaw < 0) yaw += 360.0;

    // === Scan BLE ===
    BLEDevice nearest;
    nearestRSSI = -999;

    BLEDevice peripheral = BLE.available();
    while (peripheral) {
      int rssi = peripheral.rssi();
      if (rssi > nearestRSSI) {
        nearest = peripheral;
        nearestRSSI = rssi;
      }
      peripheral = BLE.available();
    }

    if (nearest) {
      nearestDeviceName = nearest.localName();
      if (nearestDeviceName.length() == 0) {
        nearestDeviceName = "Unnamed";
      }
      nearestDeviceAddress = nearest.address();
    } else {
      nearestDeviceName = "None";
      nearestDeviceAddress = "00:00:00:00:00:00";
      nearestRSSI = -999;
    }

    // === Print data for Python ===
    Serial.print(yaw, 2); Serial.print(",");
    Serial.print(ax, 2); Serial.print(",");
    Serial.print(ay, 2); Serial.print(",");
    Serial.print(az, 2); Serial.print(",");
    Serial.print(gx, 2); Serial.print(",");
    Serial.print(gy, 2); Serial.print(",");
    Serial.print(gz, 2); Serial.print(",");
    Serial.print(nearestDeviceName); Serial.print(",");
    Serial.print(nearestDeviceAddress); Serial.print(",");
    Serial.println(nearestRSSI);
  }
}
