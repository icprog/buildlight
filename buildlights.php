<?php

$username = 'buildlight'; // was 'admin'
$password = 'bakedalmondsisgood'; // was 'password'

// $project_name = 'ca-build-dev-branch';
// $project_name = 'ca-build-test';

// handles multiple jobs
$project_names = array(
    'ca-build-demo',
    'ca-build-production',
    'ca-build-test',
    'ca-deploy-to-demo',
    'ca-deploy-to-production',
    'ca-deploy-to-test',
    'ca-landing-build-master-branch',
    'ca-landing-deploy-to-production',
);

$RED = 'red';
$GREEN = 'blue';
$YELLOW = 'yellow';
$script_dir = dirname(__FILE__) . "\\USBCMDAP\\release\\";

echo $script_dir;


function myErrorHandler($errno, $errstr, $errfile, $errline)
{
    print $errstr;
}

function exec_script($script_name)
{
    global $script_dir;
    exec($script_dir . $script_name);
}

function show_blue()
{
    exec_script('BlueOn.bat');
}

function show_green()
{
    exec_script('GreenOn.bat');
}

function show_red()
{
    exec_script('RedOn.bat');
}

function handle_job($job)
{
    global $GREEN, $RED;
    $color = $job['color'];

    if ($color == $GREEN) {
        show_green();
        return;
    }

    if ($color == $RED) {
        show_red();
        return;
    }

    show_blue();
}

function handle_jobs($jobs, $project_names)
{
    // the original code by Paul
    // only handles one job
    /*
    foreach ($jobs as &$job) {
        if ($job['name'] == $project_name) {
            return handle_job($job);
        }
    }
    */

    global $GREEN, $RED, $YELLOW;

    $jobs_matched = 0;
    $red_alert = false;

    foreach ($jobs as &$job) {

        if (in_array($job['name'], $project_names)) {

            $jobs_matched++;

            if ($job['color'] == $RED || $job['color'] == $YELLOW) {
                $red_alert = true;
                break;
            }
        }
    }

    // if no jobs are matched, assume network error
    if ($jobs_matched == 0) {
        show_blue();
    }

    else {
        if ($red_alert === true) {
            show_red();
        } else {
             show_green();
        }
    }

    return;
}

$old_error_handler = set_error_handler("myErrorHandler");
try {
    $context = stream_context_create(array(
        'http' => array(
            'header' => "Authorization: Basic " . base64_encode("$username:$password")
        )
    ));
    $string = file_get_contents("http://192.168.60.86:8080/api/json?depth=0", false, $context);
    $json_a = json_decode($string, true);
    $jobs = $json_a['jobs'];
    handle_jobs($jobs, $project_names);
} catch (Exception $e) {
    print $e->getMessage();
}