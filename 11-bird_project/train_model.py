from tensorflow.keras.preprocessing.image import ImageDataGenerator
from tensorflow.keras.applications import MobileNetV2
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Dense, GlobalAveragePooling2D
from tensorflow.keras.optimizers import Adam

img_size = 224
batch_size = 32

train_gen = ImageDataGenerator(rescale=1./255).flow_from_directory(
    'dataset/train', target_size=(img_size, img_size), batch_size=batch_size, class_mode='binary')

val_gen = ImageDataGenerator(rescale=1./255).flow_from_directory(
    'dataset/val', target_size=(img_size, img_size), batch_size=batch_size, class_mode='binary')

base_model = MobileNetV2(include_top=False, weights='imagenet', input_shape=(img_size, img_size, 3))
x = base_model.output
x = GlobalAveragePooling2D()(x)
# output = Dense(2, activation='softmax')(x)  # 2 classes: Large and Small
output = Dense(1, activation='sigmoid')(x)  # Binary classification
model = Model(inputs=base_model.input, outputs=output)

# Freeze base model
for layer in base_model.layers:
    layer.trainable = False

model.compile(optimizer=Adam(), loss='binary_crossentropy', metrics=['accuracy'])

model.fit(train_gen, validation_data=val_gen, epochs=5)

# --- Save as .h5 file ---
model.save('bird_classifier.h5')
print("✅ Model saved as 'bird_classifier.h5'")