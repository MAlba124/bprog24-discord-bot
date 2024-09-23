#ifndef __H_COMMAND
#define __H_COMMAND 1

#include <concord/discord.h>

struct Command {
  char *longf;
  char *shortf;
  void (*callback)(struct discord *client, const struct discord_message *event) ;
};

void on_calc(struct discord *client, const struct discord_message *event);
void on_ping(struct discord *client, const struct discord_message *event);
void on_plot(struct discord *client, const struct discord_message *event);
void on_help(struct discord *client, const struct discord_message *event);
void on_tobin(struct discord *client, const struct discord_message *event);
void on_tohex(struct discord *client, const struct discord_message *event);
void on_todec(struct discord *client, const struct discord_message *event);

#endif /* __H_COMMAND */