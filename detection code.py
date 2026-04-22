import torch
import cv2
import yaml
import serial
import time
import threading

# ===== CONFIG =====
yaml_path    = r"C:/Users/bshin/Downloads/mini_dataset/mini_dataset/data.yaml"
weights_path = r"C:/Users/bshin/Downloads/yolov5-master/yolov5-master/runs/train/exp/weights/best.pt"

# ===== SERIAL SETUP =====
# Change COM5 to your port (Device Manager → Ports)
ser = serial.Serial('COM6', 115200, timeout=15)
time.sleep(2)

arm_busy = False

def wait_for_ready():
    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line:
            print(f"Arduino: {line}")
        if line == "READY":
            break

def send_to_arm(waste_type):
    global arm_busy
    if arm_busy:
        return
    arm_busy = True
    ser.write((waste_type.lower() + "\n").encode())
    print(f"Sent: {waste_type}")
    wait_for_ready()
    arm_busy = False

# Run send_to_arm in background so OpenCV display doesn't freeze
def send_async(waste_type):
    t = threading.Thread(target=send_to_arm, args=(waste_type,))
    t.daemon = True
    t.start()

# ===== CLASS MAP =====
CLASS_TO_ARM = {
    'pet' : 'pet',  'PET' : 'pet',
    'hdpe': 'hdpe', 'HDPE': 'hdpe',
    'pvc' : 'pvc',  'PVC' : 'pvc',
    'ldpe': 'ldpe', 'LDPE': 'ldpe',
    'pp'  : 'pp',   'PP'  : 'pp',
    'ps'  : 'ps',   'PS'  : 'ps',
}

# ===== Load YAML =====
with open(yaml_path, 'r') as f:
    data = yaml.safe_load(f)

class_names = data['names']
allowed_class_ids = list(range(len(class_names)))

# ===== Load Model =====
model = torch.hub.load('ultralytics/yolov5', 'custom', path=weights_path, force_reload=True)
model.eval()

# ===== Detection =====
def detect_and_draw(frame):
    input_frame = cv2.resize(frame, (320, 320))
    results = model(input_frame)
    detections = results.xyxy[0]

    height, width = frame.shape[:2]
    detected_labels = []
    best_class = None
    best_conf  = 0.0

    for det in detections:
        x1, y1, x2, y2, conf, cls = det.cpu().numpy()
        class_id = int(cls)

        if conf < 0.5 or class_id not in allowed_class_ids:
            continue

        x1 = int(x1 * width / 320)
        x2 = int(x2 * width / 320)
        y1 = int(y1 * height / 320)
        y2 = int(y2 * height / 320)

        label = f"{class_names[class_id]} {conf:.2f}"
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
        y_label = y1 - 10 if y1 - 10 > 10 else y1 + 20
        cv2.putText(frame, label, (x1, y_label),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        detected_labels.append(class_names[class_id])

        # Track highest confidence detection
        if conf > best_conf:
            best_conf  = conf
            best_class = class_names[class_id]

    # Send to arm only if not busy
    if best_class and not arm_busy:
        arm_class = CLASS_TO_ARM.get(best_class)
        if arm_class:
            send_async(arm_class)

    overlay = "Detected: " + ", ".join(detected_labels) if detected_labels else "No plastic detected"
    cv2.putText(frame, overlay, (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

    # Show arm status on screen
    status = "ARM: BUSY" if arm_busy else "ARM: READY"
    color  = (0, 0, 255) if arm_busy else (0, 255, 0)
    cv2.putText(frame, status, (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)

    return frame

# ===== MAIN =====
def main():
    print("Waiting for arm to initialise...")
    wait_for_ready()
    print("Arm ready — starting detection")

    cap = cv2.VideoCapture("http://10.23.70.1:8080/video")
    cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

    if not cap.isOpened():
        print("Cannot access stream")
        return

    print("Live detection started (ESC to exit)")
    frame_count = 0

    while True:
        for _ in range(2):
            cap.read()

        ret, frame = cap.read()
        if not ret:
            print("Frame not received")
            continue

        frame_count += 1
        if frame_count % 2 == 0:
            frame = detect_and_draw(frame)

        cv2.imshow("YOLOv5 Plastic Detection", frame)
        if cv2.waitKey(1) & 0xFF == 27:
            break

    cap.release()
    cv2.destroyAllWindows()
    ser.close()
    print("Stopped")

if __name__ == "__main__":
    main()