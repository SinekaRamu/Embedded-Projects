<?php
require '../connectDB.php';

$id = $_POST['id'] ?? null;
$name = $_POST['username'] ?? null;
$register_no = $_POST['registerno'] ?? null;
$fingerprint_id = $_POST['fingerid'] ?? null;
$email = $_POST['email'] ?? null;
$section = $_POST['section'] ?? null;
$department = $_POST['department'] ?? null;
$batch = $_POST['batch'] ?? null;
$phone = $_POST['phone'] ?? null;

if ($id && $name && $register_no && $fingerprint_id && $email && $section && $department && $batch && $phone) {
    $sql = "UPDATE users SET 
                Uname='$name', 
                register_no='$register_no', 
                fingerprint_id='$fingerprint_id', 
                email='$email',
                section='$section',
                department='$department',
                batch='$batch',
                phone='$phone'
            WHERE id='$id'";

    if ($conn->query($sql) === TRUE) {
        echo json_encode(["status" => "success", "message" => "User updated successfully!"]);
    } else {
        echo json_encode(["status" => "error", "message" => "Error: " . $conn->error]);
    }
} else {
    echo json_encode(["status" => "error", "message" => "All fields are required!"]);
}

$conn->close();
?>
