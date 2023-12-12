#define EXTENSION_NAME defjs
#define LIB_NAME "defjs"
#define MODULE_NAME LIB_NAME

#define DLIB_LOG_DOMAIN LIB_NAME

#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_HTML5)

typedef void (*ObjectMessage)(const int callback_id, const char* message);

extern "C" void DefJs_RegisterCallback(ObjectMessage cb_string);
extern "C" void DefJs_RemoveCallback();
extern "C" char* DefJs_CallJs(const char* method, const char* parameters, const int callback_id);

dmScript::LuaCallbackInfo* callback;
dmArray<dmScript::LuaCallbackInfo*> callbacks;
dmArray<bool> remove_callbacks;

static int RegisterCallback(dmScript::LuaCallbackInfo* cbk, const bool remove_callback)
{
    for (int i = 0; i < callbacks.Size(); i++)
    {
        if (callbacks[i] == 0) {
            callbacks[i] = cbk;
            remove_callbacks[i] = remove_callback;
            return i + 1;
        }
    }
    if (callbacks.Full()) {
        callbacks.OffsetCapacity(1);
        remove_callbacks.OffsetCapacity(1);
    }
    callbacks.Push(cbk);
    remove_callbacks.Push(remove_callback);
    return callbacks.Size();
}

static void InvokeCallback(dmScript::LuaCallbackInfo* cbk, const char* message)
{
    if (!dmScript::IsCallbackValid(cbk)) return;
    lua_State* L = dmScript::GetCallbackLuaContext(cbk);
    DM_LUA_STACK_CHECK(L, 0);
    if (!dmScript::SetupCallback(cbk))
    {
        dmLogError("Failed to setup callback");
        return;
    }
    lua_pushstring(L, message);
    dmScript::PCall(L, 2, 0);
    dmScript::TeardownCallback(cbk);
}

static void DefJs_SendStringMessage(const int callback_id, const char* message)
{
    if (callback_id > callbacks.Size()) {
        dmLogError("Callback with id '%d' not registered!", callback_id);
        return;
    }
    int index = callback_id - 1;
    dmScript::LuaCallbackInfo* cbk = callbacks[index];
    InvokeCallback(cbk, message);
    if (cbk and remove_callbacks[index]) {
        dmScript::DestroyCallback(cbk);
        callbacks[index] = 0;
    }
}

static int Init(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    callback = dmScript::CreateCallback(L, 1);
    return 0;
}

static int CallJs(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    int callback_id = lua_isnoneornil(L, 4) ? 0 : RegisterCallback(dmScript::CreateCallback(L, 4), lua_toboolean(L, 3));
    const char* result = DefJs_CallJs(luaL_checkstring(L, 1), lua_isnoneornil(L, 2) ? "" : luaL_checkstring(L, 2), callback_id);
    if (result == 0 || strcmp(result, "") == 0) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, result);
    }
    free((void*)result);
    return 1;
}

static const luaL_reg Module_methods[] =
{
    {"init", Init},
    {"call_js", CallJs},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result InitializeExtension(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    DefJs_RegisterCallback((ObjectMessage)DefJs_SendStringMessage);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExtension(dmExtension::Params* params)
{
    DefJs_RemoveCallback();
    return dmExtension::RESULT_OK;
}

#else // unsupported platforms

static dmExtension::Result InitializeExtension(dmExtension::Params *params) {
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExtension(dmExtension::Params *params) {
    return dmExtension::RESULT_OK;
}

#endif

DM_DECLARE_EXTENSION(EXTENSION_NAME,
LIB_NAME, 0, 0, InitializeExtension, 0, 0, FinalizeExtension)