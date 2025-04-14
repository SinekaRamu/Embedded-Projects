import cv2
import numpy as np
from keras.models import load_model

import os

model_path = "G:\My Drive\TinyML\sign_language\CNNmodel.h5"
if os.path.exists(model_path):
    model = load_model(model_path)
    print("Model loaded successfully!")
else:
    print("❌ Model file not found!")

# Load your trained model
# model = load_model('C:/Users/sinek/Desktop/Embedded-Projects/6-sign_language/CNNmodel.keras')  # or 'CNNmodel.h5' if you're using that format

def crop_image(img, x, y, w, h):
    return img[y:y+h, x:x+w]

def main():
    cam_capture = cv2.VideoCapture(0)

    while True:
        ret, image_frame = cam_capture.read()
        if not ret:
            print("Failed to grab frame.")
            break

        # Crop the ROI from the frame
        im2 = crop_image(image_frame, 300, 300, 300, 300)

        # Convert to grayscale
        gray = cv2.cvtColor(im2, cv2.COLOR_BGR2GRAY)

        # Apply Gaussian blur
        blurred = cv2.GaussianBlur(gray, (15, 15), 0)

        # Resize to 28x28
        resized = cv2.resize(blurred, (28, 28), interpolation=cv2.INTER_AREA)

        # Normalize pixel values (important for model consistency)
        normalized = resized / 255.0

        # Prepare input for model: (1, 28, 28, 1)
        model_input = np.expand_dims(normalized, axis=(0, -1))

        # Make prediction
        prediction = model.predict(model_input)
        predicted_class = np.argmax(prediction)

        # Display result on frame
        cv2.putText(image_frame, f'Prediction: {predicted_class}', (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # Show the webcam feed
        cv2.imshow("Webcam Feed", image_frame)

        # Break loop on 'q' key
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cam_capture.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
