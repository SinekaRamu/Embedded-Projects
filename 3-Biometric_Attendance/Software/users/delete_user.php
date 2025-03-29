<?php
require '../connectDB.php';

$id = $_POST['id'] ?? null;

if ($id) {
    $sql = "DELETE FROM users WHERE id='$id'";
    
    if ($conn->query($sql) === TRUE) {
        echo json_encode(["status" => "success", "message" => "User deleted successfully!"]);
    } else {
        echo json_encode(["status" => "error", "message" => "Error: " . $conn->error]);
    }
} else {
    echo json_encode(["status" => "error", "message" => "User ID is required!"]);
}

$conn->close();
?>
