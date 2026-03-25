#define MINILUA_IMPLEMENTATION
#include "minilua.h"
#include "../../src/papagaio_plugin.h"
#include "../../src/papagaio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct { lua_State *L; } LuaState;

static char *lua_command_handler(Papagaio *ctx, const char *cmd_name, int argc, const char **argv, const size_t *argl, void *ud);
static void lua_finalizer(void *ud);

static int lua_print_bridge(lua_State *L) {
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++) {
        const char *s = lua_tostring(L, i);
        fprintf(stderr, "%s%s", s ? s : "nil", (i < n) ? "\t" : "");
    }
    fprintf(stderr, "\n");
    return 0;
}

/* Registration from Lua: papagaio.register("name", function) */
static int lua_pap_register(lua_State *L) {
    PapPlugin *plugin = (PapPlugin *)lua_touserdata(L, lua_upvalueindex(1));
    const char *name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    lua_getglobal(L, "_PAP_COMMANDS");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setglobal(L, "_PAP_COMMANDS");
        lua_getglobal(L, "_PAP_COMMANDS");
    }
    lua_pushstring(L, name);
    lua_pushvalue(L, 2);
    lua_settable(L, -3);
    
    plugin->register_command(plugin, name, lua_command_handler, lua_touserdata(L, lua_upvalueindex(2)));
    return 0;
}

static char *lua_command_handler(Papagaio *ctx, const char *cmd_name, int argc, const char **argv, const size_t *argl, void *ud)
{
    if (argc == 0 && strcmp(cmd_name, "lua") == 0) return strdup("");
    LuaState *s = (LuaState *)ud;
    
    /* Populate 'params' table for the current call */
    lua_newtable(s->L);
    for (int i = 0; i < argc; i++) {
        lua_pushlstring(s->L, argv[i], argl[i]);
        lua_rawseti(s->L, -2, i + 1);
    }
    lua_setglobal(s->L, "params");

    if (strcmp(cmd_name, "lua") == 0) {
        /* $lua executes the first argument as script */
        if (luaL_loadbuffer(s->L, argv[0], argl[0], "papagaio:lua") != LUA_OK ||
            lua_pcall(s->L, 0, 1, 0) != LUA_OK) {
            const char *err = lua_tostring(s->L, -1);
            char *r = (char *)malloc(strlen(err) + 32);
            sprintf(r, "[lua error: %s]", err ? err : "unknown");
            lua_pop(s->L, 1);
            return r;
        }
    } else {
        /* Other commands look up registered Lua function */
        lua_getglobal(s->L, "_PAP_COMMANDS");
        if (lua_isnil(s->L, -1)) { lua_pop(s->L, 1); return strdup(""); }
        
        lua_getfield(s->L, -1, cmd_name);
        if (!lua_isfunction(s->L, -1)) { lua_pop(s->L, 2); return strdup(""); }
        
        for (int i=0; i<argc; i++) lua_pushlstring(s->L, argv[i], argl[i]);
        if (lua_pcall(s->L, argc, 1, 0) != LUA_OK) {
            const char *err = lua_tostring(s->L, -1);
            char *r = (char *)malloc(strlen(err) + 32);
            sprintf(r, "[lua error in %s: %s]", cmd_name, err ? err : "unknown");
            lua_pop(s->L, 1);
            return r;
        }
        lua_remove(s->L, -2); /* remove _PAP_COMMANDS table */
    }

    const char *res = lua_tostring(s->L, -1);
    char *out = res ? strdup(res) : strdup("");
    lua_pop(s->L, 1);
    return out;
}

static void lua_finalizer(void *ud) {
    LuaState *s = (LuaState *)ud;
    lua_close(s->L);
    free(s);
}

int papagaio_plugin_init(PapPlugin *plugin, Papagaio *ctx) {
    LuaState *s = (LuaState *)malloc(sizeof(LuaState));
    s->L = luaL_newstate();
    luaL_openlibs(s->L);
    lua_register(s->L, "print", lua_print_bridge);

    plugin->name = "lua";
    plugin->version = "1.0.0";

    /* Create the 'args' table */
    int argc = 0; char **argv = NULL;
    plugin->get_args(plugin, &argc, &argv);
    lua_newtable(s->L);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(s->L, argv[i]);
        lua_rawseti(s->L, -2, i);
    }
    lua_setglobal(s->L, "args");

    /* papagaio table for scripting */
    lua_newtable(s->L);
    lua_pushlightuserdata(s->L, plugin);
    lua_pushlightuserdata(s->L, s);
    lua_pushcclosure(s->L, lua_pap_register, 2);
    lua_setfield(s->L, -2, "register");
    lua_setglobal(s->L, "papagaio");

    plugin->register_command(plugin, "lua", lua_command_handler, s);
    plugin->register_finalizer(plugin, lua_finalizer, s);
    return 0;
}
