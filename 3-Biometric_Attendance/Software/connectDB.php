<?php

header("Content-Type: application/json");
/* Database connection settings */
	$servername = "localhost";
    $username = "root";		//put your phpmyadmin username.(default is "root")
    $password = "";			//if your phpmyadmin has a password put it here.(default is "root")
    $dbname = "biometricattendance";
    
	$conn = mysqli_connect($servername, $username, $password, $dbname);
	
	if ($conn->connect_error) {
        echo json_encode(["status" => "error", "message" => "Database connection failed"]);
        exit();
    }
?>