import numpy as np
import tensorflow as tf
from PIL import Image

# Load TFLite model
interpreter = tf.lite.Interpreter(model_path="bird_classifier_v1.h5")
interpreter.allocate_tensors()

# Get input and output tensors
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

def classify_bird(image_path):
    # Preprocess image
    img = Image.open(image_path).resize((128, 128))
    img_array = np.array(img, dtype=np.float32) / 255.0
    img_array = np.expand_dims(img_array, axis=0)
    
    # Run inference
    interpreter.set_tensor(input_details[0]['index'], img_array)
    interpreter.invoke()
    prediction = interpreter.get_tensor(output_details[0]['index'])
    
    return "large" if prediction[0][0] > 0.5 else "small"

# Example usage
print(classify_bird("/dataset/test/large/peacock/34G8YRG6QKNG.jpg"))