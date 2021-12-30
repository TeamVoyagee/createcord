#include <orca/debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <orca/discord.h>

lua_State *Lua;

int lua_print(lua_State *L) {
    const char* txt = luaL_checkstring(L, 1);
    lua_pushstring(L, txt);

    printf("%s\n", txt);

    return 1;
}

void on_ready(struct discord *client)
{
  const struct discord_user *bot = discord_get_self(client);

  log_info("Succesfully connected to Discord as %s#%s!",
           bot->username, bot->discriminator);
}

void on_lua(struct discord *client, const struct discord_message *msg)
{
    if (msg->author->bot) return;
    if (!(msg->author->id == 908726285558239332)) {
        struct discord_create_message_params params = { .content = "no." };
        discord_create_message(client, msg->channel_id, &params, NULL);
    } else {
        struct discord_create_message_params params = { .content = "Executed." };
        discord_create_message(client, msg->channel_id, &params, NULL);
        if (luaL_dostring(Lua, msg->content) == LUA_OK) {
            lua_pop(Lua, lua_gettop(Lua));
        }
    }
}

int main(int argc, char *argv[])
{
    Lua = luaL_newstate();
    
    lua_pushcfunction(Lua, lua_print);
    lua_setglobal(Lua, "print");


    lua_pushinteger(Lua, 69);
    lua_setglobal(Lua, "token");

    const char *config_file;
    if (argc > 1)
        config_file = argv[1];
    else
        config_file = "./config.json";

    orca_global_init();
    struct discord *client = discord_config_init(config_file);

    discord_set_on_ready(client, &on_ready);
    discord_set_on_command(client, "lua", &on_lua);

    discord_run(client);

    discord_cleanup(client);
    orca_global_cleanup();
    lua_close(Lua);
}