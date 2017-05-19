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
#include <JavaScriptCore/JSValueRef.h>

#include "gepub-widget.h"

struct _GepubWidget {
    WebKitWebView parent;

    GepubDoc *doc;
    gboolean paginate;
    gint page;
    gint chapter_pages;
};

struct _GepubWidgetClass {
    WebKitWebViewClass parent_class;
};

enum {
    PROP_0,
    PROP_DOC,
    PROP_PAGINATE,
    PROP_PAGE,
    PROP_NPAGES,
    NUM_PROPS
};

static GParamSpec *properties[NUM_PROPS] = { NULL, };

G_DEFINE_TYPE (GepubWidget, gepub_widget, WEBKIT_TYPE_WEB_VIEW)

static void
pagination_initialize_finished (GObject      *object,
                                GAsyncResult *result,
                                gpointer     user_data)
{
    WebKitJavascriptResult *js_result;
    JSValueRef              value;
    JSGlobalContextRef      context;
    GError                 *error = NULL;
    GepubWidget            *widget = GEPUB_WIDGET (user_data);

    js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
    if (!js_result) {
        g_warning ("Error running javascript: %s", error->message);
        g_error_free (error);
        return;
    }

    context = webkit_javascript_result_get_global_context (js_result);
    value = webkit_javascript_result_get_value (js_result);
    if (JSValueIsNumber (context, value)) {
        double n;

        n = JSValueToNumber (context, value, NULL);
        widget->chapter_pages = (int)n;
        g_object_notify_by_pspec (G_OBJECT (widget), properties[PROP_NPAGES]);
    } else {
        g_warning ("Error running javascript: unexpected return value");
    }
    webkit_javascript_result_unref (js_result);
}

static void
paginate_cb (WebKitWebView  *web_view,
             WebKitLoadEvent load_event,
             gpointer        user_data)
{

    GepubWidget *widget = GEPUB_WIDGET (web_view);

    if (load_event == WEBKIT_LOAD_FINISHED) {
        const gchar *script = "function initialize() { "
                                  "var d = document.querySelector('body');"
                                  "var ourH = window.innerHeight - 40; "
                                  "var ourW = window.innerWidth - 20; "
                                  "var fullH = d.offsetHeight; "
                                  "var pageCount = Math.floor(fullH/ourH)+1;"
                                  "var newW = pageCount * ourW; "
                                  "d.style.height = ourH+'px';"
                                  "d.style.width = newW+'px';"
                                  "d.style.WebkitColumnCount = pageCount;"
                                  "d.style.WebkitColumnGap = '20px';"
                                  "d.style.overflow = 'hidden';"
                                  "window.currentPage = 0; "
                                  "return pageCount;"
                              "};"
                              "function next() { "
                                  "var ourW = window.innerWidth - 10; "
                                  "window.currentPage += 1; "
                                  "window.scroll(ourW * window.currentPage, 0); "
                              "};"
                              "function prev() { "
                                  "var ourW = window.innerWidth - 10; "
                                  "window.currentPage -= 1; "
                                  "window.scroll(ourW * window.currentPage, 0); "
                              "};"
                              "function nth(index) { "
                                  "var ourW = window.innerWidth - 10; "
                                  "window.currentPage = index; "
                                  "window.scroll(ourW * window.currentPage, 0); "
                              "};"
                              "initialize();";

        if (widget->paginate) {
            webkit_web_view_run_javascript (web_view, "document.querySelector('body').style.margin = '20px';", NULL, NULL, NULL);
            webkit_web_view_run_javascript (web_view, script, NULL, pagination_initialize_finished, (gpointer)widget);
        }
    }
}

static void
resource_callback (WebKitURISchemeRequest *request, gpointer user_data)
{
    GInputStream *stream;
    gchar *path;
    gchar *uri;
    gchar *mime;
    GepubWidget *widget = user_data;
    GBytes *contents;

    if (!widget->doc)
      return;

    uri = g_strdup (webkit_uri_scheme_request_get_uri (request));
    // removing "epub://"
    path = uri + 7;
    contents = gepub_doc_get_resource (widget->doc, path);
    mime = gepub_doc_get_resource_mime (widget->doc, path);

    if (!mime) {
        g_free (uri);
        return;
    }

    stream = g_memory_input_stream_new_from_bytes (contents);
    webkit_uri_scheme_request_finish (request, stream, g_bytes_get_size (contents), mime);

    g_object_unref (stream);
    g_bytes_unref (contents);
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
    case PROP_PAGINATE:
        gepub_widget_set_pagination (widget, g_value_get_boolean (value));
        break;
    case PROP_PAGE:
        gepub_widget_set_page (widget, g_value_get_int (value));
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
    case PROP_PAGINATE:
        g_value_set_boolean (value, widget->paginate);
        break;
    case PROP_PAGE:
        g_value_set_int (value, gepub_widget_get_page (widget));
        break;
    case PROP_NPAGES:
        g_value_set_int (value, gepub_widget_get_n_pages (widget));
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
    g_signal_connect (widget, "load-changed", G_CALLBACK (paginate_cb), NULL);
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

    properties[PROP_PAGINATE] =
        g_param_spec_boolean ("paginate",
                              "paginate",
                              "If the widget should paginate",
                              FALSE,
                              G_PARAM_READWRITE);

    properties[PROP_PAGE] =
        g_param_spec_int ("page",
                          "Current page",
                          "Current page in the chapter",
                          -1, G_MAXINT, 0,
                          G_PARAM_READWRITE);

    properties[PROP_NPAGES] =
        g_param_spec_int ("chapter_pages",
                          "Current chapter pages",
                          "Current chapter number of pages",
                          -1, G_MAXINT, 0,
                          G_PARAM_READABLE);

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
    g_return_val_if_fail (GEPUB_IS_WIDGET (widget), NULL);

    return widget->doc;
}

static void
reload_current_chapter (GepubWidget *widget)
{
    GBytes *current;

    current = gepub_doc_get_current_with_epub_uris (widget->doc);
    webkit_web_view_load_bytes (WEBKIT_WEB_VIEW (widget),
                                current,
                                gepub_doc_get_current_mime (widget->doc),
                                "UTF-8", NULL);
    g_bytes_unref (current);
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
    g_return_if_fail (GEPUB_IS_WIDGET (widget));

    if (widget->doc == doc)
        return;

    if (widget->doc != NULL) {
        g_signal_handlers_disconnect_by_func (widget->doc,
                                              reload_current_chapter, widget);
        g_object_unref (widget->doc);
    }

    widget->doc = doc;

    if (widget->doc != NULL) {
        g_object_ref (widget->doc);
        reload_current_chapter (widget);
        g_signal_connect_swapped (widget->doc, "notify::page",
                                  G_CALLBACK (reload_current_chapter), widget);
    }

    g_object_notify_by_pspec (G_OBJECT (widget), properties[PROP_DOC]);
}

/**
 * gepub_widget_set_pagination:
 * @widget: a #GepubWidget
 * @p: true if the widget should paginate
 *
 * Enable or disable pagination
 */
void
gepub_widget_set_pagination (GepubWidget *widget,
                             gboolean p)
{
    widget->paginate = p;
    reload_current_chapter (widget);
}

/**
 * gepub_widget_page_next:
 * @widget: a #GepubWidget
 *
 * Change the page to the next
 */
void
gepub_widget_page_next (GepubWidget *widget)
{
    if (widget->page >= widget->chapter_pages)
        return;

    webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (widget), "next();", NULL, NULL, NULL);
    widget->page += 1;
}

/**
 * gepub_widget_page_prev:
 * @widget: a #GepubWidget
 *
 * Change the page to the prev
 */
void
gepub_widget_page_prev (GepubWidget *widget)
{
    if (widget->page <= 0)
        return;

    webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (widget), "prev();", NULL, NULL, NULL);
    widget->page -= 1;
}

/**
 * gepub_widget_get_n_pages:
 * @widget: a #GepubWidget
 *
 * Returns: the number of pages in the chapter
 */
int
gepub_widget_get_n_pages (GepubWidget *widget)
{
    return widget->chapter_pages;
}

/**
 * gepub_widget_get_page:
 * @widget: a #GepubWidget
 *
 * Returns: the current page in the chapter
 */
int
gepub_widget_get_page (GepubWidget *widget)
{
    return widget->page;
}

/**
 * gepub_widget_set_page:
 * @widget: a #GepubWidget
 * @index: the new page index
 *
 * Sets the widget current page to @index.
 */
void
gepub_widget_set_page (GepubWidget *widget, gint index)
{
    gchar *script;

    if (index >= widget->chapter_pages)
        return;

    if (index < 0)
        return;

    script = g_strdup_printf ("nth(%d);", index);
    webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (widget), script, NULL, NULL, NULL);
    widget->page = index;

    g_free (script);
}
