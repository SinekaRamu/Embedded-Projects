import tensorflow as tf

# Load your saved Keras model
model = tf.keras.models.load_model('bird_classifier_v1.h5')  # or use your in-memory model

# Create converter with explicit experimental new converter
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.experimental_new_converter = True  # Force new converter

# Optional optimizations for Raspberry Pi
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS]

try:
    tflite_model = converter.convert()
    with open('converted_model.tflite', 'wb') as f:
        f.write(tflite_model)
    print("Conversion successful!")
except Exception as e:
    print(f"Conversion failed: {str(e)}")