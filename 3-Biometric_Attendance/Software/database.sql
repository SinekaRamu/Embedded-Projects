CREATE DATABASE biometricattendance;

-- Use the newly created database
USE biometricattendance;

-- Create users table for fingerprint data
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
   email VARCHAR(100) NOT NULL,
    Uname VARCHAR(100) NOT NULL,
    register_no VARCHAR(100) UNIQUE NOT NULL,
    fingerprint_id INT UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create attendance records table
CREATE TABLE attendance (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    fingerprint_id INT NOT NULL,
    check_in_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status ENUM('Present', 'Absent', 'Late') DEFAULT 'Present',
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE devices (
    device_id INT AUTO_INCREMENT PRIMARY KEY,
    device_token VARCHAR(50) UNIQUE NOT NULL,
    device_name VARCHAR(100),
    registered_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT INTO devices (device_token, device_name) 
VALUES ('BioMetric', 'Idhaya-1');

ALTER TABLE users
ADD COLUMN section VARCHAR(50) NOT NULL,
ADD COLUMN department VARCHAR(100) NOT NULL,
ADD COLUMN batch VARCHAR(20) NOT NULL,
ADD COLUMN phone VARCHAR(15) NOT NULL;

ALTER TABLE attendance 
ADD COLUMN device_id INT, 
ADD CONSTRAINT fk_device FOREIGN KEY (device_id) REFERENCES devices(device_id) ON DELETE SET NULL;
