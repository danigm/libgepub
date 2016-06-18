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
    WebKitWebView parent;

    GepubDoc *doc;
};

struct _GepubWidgetClass {
    WebKitWebViewClass parent_class;
};

enum {
    PROP_0,
    PROP_DOC,
    NUM_PROPS
};

static GParamSpec *properties[NUM_PROPS] = { NULL, };

G_DEFINE_TYPE (GepubWidget, gepub_widget, WEBKIT_TYPE_WEB_VIEW)

static void
resource_callback (WebKitURISchemeRequest *request, gpointer user_data)
{
    GInputStream *stream;
    gsize stream_length;
    gchar *path;
    gchar *uri;
    guchar *contents;
    gchar *mime;
    GepubWidget *widget = user_data;

    if (!widget->doc)
      return;

    uri = g_strdup (webkit_uri_scheme_request_get_uri (request));
    // removing "epub://"
    path = uri + 7;
    contents = gepub_doc_get_resource (widget->doc, path, &stream_length);
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
gepub_widget_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    GepubWidget *widget = GEPUB_WIDGET (object);

    switch (prop_id) {
    case PROP_DOC:
        gepub_widget_set_doc (widget, g_value_get_object (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gepub_widget_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    GepubWidget *widget = GEPUB_WIDGET (object);

    switch (prop_id) {
    case PROP_DOC:
        g_value_set_object (value, gepub_widget_get_doc (widget));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gepub_widget_finalize (GObject *object)
{
    GepubWidget *widget = GEPUB_WIDGET (object);

    g_clear_object (&widget->doc);

    G_OBJECT_CLASS (gepub_widget_parent_class)->finalize (object);
}

static void
gepub_widget_init (GepubWidget *widget)
{
}

static void
gepub_widget_constructed (GObject *object)
{
    WebKitWebContext *ctx;
    GepubWidget *widget = GEPUB_WIDGET (object);

    G_OBJECT_CLASS (gepub_widget_parent_class)->constructed (object);

    ctx = webkit_web_view_get_context (WEBKIT_WEB_VIEW (widget));
    webkit_web_context_register_uri_scheme (ctx, "epub", resource_callback, widget, NULL);
}

static void
gepub_widget_class_init (GepubWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = gepub_widget_constructed;
    object_class->finalize = gepub_widget_finalize;
    object_class->set_property = gepub_widget_set_property;
    object_class->get_property = gepub_widget_get_property;

    properties[PROP_DOC] =
        g_param_spec_object ("doc",
                             "The GepubDoc",
                             "The GepubDoc for this widget",
                             GEPUB_TYPE_DOC,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, NUM_PROPS, properties);
}

/**
 * gepub_widget_new:
 *
 * Returns: (transfer full): the new GepubWidget created
 */
GtkWidget *
gepub_widget_new (void)
{
  return g_object_new (GEPUB_TYPE_WIDGET,
                       NULL);
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
 * gepub_widget_set_doc:
 * @widget: a #GepubWidget
 * @doc: (nullable): a #GepubDoc
 *
 * Sets @doc as the document displayed by the widget.
 */
void
gepub_widget_set_doc (GepubWidget *widget,
                      GepubDoc    *doc)
{
    if (g_set_object (&widget->doc, doc)) {
        gepub_widget_reload (widget);
        g_object_notify_by_pspec (G_OBJECT (widget), properties[PROP_DOC]);
    }
}

/**
 * gepub_widget_reload:
 * @widget: a #GepubWidget
 *
 * This method reloads the data with the GepubDoc current chapter
 */
void
gepub_widget_reload (GepubWidget *widget)
{
    gsize bufsize = 0;
    guchar *buffer = NULL;

    if (!widget->doc)
      return;

    buffer = gepub_doc_get_current_with_epub_uris (widget->doc, &bufsize);

    webkit_web_view_load_bytes (WEBKIT_WEB_VIEW (widget),
                                g_bytes_new_take (buffer, bufsize),
                                gepub_doc_get_current_mime (widget->doc),
                                "UTF-8", NULL);
}
