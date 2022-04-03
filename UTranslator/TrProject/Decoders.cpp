// My header
#include "Decoders.h"

bool decode::dcpp::isAlnum(char x)
{
    return (x >= '0' && x <= '9')
        || (x >= 'a' && x <= 'z')
        || (x >= 'A' && x <= 'Z')
        || ( x == '_');
}


bool decode::dcpp::isAlpha(char x)
{
    return (x >= 'a' && x <= 'z')
        || (x >= 'A' && x <= 'Z')
        || ( x == '_');
}


int decode::hexDigitValue(char x)
{
    if (x >= '0' && x <= '9')
        return x - '0';
    if (x >= 'A' && x <= 'F')
        return x - ('A' - 10);
    if (x >= 'a' && x <= 'f')
        return x - ('a' - 10);
    return -1;
}


std::string decode::cpp(std::string_view x)
{
    enum class State {
        SPACE,      // ␣␣      prefixStart YES
        SPACE_TRAIL,// "x"␣␣   prefixStart YES
        OUTSIDE,    // ␣␣1+
        PREFIX,     // ␣␣ab    prefixStart YES
        INSIDE,     // "x
        SLASH,      // "x/       BACK slash here
        SUFFIX      // "x"ab
    };

    std::string cache;
    x = normalizeEolSv(x, cache);

    const char* p = x.data();
    const char* end = p + x.length();

    std::string r;

    State state = State::SPACE;
    const char* prefixStart = p;

    for (; p != end; ++p) {
        auto c = *p;
        switch (state) {
        case State::SPACE:
        case State::SPACE_TRAIL: // prefixStart YES
            switch (c) {
            case ' ':
            case '\n':
                break;  // do nothing
            case '"':
                state = State::INSIDE;      // prefixStart YES→NO, do not dump
                prefixStart = nullptr;
                break;
            default:
                if (dcpp::isAlpha(c)) {
                    state = State::PREFIX;  // prefixStart YES→YES, keep on stockpiling
                } else {
                    if (state == State::SPACE)
                        r.append(prefixStart, p);
                    r += c;
                    prefixStart = nullptr;
                    state = State::OUTSIDE; // prefixStart YES→NO, dump prefix+char (prefix not in SPACE_TRAIL)
                }
            }
            break;
        case State::PREFIX: // prefixStart YES
            switch (c) {
            case ' ':                       // ␣␣ab␣
            case '\n':
                state = State::SPACE;       // prefixStart YES→YES, dump
                r.append(prefixStart, p);
                prefixStart = p;
                break;
            case '"':                       // ␣␣ab"
                state = State::INSIDE;      // prefixStart YES→NO, do not dump
                prefixStart = nullptr;
                break;
            default:
                if (dcpp::isAlnum(c)) {     // ␣␣ab8
                    // do nothing, keep on stockpiling
                } else {                    // ␣␣ab+
                    state = State::OUTSIDE; // prefixStart YES→NO, dump prefix+char
                    r.append(prefixStart, p);
                    r += c;
                    prefixStart = nullptr;
                }
            }
            break;
        case State::OUTSIDE: // prefixStart NO
            switch (c) {
            case ' ':                       // ␣␣1+␣
            case '\n':
                state = State::SPACE;       // prefixStart NO→YES
                prefixStart = p;
                break;
            case '"':                       // ␣␣1+"
                state = State::INSIDE;      // prefixStart keep NO
                break;
            default:
                if (dcpp::isAlpha(c)) {     // ␣␣1+a
                    state = State::PREFIX;  // prefixStart NO→YES
                    prefixStart = p;
                } else {                    // ␣␣1+2
                    r += c;
                }
            }
            break;
        case State::INSIDE: // prefixStart NO
            switch (c) {
            case '"':
                state = State::SUFFIX;  // prefixStart keep NO
                break;
            case '\\':
                state = State::SLASH;   // prefixStart keep NO
                break;
            default:
                r += c;
            }
            break;
        case State::SLASH:
            /// @todo [decoders] \### oct, \x#### hex, \u#### BMP, \U######## other planes
            switch (c) {
            case 'a': r += '\a'; break;
            case 'b': r += '\b'; break;
            case 't': r += '\t'; break;
            case 'n':
            case 'r': r += '\n'; break;     // both /n and /r yield LF!!
            case 'f': r += '\f'; break;
            case 'v': r += '\v'; break;
            case '\n': break;               // do nothing, though is is strange string
            default:
                r += c;
            }
            state = State::INSIDE;              // prefixStart keep NO
            break;
        case State::SUFFIX:
            switch (c) {
            case ' ':                           // "x"ab␣
            case '\n':
                state = State::SPACE_TRAIL;     // prefixStart NO→YES
                prefixStart = p;
                break;
            default:
                if (dcpp::isAlnum(c)) {         // "x"ab8
                    //  do nothing
                } else {                        // "x"ab+
                    state = State::OUTSIDE;     // prefixStart keep NO
                    r += c;
                }
            }
            break;
        }   // Big switch (state)
    }
    if (prefixStart && state != State::SPACE_TRAIL) {
        r.append(prefixStart, end);
    }
    return r;
}
