<?php

//Connect to database
require '../connectDB.php';

// **Mark Users as Absent if No Attendance Recorded**
$sql = "INSERT INTO attendance (user_id, fingerprint_id, status) 
        SELECT users.id, users.fingerprint_id, 'Absent' 
        FROM users 
        WHERE users.id NOT IN (
            SELECT attendance.user_id FROM attendance WHERE DATE(check_in_time) = CURDATE()
        )";

if ($conn->query($sql) === TRUE) {
    echo "Absent records added successfully.";
} else {
    echo "Error: " . $conn->error;
}

$conn->close();
?>
