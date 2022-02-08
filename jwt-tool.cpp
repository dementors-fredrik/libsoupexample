#include <libsoup/soup.h>
#include <json-glib/json-glib.h>


gchar*
get_string_from_json_object(JsonObject* object)
{
  JsonNode* root;
  JsonGenerator* generator;
  gchar* text;

  /* Make it the root node */
  root = json_node_init_object(json_node_alloc(), object);
  generator = json_generator_new();
  json_generator_set_root(generator, root);
  text = json_generator_to_data(generator, NULL);

  /* Release everything */
  g_object_unref(generator);
  json_node_free(root);

  return text;
}
