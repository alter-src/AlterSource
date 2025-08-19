-- sandbox.lua
include("include/hook.lua")

hook.Add("GiveDefaultItems", function(ply)
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

    ply:GiveItem("weapon_crowbar")
    ply:GiveItem("weapon_stunstick")
    ply:GiveItem("weapon_pistol")
    ply:GiveItem("weapon_357")
    ply:GiveItem("weapon_smg1")
    ply:GiveItem("weapon_ar2")
    ply:GiveItem("weapon_shotgun")
    ply:GiveItem("weapon_frag")
    ply:GiveItem("weapon_crossbow")
    ply:GiveItem("weapon_rpg")
    ply:GiveItem("weapon_slam")
    ply:GiveItem("weapon_physcannon")

   	ply:GiveItem("weapon_physgun")
end, true)

hook.Add("PlayerSpawn", function(ply)
	if not ply then
        print("Player is nil!")
        return
    end

	-- ThePixelMoon: after we spawn, just give ourselves 100 armor because why not
	ply:SetArmor(100)
end, true)