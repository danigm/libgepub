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

#ifndef __GEPUB_DOUBLE_WIDGET_H__
#define __GEPUB_DOUBLE_WIDGET_H__

#include <webkit2/webkit2.h>
#include <glib-object.h>
#include <glib.h>

#include "gepub-widget.h"

G_BEGIN_DECLS

#define GEPUB_TYPE_DOUBLE_WIDGET           (gepub_double_widget_get_type ())
#define GEPUB_DOUBLE_WIDGET(obj)           (G_TYPE_CHECK_INSTANCE_CAST (obj, GEPUB_TYPE_DOUBLE_WIDGET, GepubDoubleWidget))
#define GEPUB_DOUBLE_WIDGET_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST (cls, GEPUB_TYPE_DOUBLE_WIDGET, GepubDoubleWidgetClass))
#define GEPUB_IS_DOUBLE_WIDGET(obj)        (G_TYPE_CHECK_INSTANCE_TYPE (obj, GEPUB_TYPE_DOUBLE_WIDGET))
#define GEPUB_IS_DOUBLE_WIDGET_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE (obj, GEPUB_TYPE_DOUBLE_WIDGET))
#define GEPUB_DOUBLE_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GEPUB_TYPE_DOUBLE_WIDGET, GepubDoubleWidgetClass))

typedef struct _GepubDoubleWidget      GepubDoubleWidget;
typedef struct _GepubDoubleWidgetClass GepubDoubleWidgetClass;

GType             gepub_double_widget_get_type                        (void) G_GNUC_CONST;

GtkWidget        *gepub_double_widget_new                             (void);

void              gepub_double_widget_open                            (GepubDoubleWidget *widget,
                                                                       gchar *path);

gint              gepub_double_widget_get_n_chapters                  (GepubDoubleWidget *widget);
gint              gepub_double_widget_get_chapter                     (GepubDoubleWidget *widget);
gint              gepub_double_widget_get_chapter_length              (GepubDoubleWidget *widget);
void              gepub_double_widget_set_chapter                     (GepubDoubleWidget *widget,
                                                                       gint         index);

gfloat            gepub_double_widget_get_pos                         (GepubDoubleWidget *widget);
void              gepub_double_widget_set_pos                         (GepubDoubleWidget *widget,
                                                                       gfloat       index);
gboolean          gepub_double_widget_page_next                       (GepubDoubleWidget *widget);
gboolean          gepub_double_widget_page_prev                       (GepubDoubleWidget *widget);

gint              gepub_double_widget_get_margin                      (GepubDoubleWidget *widget);
void              gepub_double_widget_set_margin                      (GepubDoubleWidget *widget,
                                                                       gint         margin);

gint              gepub_double_widget_get_fontsize                    (GepubDoubleWidget *widget);
void              gepub_double_widget_set_fontsize                    (GepubDoubleWidget *widget,
                                                                       gint         size);

gfloat            gepub_double_widget_get_lineheight                  (GepubDoubleWidget *widget);
void              gepub_double_widget_set_lineheight                  (GepubDoubleWidget *widget,
                                                                       gfloat       size);

G_END_DECLS

#endif /* __GEPUB_DOUBLE_WIDGET_H__ */

