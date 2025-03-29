<?php
require '../connectDB.php';

$date = $_POST['date'];
$department = $_POST['department'];
$section = $_POST['section'];

$sql = "SELECT u.Uname, u.register_no, u.section, u.department, u.batch
        FROM users u
        LEFT JOIN attendance a 
        ON a.fingerprint_id = u.fingerprint_id 
        AND DATE(a.check_in_time) = '$date'
        WHERE u.department = '$department' 
        AND u.section = '$section'
        AND a.fingerprint_id IS NULL 
        ORDER BY u.register_no ASC";

$result = $conn->query($sql);
$attendanceData = [];

if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $attendanceData[] = $row;
    }
    echo json_encode(["status" => "success", "data" => $attendanceData]);
} else {
    echo json_encode(["status" => "error", "message" => "No attendance records found!"]);
}

$conn->close();
?>