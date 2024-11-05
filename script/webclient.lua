local webclientlib = require "webclient"
local webclient = webclientlib.create()

requests = requests or {}

local function resopnd(request, result)
    if not request.response then
        return
    end

    local content, errmsg = webclient:get_respond(request.req)
    local info = webclient:get_info(request.req)

    if result == 0 then
        request.response(true, content, info)
    else
        request.response(false, errmsg, info)
    end
end

function tick()
    local finish_key, result = webclient:query()
    if not finish_key then
        return
    end

    local request = requests[finish_key]
    assert(request)

    pcall(resopnd, request, result)

    local content, errmsg = webclient:get_respond(request.req)
    print(finish_key, content, errmsg)

    webclient:remove_request(request.req)
    requests[finish_key] = nil
end

function request(url, get, post, response)
    if get then
        local i = 0
        for k, v in pairs(get) do
            k = webclient:url_encoding(k)
            v = webclient:url_encoding(v)

            url = string.format("%s%s%s=%s", url, i == 0 and "?" or "&", k, v)
            i = i + 1
        end
    end

    local post_bin = false
    if post and type(post) == "string" then
        post_bin = true
    end

    if post and type(post) == "table" then
        local data = {}
        for k, v in pairs(post) do
            k = webclient:url_encoding(k)
            v = webclient:url_encoding(v)

            table.insert(data, string.format("%s=%s", k, v))
        end
        post = table.concat(data, "&")
    end

    local req, key = webclient:request(url, post)
    if not req then
        return false
    end
    assert(key)

    if post_bin then
        webclient:set_httpheader(req, "Content-Type: application/octet-stream")
    end

    requests[key] = {
        url = url,
        req = req,
        response = response
    }
end
