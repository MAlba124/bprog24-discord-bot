#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include "include/command.h"

void on_ready(struct discord *client, const struct discord_ready *event) {
  (void)client;
  log_info("Logged in as %s", event->user->username);

  struct discord_activity activities[] = {
    {
        .name = "+help",
        .type = DISCORD_ACTIVITY_GAME,
      },
  };

  struct discord_presence_update status = {
      .activities =
          &(struct discord_activities){
              .size = sizeof(activities) / sizeof *activities,
              .array = activities,
          },
      .status = "online",
      .afk = false,
      .since = discord_timestamp(client),
  };

  discord_update_presence(client, &status);
}

int main(void) {
  ccord_global_init();
  struct discord *client = discord_config_init("config.json");
  assert(client != NULL);

  discord_set_on_ready(client, &on_ready);
  for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    discord_set_on_commands(
        client, (char *const[]){commands[i].longf, commands[i].shortf}, 2,
        commands[i].callback);
  }

  discord_run(client);

  discord_cleanup(client);
  ccord_global_cleanup();
}
