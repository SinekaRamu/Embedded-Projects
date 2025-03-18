<?php
header("Content-Type: application/json");
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "safebot";

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Get data from request
$device_token = $_POST['device_token'];
$methane = $_POST['methane'];
$co = $_POST['co'];
$heart_rate = $_POST['heart_rate'];

// Validate device token and get device_id
$sql = "SELECT device_id FROM devices WHERE device_token = '$device_token'";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $device_id = $row['device_id'];

    // Insert sensor data with device_id
    $insertSQL = "INSERT INTO sensor_data (device_id, methane, co, heart_rate) 
                  VALUES ('$device_id', '$methane', '$co', '$heart_rate')";

    if ($conn->query($insertSQL) === TRUE) {
        echo "Data inserted successfully";
    } else {
        echo "Error: " . $insertSQL . "<br>" . $conn->error;
    }
} else {
    echo "Invalid device token!";
}

$conn->close();
?>
