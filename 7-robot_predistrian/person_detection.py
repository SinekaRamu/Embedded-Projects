import cv2
import numpy as np
from datetime import datetime
import subprocess

# Load MobileNet-SSD
net = cv2.dnn.readNetFromCaffe(
    "MobileNetSSD_deploy.prototxt", 
    "MobileNetSSD_deploy.caffemodel"
)

CLASSES = ["background", "aeroplane", "bicycle", "bird", "boat",
           "bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
           "dog", "horse", "motorbike", "person", "pottedplant", "sheep",
           "sofa", "train", "tvmonitor"]

# audio jack
def speak(text):
    subprocess.run(["espeak", "-ven+f3", "-s120", text])

# Libcamera command (adjust settings for clarity)
def capture_libcamera_frame():
    cmd = [
        "libcamera-still",
        "--nopreview",
        "--width", "1280",
        "--height", "720",
        "--immediate",  # No focus delay
        "--denoise", "off",  # Disable denoise for sharper images
        "--sharpness", "2",  # Increase sharpness (0-7)
        "--timeout", "1",  # Minimal delay
        "--output", "-",  # Output to stdout
    ]
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode == 0:
        img = cv2.imdecode(np.frombuffer(result.stdout, dtype=np.uint8), cv2.IMREAD_COLOR)
        return img
    else:
        print("Libcamera Error:", result.stderr.decode())
        return None

try:
    while True:
        # Capture frame using libcamera
        frame = capture_libcamera_frame()
        if frame is None:
            continue

        # Resize for MobileNet-SSD (300x300)
        blob = cv2.dnn.blobFromImage(
            frame, 0.007843, (300, 300), 127.5, 
            swapRB=False, crop=False
        )

        # Run detection
        net.setInput(blob)
        detections = net.forward()

        detected_img = ""
        # Draw bounding boxes
        for i in range(detections.shape[2]):
            confidence = detections[0, 0, i, 2]
            if confidence > 0.2:  # Confidence threshold
                class_id = int(detections[0, 0, i, 1])
                detected_img = CLASSES[class_id]
                box = detections[0, 0, i, 3:7] * [frame.shape[1], frame.shape[0], frame.shape[1], frame.shape[0]]
                (left, top, right, bottom) = box.astype("int")
                cv2.rectangle(frame, (left, top), (right, bottom), (0, 255, 0), 2)
                label = f"{CLASSES[class_id]}: {confidence:.2f}"
                cv2.putText(frame, label, (left, top - 10), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

        # Display output
        speak(detected_img)
        cv2.imshow("Libcamera + MobileNet-SSD", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

finally:
    cv2.destroyAllWindows()
