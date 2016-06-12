/* GepubWidget
 *
 * Copyright (C) 2016 Daniel Garcia <danigm@wadobo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>
#include <gtk/gtk.h>

#include "gepub-widget.h"

struct _GepubWidget {
    GtkBox parent;

    GepubDoc *doc;
    WebKitWebView *view;
};

struct _GepubWidgetClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE (GepubWidget, gepub_widget, GTK_TYPE_BOX)

static void
resource_callback (WebKitURISchemeRequest *request, gpointer user_data)
{
    GInputStream *stream;
    gsize stream_length;
    gchar *path;
    gchar *uri;
    guchar *contents;
    gchar *mime;
    GepubWidget *widget;

    widget = GEPUB_WIDGET (user_data);
    uri = g_strdup (webkit_uri_scheme_request_get_uri (request));
    // removing "epub://"
    path = uri + 7;
    contents = gepub_doc_get_resource_v (widget->doc, path, &stream_length);
    mime = gepub_doc_get_resource_mime (widget->doc, path);

    if (!mime) {
        g_free (uri);
        return;
    }

    stream = g_memory_input_stream_new_from_data (contents, stream_length, g_free);
    webkit_uri_scheme_request_finish (request, stream, stream_length, mime);

    g_object_unref (stream);
    g_free (mime);
    g_free (uri);
}

static void
gepub_widget_finalize (GObject *object)
{
    GepubWidget *widget = GEPUB_WIDGET (object);

    if (widget->doc) {
        g_object_unref (widget->doc);
        widget->doc = NULL;
    }
    if (widget->view) {
        gtk_widget_destroy (GTK_WIDGET (widget->view));
        widget->view = NULL;
    }

    G_OBJECT_CLASS (gepub_widget_parent_class)->finalize (object);
}

static void
gepub_widget_init (GepubWidget *doc)
{
}

static void
gepub_widget_class_init (GepubWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = gepub_widget_finalize;
}

/**
 * gepub_widget_new:
 * @path: the epub doc path
 * @error: location to store a #GError, or %NULL
 *
 * Returns: (transfer full): the new GepubWidget created
 */
GtkWidget *
gepub_widget_new (const gchar *path)
{
    GepubWidget *widget = g_object_new (GEPUB_TYPE_WIDGET,
                                        "orientation", GTK_ORIENTATION_HORIZONTAL,
                                        "spacing", 0,
                                        NULL);

    widget->doc = NULL;
    widget->view = NULL;

    return GTK_WIDGET (widget);
}

/**
 * gepub_widget_get_doc:
 * @widget: a #GepubWidget
 *
 * Returns: (transfer none): the #GepubDoc
 */
GepubDoc *
gepub_widget_get_doc (GepubWidget *widget)
{
    return widget->doc;
}

/**
 * gepub_widget_get_wkview:
 * @widget: a #GepubWidget
 *
 * Returns: (transfer none): the #WebKitWebView related with the widget
 */
WebKitWebView *
gepub_widget_get_wkview (GepubWidget *widget)
{
    return widget->view;
}

/**
 * gepub_widget_load_epub:
 * @widget: a #GepubWidget
 * @path: The epub doc path
 *
 * This method reloads the widget with the new document
 */
void
gepub_widget_load_epub (GepubWidget *widget, const gchar *path)
{
    GtkBox *box = GTK_BOX (widget);
    WebKitWebContext *ctx = NULL;

    if (widget->doc) {
        g_object_unref (widget->doc);
        widget->doc = NULL;
    }
    if (widget->view) {
        gtk_widget_destroy (GTK_WIDGET (widget->view));
        widget->view = NULL;
    }

    widget->doc = gepub_doc_new (path);
    widget->view = WEBKIT_WEB_VIEW (webkit_web_view_new ());

    ctx = webkit_web_view_get_context (WEBKIT_WEB_VIEW (widget->view));
    webkit_web_context_register_uri_scheme (ctx, "epub", resource_callback, widget, NULL);

    gepub_widget_reload (widget);

    gtk_box_pack_start (box, GTK_WIDGET (widget->view), TRUE, TRUE, 0);
}

/**
 * gepub_widget_load_epub:
 * @widget: a #GepubWidget
 *
 * This method reloads the data with the GepubDoc current chapter
 */
void
gepub_widget_reload (GepubWidget *widget)
{
    gsize bufsize = 0;
    guchar *buffer = NULL;

    buffer = gepub_doc_get_current_with_epub_uris (widget->doc, &bufsize);

    webkit_web_view_load_bytes (
        widget->view,
        g_bytes_new_take (buffer, bufsize),
        gepub_doc_get_current_mime (widget->doc),
        "UTF-8", NULL);
}
