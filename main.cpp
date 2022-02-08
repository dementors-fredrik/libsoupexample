#include <iostream>
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <vector>
#include "Utilities.h"
#include "json.h"
#include "log_macros.h"

using namespace std;


static JsonNode* parse_json(const gchar* data)
{
  FENTER;
  auto parser = json_parser_new();
  int success = json_parser_load_from_data(parser, data, -1, NULL);
  if (!success) {
    throw runtime_error("Unable to parse JSON ");
  }

  JsonNode* root = json_parser_steal_root(parser);
  json_node_ref(root);

  g_object_unref(parser);
  FEXIT;
  return root;
}

static JsonObject *parse_json_object(const gchar* data) {
  FENTER;

  auto *root = parse_json(data);
  if (!JSON_NODE_HOLDS_OBJECT(root)) {
    FEXIT;
    throw runtime_error("!JSON_NODE_HOLDS_OBJECT");
  }
  JsonObject* obj = json_node_get_object(root);

  if (obj == NULL) {
    FEXIT;
    throw runtime_error("Got NULL object");
  }

  json_object_ref(obj);
  json_node_unref(root);
  FEXIT;
  return obj;
}



static SoupMessage *allocate_soup_message(const char* method, const char* url) {
  FENTER;
  SoupMessage *msg = soup_message_new(method, url);
  msg->request_headers = soup_message_headers_new(SOUP_MESSAGE_HEADERS_REQUEST);
  g_object_ref(msg);
  FEXIT;
  return msg;
}

static SoupMessage *
generate_soup_get_message(const char *url) {
  FENTER;
  auto msg = allocate_soup_message("GET", url);
  FEXIT;
  return msg;
}


static SoupMessage *generate_soup_post_message(const char* url, const char* buffer, guint buffersize) {
  FENTER;
  SoupMessage *msg = allocate_soup_message("POST", url);
  soup_message_set_request(msg, "application/json", SOUP_MEMORY_COPY, buffer, buffersize);
  FEXIT;
  return msg;
}

static SoupMessage *generate_soup_post_message(const char* url, JsonObject *objectToPost) {
  FENTER;
  auto res = get_string_from_json_object(objectToPost);
  auto t = generate_soup_post_message(url, res, strlen(res) );
  FEXIT;
  return t;
}


static void authenticate_message_basic(SoupMessage** msg, const char* username, const char* password) {
  FENTER;
  char buffer[0xff];
  snprintf(buffer, sizeof(buffer), "%s:%s", username, password);
  GString *string = g_string_new("");

  g_string_printf(string, "Basic %s", g_base64_encode((guchar*)buffer, (gsize)strlen(buffer)));
  soup_message_headers_append((*msg)->request_headers, "Authorization", string->str);
  FEXIT;
}

static void authenticate_message_bearer(SoupMessage** msg,const char*token) {
  FENTER;
  GString *string = g_string_new("");
  g_string_printf(string, "Bearer %s", token);
  soup_message_headers_append((*msg)->request_headers, "Authorization", token);
  FEXIT;
}


string get_timestamp() {
  FENTER;
  time_t time1;
  time(&time1);
  auto tm = gmtime(&time1);
  char buffer[0xff];
  strftime(buffer,sizeof(buffer),"%F %T.0000000", tm);
  FEXIT;
  return {buffer};
}


JsonObject* get_token_content(const char* token){
  FENTER;
  string jwt (token);
  string del(".");

  if (jwt.empty()) {
    throw runtime_error("JWT parse: Empty string");
  }
  auto parts = split_string(jwt,del , 3);
  if (parts.size() != 3) {
    throw runtime_error("JWT parse: Unexpected JWT format");
  }

  vector<unsigned char> header_data = base64url_decode(parts[1]);
  if (header_data.empty()) {
    throw runtime_error("JWT parse: Empty header data");
  }
  header_data.push_back('\0');
  const char* header = (const char*)&(header_data[0]);

  auto res = parse_json_object(header);
  FEXIT;
  return res;
}

JsonObject* mock_init_session() {
  FENTER;
  auto mock = json_object_new();
  json_object_ref(mock);
  json_object_set_string_member(mock, "targetId", "abcde");
  json_object_set_string_member(mock, "authorization", "accesstoken");
  FEXIT;
  return mock;
}

JsonObject* generate_audit_entry(const char* camera_id, const char* user_name, const char* sid, const char* computer_name){
  FENTER;
  auto audit_entry = json_object_new();
  json_object_ref(audit_entry);
  json_object_set_string_member(audit_entry, "PluginId", "webrtc");
  json_object_set_string_member(audit_entry, "OccurrenceTime", get_timestamp().c_str());

  GString *message = g_string_new("Opened video connection to");
  json_object_set_string_member(audit_entry, "Message",  g_string_append(message, camera_id)->str);

  auto ud = json_object_new();
  json_object_ref(ud);

  json_object_set_string_member(ud, "UserName", user_name);
  json_object_set_string_member(ud, "UserSid", sid);
  json_object_set_string_member(ud, "ComputerName", computer_name);

  json_object_set_object_member(audit_entry, "UserDetails", ud);

  FEXIT;
  return audit_entry;
}

struct AuditDetailsHandle {
  const gchar *token;
  const gchar *cameraId;
};

JsonObject *generate_audit_entry_from_header(SoupMessage *msg);
int main(int argc, char **argv) {
  FENTER;
  GMainLoop *mainLoop = g_main_loop_new(nullptr, TRUE);
// Start
  auto json_object = mock_init_session();
  const gchar* access_token = json_object_get_string_member(json_object,"authorization");
  const gchar* camera_id = json_object_get_string_member(json_object,"targetId");

  auto *adh = new AuditDetailsHandle();
  adh->token = access_token;
  adh->cameraId = camera_id;

  SoupSession *session = soup_session_new_with_options(SOUP_SESSION_SSL_STRICT, FALSE, NULL);
  SoupMessage* msg = generate_soup_get_message("https://localhost:60201/v1/auth");
  authenticate_message_bearer(&msg, access_token);

  soup_session_queue_message(
          session, msg, [](SoupSession *session, SoupMessage *msg, gpointer usr_data) {
            FENTER_LAMBDA;

            if (SOUP_STATUS_IS_SUCCESSFUL(msg->status_code)) {
              JsonObject *ae = generate_audit_entry_from_header(msg);
              SoupMessage*  msg2 = generate_soup_post_message("https://172.20.124.55:55756/Acs/Api/AuditLogFacade/AddAuditEntry", ae);
              // Använd API-nyckel
              authenticate_message_basic(&msg2, "Dementors", "Mobbare@1.2");

              soup_session_queue_message(
                      session, msg2, [](SoupSession *session, SoupMessage *msg, gpointer usr_data) {
                        FENTER_LAMBDA;
                        auto *handle = (AuditDetailsHandle*)usr_data;
                        debug("Handle details: %s %s\n",handle->cameraId , handle->token);

                        if (!SOUP_STATUS_IS_SUCCESSFUL(msg->status_code)) {
                          cerr << "Failed to perform request: " << soup_message_get_uri(msg)->path << " " << msg->status_code << " " << msg->reason_phrase << endl << msg->response_body->data;
                        }

                        g_object_unref(msg);

                        FEXIT_LAMBDA;
                      },
                      (gpointer) usr_data);

            } else {
              error("Failed to perform request to path: %s status: %d reason: %s\n", soup_message_get_uri(msg)->path,
                    msg->status_code, msg->response_body->data);
            }
            FEXIT_LAMBDA;
            g_object_unref(msg);
          },
          (gpointer) adh);

  g_object_unref(session);

  // Stop
  g_main_loop_run(mainLoop);

  g_main_loop_unref(mainLoop);
  FEXIT;
  return 0;
}

JsonObject *generate_audit_entry_from_header(SoupMessage *msg) {
  FENTER;
  auto token = soup_message_headers_get_one(msg->response_headers, "X-Axis-Token");

  JsonObject* jwtContent = get_token_content(token);
  const gchar* uid = json_object_get_string_member(jwtContent,"uid");
  const gchar* client_id = json_object_get_string_member(jwtContent,"client_id");
  const gchar* sub = json_object_get_string_member(jwtContent,"sub");

  auto audit_entry_for_acs_processing = generate_audit_entry("cameraId",sub,uid,client_id);
  FEXIT;
  return audit_entry_for_acs_processing;
}
