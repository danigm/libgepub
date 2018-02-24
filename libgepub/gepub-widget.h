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

#ifndef __GEPUB_WIDGET_H__
#define __GEPUB_WIDGET_H__

#include <webkit2/webkit2.h>
#include <glib-object.h>
#include <glib.h>

#include "gepub-doc.h"

G_BEGIN_DECLS

#define GEPUB_TYPE_WIDGET           (gepub_widget_get_type ())
#define GEPUB_WIDGET(obj)           (G_TYPE_CHECK_INSTANCE_CAST (obj, GEPUB_TYPE_WIDGET, GepubWidget))
#define GEPUB_WIDGET_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST (cls, GEPUB_TYPE_WIDGET, GepubWidgetClass))
#define GEPUB_IS_WIDGET(obj)        (G_TYPE_CHECK_INSTANCE_TYPE (obj, GEPUB_TYPE_WIDGET))
#define GEPUB_IS_WIDGET_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE (obj, GEPUB_TYPE_WIDGET))
#define GEPUB_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GEPUB_TYPE_WIDGET, GepubWidgetClass))

typedef struct _GepubWidget      GepubWidget;
typedef struct _GepubWidgetClass GepubWidgetClass;

GType             gepub_widget_get_type                        (void) G_GNUC_CONST;

GtkWidget        *gepub_widget_new                             (void);

GepubDoc         *gepub_widget_get_doc                         (GepubWidget *widget);
void              gepub_widget_set_doc                         (GepubWidget *widget,
                                                                GepubDoc    *doc);

gboolean          gepub_widget_get_paginate                    (GepubWidget *widget);
void              gepub_widget_set_paginate                    (GepubWidget *widget, gboolean p);

gint              gepub_widget_get_n_chapters                  (GepubWidget *widget);
gint              gepub_widget_get_chapter                     (GepubWidget *widget);
gint              gepub_widget_get_chapter_length              (GepubWidget *widget);
void              gepub_widget_set_chapter                     (GepubWidget *widget,
                                                                gint         index);
gboolean          gepub_widget_chapter_next                    (GepubWidget *widget);
gboolean          gepub_widget_chapter_prev                    (GepubWidget *widget);

gfloat            gepub_widget_get_pos                         (GepubWidget *widget);
void              gepub_widget_set_pos                         (GepubWidget *widget,
                                                                gfloat       index);
gboolean          gepub_widget_page_next                       (GepubWidget *widget);
gboolean          gepub_widget_page_prev                       (GepubWidget *widget);

gint              gepub_widget_get_margin                      (GepubWidget *widget);
void              gepub_widget_set_margin                      (GepubWidget *widget,
                                                                gint         margin);

gint              gepub_widget_get_fontsize                    (GepubWidget *widget);
void              gepub_widget_set_fontsize                    (GepubWidget *widget,
                                                                gint         size);

gchar            *gepub_widget_get_fontfamily                  (GepubWidget *widget);
void              gepub_widget_set_fontfamily                  (GepubWidget *widget,
                                                                gchar       *family);

gfloat            gepub_widget_get_lineheight                  (GepubWidget *widget);
void              gepub_widget_set_lineheight                  (GepubWidget *widget,
                                                                gfloat       size);

G_END_DECLS

#endif /* __GEPUB_WIDGET_H__ */

