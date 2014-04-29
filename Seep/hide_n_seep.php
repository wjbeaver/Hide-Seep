<?php

// get input
$inputArray = array(
'condition' => $_POST['condition'],
'description'=>$_POST['description'],
'device'=>$_POST['device'],
'email'=>$_POST['email'],
'firstname'=>$_POST['firstname'],
'flow'=>$_POST['flow'],
'found'=>$_POST['found'],
'hash'=>$_POST['hash'],
'honorific'=>$_POST['honorific'],
'lastname'=>$_POST['lastname'],
'latitude'=>$_POST['latitude'],
'longitude'=>$_POST['longitude'],
'middlename'=>$_POST['']
);

// get images
require("includes/UploadFile.php");

// create GPX

// if ($_POST[ 'condition']=='1') { echo json_encode(array( 'status'=>'success', 'message' => 'Logged in' )); } else { echo json_encode(array( 'status' => 'failu