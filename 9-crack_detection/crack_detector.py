import os
import time
import cv2
import smtplib
import serial
import subprocess
from email.message import EmailMessage

# === Serial Port Setup ===
SERIAL_PORT = '/dev/serial0'
BAUD_RATE = 9600

try:
    ser=serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    print(f"Serial connected on {SERIAL_PORT}")
except Exception as e:
    print(f"Failed to open serial port: {e}")
    ser = None

# === Config ===
SENDER_EMAIL = 'webstannescet@gmail.com'
APP_PASSWORD = 'trfniocdfegurawn'
RECEIVER_EMAIL = 'gayathrimurugan108@gmail.com'

# === Camera image Capture ===
def get_frame():
    """Capture a single high-res image with timestamp"""
    filename = "crack_img.jpg"
    
    try:
        subprocess.run([
            "libcamera-jpeg",
            "-o", filename,
            "--immediate",
            "--width", "640",
            "--height", "480",
            "--denoise", "off",
            "--sharpness", "2",
            "--timeout", "2000",
            "--nopreview"
            ], check=True)
        print(f"IMAGE SAVED: {filename}")
        return True
    
    except subprocess.CalledProcessError as e:
        print(f"CAPTURE FAILED: {e}")
        return False
    
# === Crack Detection Function ===
def detect_crack(image_path):
    image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    inverted = cv2.bitwise_not(image)
    _, thresh = cv2.threshold(inverted, 200, 255, cv2.THRESH_BINARY)
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    for cnt in contours:
        x,y,w,h = cv2.boundingRect(cnt)
        if w >20 and h>20:
            return True, x, y,w, h
    return False, 0,0,0,0

# === Serial Data Reader Functio ===
def read_serial_data():
    if ser and ser.in_waiting:
        try:
            line = ser.readline().decode('utf-8').strip()
            print(f"Serial Data: {line}")
            return line
        except Exception as e:
            print(f"Error reading serial: {e}")
    print("No serial data received")
    return None

# === Email Alert Function ===
def send_email_alert(lat, lon, crack_img, width, height, x, y, serial_data):
    msg = EmailMessage()
    msg['Subject'] = 'Crack Detected from Camera'
    msg['From'] = SENDER_EMAIL
    msg['To'] = RECEIVER_EMAIL
    msg.set_content(
        f"Serial Data: \n{serial_data}\n"
        #f"Crack detected at: \nLatitude: {lat}\nLongitude: {lon}\n"
        f"Crack detected at: {width}px x {height}px\n Position: ({x}, {y})\n\n"
        )
    
    image_with_rect = cv2.imread(crack_img)
    cv2.rectangle(image_with_rect, (x,y), (x + width, y+height), (0,0,255), 2)
    modified_img_path = "crack_with_rectangle.jpg"
    cv2.imwrite(modified_img_path, image_with_rect)
    
    with open(modified_img_path, 'rb') as f:
        msg.add_attachment(f.read(), maintype='image', subtype='jpeg', filename='crack_detected.jpg')
        
    with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp:
        smtp.login(SENDER_EMAIL, APP_PASSWORD)
        smtp.send_message(msg)
        ser.write(b"/CONFIRMED\n")
        print("Email sent successfully")
        
# === Main Logic ===
while True:
    # read serial data
    serial_info = read_serial_data()
    print(f"Current Serial Info: {serial_info}")
    
    if serial_info:
        frame = get_frame()
        img_path = "crack_img.jpg"
        
        if not frame or not os.path.exists(img_path):
            print(f"Image Error: not found at {img_path}")
            
        # Crack detection
        crack_found, x, y, w, h = detect_crack(img_path)
        if crack_found:
            print(f"Crack Detected: {w}px x {h}px at position ({x}, {y})")
            serial_info = read_serial_data()
            print(serial_info)
            send_email_alert("12.9716", "77.5946", img_path, w, h, x, y, serial_info)
        else:
            print("No crack detected.")
        
    time.sleep(5)
        
        
