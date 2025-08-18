-- sandbox.lua
GM = GM or {}

function GM:GiveDefaultItems( ply )
    if not ply then
        print("Player is nil!")
        return
    end

    ply:GiveAmmo(255, "Pistol")
    ply:GiveAmmo(255, "AR2")
    ply:GiveAmmo(5, "AR2AltFire")
    ply:GiveAmmo(255, "SMG1")
    ply:GiveAmmo(1, "smg1_grenade")
    ply:GiveAmmo(255, "Buckshot")
    ply:GiveAmmo(32, "357")
    ply:GiveAmmo(3, "rpg_round")
    ply:GiveAmmo(16, "XBowBolt")
    ply:GiveAmmo(1, "grenade")
    ply:GiveAmmo(2, "slam")

    ply:GiveWeapon("weapon_crowbar")
    ply:GiveWeapon("weapon_stunstick")
    ply:GiveWeapon("weapon_pistol")
    ply:GiveWeapon("weapon_357")
    ply:GiveWeapon("weapon_smg1")
    ply:GiveWeapon("weapon_ar2")
    ply:GiveWeapon("weapon_shotgun")
    ply:GiveWeapon("weapon_frag")
    ply:GiveWeapon("weapon_crossbow")
    ply:GiveWeapon("weapon_rpg")
    ply:GiveWeapon("weapon_slam")
    ply:GiveWeapon("weapon_physcannon")

   	ply:GiveWeapon("weapon_physgun")
end
