#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/log.h>

#include "include/command.h"
#include "include/conversion.h"
#include "include/evaluator.h"
#include "include/gnuplot.h"
#include "include/parser.h"
#include "include/vector.h"

const struct Command commands[N_COMMANDS] = {
    {.longf = "calc",
     .shortf = "c",
     .description =
         "`<expression>`. Caluclate an expression e.g. `+calc 10 / 5`. "
         "Supports `+-*/^` and these functions: [`sqrt`, `sin`, `tan`, `cos`]",
     .callback = &on_calc},
    {.longf = "ping",
     .shortf = "p",
     .description = "Show the bots latency",
     .callback = &on_ping},
    {.longf = "plot",
     .shortf = "pl",
     .description = "`<expression>`. Plot a function e.g. `+plot x**2` or "
                    "multiple functions: `+plot log10(pi * x), sin(x)`",
     .callback = &on_plot},
    {.longf = "tobin",
     .shortf = "tb",
     .description = "`<number>` Convert `<number>` to binary representation",
     .callback = &on_tobin},
    {.longf = "tohex",
     .shortf = "th",
     .description = "`<number>` Convert `<number>` to hexadecimal representation",
     .callback = &on_tohex},
    {.longf = "todec",
     .shortf = "td",
     .description = "`<number>` Convert `<number>` to decimal representation",
     .callback = &on_todec},
    {.longf = "why",
     .shortf = "w",
     .description = "Describe why something is the way it is",
     .callback = &on_why},
    {.longf = "help",
     .shortf = "h",
     .description = "Show this message",
     .callback = &on_help}};

static void reply_msg(struct discord *client, const struct discord_message *msg,
                      char *content) {
  struct discord_create_message params = {
      .content = content,
      .allowed_mentions =
          &(struct discord_allowed_mention){.replied_user = false},
      .message_reference =
          &(struct discord_message_reference){.message_id = msg->id,
                                              .channel_id = msg->channel_id,
                                              .guild_id = msg->guild_id}};
  discord_create_message(client, msg->channel_id, &params, NULL);
}

void on_calc(struct discord *client, const struct discord_message *event) {
  if (strlen(event->content) == 0) {
    reply_msg(client, event, "You're missing and expression!");
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
        assert(asprintf(&res_str, "`%.0f`", res) != -1);
      } else {
        assert(asprintf(&res_str, "`%f`", res) != -1);
      }
    }
  }

  reply_msg(client, event, res_str);
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

void on_plot(struct discord *client, const struct discord_message *event) {
  int exit_status;
  struct vector_char pngbuf = gnuplot_plot(event->content, &exit_status);
  if (exit_status == EXIT_FAILURE || pngbuf.len == 0) {
    reply_msg(client, event, "Something went wrong ploting your expression :(");
    vector_free_char(&pngbuf);
    return;
  }

  struct discord_create_message params = {
      .attachments =
          &(struct discord_attachments){
              .size = 1,
              .array = &(struct discord_attachment){.filename = "plot.png",
                                                    .content = pngbuf.buf,
                                                    .size = pngbuf.len}},
      .allowed_mentions =
          &(struct discord_allowed_mention){.replied_user = false},
      .message_reference =
          &(struct discord_message_reference){.message_id = event->id,
                                              .channel_id = event->channel_id,
                                              .guild_id = event->guild_id}};
  discord_create_message(client, event->channel_id, &params, NULL);

  vector_free_char(&pngbuf);
}

static void vector_char_push_str(struct vector_char *vec, char *str) {
  while (*str != '\0') {
    /* vector_push_char(vec, *str++); */ // TODO: Test
    vector_push_char(vec, *str);
    str++;
  }
}

void on_help(struct discord *client, const struct discord_message *event) {
  struct vector_char help_msg;
  vector_init_char(&help_msg);
  vector_char_push_str(&help_msg, "**Commands**\n");

  for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    char *command_help;
    struct Command cmd = commands[i];
    assert(asprintf(&command_help, "`+%s` (`+%s`) %s\n", cmd.longf, cmd.shortf, cmd.description) != -1);
    vector_char_push_str(&help_msg, command_help);
    free(command_help);
  }

  vector_push_char(&help_msg, '\0');

  struct discord_create_message params = {
    .content = help_msg.buf,
  };
  discord_create_message(client, event->channel_id, &params, NULL);

  vector_free_char(&help_msg);
}

void on_tobin(struct discord *client, const struct discord_message *event) {
  enum ConversionError cerr;
  long long num = convert_from_string(event->content, &cerr);
  if (cerr != CE_OK) {
    char *res_str = NULL;
    assert(asprintf(&res_str, "Failed to covert! Error code: `%s`",
                    conversion_error_to_str(cerr)) != -1);
    reply_msg(client, event, res_str);
    free(res_str);
    return;
  }

  struct vector_char res_str;
  vector_init_char(&res_str);
  convert_to_bin(num, &res_str);
  char *bin_str = NULL;
  assert(asprintf(&bin_str, "`%s`", res_str.buf) != -1);
  reply_msg(client, event, bin_str);

  vector_free_char(&res_str);
  free(bin_str);
}

void on_tohex(struct discord *client, const struct discord_message *event) {
  enum ConversionError cerr;
  long long num = convert_from_string(event->content, &cerr);
  if (cerr != CE_OK) {
    char *res_str = NULL;
    assert(asprintf(&res_str, "Failed to covert! Error code: `%s`",
                    conversion_error_to_str(cerr)) != -1);
    reply_msg(client, event, res_str);
    free(res_str);
    return;
  }

  struct vector_char res_str;
  vector_init_char(&res_str);
  convert_to_hex(num, &res_str);
  char *hex_str = NULL;
  assert(asprintf(&hex_str, "`%s`", res_str.buf) != -1);
  reply_msg(client, event, hex_str);

  vector_free_char(&res_str);
  free(hex_str);
}

void on_todec(struct discord *client, const struct discord_message *event) {
  enum ConversionError cerr;
  long long num = convert_from_string(event->content, &cerr);
  if (cerr != CE_OK) {
    char *res_str = NULL;
    assert(asprintf(&res_str, "Failed to covert! Error code: `%s`",
                    conversion_error_to_str(cerr)) != -1);
    reply_msg(client, event, res_str);
    free(res_str);
    return;
  }

  char *dec_str = NULL;
  assert(asprintf(&dec_str, "`%lld`", num) != -1);
  reply_msg(client, event, dec_str);

  free(dec_str);
}

void on_why(struct discord *client, const struct discord_message *event) {
  reply_msg(client, event, "¯\\_(ツ)_/¯");
}
