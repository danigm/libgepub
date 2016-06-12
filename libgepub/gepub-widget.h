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

GtkWidget        *gepub_widget_new                             ();
GepubDoc         *gepub_widget_get_doc                         (GepubWidget *widget);
WebKitWebView    *gepub_widget_get_wkview                      (GepubWidget *widget);
void              gepub_widget_load_epub                       (GepubWidget *widget, const gchar *path);
void              gepub_widget_reload                          (GepubWidget *widget);

G_END_DECLS

#endif /* __GEPUB_WIDGET_H__ */

