# YOLO-Based Waste Detection and Arduino Segregation System

## Overview
AI-based system that detects plastic waste using YOLO and segregates it using an Arduino-controlled robotic arm.

## Tech Stack
- Python (YOLO, OpenCV)
- Arduino (Servo motors)
- Serial Communication

## Working
1. Live video feed from camera
2. YOLO detects object
3. Signal sent to Arduino
4. Robotic arm sorts waste

## Model Training
- Collected and labeled a custom dataset of plastic waste images sourced from interent.
- Trained/fine-tuned a YOLO model for plastic detection.
- Used OpenCV for preprocessing and real-time inference on video stream.
