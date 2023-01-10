#include <tree_sitter/parser.h>

#undef DEBUGGING

/* for debug */
#ifdef DEBUGGING
#  include <stdio.h>
#  define DEBUG(fmt,...)  fprintf(stderr, "scanner.c DEBUG: " fmt, __VA_ARGS__)
#else
#  define DEBUG(fmt,...)
#endif

enum TokenType {
  TOKEN_Q_STRING_BEGIN,
  TOKEN_QUOTELIKE_END,
  TOKEN_Q_STRING_CONTENT,
  TOKEN_ESCAPE_SEQUENCE,
};

#define TOKEN(type) \
  do {                            \
    DEBUG("token(%s)\n", #type);  \
    lexer->result_symbol = type;  \
    return true;                  \
  } while(0)

static void skip_whitespace(TSLexer *lexer)
{
  while(1) {
    int c = lexer->lookahead;
    if(!c)
      return;
    if(iswspace(c))
      lexer->advance(lexer, true);
      /* continue */
    else
      return;
  }
}

void *tree_sitter_perl_external_scanner_create() { return NULL; }
void tree_sitter_perl_external_scanner_destroy(void *p) {}
void tree_sitter_perl_external_scanner_reset(void *p) {}
unsigned tree_sitter_perl_external_scanner_serialize(void *p, char *buffer) { return 0; }
void tree_sitter_perl_external_scanner_deserialize(void *p, const char *b, unsigned n) {}
bool tree_sitter_perl_external_scanner_scan(
  void *payload,
  TSLexer *lexer,
  const bool *valid_symbols
) {
  /* TODO: If any of the symbols could match a wordlike, scan for a wordlike
   * and capture it here
   */
  DEBUG("Scanning [%d,%d,%d,%d] at > '%c' (U+%04X)\n",
      valid_symbols[0], valid_symbols[1], valid_symbols[2], valid_symbols[3],
      lexer->lookahead, lexer->lookahead);

  // The only time we'd ever be looking for both BEGIN and END is during an error
  // condition. Abort in that case
  if(valid_symbols[TOKEN_Q_STRING_BEGIN] && valid_symbols[TOKEN_QUOTELIKE_END])
    return false;

  if(valid_symbols[TOKEN_Q_STRING_BEGIN]) {
    skip_whitespace(lexer);

    if(lexer->lookahead == '\'') {
      lexer->advance(lexer, false);

      TOKEN(TOKEN_Q_STRING_BEGIN);
    }

    /* TODO: capture q SYMBOL */
  }

  if(valid_symbols[TOKEN_ESCAPE_SEQUENCE]) {
    if(lexer->lookahead == '\\') {
      lexer->advance(lexer, false);
      // TODO: \xDD, \uXXXX, etc...
      lexer->advance(lexer, false);

      TOKEN(TOKEN_ESCAPE_SEQUENCE);
    }
  }

  if(valid_symbols[TOKEN_Q_STRING_CONTENT]) {
    bool valid = false;
    // TODO: custom ending token
    while(lexer->lookahead && lexer->lookahead != '\\' && lexer->lookahead != '\'') {
      valid = true;
      lexer->advance(lexer, false);
    }

    if(valid)
      TOKEN(TOKEN_Q_STRING_CONTENT);
  }

  if(valid_symbols[TOKEN_QUOTELIKE_END]) {
    if(lexer->lookahead == '\'') {
      lexer->advance(lexer, false);

      TOKEN(TOKEN_QUOTELIKE_END);
    }
  }

  return false;
}
