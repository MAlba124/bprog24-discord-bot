#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include "evaluator.h"
#include "parser.h"
#include "vector.h"

void on_ready(struct discord *client, const struct discord_ready *event) {
  (void)client;
  log_info("Logged in as %s", event->user->username);
}

void on_calc(struct discord *client, const struct discord_message *event) {
  if (strlen(event->content) == 0) {
    struct discord_create_message params = {
        .content = "You're missing an expression!",
        .allowed_mentions =
            &(struct discord_allowed_mention){.replied_user = false},
        .message_reference =
            &(struct discord_message_reference){.message_id = event->id,
                                                .channel_id = event->channel_id,
                                                .guild_id = event->guild_id}};
    discord_create_message(client, event->channel_id, &params, NULL);
    return;
  }

  char *res_str = NULL;

  ParseError parse_error = PE_OK;
  size_t parse_error_index = 0;
  struct vector_token parsed_tokens =
      parse_math(event->content, &parse_error, &parse_error_index);
  if (parse_error != PE_OK) {
    vector_free_token(&parsed_tokens);

    struct vector_char error_format_pointer_str;
    vector_init_char(&error_format_pointer_str);
    for (size_t i = 0; i < parse_error_index - 1; i++) {
      vector_push_char(&error_format_pointer_str, ' ');
    }
    vector_push_char(&error_format_pointer_str, '\0');
    assert(asprintf(&res_str,
                    "Failed to parse your expression. Error code: `%s`\n"
                    "```ansi\n"
                    "%s\n"
                    "%s\033[1;31m^\033[0m Here"
                    "```",
                    parse_error_to_str(parse_error), event->content,
                    error_format_pointer_str.buf) != -1);
    vector_free_char(&error_format_pointer_str);
  } else {
    EvaluatorResult error;
    double res = evaluate(&parsed_tokens, &error);
    vector_free_token(&parsed_tokens);
    if (error != ER_OK) {
      assert(asprintf(&res_str,
                      "Failed to evaluate your expression. Error code: `%s`",
                      evaluator_result_to_str(error)) != -1);
    } else {
      if (res == (long long)res) {
        assert(asprintf(&res_str, "```%.0f```", res) != -1);
      } else {
        assert(asprintf(&res_str, "```%f```", res) != -1);
      }
    }
  }

  struct discord_create_message params = {
      .content = res_str,
      .allowed_mentions =
          &(struct discord_allowed_mention){.replied_user = false},
      .message_reference =
          &(struct discord_message_reference){.message_id = event->id,
                                              .channel_id = event->channel_id,
                                              .guild_id = event->guild_id}};
  discord_create_message(client, event->channel_id, &params, NULL);
  free(res_str);
}

void on_ping(struct discord *client, const struct discord_message *event) {
  char *res_str = NULL;
  assert(asprintf(&res_str, ":ping_pong: Pong! Latency: `%i` ms",
                  discord_get_ping(client)) != -1);
  struct discord_create_message params = {.content = res_str};
  discord_create_message(client, event->channel_id, &params, NULL);
  free(res_str);
}

int main(void) {
  ccord_global_init();
  struct discord *client = discord_config_init("config.json");
  assert(client != NULL);

  discord_set_on_ready(client, &on_ready);
  discord_set_on_commands(client, (char *const[]){"calc", "c"}, 2, &on_calc);
  discord_set_on_commands(client, (char *const[]){"ping", "p"}, 2, &on_ping);

  discord_run(client);

  discord_cleanup(client);
  ccord_global_cleanup();
}
