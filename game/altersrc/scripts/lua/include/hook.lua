-- scripts/lua/include/hook.lua
hook = hook or {}

_G.GM    = _G.GM    or {}

-- Add a hook.
-- Usages supported:
--   hook.Add("Name", function(...) end)                 	 -- normal hook (Hooks)
--   hook.Add("Name", function(...) end, true)         		 -- GM hook (isGM = true)
--   hook.Add("Name", "UniqueName", function(...) end) 		 -- named hook (Hooks)
--   hook.Add("Name", "UniqueName", function(...) end, true) -- named GM hook
function hook.Add(hookName, nameOrFunc, maybeFunc, maybeIsGM)
    -- determine if caller passed (hookName, func, isGM)
    if type(nameOrFunc) == "function" then
        local func = nameOrFunc
        local isGM = (type(maybeFunc) == "boolean" and maybeFunc) or (type(maybeIsGM) == "boolean" and maybeIsGM) or false
        local t = isGM and _G.GM
        t[hookName] = t[hookName] or {}

        -- insert as anonymous (numeric) entry
        table.insert(t[hookName], func)
        return
    end

    -- otherwise caller used (hookName, uniqueName, func, [isGM])
    if type(nameOrFunc) == "string" and type(maybeFunc) == "function" then
        local uniqueName = nameOrFunc
        local func = maybeFunc
        local isGM = (type(maybeIsGM) == "boolean" and maybeIsGM) or false
        local t = isGM and _G.GM
        t[hookName] = t[hookName] or {}

        -- set or replace by string key
        t[hookName][uniqueName] = func
        return
    end

    error("hook.Add: invalid arguments")
end

function hook.Call(hookName, ...)
    local function runHooks(tbl, ...)
        local hks = tbl[hookName]
        if not hks then return end
        for k, v in pairs(hks) do
            if type(v) == "function" then
                local ok, ret = pcall(v, ...)
                if not ok then
                    local name = (type(k) == "string" and k) or "(anonymous)"
                    print(string.format("[HOOK ERROR] %s in hook %s: %s", name, hookName, tostring(ret)))
                end
            else
                -- if somehow non-function got stored, warn (helps debug)
                print(string.format("[HOOK WARN] Non-function hook entry for %s (key=%s)", hookName, tostring(k)))
            end
        end
    end

    -- run gamemode hooks first (to match earlier behaviour)
    runHooks(_G.GM, ...)
end
