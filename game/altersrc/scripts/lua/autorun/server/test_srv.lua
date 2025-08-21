-- test_srv.lua
include("include/hook.lua")

hook.Add("PlayerSay", "BlockTest", function(ply, text, team)
    if text == "blockme" then
        print("blocking message")
        return false
    end

	return true
end, false)
