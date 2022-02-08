#include <iostream>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include "json.h"

using namespace std;


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

static JsonNode* parse_json(const gchar* data)
{
  auto parser = json_parser_new();
  int success = json_parser_load_from_data(parser, data, -1, NULL);
  if (!success) {
    throw runtime_error("Unable to parse JSON ");
  }

  JsonNode* root = json_parser_steal_root(parser);
  json_node_ref(root);

  g_object_unref(parser);
  return root;
}

static JsonObject *parse_json_object(const gchar* data) {
  auto *root = parse_json(data);
  if (!JSON_NODE_HOLDS_OBJECT(root)) {
    throw runtime_error("!JSON_NODE_HOLDS_OBJECT");
  }
  JsonObject* obj = json_node_get_object(root);

  if (obj == NULL) {
    throw runtime_error("Got NULL object");
  }

  json_object_ref(obj);
  json_node_unref(root);
  return obj;
}



static SoupMessage *allocate_soup_message(const char* method, const char* url) {
  SoupMessage *msg = soup_message_new(method, url);
  msg->request_headers = soup_message_headers_new(SOUP_MESSAGE_HEADERS_REQUEST);
  g_object_ref(msg);
  return msg;
}

static SoupMessage *
generate_soup_get_message(const char *url) {
  return allocate_soup_message("GET", url);
}


static SoupMessage *generate_soup_post_message(const char* url, const char* buffer, guint buffersize) {
  SoupMessage *msg = allocate_soup_message("POST", url);
  soup_message_set_request(msg, "application/json", SOUP_MEMORY_COPY, buffer, buffersize);
  return msg;
}

static SoupMessage *generate_soup_post_message(const char* url, JsonObject *objectToPost) {
  auto res = get_string_from_json_object(objectToPost);
  return generate_soup_post_message(url, res, strlen(res) );
}


static void authenticate_message(SoupMessage** msg, const char* username, const char* password) {
  char buffer[0xff];
  snprintf(buffer, sizeof(buffer), "%s:%s", username, password);
  GString *string = g_string_new("");

  g_string_printf(string, "Basic %s", g_base64_encode((guchar*)buffer, (gsize)strlen(buffer)));
  g_print("Sending header %s\n", string->str);
  soup_message_headers_append((*msg)->request_headers, "Authorization", string->str);
}

string get_timestamp() {
  time_t time1;
  time(&time1);
  auto tm = gmtime(&time1);
  char buffer[0xff];
  strftime(buffer,sizeof(buffer),"%F %T.0000000", tm);
  return {buffer};
}


int main(int argc, char **argv) {
  SoupSession *session = soup_session_new_with_options(SOUP_SESSION_SSL_STRICT, FALSE, NULL);

  GMainLoop *mainLoop = g_main_loop_new(nullptr, TRUE);

  string res = get_timestamp();

  JsonObject* p = json_object_new();

  json_object_set_string_member(p, "PluginId", "webrtc");
  json_object_set_string_member(p, "OccurrenceTime", g_string_new(res.c_str())->str);
  json_object_set_string_member(p, "Message", "Opened video connection to bla");

  string foo = get_string_from_json_object(p);

  JsonObject* obj = parse_json_object(R"({ "hejtest" : "foo" })");

  cout << get_string_from_json_object(obj) << endl;


  SoupMessage*  msg = generate_soup_get_message("https://localhost:60201/v1/auth");

  soup_session_queue_message(
          session, msg, [](SoupSession *session, SoupMessage *msg, gpointer usr_data) {
            if (SOUP_STATUS_IS_SUCCESSFUL(msg->status_code)) {
              cout << "Lambda completion!" << endl;
              cout << "Body:" << endl
                   << msg->response_body->data << endl;
            } else {
              cerr << "Failed to perform request: " << soup_message_get_uri(msg)->path << " " << msg->status_code << " " << msg->reason_phrase << endl << msg->response_body->data;
            }
            cout << "Callback " << msg->status_code << endl;

            g_object_unref(msg);

            g_main_loop_quit((GMainLoop *) usr_data);
          },
          (gpointer) mainLoop);

  g_main_loop_run(mainLoop);

  exit(0);



/*  JsonObject* p = json_object_new();

  json_object_set_string_member(p, "PluginId", "webrtc");
  json_object_set_string_member(p, "OccurrenceTime", "2022-02-08 09:30:40.0000000");
  json_object_set_string_member(p, "Message", "Opened video connection to ");

  JsonObject *ud = json_object_new();
  json_object_set_string_member(ud, "UserName", "user");
  json_object_set_string_member(ud, "UserSid", "sid");
  json_object_set_string_member(ud, "ComputerName", "computer");

  json_object_set_object_member(p, "UserDetails", ud);

  SoupMessage*  msg = generate_soup_post_message("https://172.20.124.55:55756/Acs/Api/AuditLogFacade/AddAuditEntry", p);


  authenticate_message(&msg, "Dementors", "Mobbare@1.2");

  soup_session_queue_message(
          session, msg, [](SoupSession *session, SoupMessage *msg, gpointer usr_data) {
            if (SOUP_STATUS_IS_SUCCESSFUL(msg->status_code)) {
              cout << "Lambda completion!" << endl;
              cout << "Body:" << endl
                   << msg->response_body->data << endl;
            } else {
              cerr << "Failed to perform request: " << soup_message_get_uri(msg)->path << " " << msg->status_code << " " << msg->reason_phrase << endl << msg->response_body->data;
            }
            cout << "Callback " << msg->status_code << endl;

            g_object_unref(msg);

            g_main_loop_quit((GMainLoop *) usr_data);
          },
          (gpointer) mainLoop);

  g_main_loop_run(mainLoop);

  g_main_loop_unref(mainLoop);
  g_object_unref(session);*/

  return 0;
}
