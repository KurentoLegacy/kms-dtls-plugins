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
#include <string.h>

#define GST_CAT_DEFAULT webrtc_http_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define DEBUG_NAME "webrtc_http_server"

#define PORT 8080
#define MIME_TYPE "text/html"
#define HTML_FILE "webrtc_loopback.html"
#define PEMFILE "certkey.pem"

typedef struct _MediaSession {
  GMainContext *context;
  GMainLoop * loop;

  SoupServer *server;
  SoupMessage *msg;

  NiceAgent *agent;
  guint stream_id;
} MesiaSession;

static gchar*
generate_fingerprint (const gchar *pem_file)
{
  GTlsCertificate *cert;
  GError *error = NULL;
  GByteArray *ba;
  gssize length;
  int size;
  gchar *fingerprint;
  gchar *fingerprint_colon;
  int i, j;

  cert = g_tls_certificate_new_from_file (pem_file, &error);
  g_object_get (cert, "certificate", &ba, NULL);
  fingerprint = g_compute_checksum_for_data (G_CHECKSUM_SHA1, ba->data, ba->len);
  g_object_unref (cert);

  length = g_checksum_type_get_length (G_CHECKSUM_SHA1);
  size = (int)(length*2 + length) * sizeof (gchar);
  fingerprint_colon = g_malloc0 (size);

  j = 0;
  for (i=0; i< length*2; i+=2) {
    fingerprint_colon[j] = fingerprint[i];
    fingerprint_colon[++j] = fingerprint[i+1];
    fingerprint_colon[++j] = ':';
    j++;
  };
  fingerprint_colon[size-1] = '\0';
  g_free (fingerprint);

  return fingerprint_colon;
}

static void
gathering_done (NiceAgent *agent, guint stream_id, MesiaSession *mediaSession)
{
  SoupServer *server = mediaSession->server;
  SoupMessage *msg = mediaSession->msg;
  GString *sdpStr;
  GSList *candidates;
  GSList *walk;
  NiceCandidate *lowest_prio_cand = NULL;
  gchar addr[NICE_ADDRESS_STRING_LEN+1];
  gchar *ufrag, *pwd;
  gchar *fingerprint;
  gchar *line;

  nice_agent_get_local_credentials (mediaSession->agent, mediaSession->stream_id, &ufrag, &pwd);
  candidates = nice_agent_get_local_candidates (mediaSession->agent, mediaSession->stream_id, 1);

  for (walk = candidates; walk; walk = walk->next) {
    NiceCandidate *cand = walk->data;

    if (!lowest_prio_cand ||
        lowest_prio_cand->priority < cand->priority)
      lowest_prio_cand = cand;
  }

  nice_address_to_string (&lowest_prio_cand->addr, addr);
  fingerprint = generate_fingerprint (PEMFILE);

  sdpStr = g_string_new ("");
  g_string_append_printf (sdpStr,
      "\"v=0\\r\\n\" +\n"
      "\"o=- 2750483185 0 IN IP4 %s\\r\\n\" +\n"
      "\"s=\\r\\n\" +\n"
      "\"t=0 0\\r\\n\" +\n"
      "\"a=ice-ufrag:%s\\r\\n\" +\n"
      "\"a=ice-pwd:%s\\r\\n\" +\n"
      "\"a=fingerprint:sha-1 %s\\r\\n\" +\n"
      "\"a=group:BUNDLE video\\r\\n\" +\n"
      "\"m=video %d RTP/SAVPF 96\\r\\n\" +\n"
      "\"c=IN IP4 %s\\r\\n\" +\n"
      "\"a=rtpmap:96 VP8/90000\\r\\n\" +\n"
      "\"a=sendrecv\\r\\n\" +\n"
      "\"a=mid:video\\r\\n\" +\n"
      "\"a=rtcp-mux\\r\\n\"",
      addr, ufrag, pwd, fingerprint, nice_address_get_port (&lowest_prio_cand->addr), addr);

  g_free (ufrag);
  g_free (pwd);
  g_free (fingerprint);

  for (walk = candidates; walk; walk = walk->next) {
    NiceCandidate *cand = walk->data;

    nice_address_to_string (&cand->addr, addr);
    g_string_append_printf (sdpStr,
        "+\n\"a=candidate:%s %d UDP %d %s %d typ host\\r\\n\"",
        cand->foundation, cand->component_id, cand->priority, addr,
        nice_address_get_port (&cand->addr));
  }
  g_slist_free_full (candidates, (GDestroyNotify) nice_candidate_free);

  line = g_strdup_printf("sdp = %s;\n", sdpStr->str);
  soup_message_body_append (msg->response_body, SOUP_MEMORY_TAKE, line,
                            strlen(line));
  g_string_free (sdpStr, FALSE);

  line = "</script>\n</body>\n</html>\n";
  soup_message_body_append (msg->response_body, SOUP_MEMORY_STATIC, line,
                            strlen(line));

  soup_message_set_status (msg, SOUP_STATUS_OK);
  soup_server_unpause_message (server, msg);

  g_object_unref (server);
  g_object_unref (msg);
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
  soup_server_pause_message (server, msg);

  init_media_session (server, msg);
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
