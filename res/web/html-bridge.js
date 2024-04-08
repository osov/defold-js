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

    // Если вызывать эту функцию через call_js_with_callback, то так не стоит делать, т.к. можно вызвать "чужой" колбек
    // setInterval(callback, params.timeout, "callback test_function3");
}