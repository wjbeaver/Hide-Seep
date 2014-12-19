<?php
	$datadir = "data/images/";
	$response=new stdClass();
	
	// get json
	$markup = file_get_contents('php://input');
	
	$parsed = json_decode($markup);
	
	$indx = count($parsed->objects);
	$indx--;
	
	$uploadID = $parsed->objects[$indx]->UPLOADID;
	
	$imageID = $parsed->objects[$indx]->IMAGEID;
	
	$markupCount = $parsed->objects[$indx]->markup_count;
	
	$datadir .= $uploadID."/";
	
	$markup_path = $datadir."/markup_".$imageID;
	
	// check to see if exists
	if (!file_exists ( $markup_path )) {
		mkdir($markup_path, 0775);
    }
    
    $count = 0;
    foreach (scandir($markup_path) as $file) {
        if ('.' === $file) continue;
        if ('..' === $file) continue;

        $temp = explode("_", $file);
        $num = explode(".", $temp[1]);
        if (intVal($num[0])>$count) {
            $count = intVal($num[0]);
        }
    } 
    
    if ($count>$markupCount) {
        $markupCount += $count-$markupCount;
    }   
    
    $parsed->objects[$indx]->markup_count = $markupCount;
        
    if (!file_put_contents($markup_path."/markup_".$markupCount.".txt", json_encode($parsed))) {
        $response->response = "Save Failed!";
    } else {
        $response->response = "Saved!";
    }
	
	echo json_encode($response);
?>