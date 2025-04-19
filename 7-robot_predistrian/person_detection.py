import subprocess
from datetime import datetime
import os
import cv2

# Load the model
net = cv2.dnn.readNetFromCaffe("MobileNetSSD_deploy.prototxt", "MobileNetSSD_deploy.caffemodel")

CLASSES = ["background", "aeroplane", "bicycle", "bird", "boat",
           "bottle", "bus", "car", "cat", "chair", "cow", "diningtable",
           "dog", "horse", "motorbike", "person", "pottedplant", "sheep",
           "sofa", "train", "tvmonitor"]

image_save_dir = "/home/ediylabs/Desktop/opencv/mobilenet-ssd/images/"  # Update this path to your desired folder
if not os.path.exists(image_save_dir):
    os.makedirs(image_save_dir)

# Capture control variables
last_capture_time = 0
debounce_delay = 5  # Minimum seconds between captures
timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

def capture_image():
    """Capture a single high-res image with timestamp"""   
    filename = os.path.join(image_save_dir, f"img_{timestamp}.jpg")
    
    try:
        subprocess.run([
            "libcamera-jpeg",
            "-o", filename,
            "--width", "1280",
            "--height", "720",
            "--timeout", "2000",
            "--nopreview"
        ], check=True)
        print(f"IMAGE SAVED: {filename}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"CAPTURE FAILED: {e}")
        return False

# Capture the image
image = capture_image()
image_path = f"/home/ediylabs/Desktop/opencv/mobilenet-ssd/images/img_{timestamp}.jpg"
print(image_path)
if not image or not os.path.exists(image_path):
    print(f"Error: Image not found at {image_path}")
else:
    # Load the captured image
    img = cv2.imread(image_path)

    # Check if the image is loaded correctly
    if img is None:
        print("Error: Image not loaded. Please check the file path.")
    else:
        print("Image loaded successfully.")

        # Resize the image to 300x300 for the network
        blob = cv2.dnn.blobFromImage(img, 1.0, (300, 300), (104.0, 177.0, 123.0), swapRB=True, crop=False)
        net.setInput(blob)

        # Perform forward pass to get detections
        detections = net.forward()

        # Process the detections (this part is specific to your detection needs)
        for i in range(detections.shape[2]):
            confidence = detections[0, 0, i, 2]
            if confidence > 0.2:  # Confidence threshold
                class_id = int(detections[0, 0, i, 1])
                left = int(detections[0, 0, i, 3] * img.shape[1])
                top = int(detections[0, 0, i, 4] * img.shape[0])
                right = int(detections[0, 0, i, 5] * img.shape[1])
                bottom = int(detections[0, 0, i, 6] * img.shape[0])

                # Draw bounding box around the detected object
                cv2.rectangle(img, (left, top), (right, bottom), (0, 255, 0), 2)

        # Show the image with detections
        cv2.namedWindow("Detection", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("Detection", 800, 600)
        cv2.imshow('Detection', img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
