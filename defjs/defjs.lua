local M = {}

local function parse_result(result)
    if result ~= nil then
        local result_object = json.decode(result)
        if result_object.error then
            error(result_object.error, 3)
        end
        return result_object.value
    end
end

local function call_js_with_callback(method, parameters, callback, remove_callback)
    if defjs then
        defjs.call_js(method, parameters ~= nil and json.encode({ parameters }) or nil, remove_callback, function(self, result)
            local is_ok, tbl = pcall(json.decode, result)
            if is_ok then
                callback(unpack(tbl))
            else
                callback()
            end
        end)
    end
end

---Выполнить функцию JS или получить значение переменной.
---@param method string выполняемый метод js, объект или имя переменной
---@param ... any параметры функции
---@return any результат выполнения функции или значение переменной
function M.call_js(method, ...)
    local result
    if defjs then
        if ... == nil then
            result = defjs.call_js(method)
        else
            result = defjs.call_js(method, json.encode({ ... }))
        end
    end
    return parse_result(result)
end

---Выполняет функцию JS с передачей колбека в функцию в качестве параметра (method(parameters, callback))).
---@param method string выполняемая метод JS
---@param parameters any параметры метода
---@param callback function функция обратного вызова
function M.call_js_with_callback(method, parameters, callback)
    call_js_with_callback(method, parameters, callback, true)
end

---Выполняет функцию JS с передачей "статического" колбека в функцию в качестве параметра (method(parameters, callback))).
---@param method string выполняемый метод JS
---@param parameters any параметры метода
---@param callback function функция обратного вызова
function M.call_js_with_static_callback(method, parameters, callback)
    call_js_with_callback(method, parameters, callback, false)
end

return M