#include "Utilities.h"

#include "log_macros.h"
#include <cctype>
#include <glib/gi18n.h>

std::string
base64url_encode(const guchar* s, size_t len)
{
    gchar* a = g_base64_encode(s, len);

    // convert Base64 to Base64url, as described on
    // https://en.wikipedia.org/wiki/JSON_Web_Token
    // https://en.wikipedia.org/wiki/Base64#URL_applications
    gchar* t = a;
    for (; *t != '\0'; ++t) {
        if (*t == '+') {
            *t = '-';

        } else if (*t == '/') {
            *t = '_';

        } else if (*t == '=') {
            *t = '\0';
        }
    }

    std::string b(a);
    g_free(a);
    return b;
}

std::string
base64url_encode_string(const std::string& s)
{
    return base64url_encode((const guchar*)s.c_str(), s.length());
}

std::vector<unsigned char>
base64url_decode(const std::string& b)
{
    // This function is used to parse data from a client and must be robust
    // against corrupt input data.

    // convert Base64url to Base64, as described on
    // https://en.wikipedia.org/wiki/JSON_Web_Token
    // https://en.wikipedia.org/wiki/Base64#URL_applications
    std::string bcopy = b;
    for (size_t i = 0; i < bcopy.length(); ++i) {
        if (bcopy[i] == '-') {
            bcopy[i] = '+';

        } else if (bcopy[i] == '_') {
            bcopy[i] = '/';
        }
    }
    size_t padding_needed = 4 - bcopy.length() % 4;
    if (padding_needed == 4)
        padding_needed = 0;
    g_assert(padding_needed < 4);
    // for a Base64url string there should only be 0, 1 or 2 padding characters
    // needed, but we must be ready to handle corrupt input data so can't assert
    // on that
    for (size_t i = 0; i < padding_needed; ++i) {
        bcopy += '=';
    }
    g_assert(bcopy.length() % 4 == 0);

    gsize len = 0;
    g_autofree guchar* a = g_base64_decode(bcopy.c_str(), &len);
    if (a == NULL || len == 0) {
        return std::vector<unsigned char>();

    } else {
        std::vector<unsigned char> v;
        for (gsize i = 0; i < len; ++i) {
            v.push_back(a[i]);
        }
        return v;
    }
}

void
c_str_to_upper(gchar* s)
{
    g_assert(s);
    for (size_t i = 0; s[i] != '\0'; ++i) {
        s[i] = g_ascii_toupper(s[i]);
    }
}

std::string
prettify_sdp(const gchar* sdp)
{
    g_assert(sdp);
    gchar** v = g_strsplit(sdp, "\n", 0);
    g_assert(v);

    std::string p;
    for (gchar** u = v; *u != NULL; ++u) {
        gchar* s = *u;
        if (strlen(s) > 0) {
            p += '\t';
            if (s[0] == 'a' || s[0] == 'c') {
                p += '\t';
            }
            p += s;
            p += '\n';
        }
    }

    g_strfreev(v);

    if ((!p.empty()) && p.back() == '\n') {
        p.erase(p.size() - 1);
    }
    return p;
}

std::pair<int, std::string>
make_system_call(const std::string& command)
{
    std::string output;
    char buf[512];
    FILE* c = NULL;

    if ((c = popen(command.c_str(), "r")) == NULL) {
        error("Error when making system call: %s", command.c_str());
        g_assert(0);
    }

    while (fgets(buf, sizeof(buf), c) != NULL) {
        output += buf;
    }

    int ret = pclose(c);

    if (!output.empty() && output.back() == '\n') {
        output.pop_back(); // remove trailing line break
    }
    return std::make_pair(ret, output);
}

void
set_private_file_permissions(const std::string& filename)
{
#if WEBRTC_APP_PRODUCT_WIN
    // TODO: what's the equivalent for windows?
#else
    auto r = make_system_call("chmod 600 " + filename);
    g_assert(r.first == 0);
#endif
}

bool
has_only_nice_characters(const std::string& s)
{
    const std::string nice_chars = "ABCDEFGHIJKLMNOPQRSTUVXYZ"
                                   "abcdefghijklmnopqrstuvxyz"
                                   "0123456789"
                                   "-_";

    for (char c : s) {
        if (nice_chars.find(c) == std::string::npos) {
            return false;
        }
    }

    return true;
}

std::string
trim_quotation_marks(std::string s)
{
    if (!s.empty() && s.back() == '\"')
        s.pop_back();
    if (!s.empty() && s.front() == '\"')
        s.erase(0, 1);
    return s;
}

std::vector<std::string>
split_string(const std::string& s,
             const std::string& delimiter,
             size_t max_tokens)
{
    gchar** parts = g_strsplit(s.c_str(), ".", 3);
    std::vector<std::string> v;
    for (gchar** p = parts; *p != NULL; ++p) {
        v.push_back(*p);
    }
    g_strfreev(parts);
    return v;
}

std::string
make_printable_for_logging(const char* a)
{
    g_assert(a);
    size_t len = strlen(a);
    g_autofree char* b = (char*)malloc(len + 1);
    size_t i = 0;
    for (; i < len; ++i) {
        // convert non-printable characters
        if (32 <= a[i] && a[i] <= 126) {
            b[i] = a[i];
        } else {
            b[i] = '?';
        }
    }
    b[i] = '\0';
    return std::string(b);
}

gint64
profiling_get_start_time()
{
    return g_get_real_time();
}

double
profiling_get_elapsed_time_seconds(gint64 start_time)
{
    return ((double)(g_get_real_time() - start_time)) / 1000000;
}

void
clear_member_value(const std::string& member, std::string& s)
{
    // this function must be robust, i.e. it must not crash for any
    // input data whatsoever, but otherwise its functionality is not
    // critical, the output is only intended for logging, not to
    // be parsed or anything like that

    std::string object_to_find = '\"' + member + '\"';

    int previous_find = 0;
    while (true) {
        size_t i = s.find(object_to_find, previous_find + 1);
        if (i != std::string::npos) {
            g_assert((int)i > previous_find);
            previous_find = i;

            // step over the object_to_find itself
            i += member.size();

            // find colon (there may be spaces before it)
            while (i < s.size() && s[i] != ':' && s[i] != '}') {
                ++i;
            }
            if (s[i] == '}') {
                // empty member value, skip this finding
                break;
            }
            ++i; // step over the colon
            // step over any spaces
            while (i < s.size() && s[i] == ' ') {
                ++i;
            }
            if (s[i] == '\"') {
                // Start of string member value.
                // Hide everything until the next '\"'
                ++i;
                while (i < s.size() && s[i] != '\"') {
                    s[i] = '*';
                    ++i;
                }
            } else if (std::isdigit(s[i]) || std::isalpha(s[i])) {
                // Start of numeric member value, or probably start of bool
                // member value. Hide everything until the next space or ',' or
                // '}'
                while (i < s.size() && s[i] != ' ' && s[i] != ',' &&
                       s[i] != '}') {
                    s[i] = '*';
                    ++i;
                }
            } else {
                // Start of something else (probably a json object then).
                // TODO: This is more complex to handle, for now just hide the
                // rest of the entire string, this should anyway not occur
                // in our current API
                while (i < s.size()) {
                    s[i] = '*';
                    ++i;
                }
            }

        } else {
            return;
        }
    }
}
