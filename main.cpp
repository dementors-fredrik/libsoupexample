#include <iostream>
#include <libsoup/soup.h>

using namespace std;

static SoupMessage *
generate_soup_get_message(GMainLoop *loop, SoupSession *session, const char *url) {
  SoupMessage *msg = soup_message_new("GET", url);
  g_object_ref(msg);
  return msg;
}

int main(int argc, char **argv) {
  SoupSession *session = soup_session_new();
  GMainLoop *mainLoop = g_main_loop_new(nullptr, TRUE);

  SoupMessage *msg = generate_soup_get_message(mainLoop, session, "https://example.com");

  soup_session_queue_message(
          session, msg, [](SoupSession *session, SoupMessage *msg, gpointer usr_data) {
            if (SOUP_STATUS_IS_SUCCESSFUL(msg->status_code)) {
              cout << "Lambda completion!" << endl;
              cout << "Body:" << endl
                   << msg->response_body->data << endl;
            } else {
              cerr << "Failed to perform request: " << soup_message_get_uri(msg)->path << " " << msg->status_code << " " << msg->reason_phrase << endl;
            }
            g_object_unref(msg);

            g_main_loop_quit((GMainLoop *) usr_data);
          },
          (gpointer) mainLoop);

  g_main_loop_run(mainLoop);

  g_main_loop_unref(mainLoop);
  g_object_unref(session);

  return 0;
}
