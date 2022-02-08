#ifndef _WEBRTC_UTILITIES_H_
#define _WEBRTC_UTILITIES_H_

#include <glib.h>
#include <string>
#include <vector>

std::string
base64url_encode(const guchar* s, size_t len);

std::string
base64url_encode_string(const std::string& s);

std::vector<unsigned char>
base64url_decode(const std::string& b);

void
c_str_to_upper(gchar* s);

std::string
prettify_sdp(const gchar* sdp);

std::pair<int, std::string>
make_system_call(const std::string& command);

void
set_private_file_permissions(const std::string& filename);

bool
has_only_nice_characters(const std::string& s);

inline gboolean
string_contains(const std::string& a, const std::string& b)
{
  g_assert(!a.empty());
  g_assert(!b.empty());
  return a.find(b) != std::string::npos;
}

inline gboolean
begins_with(const std::string& a, const std::string& b)
{
  g_assert(!a.empty());
  g_assert(!b.empty());
  return a.rfind(b, 0) == 0;
}

inline gboolean
ends_with(const std::string& a, const std::string& b)
{
  return a.size() >= b.size() &&
         0 == a.compare(a.size() - b.size(), b.size(), b);
}

std::string
trim_quotation_marks(std::string s);

std::vector<std::string>
split_string(const std::string& s,
             const std::string& delimiter,
             size_t max_tokens);

std::string
make_printable_for_logging(const char* s);

gint64
profiling_get_start_time();

double
profiling_get_elapsed_time_seconds(gint64 start_time);

// this function is used to obfuscate things like usernames and passwords in
// json objects, so that one can log them without exposing credentials
void
clear_member_value(const std::string& member, std::string& s);

#endif /* _WEBRTC_UTILITIES_H_ */
