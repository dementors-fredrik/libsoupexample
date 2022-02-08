#ifndef _WEBRTC_COMMON_JSON_H_
#define _WEBRTC_COMMON_JSON_H_

#include <json-glib/json-glib.h>

#include "RAII.h"

using JsonParserPtr =
  GRefCountedObject<JsonParser, void, g_object_ref, g_object_unref>;

using JsonNodePtr =
  GRefCountedObject<JsonNode, JsonNode, json_node_ref, json_node_unref>;

using JsonObjectPtr =
  GRefCountedObject<JsonObject, JsonObject, json_object_ref, json_object_unref>;




gchar*
get_string_from_json_object(JsonObject* object);

gboolean
has_string_member(JsonObject* object, const gchar* member);
gboolean
has_true_string_member(JsonObject* object, const gchar* member);
gboolean
has_int_member(JsonObject* object, const gchar* member);
gboolean
has_bool_member(JsonObject* object, const gchar* member);
gboolean
has_object_member(JsonObject* object, const gchar* member);
gboolean
has_array_member(JsonObject* object, const gchar* member);

#endif /* _WEBRTC_COMMON_JSON_H_ */
