# Мост Lua <-> JS

## Сторона Lua

### function call_js(method, ...)

Возвращает результат выполнения синхронной функции js, объект или значение переменной.

- method: выполняемый метод js, объект или имя переменной
- ...: параметры функции

### function call_js_with_callback(method, parameters, callback)

Выполняет функцию JS с передачей колбека в функцию в качестве параметра (method(parameters, callback))). Вызов колбека
из js должен вызываться однократно, иначе может быть вызван "чужой" колбек.

- method: выполняемый метод JS
- parameters: параметры метода
- callback: функция обратного вызова

### function call_js_with_static_callback(method, parameters, callback)

Выполняет функцию JS с передачей "статического" колбека в функцию в качестве параметра (method(parameters, callback))).
Отличие от call_js_with_callback, в том, что колбек будет сохранен и может использоваться многократно.

- method: выполняемый метод JS
- parameters: параметры метода
- callback: функция обратного вызова

## Сторона JS

```js
function test_function(param1, param2) {
    console.log("test_function");
    return param1 + param2;
}

function test_function2(params, callback) {
    console.log("test_function2 params:", params)
    addEventListener("click", function (ev) {
        console.log(ev);
        callback(ev.clientX, ev.clientY);
    })
}

function test_function3(params, callback) {
    console.log("test_function3 params:", params);
    setTimeout(callback, params.timeout, "callback test_function3");
    
    // Если вызывать функцию через call_js_with_callback, то так не стоит делать, т.к. можно вызвать "чужой" колбек
    // setInterval(callback, params.timeout, "callback test_function3");
}

```

Вызов из lua:

```lua
local js = require("defjs.defjs")

js.call_js("console.log", "asd", 1234)
js.call_js("console.log", { sdc = 23 })

print("location", js.call_js("location.href"))
local result = js.call_js("test_function", 25, 50)
print("result test_function", result)

js.call_js_with_static_callback("test_function2", { param1 = "param1", param2 = 123 }, function(x, y)
    print("mouse_click", x, y)
end)

js.call_js_with_callback("test_function3", { timeout = 2000 }, function(value)
    print("timer:", value)
end)
```