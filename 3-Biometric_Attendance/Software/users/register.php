<?php

//Connect to database
require '../connectDB.php';

// Get data from HTML form
$email = $_POST['email'] ?? null;
$name = $_POST['username'] ?? null;
$register_no = $_POST['registerno'] ?? null;
$section = $_POST['section'] ?? null;
$department = $_POST['department'] ?? null;
$batch = $_POST['batch'] ?? null;
$phone = $_POST['phone'] ?? null;
$fingerprint_id = $_POST['fingerid'] ?? null;

if ($fingerprint_id) {
    if ($name && $register_no && $section && $department && $batch && $email && $phone) {
        // **Case 1: Register new user (HTML Form Submission)**
        $sql = "INSERT INTO users (email, Uname, register_no, section, department, batch, phone, fingerprint_id) 
                VALUES ('$email', '$name', '$register_no', '$section', '$department', '$batch', '$phone', '$fingerprint_id')";      
        if ($conn->query($sql) === TRUE) {
            echo json_encode(["status" => "success", "message" => "User $name registered successfully!"]);
        } else {
            echo json_encode(["status" => "error", "message" => "Database insert error: " . $conn->error]);
        }
    }
    else {
        // **Case 2: ESP8266 Request (Checking Fingerprint)**
        $sql = "SELECT Uname FROM users WHERE fingerprint_id = '$fingerprint_id'";
        $result = $conn->query($sql);

        if ($result->num_rows > 0) {
            $row = $result->fetch_assoc();
            echo json_encode(["status" => "success", "message" => "Welcome   " . $row['Uname'] . "!"]);
        } else {
            echo json_encode(["status" => "error", "message" => "Fingerprint not recognized!"]);
        }
    }
}
else {
    echo json_encode(["status" => "error", "message" => "Invalid request!"]);
}

$conn->close();
?>
