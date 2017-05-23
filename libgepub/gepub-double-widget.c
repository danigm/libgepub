/* GepubDoubleWidget
 *
 * Copyright (C) 2017 Daniel Garcia <danigm@wadobo.com>
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

#include "gepub-double-widget.h"

struct _GepubDoubleWidget {
    GtkBox parent;

    GepubWidget *page1;
    GepubWidget *page2;

    GepubDoc *doc1;
    GepubDoc *doc2;
};

struct _GepubDoubleWidgetClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE (GepubDoubleWidget, gepub_double_widget, GTK_TYPE_BOX)

static void
gepub_double_widget_finalize (GObject *object)
{
    GepubDoubleWidget *widget = GEPUB_DOUBLE_WIDGET (object);

    if (widget->doc1) {
        g_object_unref (G_OBJECT (widget->doc1));
    }
    if (widget->doc2) {
        g_object_unref (G_OBJECT (widget->doc2));
    }

    G_OBJECT_CLASS (gepub_double_widget_parent_class)->finalize (object);
}

static gboolean
page_next (GtkWidget *w, gboolean ok)
{
    gepub_widget_set_pagination_cb (w, NULL);

    if (!ok) {
        printf ("END\n");
    } else {
        gepub_widget_page_next (w);
    }

    return FALSE;
}

static gboolean
page_prev (GtkWidget *w, gboolean ok)
{
    gepub_widget_set_pagination_cb (w, NULL);

    if (!ok) {
        printf ("begining\n");
    } else {
        gepub_widget_page_prev (w);
    }

    return FALSE;
}

static void
on_finish_load (GtkWidget *widget,
                gpointer data)
{
    gint page = (int)data;
    printf ("finish loaded %d\n");
}

static void
gepub_double_widget_init (GepubDoubleWidget *widget)
{
    widget->page1 = GEPUB_WIDGET (gepub_widget_new ());
    widget->page2 = GEPUB_WIDGET (gepub_widget_new ());

    widget->doc1 = NULL;
    widget->doc2 = NULL;

    gtk_box_pack_start (GTK_BOX (widget), GTK_WIDGET (widget->page1), TRUE, TRUE, 1);
    gtk_box_pack_start (GTK_BOX (widget), GTK_WIDGET (widget->page2), TRUE, TRUE, 1);
}

static void
gepub_double_widget_class_init (GepubDoubleWidgetClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = gepub_double_widget_finalize;
}

/**
 * gepub_double_widget_new:
 *
 * Returns: (transfer full): the new GepubDoubleWidget created
 */
GtkWidget *
gepub_double_widget_new (void)
{
    GObject *self;

    self = g_object_new (GEPUB_TYPE_DOUBLE_WIDGET,
                         "orientation", GTK_ORIENTATION_HORIZONTAL,
                         NULL);

    return GTK_WIDGET (self);
}

/**
 * gepub_double_widget_open:
 * @widget: a #GepubDoubleWidget
 * @path: The full path to a epub document
 *
 * loads the epub by path
 */
void
gepub_double_widget_open (GepubDoubleWidget *widget,
                          gchar             *path)
{
    g_return_if_fail (GEPUB_IS_DOUBLE_WIDGET (widget));

    widget->doc1 = gepub_doc_new (path);
    widget->doc2 = gepub_doc_new (path);

    gepub_widget_set_doc (widget->page1, widget->doc1);
    gepub_widget_set_doc (widget->page2, widget->doc2);

    gepub_widget_set_pagination (widget->page1, TRUE);
    gepub_widget_set_pagination (widget->page2, TRUE);

    g_timeout_add(300, page_next, widget->page2);
}

/**
 * gepub_double_widget_get_n_chapters:
 * @widget: a #GepubDoubleWidget
 *
 * Returns: the number of chapters in the document
 */
gint
gepub_double_widget_get_n_chapters (GepubDoubleWidget *widget)
{
    return gepub_widget_get_n_chapters (widget->page1);
}

/**
 * gepub_double_widget_get_chapter:
 * @widget: a #GepubDoubleWidget
 *
 * Returns: the current chapter in the document
 */
gint
gepub_double_widget_get_chapter (GepubDoubleWidget *widget)
{
    return gepub_widget_get_chapter (widget->page1);
}

/**
 * gepub_double_widget_get_chapter_length:
 * @widget: a #GepubDoubleWidget
 *
 * Returns: the current chapter length
 */
gint
gepub_double_widget_get_chapter_length (GepubDoubleWidget *widget)
{
    return gepub_widget_get_chapter_length (widget->page1);
}

/**
 * gepub_double_widget_set_chapter:
 * @widget: a #GepubDoubleWidget
 * @index: the new chapter
 *
 * Sets the current chapter in the doc
 */
void
gepub_double_widget_set_chapter (GepubDoubleWidget *widget,
                                 gint               index)
{
    gepub_widget_set_chapter (widget->page1, index);
    gepub_widget_set_chapter (widget->page2, index);
}

/**
 * gepub_double_widget_page_next:
 * @widget: a #GepubDoubleWidget
 *
 * Returns: TRUE on success, FALSE if there's no next page
 */
gboolean
gepub_double_widget_page_next (GepubDoubleWidget *widget)
{
    gepub_widget_set_pagination_cb (widget->page1, G_CALLBACK (page_next));
    gepub_widget_set_pagination_cb (widget->page2, G_CALLBACK (page_next));

    gepub_widget_page_next (widget->page1);
    gepub_widget_page_next (widget->page2);

    return TRUE;
}

/**
 * gepub_double_widget_page_prev:
 * @widget: a #GepubDoubleWidget
 *
 * Returns: TRUE on success, FALSE if there's no next page
 */
gboolean
gepub_double_widget_page_prev (GepubDoubleWidget *widget)
{
    gepub_widget_set_pagination_cb (widget->page1, G_CALLBACK (page_prev));
    gepub_widget_set_pagination_cb (widget->page2, G_CALLBACK (page_prev));

    gepub_widget_page_prev (widget->page1);
    gepub_widget_page_prev (widget->page2);
    return TRUE;
}

/**
 * gepub_double_widget_get_pos:
 * @widget: a #GepubDoubleWidget
 *
 * Returns: the current position in the chapter
 */
gfloat
gepub_double_widget_get_pos (GepubDoubleWidget *widget)
{
    return gepub_widget_get_pos (widget->page1);
}

/**
 * gepub_double_widget_set_pos:
 * @widget: a #GepubDoubleWidget
 * @index: the new pos
 *
 * Sets the current position in the chapter
 */
void
gepub_double_widget_set_pos (GepubDoubleWidget *widget,
                             gfloat             index)
{
    gepub_widget_set_pos (widget->page1, index);
    gepub_widget_set_pos (widget->page2, index);
    gepub_widget_page_next (widget->page2);
}


/**
 * gepub_double_widget_set_margin:
 * @widget: a #GepubDoubleWidget
 * @margin: the margin in pixels
 *
 * Sets the widget left and right margin
 */
void
gepub_double_widget_set_margin (GepubDoubleWidget *widget,
                                gint               margin)
{
    gepub_widget_set_margin (widget->page1, margin);
    gepub_widget_set_margin (widget->page2, margin);
}

/**
 * gepub_double_widget_get_margin:
 * @widget: a #GepubDoubleWidget
 *
 * Gets the widget left and right margin
 */
gint
gepub_double_widget_get_margin (GepubDoubleWidget *widget)
{
    return gepub_widget_get_margin (widget->page1);
}

/**
 * gepub_double_widget_get_fontsize:
 * @widget: a #GepubDoubleWidget
 *
 * Gets the widget custom font size in pt, if 0, it's not set
 */
gint
gepub_double_widget_get_fontsize (GepubDoubleWidget *widget)
{
    return gepub_widget_get_fontsize (widget->page1);
}

/**
 * gepub_double_widget_set_fontsize:
 * @widget: a #GepubDouble_Widget
 * @size: the custom font size in pt
 *
 * Sets the widget custom font size, use 0 to show book's styles
 */
void
gepub_double_widget_set_fontsize (GepubDoubleWidget *widget,
                                  gint               size)
{
    gepub_widget_set_fontsize (widget->page1, size);
    gepub_widget_set_fontsize (widget->page2, size);
}

/**
 * gepub_double_widget_get_lineheight:
 * @widget: a #GepubDoubleWidget
 *
 * Gets the widget custom line height, if 0, it's not set
 */
gfloat
gepub_double_widget_get_lineheight (GepubDoubleWidget *widget)
{
    return gepub_widget_get_lineheight (widget->page1);
}

/**
 * gepub_double_widget_set_lineheight:
 * @widget: a #GepubDoubleWidget
 * @size: the custom line height
 *
 * Sets the widget custom line height, the real size will be this
 * number multiplied by the font size.
 * Use 0 to show book's styles
 */
void
gepub_double_widget_set_lineheight (GepubDoubleWidget *widget,
                                    gfloat             size)
{
    gepub_widget_set_lineheight (widget->page1, size);
    gepub_widget_set_lineheight (widget->page2, size);
}
