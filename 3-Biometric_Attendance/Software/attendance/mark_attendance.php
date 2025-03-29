<?php

//Connect to database
require '../connectDB.php';

// Get fingerprint ID from ESP8266
$device_token = $_POST['device_token'];
$fingerprint_id = $_POST['fingerid'] ?? null;

// Validate device token and get device_id
$sql = "SELECT device_id FROM devices WHERE device_token = '$device_token'";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $device_id = $row['device_id'];
    
    if ($fingerprint_id) {
        // **Step 1: Check if Fingerprint ID Exists in Users Table**
        $sql = "SELECT id, Uname FROM users WHERE fingerprint_id = ?";
        $stmt = $conn->prepare($sql);
        $stmt->bind_param("i", $fingerprint_id);
        $stmt->execute();
        $result = $stmt->get_result();
    
        if ($result->num_rows > 0) {
            $row = $result->fetch_assoc();
            $user_id = $row['id'];
            $name = $row['Uname'];
    
            // **Step 2: Check if Attendance is Already Marked Today**
            $sql = "SELECT id FROM attendance WHERE user_id = ? AND DATE(check_in_time) = CURDATE()";
            $stmt = $conn->prepare($sql);
            $stmt->bind_param("i", $user_id);
            $stmt->execute();
            $result = $stmt->get_result();
    
            if ($result->num_rows > 0) {
                echo json_encode(["status" => "error", "message" => "Attendance already marked for today"]);
            } else {
                // **Step 3: Determine Status (Present or Late)**
                $current_time = date("H:i:s"); // Get current time
                $late_time = "09:30:00"; // Time after which attendance is marked as "Late"
                $status = ($current_time > $late_time) ? "Late" : "Present";
    
                // **Step 4: Insert Attendance Record**
                $sql = "INSERT INTO attendance (user_id, fingerprint_id, check_in_time, status) VALUES (?, ?, NOW(), ?)";
                $stmt = $conn->prepare($sql);
                $stmt->bind_param("iis", $user_id, $fingerprint_id, $status);
    
                if ($stmt->execute()) {
                    echo json_encode(["status" => "success", "message" => " $status  for $name"]);
                } else {
                    echo json_encode(["status" => "error", "message" => "Failed to mark attendance"]);
                }
            }
        } else {
            echo json_encode(["status" => "error", "message" => "Fingerprint not recognized"]);
        }
        $stmt->close();
    } else {
        echo json_encode(["status" => "error", "message" => "Invalid request!"]);
    }
} else {
    echo "Invalid device token!";
}


$conn->close();
?>