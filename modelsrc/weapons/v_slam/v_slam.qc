/*
==============================================================================

Slam

==============================================================================
*/

$modelname weapons/v_slam.mdl
$cdmaterials models\weapons\v_hand models\weapons\v_slam

// whole body
$body studio "V_slam_reference.smd" 

$origin 0 0 68

$sequence throw_idle "throw_idle" fps 30 activity ACT_SLAM_THROW_IDLE 1
$sequence throw_idle_ND "throw_idle_ND"	fps 30 activity ACT_SLAM_THROW_ND_IDLE 1
$sequence throw_throw1 "throw_throw1" fps 30 activity ACT_SLAM_THROW_THROW 1 
$sequence throw_throw2 "throw_throw2" fps 30 activity ACT_SLAM_THROW_THROW2 1 
$sequence throw_throw_ND1 "throw_throw1_ND" fps 30 activity ACT_SLAM_THROW_THROW_ND 1 
$sequence throw_throw_ND2 "throw_throw2_ND" fps 30 activity ACT_SLAM_THROW_THROW_ND2 1
$sequence throw_draw "throw_draw" snap fps 90 activity ACT_SLAM_THROW_DRAW 1
$sequence throw_draw_ND "throw_draw_ND" snap fps 90 activity ACT_SLAM_THROW_ND_DRAW 1
$sequence throw_to_tripmine_ND "throw_to_tripmine_ND" fps 30 activity ACT_SLAM_THROW_TO_TRIPMINE_ND 1 { event AE_CL_PLAYSOUND 30 "Weapon_SLAM.TripMineMode" }
$sequence throw_detonate "throw_detonate" snap fps 30 activity ACT_SLAM_THROW_DETONATE 1
$sequence throw_to_stickwall "throw_to_stickwall" fps 30 activity ACT_SLAM_THROW_TO_STICKWALL 1
$sequence throw_to_stickwall_ND "throw_to_stickwall_ND" fps 30 activity ACT_SLAM_THROW_TO_STICKWALL_ND 1

$sequence detonator_draw "detonator_draw" snap fps 90 activity ACT_SLAM_DETONATOR_DRAW 1
$sequence detonator_idle "detonator_idle" fps 30 activity ACT_SLAM_DETONATOR_IDLE 1
$sequence detonator_detonate "detonator_detonate" snap fps 30 activity ACT_SLAM_DETONATOR_DETONATE 1
$sequence detonator_throw_draw "detonator_throw_draw" snap fps 90 activity ACT_SLAM_DETONATOR_THROW_DRAW 1
$sequence detonator_holster "detonator_holster" fps 30 activity ACT_SLAM_DETONATOR_HOLSTER 1

$sequence tripmine_idle "tripmine_idle" fps 30 activity ACT_SLAM_TRIPMINE_IDLE 1
$sequence tripmine_draw "tripmine_draw" snap fps 90 activity ACT_SLAM_TRIPMINE_DRAW 1
$sequence tripmine_to_throw "tripmine_to_throw" fps 30 activity ACT_SLAM_TRIPMINE_TO_THROW_ND 1
$sequence tripmine_attach1 "tripmine_attach1" snap fps 30 activity ACT_SLAM_TRIPMINE_ATTACH 1
$sequence tripmine_attach2 "tripmine_attach2" snap fps 30 activity ACT_SLAM_TRIPMINE_ATTACH2 1

$sequence stickwall_idle "stickwall_idle" fps 30 activity ACT_SLAM_STICKWALL_IDLE 1
$sequence stickwall_idle_ND "stickwall_idle_ND" fps 30 activity	ACT_SLAM_STICKWALL_ND_IDLE 1
$sequence stickwall_attach1 "stickwall_attach1" snap fps 30 activity ACT_SLAM_STICKWALL_ATTACH 1
$sequence stickwall_attach2 "stickwall_attach2"	snap fps 30 activity	ACT_SLAM_STICKWALL_ATTACH2 1
$sequence stickwall_to_throw "stickwall_to_throw" fps 30 activity ACT_SLAM_STICKWALL_TO_THROW 1
$sequence stickwall_to_throw_ND "stickwall_to_throw_ND" fps 30 activity	ACT_SLAM_STICKWALL_TO_THROW_ND 1

/*

$sequence stickwall_attach_ND1 "stickwall_attach_ND1" fps 30 activity ACT_SLAM_STICKWALL_ND_ATTACH 1
$sequence stickwall_attach_ND2 "stickwall_attach_ND2" fps 30 activity ACT_SLAM_STICKWALL_ND_ATTACH2 1
$sequence stickwall_draw "stickwall_draw" snap fps 30 activity	ACT_SLAM_STICKWALL_DRAW 1
$sequence stickwall_draw_ND "stickwall_draw_ND"	snap fps 30 activity ACT_SLAM_STICKWALL_ND_DRAW 1

$sequence stickwall_to_tripmine_ND "stickwall_to_tripmine" fps 30 activity ACT_SLAM_STICKWALL_TO_TRIPMINE_ND 1
$sequence stickwall_detonate "stickwall_detonate" fps 30 activity ACT_SLAM_STICKWALL_DETONATE 1
$sequence stickwall_deton_holster "stickwall_detonator_holster"	fps 30 activity ACT_SLAM_STICKWALL_DETONATOR_HOLSTER 1

$sequence throw_detonater_holster "throw_detonator_holster" fps 30 activity ACT_SLAM_THROW_DETONATOR_HOLSTER 1
$sequence tripmine_to_stickwall "tripmine_to_stickwall" fps 30 activity ACT_SLAM_TRIPMINE_TO_STICKWALL_ND 1
$sequence detonator_stickwall_draw "detonator_stickwall_draw" snap fps 30 activity ACT_SLAM_DETONATOR_STICKWALL_DRAW 1

*/


