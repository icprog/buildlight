<?php
$username = 'admin';
$password = 'password';
// $project_name = 'ca-build-dev-branch';
$project_name = 'ca-build-test';

$RED = 'red';
$GREEN = 'blue';
$script_dir = dirname(__FILE__)."\\USBCMDAP\\release\\";

echo $script_dir;


function myErrorHandler($errno, $errstr, $errfile, $errline) {
  print $errstr;
}

function exec_script($script_name) {
  global $script_dir;
  exec($script_dir.$script_name);
}

function show_blue() {
  exec_script('BlueOn.bat');
}

function show_green() {
  exec_script('GreenOn.bat');
}

function show_red() {
  exec_script('RedOn.bat');
}

function handle_job($job) {
  global $GREEN, $RED;
  $color = $job['color'];

  if($color == $GREEN) {
      show_green();
    return;
  }

  if($color == $RED) {
    show_red();
    return;
  }

  show_blue();
}

function handle_jobs($jobs, $project_name) {
  foreach ($jobs as &$job) {
      if($job['name'] == $project_name) {
        return handle_job($job);
      }
  }
  show_blue();
}

$old_error_handler = set_error_handler("myErrorHandler");
try {
  $context = stream_context_create(array(
      'http' => array(
          'header'  => "Authorization: Basic " . base64_encode("$username:$password")
      )
  ));
  $string = file_get_contents("http://192.168.60.86:8080/api/json?depth=0", false, $context);
  $json_a=json_decode($string,true);
  $jobs = $json_a['jobs'];
  handle_jobs($jobs, $project_name);
} catch(Exception $e) {
  print $e->getMessage();
}

?>