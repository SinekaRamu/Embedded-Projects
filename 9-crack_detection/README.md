# Raspberry pi zero 2W

1. Flash Raspberry Pi OS

   - Download Raspberry Pi OS Lite (32-bit)

   - Use Raspberry Pi Imager to flash to microSD

2. Install Dependencies

```
# Update system
sudo apt update && sudo apt upgrade -y

# Install core packages
sudo apt install -y \
    python3-opencv \
    libopencv-dev \
    python3-pip \
    python3-smbus \
    i2c-tools \
    python3-picamera2

# Python packages
pip3 install --upgrade pip
pip3 install pyserial smtplib email-to
```
