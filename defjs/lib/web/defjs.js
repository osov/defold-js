// https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html
let LibraryDefJs = {
    // This can be accessed from the bootstrap code in the .html file
    $DefJsLib: {
        _callback: null,

        execute_callback: function (callback_id) {
            if (DefJsLib._callback) {
                let message = "";
                if (arguments.length > 1) {
                    let parameters = [];
                    for (let i = 1; i < arguments.length; i++) parameters.push(arguments[i]);
                    message = JSON.stringify(parameters);
                }
                let msg = allocate(intArrayFromString(message), ALLOC_NORMAL);
                {{{makeDynCall("viii", "DefJsLib._callback")}}}(callback_id, msg);
                Module._free(msg);
            }
        },

        call_js: function (method, parameters, callback_id) {
            let method_name = UTF8ToString(method);
            let string_parameters = UTF8ToString(parameters);
            let path = method_name.split(".");
            let parent_object = window;
            let result_object = parent_object;
            let last_index = path.length - 1
            for (let index = 0; index < path.length; index++) {
                let item = path[index];
                if (parent_object[item]) {
                    if (index === last_index) {
                        result_object = parent_object[item];
                    } else {
                        parent_object = parent_object[item];
                    }
                } else {
                    let error = `Field or function "${method_name}" not found!`;
                    return JSON.stringify({error: error});
                }
            }
            let returned_value = result_object;
            if (typeof result_object === "function") {
                let array_parameters = [];
                try {
                    array_parameters = JSON.parse(string_parameters);
                } catch {
                }
                try {
                    let called_function = result_object.bind(parent_object);
                    if (callback_id > 0)
                        called_function(array_parameters[0], function () {
                            let return_parameters = [];
                            for (let i = 0; i < arguments.length; i++) return_parameters.push(arguments[i]);
                            DefJsLib.execute_callback(callback_id, ...return_parameters);
                        });
                    else
                        returned_value = called_function(...array_parameters);
                } catch (error) {
                    return JSON.stringify({error: error});
                }
            }
            if (returned_value !== undefined) {
                try {
                    return JSON.stringify({value: returned_value});
                } catch (error) {
                    return JSON.stringify({error: error});
                }
            }
        }
    },

    // These can be called from within the extension, in C++
    DefJs_RegisterCallback: function (callback) {
        DefJsLib._callback = callback;
    },

    DefJs_RemoveCallback: function () {
        DefJsLib._callback = null;
    },

    DefJs_CallJs: function (method, parameters, callback_id) {
        let result = DefJsLib.call_js(method, parameters, callback_id);
        if (result) return allocate(intArrayFromString(result), ALLOC_NORMAL);
    },
}

autoAddDeps(LibraryDefJs, '$DefJsLib');
mergeInto(LibraryManager.library, LibraryDefJs);