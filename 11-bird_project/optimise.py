import cv2
import tensorflow as tf
import numpy as np

# Load your binary classification model
model = tf.keras.models.load_model("bird_classifier.h5")

# Set video capture
cap = cv2.VideoCapture(0)
frame_width, frame_height = 224, 224  # Use the size your model expects

if not cap.isOpened():
    print("Cannot open camera")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break

    # Resize and preprocess the frame
    resized_frame = cv2.resize(frame, (frame_width, frame_height))
    input_frame = np.expand_dims(resized_frame / 255.0, axis=0)  # Normalize

    # Predict
    prediction = model.predict(input_frame)
    label = "Large Bird" if prediction[0][0] > 0.5 else "Small Bird"

    # Show result on screen
    cv2.putText(frame, f"Prediction: {label}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 
                1, (0, 255, 0), 2, cv2.LINE_AA)
    

    cv2.imshow('Bird Classification', frame)

    if cv2.waitKey(1) == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
