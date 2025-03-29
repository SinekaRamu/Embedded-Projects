<?php
require '../connectDB.php';

$sql = "SELECT a.id, u.Uname, u.register_no, u.section, u.department, u.batch, 
               DATE(a.check_in_time) AS date, TIME(a.check_in_time) AS time, a.status 
        FROM attendance a
        INNER JOIN users u ON a.fingerprint_id = u.fingerprint_id 
        ORDER BY a.check_in_time DESC";

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
