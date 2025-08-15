
$modelname weapons/v_bugbait.mdl
$cdmaterials models\weapons\v_hand models\weapons\v_bugbait

// whole body
$body studio "Bugbait_reference.smd"

$origin 0 0 68

$attachment 0 "ValveBiped.Bip01_R_Hand" 4 -4 0

$sequence drawback "Throw" frames 0 4 ACT_VM_HAULBACK 1 { event 3900 4 }
$sequence throw "Throw" frames 9 20 ACT_VM_THROW 1 { event 3005 1 }

$sequence idle01 "Idle01" loop fps 30 activity ACT_VM_IDLE 1
$sequence draw "Draw" fps 90 snap activity ACT_VM_DRAW 1
$sequence holster "Holster" fps 30 activity ACT_VM_HOLSTER 1
$sequence squeeze "Squeeze" fps 30 activity ACT_VM_SECONDARYATTACK 1
