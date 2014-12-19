<?php
    $host =     'localhost';
    $database = 'geo1';
    $table =    'test';
    $column =   'geom';
    $user =     'billuser';
    $pass =     'myCory79+';
    $table =    'public.video';
    $schema =   'public';

	$response=new stdClass();
	
	$videoTrack = file_get_contents('php://input');
	
	$parsedVideoTrack = json_decode($videoTrack);
	
	$videos = $parsedVideoTrack->videos;

	$tracks = $parsedVideoTrack->tracks;
	
	function getPointsToLine($track) {
	    $result = "ST_SetSRID(ST_MakeLine(ARRAY[";
	    
	    foreach ($track->points as $point) {
	        $result .= "ST_MakePoint($point->lon, $point->lat), ";
	    }
	    
	    $result .= "]), 4326)";
	    return $result;
	}
	
    $connection = pg_connect("host=$host dbname=$database user=$user password=$pass")
        or die("Could not connect");
    echo "Connected successfully\n";
    
    $query = "SELECT id_pk FROM $table ORDER BY id_pk DESC LIMIT 1";
    $result = pg_query($connection, $query);
    
    if (pg_num_rows($result)>0) {
        $id_pk = pg_fetch_result($result, 0, 0)+1;
    } else {
        $id_pk = 1;
    }
    
	foreach ($videos as $video) {
	
        $query = "INSERT INTO $table ( id_pk, videoid_pk, uploadid_fk, title, url, type, trackid_fk, timezone, utc, description, geom) VALUES ($id_pk, '$video->videoid_pk', '$video->uploadid_fk', '$video->title', '$video->url', $video->type, '$video->trackid_fk', '$video->timezone', '$video->utc', '$video->description', ST_SetSRID(ST_MakePoint($video->longitude, $video->latitude), 4326))";
        $result = pg_query($connection, $query);   
	    
	    $id_pk++;
	}
	
	$table = "public.track_lines";
	$table1 = "public.track_polypoints";
	
    $query = "SELECT id_pk FROM $table ORDER BY id_pk DESC LIMIT 1";
    $result = pg_query($connection, $query);
    
    if (pg_num_rows($result)>0) {
        $id_pk = pg_fetch_result($result, 0, 0)+1;
    } else {
        $id_pk = 1;
    }
	
    $query = "SELECT id_pk FROM $table1 ORDER BY id_pk DESC LIMIT 1";
    $result = pg_query($connection, $query);
    
    if (pg_num_rows($result)>0) {
        $id_pk1 = pg_fetch_result($result, 0, 0)+1;
    } else {
        $id_pk1 = 1;
    }
	
	$response->query = $query;
	$response->response = "ok";
	
	foreach ($tracks as $track) {
	    
        $query = "INSERT INTO $table ( id_pk, trackid_pk, uploadid_fk, title, type, description, geom) VALUES ($id_pk, '$track->trackid_pk', '$track->uploadid_fk', '$track->title', $track->type, '$track->timezone', '$track->description', ".getPointsToLine($track).")";
        $result = pg_query($connection, $query);   
	    
	    $id_pk++;
    }

    pg_close($connection);
  
    echo json_encode($response);
?>