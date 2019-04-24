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
#include <locale.h>

#include "gepub-widget.h"

struct _GepubWidget {
    WebKitWebView parent;

    GepubDoc *doc;
    gboolean paginate;
    gint chapter_length; // real chapter length
    gint chapter_pos; // position in the chapter, a percentage based on chapter_length
    gint length;
    gint init_chapter_pos;
    gint margin; // lateral margin in px
    gint font_size; // font size in pt
    gchar *font_family;
    gfloat line_height;
};

struct _GepubWidgetClass {
    WebKitWebViewClass parent_class;
};

enum {
    PROP_0,
    PROP_DOC,
    PROP_PAGINATE,
    PROP_CHAPTER,
    PROP_N_CHAPTERS,
    PROP_CHAPTER_POS,
    NUM_PROPS
};

static GParamSpec *properties[NUM_PROPS] = { NULL, };

G_DEFINE_TYPE (GepubWidget, gepub_widget, WEBKIT_TYPE_WEB_VIEW)

#define HUNDRED_PERCENT 100.0

static void
scroll_to_chapter_pos (GepubWidget *widget) {
    gchar *script = g_strdup_printf("document.querySelector('body').scrollTo(%d, 0)", widget->chapter_pos);
    webkit_web_view_run_javascript (WEBKIT_WEB_VIEW (widget), script, NULL, NULL, NULL);
    g_free(script);
}

static void
adjust_chapter_pos (GepubWidget *widget)
{
    // integer division to make a page start
    gint page = widget->chapter_pos / widget->length;
    gint next = page + 1;
    gint d1 = widget->chapter_pos - (widget->length * page);
    gint d2 = (widget->length * next) - widget->chapter_pos;

    if (d1 < d2) {
        widget->chapter_pos = widget->length * page;
    } else {
        widget->chapter_pos = widget->length * next;
    }
    scroll_to_chapter_pos (widget);
}

static void
pagination_initialize_finished (GObject      *object,
                                GAsyncResult *result,
                                gpointer     user_data)
{
    WebKitJavascriptResult *js_result;
    JSCValue               *value;
    GError                 *error = NULL;
    GepubWidget            *widget = GEPUB_WIDGET (user_data);

    js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
    if (!js_result) {
        g_warning ("Error running javascript: %s", error->message);
        g_error_free (error);
        return;
    }

    value = webkit_javascript_result_get_js_value (js_result);
    if (jsc_value_is_number (value)) {
        double n;

        n = jsc_value_to_double (value);
        widget->chapter_length = (int)n;

        if (widget->init_chapter_pos) {
            widget->chapter_pos = widget->init_chapter_pos * widget->chapter_length / HUNDRED_PERCENT;
            if (widget->chapter_pos > (widget->chapter_length - widget->length)) {
                widget->chapter_pos = (widget->chapter_length - widget->length);
            }
            widget->init_chapter_pos = 0;
        }

        if (widget->chapter_pos) {
            adjust_chapter_pos (widget);
        }
    } else {
        g_warning ("Error running javascript: unexpected return value");
    }
    webkit_javascript_result_unref (js_result);
}

static void
get_length_finished (GObject      *object,
                     GAsyncResult *result,
                     gpointer     user_data)
{
    WebKitJavascriptResult *js_result;
    JSCValue               *value;
    GError                 *error = NULL;
    GepubWidget            *widget = GEPUB_WIDGET (user_data);

    js_result = webkit_web_view_run_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
    if (!js_result) {
        g_warning ("Error running javascript: %s", error->message);
        g_error_free (error);
        return;
    }

    value = webkit_javascript_result_get_js_value (js_result);
    if (jsc_value_is_number (value)) {
        double n;

        n = jsc_value_to_double (value);
        widget->length = (int)n;
    } else {
        g_warning ("Error running javascript: unexpected return value");
    }
    webkit_javascript_result_unref (js_result);
}

static void
reload_length_cb (GtkWidget *widget,
                  GdkRectangle *allocation,
                  gpointer      user_data)
{
    GepubWidget *gwidget = GEPUB_WIDGET (widget);
    WebKitWebView *web_view = WEBKIT_WEB_VIEW (widget);
    int margin, font_size;
    float line_height;
    gchar *script, *font_family;

    webkit_web_view_run_javascript (web_view,
        "window.innerWidth",
        NULL, get_length_finished, (gpointer)widget);

    margin = gwidget->margin;
    font_size = gwidget->font_size;
    font_family = gwidget->font_family;
    line_height = gwidget->line_height;

    script = g_strdup_printf (
        "if (!document.querySelector('#gepubwrap'))"
        "document.body.innerHTML = '<div id=\"gepubwrap\">' + document.body.innerHTML + '</div>';"

        "document.querySelector('#gepubwrap').style.marginLeft = '%dpx';"
        "document.querySelector('#gepubwrap').style.marginRight = '%dpx';"
        , margin, margin);
    webkit_web_view_run_javascript (web_view, script, NULL, NULL, NULL);
    g_free (script);

    if (font_size) {
        script = g_strdup_printf (
            "document.querySelector('#gepubwrap').style.fontSize = '%dpt';"
            , font_size);
        webkit_web_view_run_javascript (web_view, script, NULL, NULL, NULL);
        g_free (script);
    }

    if (font_family) {
        script = g_strdup_printf (
            "document.querySelector('#gepubwrap').style.fontFamily = '%s';"
            , font_family);
        webkit_web_view_run_javascript (web_view, script, NULL, NULL, NULL);
        g_free (script);
    }

    if (line_height) {
        gchar line_height_buffer[G_ASCII_DTOSTR_BUF_SIZE];

        g_ascii_formatd (line_height_buffer,
                         G_ASCII_DTOSTR_BUF_SIZE,
                         "%f",
                         line_height);
        script = g_strdup_printf (
            "document.querySelector('#gepubwrap').style.lineHeight = %s;"
            , line_height_buffer);
        webkit_web_view_run_javascript (web_view, script, NULL, NULL, NULL);
        g_free (script);
    }

    if (gwidget->paginate) {
        webkit_web_view_run_javascript (web_view,
                "document.body.style.overflow = 'hidden';"
                "document.body.style.margin = '20px 0px 20px 0px';"
                "document.body.style.padding = '0px';"
                "document.body.style.columnWidth = window.innerWidth+'px';"
                "document.body.style.height = (window.innerHeight - 40) +'px';"
                "document.body.style.columnGap = '0px';"
                "document.body.scrollWidth",
                NULL, pagination_initialize_finished, (gpointer)widget);
    }
}

static void
docready_cb (WebKitWebView  *web_view,
             WebKitLoadEvent load_event,
             gpointer        user_data)
{
    GepubWidget *widget = GEPUB_WIDGET (web_view);

    if (load_event == WEBKIT_LOAD_FINISHED) {
        reload_length_cb (GTK_WIDGET (widget), NULL, NULL);
    }
}

static void
resource_callback (WebKitURISchemeRequest *request, gpointer user_data)
{
    GInputStream *stream;
    gchar *path;
    gchar *mime;
    GepubWidget *widget = user_data;
    GBytes *contents;

    if (!widget->doc)
      return;

    path = g_strdup (webkit_uri_scheme_request_get_path (request));
    contents = gepub_doc_get_resource (widget->doc, path);
    mime = gepub_doc_get_resource_mime (widget->doc, path);

    // if the resource requested doesn't exist, we should serve an
    // empty document instead of nothing at all (otherwise some
    // poorly-structured ebooks will fail to render).
    if (!contents) {
        contents = g_byte_array_free_to_bytes(g_byte_array_sized_new(0));
        mime = g_strdup("application/octet-stream");
    }

    if (!mime) {
        mime = g_strdup("application/octet-stream");
    }

    stream = g_memory_input_stream_new_from_bytes (contents);
    webkit_uri_scheme_request_finish (request, stream, g_bytes_get_size (contents), mime);

    g_object_unref (stream);
    g_bytes_unref (contents);
    g_free (mime);
    g_free (path);
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
        gepub_widget_set_paginate (widget, g_value_get_boolean (value));
        break;
    case PROP_CHAPTER:
        gepub_doc_set_chapter (widget->doc, g_value_get_int (value));
        break;
    case PROP_CHAPTER_POS:
        gepub_widget_set_pos (widget, g_value_get_float (value));
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
    case PROP_CHAPTER:
        g_value_set_int (value, gepub_doc_get_chapter (widget->doc));
        break;
    case PROP_N_CHAPTERS:
        g_value_set_int (value, gepub_doc_get_n_chapters (widget->doc));
        break;
    case PROP_CHAPTER_POS:
        g_value_set_float (value, gepub_widget_get_pos (widget));
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

    g_clear_pointer (&widget->font_family, g_free);
    g_clear_object (&widget->doc);

    G_OBJECT_CLASS (gepub_widget_parent_class)->finalize (object);
}

static void
gepub_widget_init (GepubWidget *widget)
{
    widget->chapter_length = 0;
    widget->paginate = FALSE;
    widget->chapter_pos = 0;
    widget->length = 0;
    widget->init_chapter_pos = 0;
    widget->margin = 20;
    widget->font_size = 0;
    widget->font_family = NULL;
    widget->line_height = 0;
}

static void
gepub_widget_constructed (GObject *object)
{
    WebKitWebContext *ctx;
    GepubWidget *widget = GEPUB_WIDGET (object);

    G_OBJECT_CLASS (gepub_widget_parent_class)->constructed (object);

    ctx = webkit_web_view_get_context (WEBKIT_WEB_VIEW (widget));
    webkit_web_context_register_uri_scheme (ctx, "epub", resource_callback, widget, NULL);
    g_signal_connect (widget, "load-changed", G_CALLBACK (docready_cb), NULL);
    g_signal_connect (widget, "size-allocate", G_CALLBACK (reload_length_cb), NULL);
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

    properties[PROP_CHAPTER] =
        g_param_spec_int ("chapter",
                          "Current chapter",
                          "Current chapter in the doc",
                          -1, G_MAXINT, 0,
                          G_PARAM_READWRITE);

    properties[PROP_N_CHAPTERS] =
        g_param_spec_int ("nchapters",
                          "Number of chapters in the doc",
                          "Number of chapters in the doc",
                          -1, G_MAXINT, 0,
                          G_PARAM_READABLE);

    properties[PROP_CHAPTER_POS] =
        g_param_spec_float ("chapter_pos",
                            "Current position in chapter",
                            "Current position in chapter as a percentage",
                            0.0, HUNDRED_PERCENT, 0.0,
                            G_PARAM_READWRITE);

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

    widget->chapter_length = 0;
    widget->chapter_pos = 0;
    widget->length = 0;

    if (widget->doc == NULL)
        return;

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
        g_signal_connect_swapped (widget->doc, "notify::chapter",
                                  G_CALLBACK (reload_current_chapter), widget);
    }

    g_object_notify_by_pspec (G_OBJECT (widget), properties[PROP_DOC]);
}

/**
 * gepub_widget_get_paginate:
 * @widget: a #GepubWidget
 *
 * Returns whether pagination is enabled or disabled
 */
gboolean
gepub_widget_get_paginate (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_WIDGET (widget), FALSE);

    return widget->paginate;
}

/**
 * gepub_widget_set_paginate:
 * @widget: a #GepubWidget
 * @p: true if the widget should paginate
 *
 * Enable or disable pagination
 */
void
gepub_widget_set_paginate (GepubWidget *widget,
                           gboolean p)
{
    g_return_if_fail (GEPUB_IS_WIDGET (widget));

    widget->paginate = p;
    reload_current_chapter (widget);
}

/**
 * gepub_widget_get_n_chapters:
 * @widget: a #GepubWidget
 *
 * Returns: the number of chapters in the document
 */
gint
gepub_widget_get_n_chapters (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), 0);
    return gepub_doc_get_n_chapters (widget->doc);
}

/**
 * gepub_widget_get_chapter:
 * @widget: a #GepubWidget
 *
 * Returns: the current chapter in the document
 */
gint
gepub_widget_get_chapter (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), 0);
    return gepub_doc_get_chapter (widget->doc);
}

/**
 * gepub_widget_get_chapter_length:
 * @widget: a #GepubWidget
 *
 * Returns: the current chapter length
 */
gint
gepub_widget_get_chapter_length (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), 0);
    return widget->chapter_length;
}

/**
 * gepub_widget_set_chapter:
 * @widget: a #GepubWidget
 * @index: the new chapter
 *
 * Sets the current chapter in the doc
 */
void
gepub_widget_set_chapter (GepubWidget *widget,
                          gint         index)
{
    g_return_if_fail (GEPUB_IS_DOC (widget->doc));
    return gepub_doc_set_chapter (widget->doc, index);
}

/**
 * gepub_widget_chapter_next:
 * @widget: a #GepubWidget
 *
 * Returns: TRUE on success, FALSE if there's no next chapter
 */
gboolean
gepub_widget_chapter_next (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), FALSE);
    return gepub_doc_go_next (widget->doc);
}

/**
 * gepub_widget_chapter_prev:
 * @widget: a #GepubWidget
 *
 * Returns: TRUE on success, FALSE if there's no prev chapter
 */
gboolean
gepub_widget_chapter_prev (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), FALSE);
    return gepub_doc_go_prev (widget->doc);
}

/**
 * gepub_widget_page_next:
 * @widget: a #GepubWidget
 *
 * Returns: TRUE on success, FALSE if there's no next page
 */
gboolean
gepub_widget_page_next (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), FALSE);
    widget->chapter_pos = widget->chapter_pos + widget->length;

    if (widget->chapter_pos > (widget->chapter_length - widget->length)) {
        widget->chapter_pos = (widget->chapter_length - widget->length);
        return gepub_doc_go_next (widget->doc);
    }

    scroll_to_chapter_pos (widget);

    g_object_notify_by_pspec (G_OBJECT (widget), properties[PROP_CHAPTER_POS]);
    return TRUE;
}

/**
 * gepub_widget_page_prev:
 * @widget: a #GepubWidget
 *
 * Returns: TRUE on success, FALSE if there's no next page
 */
gboolean
gepub_widget_page_prev (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), FALSE);
    widget->chapter_pos = widget->chapter_pos - widget->length;

    if (widget->chapter_pos < 0) {
        widget->init_chapter_pos = HUNDRED_PERCENT;
        return gepub_doc_go_prev (widget->doc);
    }

    scroll_to_chapter_pos (widget);

    g_object_notify_by_pspec (G_OBJECT (widget), properties[PROP_CHAPTER_POS]);
    return TRUE;
}

/**
 * gepub_widget_get_pos:
 * @widget: a #GepubWidget
 *
 * Returns: the current position in the chapter
 */
gfloat
gepub_widget_get_pos (GepubWidget *widget)
{
    g_return_val_if_fail (GEPUB_IS_DOC (widget->doc), 0);

    if (!widget->chapter_length) {
        return 0;
    }

    return widget->chapter_pos * HUNDRED_PERCENT / (float)(widget->chapter_length);
}

/**
 * gepub_widget_set_pos:
 * @widget: a #GepubWidget
 * @index: the new pos
 *
 * Sets the current position in the chapter
 */
void
gepub_widget_set_pos (GepubWidget *widget,
                      gfloat       index)
{
    g_return_if_fail (GEPUB_IS_DOC (widget->doc));
    widget->chapter_pos = index * widget->chapter_length / HUNDRED_PERCENT;
    adjust_chapter_pos (widget);

    g_object_notify_by_pspec (G_OBJECT (widget), properties[PROP_CHAPTER_POS]);
}


/**
 * gepub_widget_set_margin:
 * @widget: a #GepubWidget
 * @margin: the margin in pixels
 *
 * Sets the widget left and right margin
 */
void
gepub_widget_set_margin (GepubWidget *widget,
                         gint         margin)
{
    widget->margin = margin;
    reload_length_cb (GTK_WIDGET (widget), NULL, NULL);
}

/**
 * gepub_widget_get_margin:
 * @widget: a #GepubWidget
 *
 * Gets the widget left and right margin
 */
gint
gepub_widget_get_margin (GepubWidget *widget)
{
    return widget->margin;
}

/**
 * gepub_widget_get_fontsize:
 * @widget: a #GepubWidget
 *
 * Gets the widget custom font size in pt, if 0, it's not set
 */
gint
gepub_widget_get_fontsize (GepubWidget *widget)
{
    return widget->font_size;
}

/**
 * gepub_widget_set_fontsize:
 * @widget: a #GepubWidget
 * @size: the custom font size in pt
 *
 * Sets the widget custom font size, use 0 to show book's styles
 */
void
gepub_widget_set_fontsize (GepubWidget *widget,
                           gint         size)
{
    widget->font_size = size;
    reload_length_cb (GTK_WIDGET (widget), NULL, NULL);
}

/**
 * gepub_widget_get_fontfamily:
 * @widget: a #GepubWidget
 *
 * Gets the widget custom font family
 */
gchar *
gepub_widget_get_fontfamily (GepubWidget *widget)
{
    return widget->font_family;
}

/**
 * gepub_widget_set_fontfamily:
 * @widget: a #GepubWidget
 * @family: the custom font family name
 *
 * Sets the widget custom font family
 */
void
gepub_widget_set_fontfamily (GepubWidget *widget,
                             gchar       *family)
{
    g_clear_pointer (&widget->font_family, g_free);

    widget->font_family = g_strdup (family);
    reload_length_cb (GTK_WIDGET (widget), NULL, NULL);
}

/**
 * gepub_widget_get_lineheight:
 * @widget: a #GepubWidget
 *
 * Gets the widget custom line height, if 0, it's not set
 */
gfloat
gepub_widget_get_lineheight (GepubWidget *widget)
{
    return widget->line_height;
}

/**
 * gepub_widget_set_lineheight:
 * @widget: a #GepubWidget
 * @size: the custom line height
 *
 * Sets the widget custom line height, the real size will be this
 * number multiplied by the font size.
 * Use 0 to show book's styles
 */
void
gepub_widget_set_lineheight (GepubWidget *widget,
                             gfloat       size)
{
    widget->line_height = size;
    reload_length_cb (GTK_WIDGET (widget), NULL, NULL);
}
