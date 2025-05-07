import tensorflow as tf
import numpy as np
import cv2
import os

# Load your model
model = tf.keras.models.load_model("fingerprint_model.h5")

# Label mapping (example – modify based on your training labels)
labels = ['A+', 'A-', 'AB+', 'AB-', 'B+', 'B--', 'O+', 'O-']

# Load and preprocess a fingerprint image
img = cv2.imread("dataset/dataset_blood_group/AB+/cluster_2_419.BMP", cv2.IMREAD_GRAYSCALE)  # test image path
img = np.stack([img]*3, axis=-1)
img = cv2.resize(img, (64, 64))           # match training size
img = img.astype("float32") / 255.0       # normalize if trained that way
img = np.expand_dims(img, axis=0)         # add batch dimension

# Predict
prediction = model.predict(img)
predicted_label = labels[np.argmax(prediction)]

print("Predicted Blood Group:", predicted_label)