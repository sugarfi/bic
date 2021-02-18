/* Host used when "-h" is not given */
#define DEFAULT_HOST "irc.freenode.net"

/* Port used when "-p" is not given */
#define DEFAULT_PORT "6667"

/* Timestamp format; see strftime(3). */
#define TIMESTAMP_FORMAT "%Y-%m-%d %R"

/* Command prefix character. In most IRC clients this is '/'. */
#define COMMAND_PREFIX_CHARACTER ':'

/* Parting message used when none is specified with ":l ..." command. */
#define DEFAULT_PARTING_MESSAGE "ĝis"

/* Colors */
#ifdef NOCOLOR
#define CHANCOLOR ""
#define TIMECOLOR ""
#else
#define CHANCOLOR "\x1b[34m"
#define TIMECOLOR "\x1b[33m"
#endif
#define DEFAULT_FG 0
#define DEFAULT_BG 0

static unsigned char COLORS[16] = {
#ifndef NOCOLOR
    15,
    0,
    4,
    2,
    1,
    3,
    5,
    202,
    11,
    10,
    6,
    14,
    12,
    13,
    8,
    7
#else
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
#endif
};

#define ESCAPE_CHAR '\\'
#define COLOR_ALT_CHAR '$'
#define BOLD_ALT_CHAR ';'
#define ITALICS_ALT_CHAR '*'
#define UNDERLINE_ALT_CHAR '_'
#define STRIKETHROUGH_ALT_CHAR '~'
