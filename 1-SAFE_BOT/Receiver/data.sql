CREATE DATABASE safebot;

use safebot;

CREATE TABLE devices (
    device_id INT AUTO_INCREMENT PRIMARY KEY,
    device_token VARCHAR(50) UNIQUE NOT NULL,
    device_name VARCHAR(100),
    registered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE sensor_data (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id INT,
    methane FLOAT,
    co FLOAT,
    heart_rate INT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE CASCADE
);

INSERT INTO devices (device_token,device_name) VALUES("safe1234","BOT-1");
INSERT INTO devices (device_token,device_name) VALUES("BioMetric","Idhaya-1");