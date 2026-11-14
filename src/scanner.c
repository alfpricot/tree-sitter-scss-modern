#include "tree_sitter/parser.h"

#include <wctype.h>

enum TokenType {
    DESCENDANT_OP,
    PSEUDO_CLASS_SELECTOR_COLON,
    ERROR_RECOVERY,
    CONCAT,
    MAP_KEY,
};

static inline void advance(TSLexer *lexer) { lexer->advance(lexer, false); }

static inline void skip(TSLexer *lexer) { lexer->advance(lexer, true); }

void *tree_sitter_scss_external_scanner_create() { return NULL; }

void tree_sitter_scss_external_scanner_destroy(void *payload) {}

void tree_sitter_scss_external_scanner_reset(void *payload) {}

unsigned tree_sitter_scss_external_scanner_serialize(void *payload, char *buffer) { return 0; }

void tree_sitter_scss_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {}

// Whether `c` can appear in a name: an ASCII letter or digit, an underscore, a hyphen, or any
// non-ASCII character. `iswalnum` alone would reject non-ASCII letters under the C locale.
static inline bool is_name_char(int32_t c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-' ||
           c >= 0x80;
}

// Advances past a quoted string: the lookahead is the opening quote, and on return it is the
// character after the closing quote. A raw newline ends an unterminated string, since a CSS string
// cannot contain one.
static void skip_string(TSLexer *lexer) {
    int32_t quote = lexer->lookahead;
    advance(lexer);
    while (lexer->lookahead != quote && lexer->lookahead != '\n' && !lexer->eof(lexer)) {
        if (lexer->lookahead == '\\') {
            advance(lexer);
            if (lexer->eof(lexer)) {
                return;
            }
        }
        advance(lexer);
    }
    if (lexer->lookahead == quote) {
        advance(lexer);
    }
}

// Advances past an interpolation body: the lookahead is the opening brace, and on return it is the
// character after the matching closing brace. Braces inside a quoted string do not count.
static void skip_interpolation(TSLexer *lexer) {
    int depth = 0;
    do {
        if (lexer->lookahead == '"' || lexer->lookahead == '\'') {
            skip_string(lexer);
            continue;
        }
        if (lexer->lookahead == '{') {
            depth++;
        } else if (lexer->lookahead == '}') {
            depth--;
        }
        advance(lexer);
    } while (depth > 0 && !lexer->eof(lexer));
}

// Whether `c` can open what follows the colon of a pseudo-class selector: the first character of
// an identifier (ASCII letter, underscore, hyphen, or any non-ASCII character), an escape, an
// interpolation, or the start of a comment that may sit between the colon and the name.
static inline bool can_follow_pseudo_class_colon(int32_t c) {
    if (c < 0x80) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-' || c == '\\' ||
               c == '#' || c == '/';
    }
    return true;
}

// Reads ahead from the character after a colon to decide whether the colon belongs to a selector:
// true when a `{` opens a block before a `;` or `}` ends the text, false otherwise. A `{` that
// opens an interpolation, and anything inside a quoted string or a comment, does not count. A `//`
// line comment is recognized only outside parentheses, so `url(http://host/path)` is not cut short.
static bool reaches_block_before_end(TSLexer *lexer) {
    bool in_block_comment = false;
    int paren_depth = 0;
    int32_t previous = 0;
    while (!lexer->eof(lexer)) {
        int32_t c = lexer->lookahead;
        if (in_block_comment) {
            advance(lexer);
            if (c == '*' && lexer->lookahead == '/') {
                advance(lexer);
                in_block_comment = false;
                previous = 0;
            }
            continue;
        }
        if (c == ';' || c == '}') {
            return false;
        }
        if (c == '{') {
            if (previous != '#') {
                return true;
            }
            skip_interpolation(lexer);
            previous = '}';
            continue;
        }
        if (c == '"' || c == '\'') {
            skip_string(lexer);
            previous = c;
            continue;
        }
        if (c == '\\') {
            advance(lexer);
            if (!lexer->eof(lexer)) {
                advance(lexer);
            }
            previous = 0;
            continue;
        }
        if (c == '/') {
            advance(lexer);
            if (lexer->lookahead == '*') {
                advance(lexer);
                in_block_comment = true;
            } else if (lexer->lookahead == '/' && paren_depth == 0) {
                while (lexer->lookahead != '\n' && !lexer->eof(lexer)) {
                    advance(lexer);
                }
            }
            previous = 0;
            continue;
        }
        if (c == '(') {
            paren_depth++;
        } else if (c == ')' && paren_depth > 0) {
            paren_depth--;
        }
        previous = c;
        advance(lexer);
    }
    return false;
}

// Scans a bare-word map key: identifier characters followed, after optional whitespace, by a single
// colon. The token ends before the colon. The base grammar's plain value token would otherwise
// swallow the colon (`key:` is a valid plain value), so the key has to be recognized here.
static bool scan_map_key(TSLexer *lexer) {
    while (iswspace(lexer->lookahead)) {
        skip(lexer);
    }
    if (!(is_name_char(lexer->lookahead) && !iswdigit(lexer->lookahead))) {
        return false;
    }
    while (is_name_char(lexer->lookahead)) {
        advance(lexer);
    }
    lexer->mark_end(lexer);
    while (iswspace(lexer->lookahead)) {
        advance(lexer);
    }
    if (lexer->lookahead != ':') {
        return false;
    }
    advance(lexer);
    if (lexer->lookahead == ':') {
        return false;
    }
    lexer->result_symbol = MAP_KEY;
    return true;
}

bool tree_sitter_scss_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
    if (valid_symbols[ERROR_RECOVERY]) {
        return false;
    }

    if (valid_symbols[CONCAT]) {
        if (is_name_char(lexer->lookahead) || lexer->lookahead == '#') {
            lexer->result_symbol = CONCAT;
            if (lexer->lookahead == '#') {
                lexer->mark_end(lexer);
                advance(lexer);
                return lexer->lookahead == '{';
            }
            return true;
        }
    }

    if (iswspace(lexer->lookahead) && valid_symbols[DESCENDANT_OP]) {
        lexer->result_symbol = DESCENDANT_OP;

        skip(lexer);
        while (iswspace(lexer->lookahead)) {
            skip(lexer);
        }
        lexer->mark_end(lexer);

        if (lexer->lookahead == '#' || lexer->lookahead == '.' || lexer->lookahead == '[' || lexer->lookahead == '-' ||
            lexer->lookahead == '*' || lexer->lookahead == '&' || is_name_char(lexer->lookahead)) {
            return true;
        }

        if (lexer->lookahead == ':') {
            advance(lexer);
            // A second colon starts a pseudo-element (`.a ::before`), which is also a selector.
            if (iswspace(lexer->lookahead) ||
                (lexer->lookahead != ':' && !can_follow_pseudo_class_colon(lexer->lookahead))) {
                return false;
            }
            return reaches_block_before_end(lexer);
        }
    }

    if (valid_symbols[PSEUDO_CLASS_SELECTOR_COLON]) {
        while (iswspace(lexer->lookahead)) {
            skip(lexer);
        }
        if (lexer->lookahead == ':') {
            advance(lexer);
            if (lexer->lookahead == ':') {
                return false;
            }
            // Whitespace after the colon means a declaration: `font: { family: x; }` is a nested
            // property block, not a pseudo class selector. So does a first character that cannot
            // start a class name, such as a digit, `$`, `(` or a quote: `margin: 0 auto;` and
            // `content:"{"` are declarations whatever follows.
            if (iswspace(lexer->lookahead) || !can_follow_pseudo_class_colon(lexer->lookahead)) {
                return false;
            }
            lexer->mark_end(lexer);
            // A `{` that opens a block makes this a pseudo class selector, a `;` indicates a
            // property.
            if (reaches_block_before_end(lexer)) {
                lexer->result_symbol = PSEUDO_CLASS_SELECTOR_COLON;
                return true;
            }
            return false;
        }
    }

    if (valid_symbols[MAP_KEY]) {
        return scan_map_key(lexer);
    }

    return false;
}
