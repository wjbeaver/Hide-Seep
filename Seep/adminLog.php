<?php
require("../proxy/seeper.php");

if ($_POST['username']==$string2 && $_POST['password']==$string) {
    $data = "pass";
} else {
    $data = "fail";
}
echo $data;
?>