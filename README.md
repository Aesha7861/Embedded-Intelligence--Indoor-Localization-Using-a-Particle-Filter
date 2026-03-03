#  Indoor Localization System using IMU, BLE, and Particle Filter

This project implements a complete indoor localization pipeline using Arduino, IMU sensors, BLE beacons, and particle filtering. The goal is to estimate a user's position and heading within an indoor environment using sensor fusion and probabilistic methods.

---

##  Overview

The project flow involves:
- Collecting motion and BLE data using Arduino
- Creating and visualizing an indoor map
- Logging sensor data during movement
- Detecting steps and heading using IMU
- Applying particle filters with/without map and RSSI constraints
- Comparing localization accuracy across configurations

---

## 1. Initialization Phase

### ➤ Sensor and BLE Setup

- **Initialize IMU Sensors**  
  - Using BMI270 and BMM150 sensors via Arduino libraries.
  - Collect raw accelerometer and gyroscope data.

- **Use Madgwick Filter for Accurate Heading**
  - The `#include <MadgwickAHRS.h>` filter is used to compute orientation (yaw) from IMU readings.
  - It fuses gyroscope and accelerometer data to provide low-drift heading estimation.

- **Initialize BLE Scanning**
  - Using `#include <ArduinoBLE.h>` for scanning nearby BLE beacons.
  - Captures beacon names, addresses, and RSSI values.

- **Collect IMU and RSSI Data**
  - Log data including yaw, acceleration, gyroscope, beacon RSSI, and identifiers.

- **Heading Check**
  - Verify device heading using yaw from the filter.
  - Apply offset correction if misaligned.(First when I record the data at that time I got the some offset around 40 degree, but in the final result we managed the offest and don't need to add.)

---

## 2. Indoor Map Creation

- **Measure Indoor Environment**
  - Physically measure room and hallway dimensions.

- **Generate Indoor Plan**
  - Create a 2D NumPy matrix map, where each cell = 1 meter.

- **Validate and Visualize Map**
  - Validate dimensions and walls.
  - Render the map using Pygame for dynamic interaction and visualization.

---

## 3. Beacon Detection and Data Collection
  ## 3.1 Data Recording Methodology

Accurate data recording was a critical part of this project and was conducted collaboratively by both authors.

###  Roles and Setup

- **One person** was responsible for:
  - Validating the **Arduino's heading (yaw)** using real-time output.
  - Monitoring the **RSSI values** to confirm they changed correctly as the device approached or moved away from beacons.

- **The other person** held the **Arduino Nano 33 BLE Sense** and **walked through the predefined indoor path**.

---

###  Heading Alignment Challenge

Initially, we observed a **significant heading offset of ~40°** from the actual movement direction. This misalignment posed a major challenge in tracking the true direction of motion using IMU data alone.

To address this:

- We implemented a **precise heading calibration** using the Madgwick filter (`MadgwickAHRS.h`) and physical alignment at the starting position.
- After applying this calibration, the device began reporting **accurate yaw values**, greatly improving heading estimation during movement.

---

###  Orientation Reference

- The experiment started from a known reference point, which was assigned as the **0° heading**.
- From there, the person holding the Arduino:
  - Moved **left**, which was interpreted as **90° yaw**.
  - Continued in that direction for some distance.
  - Then **turned right**, returning the heading to approximately **0°**, completing a defined turn.

---

###  RSSI Behavior Monitoring

- During the movement, we actively verified that the **RSSI values** were behaving correctly:
  - RSSI increased as the device approached a beacon.
  - RSSI decreased as it moved away from the beacon.
- This real-time validation helped us ensure **reliable signal-based proximity estimation** for later use in the particle filter.

---

This carefully coordinated recording process ensured:
- Consistent and calibrated **heading (yaw) data**.
- Validated **RSSI signal strength behavior**.
- Accurate **step-wise motion tracking** aligned with the physical indoor map.

You can observe the effectiveness of this method in the step detection graphs, trajectory plots, and final particle filter visualizations.

- **Fetch Beacon RSSI via Python Script**
  - Connect to Arduino via serial and parse beacon signals.

- **Target Four Beacons**
  - Choose 4 known beacons for use in RSSI localization.

- **Record Sensor Data to CSV**
  - Save yaw, acceleration, gyroscope, and RSSI values to CSV while walking through the map.

---

## 4. IMU Data Processing

- **Step Detection**
  - Use accelerometer patterns to detect steps.

- **Plot IMU Data for Step Validation**
  - Visualize raw acceleration to verify step timing.

- **Heading Estimation**
  - Use Madgwick filter output for heading (yaw).

- **Analyze Yaw and Motion**
  - Check for smoothness, accuracy, and rotation consistency.

- **Track and Record Path**
  - Combine step count and heading to reconstruct walking trajectory.

---

## 5. Map Visualization of Recorded Data

- **Overlay Data on Indoor Map**
  - Plot recorded steps and orientation on the NumPy + Pygame map.

- **Ensure Accurate Overlay**
  - Confirm that trajectory aligns with real walking path.

---

## 6. Particle Filter Processing

### A. Without Resampling

- **Particle Filter without Resampling**
  - Initialize particles randomly and move them using IMU data.
  
- **With RSSI**
  - Weight particles based on proximity to detected beacons.

- **No Map Constraints**
  - Particles may pass through walls and invalid regions.

### B. With Map-Based Elimination & Resampling

- **Wall-Based Particle Elimination**
  - Remove particles that move into forbidden map areas.

- **Resample for Accuracy**
  - Replace removed particles near the most probable regions.

---



## 7. Localization Accuracy Analysis: Comparison and Discussion of Results: Particle Filter Accuracy Across Configurations

To better understand the role of map constraints and RSSI in indoor localization, we evaluated the particle filter across four key configurations. Each setup demonstrates how different sensor combinations impact localization performance in terms of accuracy, path adherence, and particle behavior.

---

### 1. Particle Filter with Map and RSSI (Ideal Case)

This is the most complete and ideal scenario. The map ensures particles remain in walkable areas, while the RSSI provides absolute distance cues from known beacon positions. Together, they complement the IMU’s step-based movement.

- **Observation**: The estimated path is highly accurate, smoothly following the true trajectory and capturing turns correctly.
- **Particle Behavior**: The particles stay constrained within the defined corridors and adjust naturally near beacons.
- **Conclusion**: Combining map constraints, RSSI-based distance estimates, and IMU dead reckoning yields the most robust and precise localization. It corrects drift and avoids impossible movements (like walking through walls).

---

### 2. Particle Filter with Map but Without RSSI

In this setup, particles are constrained by the map but do not receive any beacon signals (RSSI). As a result, particles rely entirely on step detection and heading from the IMU.

- **Observation**: The path generally follows the correct corridor, but we start seeing some drift along the walking direction, especially toward the end.
- **Particle Behavior**: Particles are confined within valid areas due to the map, but there's no correction from external references like beacons.
- **Conclusion**: Without RSSI, the system depends solely on the IMU. While the map keeps particles in legal areas, there's no way to correct accumulated distance errors (e.g., under- or over-estimated step length). The trajectory is plausible but not as accurate as the ideal case.

---

### 3. Particle Filter with RSSI but Without Map

This case uses RSSI for distance estimation and IMU for movement, but no map is applied to eliminate implausible particles.

- **Observation**: The path somewhat follows the intended direction, and beacons can still be detected using RSSI. However, without map constraints, particles are free to drift into non-walkable areas.
- **Particle Behavior**: The particles follow the IMU direction and get pulled toward beacon signals, but because there’s no wall-checking, some particles float into impossible positions (like through walls).
- **Conclusion**: We can partially detect beacons and roughly localize, but the absence of a map leads to instability. There's no particle "killing" or boundary enforcement, so the final result is noisy and less reliable. It highlights the importance of maps in filtering out implausible estimates.

---

### 4. Particle Filter Without Map and Without RSSI (Only IMU)

This is the most limited configuration. The particle filter relies solely on IMU data, with no map or beacon input.

- **Observation**: The trajectory quickly deviates from the real path, especially after turns or long straight segments.
- **Particle Behavior**: Without external corrections, particles simply follow the IMU’s perceived movement. Over time, heading and step-length errors accumulate, causing large drift.
- **Conclusion**: With no map and no RSSI, the system cannot self-correct. The particles are unbounded and can move unrealistically, resulting in significant error. This configuration is unsuitable for reliable indoor localization.

---

### Summary

This comparison clearly demonstrates how each component contributes:
- **Map**: Enforces environmental constraints and prevents implausible paths.
- **RSSI**: Provides absolute positioning cues and helps correct drift.
- **IMU**: Provides continuous step-based updates but needs support from map and RSSI to stay accurate.

The **fusion of all three (IMU + Map + RSSI)** is necessary for high-accuracy, drift-resistant indoor localization.


## 8. Comparison and Discussion of Results

###  Summary of Observations

| Configuration             | Accuracy | Stability | Notes |
|--------------------------|----------|-----------|-------|
| RSSI + Map (Ideal)       |  High  |  High    | Best performance |
| RSSI Only                |  Medium |  Low     | Path drifts |
| Map Only                 |  Medium |  High    | No correction |
| IMU Only                 |  Low   |  Low     | Very poor accuracy |

### Final Conclusion

Combining **IMU + RSSI + Map constraints** gives the most reliable and accurate indoor localization.

---

##  Tools & Libraries

- **Arduino IDE + Nano 33 BLE Sense**
  - `ArduinoBLE.h`
  - `Arduino_BMI270_BMM150.h`
  - `MadgwickAHRS.h`

- **Python (Data Logging & Processing)**
  - `pyserial`, `pandas`, `numpy`, `pygame`, `matplotlib`

- **For reading the Readme.md**
  - Install `Auto-Open Markdown Preview` 
  - Ctrl+Shift+V (or Cmd+Shift+V on Mac) 

- **Visualization**
  - Pygame (Map Rendering)
  - CSV plotting and trajectory estimation

---
