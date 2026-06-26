# PROJECT: VISIONBIN - AUTOMATED WASTE SORTING SYSTEM
# TEAM: DESIRED IOT TEAM
# NEO LEAGUE 2026 - ROUND 2

## 1. DIRECTORY STRUCTURE
- /1_Hardware_ESP32: Contains C++ code for ESP32-CAM, ultrasonic sensors (HC-SR04), and Servo motor control.
- /2_UNITY: Contains the FOMO object detection model files and configurations.
- /3_Cloud_Backend_AWS: Contains Python scripts for AWS Lambda (UpdateTrashLevel & GetTrashData) interfacing with Amazon DynamoDB and API Gateway.
- /4_Mobile_App: Contains the source code for the Janitor Hub mobile application dashboard.

## 2. REQUIRED LIBRARIES & DEPLOYMENT
- Hardware: Requires Arduino IDE with ESP32 board manager installed. Libraries needed: WiFi.h, HTTPClient.h, ESP32Servo.h.
- Cloud Backend: AWS Lambda environment running Python 3.12. Requires standard 'boto3' and 'json' libraries. Execution role must have AmazonDynamoDBFullAccess and AmazonSNSFullAccess.
- AI Model: Edge Impulse Arduino library exported from the project dashboard.

## 3. HOW TO RUN
1. Flash the ESP32 code from /1_Hardware_ESP32. Ensure Wi-Fi credentials are updated in the code.
2. Deploy the Python scripts in /3_Cloud_Backend_AWS to AWS Lambda and configure the API Gateway triggers.
3. Update the API Invoke URL in both the ESP32 C++ code and the Mobile App configuration.
4. Run the Mobile App from /4_Mobile_App to monitor the bin status via the GET API.
5.Step by step to open VisionBin in Unity.

# Step 1
Download Unity Hub, register free license and download latest version of Unity 2019.

# Step 2
Unzip VisionBin inside VisionBin_SourceCodeUnity into different folder.

# Step 3
Use Unity Hub to open that folder (Make sure the folder has Assets, Packages and Project Settings).