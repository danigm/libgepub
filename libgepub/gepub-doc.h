/* GEPUBDoc
 *
 * Copyright (C) 2011  Daniel Garcia <danigm@wadobo.com>
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

#ifndef __GEPUB_DOC_H__
#define __GEPUB_DOC_H__

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define GEPUB_TYPE_DOC           (gepub_doc_get_type ())
#define GEPUB_DOC(obj)           (G_TYPE_CHECK_INSTANCE_CAST (obj, GEPUB_TYPE_DOC, GEPUBDoc))
#define GEPUB_DOC_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST (cls, GEPUB_TYPE_DOC, GEPUBDocClass))
#define GEPUB_IS_DOC(obj)        (G_TYPE_CHECK_INSTANCE_TYPE (obj, GEPUB_TYPE_DOC))
#define GEPUB_IS_DOC_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE (obj, GEPUB_TYPE_DOC))
#define GEPUB_DOC_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GEPUB_TYPE_DOC, GEPUBDocClass))

typedef struct _GEPUBDoc      GEPUBDoc;
typedef struct _GEPUBDocClass GEPUBDocClass;

GType             gepub_doc_get_type       (void) G_GNUC_CONST;

GEPUBDoc         *gepub_doc_new            (const gchar *path);
gchar            *gepub_doc_get_content    (GEPUBDoc *doc);
gchar            *gepub_doc_get_metadata   (GEPUBDoc *doc, gchar *mdata);
guchar           *gepub_doc_get_resource   (GEPUBDoc *doc, gchar *id);
GList            *gepub_doc_get_spine      (GEPUBDoc *doc);

G_END_DECLS

#define META_TITLE "title"
#define META_LANG "language"
#define META_ID "identifier"
#define META_AUTHOR "creator"
#define META_DESC "description"

#endif /* __GEPUB_DOC_H__ */

