import cv2
import tensorflow as tf
import numpy as np
from ultralytics import YOLO

# Load object detector (YOLOv8)
detector = YOLO("yolov8n.pt")  # make sure it's installed: `pip install ultralytics`

# Load bird classifier
classifier = tf.keras.models.load_model("bird_classifier.h5")

# Setup webcam
cap = cv2.VideoCapture(0)
frame_width, frame_height = 224, 224  # Input size for classifier

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Detect objects in the frame
    results = detector(frame)[0]

    for box in results.boxes:
        cls_id = int(box.cls[0])
        label = detector.names[cls_id]

        if "bird" in label.lower():
            # Get bounding box
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            bird_crop = frame[y1:y2, x1:x2]

            # Preprocess crop for classifier
            try:
                resized = cv2.resize(bird_crop, (frame_width, frame_height))
                input_data = np.expand_dims(resized / 255.0, axis=0)
                pred = classifier.predict(input_data, verbose=0)

                # Binary classification output
                bird_type = "Large Bird" if pred[0][0] > 0.8 else "Small Bird"

                # Draw bounding box
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                text = f"{label}: {bird_type}"
                cv2.putText(frame, text, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX,
                            0.8, (255, 255, 255), 2, cv2.LINE_AA)
                            
            except:
                continue  # Ignore resizing errors

    cv2.imshow("Bird Detection and Classification", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
