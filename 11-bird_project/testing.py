import cv2
import tensorflow as tf
import numpy as np

# Load your binary classification model
model = tf.keras.models.load_model("bird_classifier_v1.h5")

# Print model summary to understand expected input shape
model.summary()

# Get the expected input shape from the model
expected_height, expected_width = model.input_shape[1:3]  # This should be (128, 128) based on your training code

# Set video capture
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Cannot open camera")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break

    # Resize to the model's expected input size (not 224x224)
    resized_frame = cv2.resize(frame, (expected_width, expected_height))
    input_frame = np.expand_dims(resized_frame / 255.0, axis=0)  # Normalize

    try:
        # Predict
        prediction = model.predict(input_frame, verbose=0)
        confidence = prediction[0][0] if prediction[0][0] > 0.5 else 1 - prediction[0][0]
        confidence_percent = int(confidence * 100)
        label = "Large Bird" if prediction[0][0] > 0.5 else "Small Bird"

        # Draw bounding box (using full frame dimensions)
        box_color = (0, 255, 0) if label == "Large Bird" else (255, 0, 0)
        cv2.rectangle(frame, (0, 0), (frame.shape[1], frame.shape[0]), box_color, 2)
        
        # Show result on screen
        cv2.putText(frame, f"{label} ({confidence_percent}%)", 
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 
                    1, box_color, 2, cv2.LINE_AA)
        
        # Additional info
        cv2.putText(frame, "Press 'Q' to quit", 
                    (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 
                    0.6, (255, 255, 255), 1, cv2.LINE_AA)

        cv2.imshow('Bird Size Classification', frame)

    except Exception as e:
        print(f"Prediction error: {e}")
        cv2.imshow('Bird Size Classification', frame)

    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()