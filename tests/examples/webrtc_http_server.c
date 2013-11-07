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

#define GST_CAT_DEFAULT webrtc_http_server
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

#define DEBUG_NAME "webrtc_http_server"

#define PORT 8080

static void
server_callback (SoupServer *server, SoupMessage *msg, const char *path,
                 GHashTable *query, SoupClientContext *client,
                 gpointer user_data)
{
  GST_WARNING ("TODO:implement");
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
