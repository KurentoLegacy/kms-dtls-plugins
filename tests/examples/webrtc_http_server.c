/*
 * (C) Copyright 2013 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include <gst/gst.h>
#include <libsoup/soup.h>
#include <nice/nice.h>

#define GST_CAT_DEFAULT webrtc_http_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define DEBUG_NAME "webrtc_http_server"

#define PORT 8080
#define MIME_TYPE "text/html"
#define HTML_FILE "webrtc_loopback.html"

typedef struct _MediaSession {
  GMainContext *context;
  GMainLoop * loop;

  SoupServer *server;
  SoupMessage *msg;

  NiceAgent *agent;
  guint stream_id;
} MesiaSession;


static void
gathering_done (NiceAgent *agent, guint stream_id, MesiaSession *mediaSession)
{
  GST_WARNING ("TODO: implement");
}

static void
nice_agent_recv (NiceAgent *agent, guint stream_id, guint component_id,
                 guint len, gchar *buf, gpointer user_data)
{
  /* Nothing to do, this callback is only for negotiation */
}

static gpointer
loop_thread (gpointer loop)
{
  g_main_loop_run (loop); /* TODO: g_main_loop_quit */

  return NULL;
}

static void
init_media_session (SoupServer *server, SoupMessage *msg)
{
  MesiaSession *mediaSession;
  GThread * lt;

  mediaSession = g_slice_new0 (MesiaSession); /* TODO: free */

  mediaSession->context = g_main_context_new ();
  mediaSession->loop = g_main_loop_new (mediaSession->context, TRUE); /* TODO: g_main_loop_unref */
  lt = g_thread_new ("ctx", loop_thread, mediaSession->loop);
  g_thread_unref (lt);

  mediaSession->server = g_object_ref (server);
  mediaSession->msg = g_object_ref (msg);

  mediaSession->agent = nice_agent_new (mediaSession->context, NICE_COMPATIBILITY_RFC5245);
  g_object_set (mediaSession->agent, "upnp", FALSE, NULL);
  g_object_set(G_OBJECT (mediaSession->agent),
               "stun-server", "77.72.174.167",
               "stun-server-port", 3478,
               NULL);

  mediaSession->stream_id = nice_agent_add_stream (mediaSession->agent, 1);
  nice_agent_attach_recv (mediaSession->agent, mediaSession->stream_id, 1, mediaSession->context,
                         nice_agent_recv, NULL);
  g_signal_connect (mediaSession->agent, "candidate-gathering-done",
    G_CALLBACK (gathering_done), mediaSession);

  nice_agent_gather_candidates (mediaSession->agent, mediaSession->stream_id);
}

static void
server_callback (SoupServer *server, SoupMessage *msg, const char *path,
                 GHashTable *query, SoupClientContext *client,
                 gpointer user_data)
{
  gboolean ret;
  gchar *contents;
  gsize length;

  GST_DEBUG ("Request: %s", path);

  if (msg->method != SOUP_METHOD_GET) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    GST_DEBUG ("Not implemented");
    return;
  }

  if (g_strcmp0 (path, "/") != 0) {
    soup_message_set_status (msg, SOUP_STATUS_NOT_FOUND);
    GST_DEBUG ("Not found");
    return;
  }

  ret = g_file_get_contents (HTML_FILE, &contents, &length, NULL);
  if (!ret) {
    soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);
    GST_ERROR ("Error loading %s file", HTML_FILE);
    return;
  }

  soup_message_set_response (msg, MIME_TYPE, SOUP_MEMORY_STATIC, "", 0);
  soup_message_body_append (msg->response_body, SOUP_MEMORY_TAKE, contents, length);

  init_media_session (server, msg);
  GST_WARNING ("TODO: complete response");
}

int
main (int argc, char ** argv)
{
  SoupServer *server;

  gst_init (&argc, &argv);
  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, DEBUG_NAME, 0, DEBUG_NAME);

  GST_INFO ("Start Kurento WebRTC HTTP server");
  server = soup_server_new (SOUP_SERVER_PORT, PORT,
                            NULL);
  soup_server_add_handler (server, "/", server_callback, NULL, NULL);
  soup_server_run (server);

  return 0;
}
