/* GepubDoc
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
#define GEPUB_DOC(obj)           (G_TYPE_CHECK_INSTANCE_CAST (obj, GEPUB_TYPE_DOC, GepubDoc))
#define GEPUB_DOC_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST (cls, GEPUB_TYPE_DOC, GepubDocClass))
#define GEPUB_IS_DOC(obj)        (G_TYPE_CHECK_INSTANCE_TYPE (obj, GEPUB_TYPE_DOC))
#define GEPUB_IS_DOC_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE (obj, GEPUB_TYPE_DOC))
#define GEPUB_DOC_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GEPUB_TYPE_DOC, GepubDocClass))

typedef struct _GepubDoc      GepubDoc;
typedef struct _GepubDocClass GepubDocClass;

struct _GepubResource {
    gchar *mime;
    gchar *uri;
};

struct _GepubNavPoint {
    gchar *label;
    gchar *content;
    guint64 playorder;
};

typedef struct _GepubResource GepubResource;
typedef struct _GepubNavPoint GepubNavPoint;

GType             gepub_doc_get_type                        (void) G_GNUC_CONST;

GepubDoc         *gepub_doc_new                             (const gchar *path, GError **error);
GBytes           *gepub_doc_get_content                     (GepubDoc *doc);
gchar            *gepub_doc_get_metadata                    (GepubDoc *doc, const gchar *mdata);
GBytes           *gepub_doc_get_resource                    (GepubDoc *doc, const gchar *path);
GBytes           *gepub_doc_get_resource_by_id              (GepubDoc *doc, const gchar *id);
GHashTable       *gepub_doc_get_resources                   (GepubDoc *doc);
gchar            *gepub_doc_get_resource_mime               (GepubDoc *doc, const gchar *path);
gchar            *gepub_doc_get_resource_mime_by_id         (GepubDoc *doc, const gchar *id);
gchar            *gepub_doc_get_current_mime                (GepubDoc *doc);
GList            *gepub_doc_get_text                        (GepubDoc *doc);
GList            *gepub_doc_get_text_by_id                  (GepubDoc *doc, const gchar *id);
GBytes           *gepub_doc_get_current                     (GepubDoc *doc);
GBytes           *gepub_doc_get_current_with_epub_uris      (GepubDoc *doc);
gchar            *gepub_doc_get_cover                       (GepubDoc *doc);
gchar            *gepub_doc_get_resource_path               (GepubDoc *doc, const gchar *id);
gchar            *gepub_doc_get_current_path                (GepubDoc *doc);
const gchar      *gepub_doc_get_current_id                  (GepubDoc *doc);

gboolean          gepub_doc_go_next                         (GepubDoc *doc);
gboolean          gepub_doc_go_prev                         (GepubDoc *doc);
gint              gepub_doc_get_n_chapters                  (GepubDoc *doc);
gint              gepub_doc_get_chapter                     (GepubDoc *doc);
void              gepub_doc_set_chapter                     (GepubDoc *doc,
                                                             gint      index);

GList            *gepub_doc_get_toc                         (GepubDoc *doc);
gint              gepub_doc_resource_uri_to_chapter         (GepubDoc *doc,
                                                             const gchar *uri);
gint              gepub_doc_resource_id_to_chapter          (GepubDoc *doc,
                                                             const gchar *id);

G_END_DECLS

/**
 * GEPUB_META_TITLE:
 * The book title.
 */
#define GEPUB_META_TITLE "title"

/**
 * GEPUB_META_LANG:
 * The book lang.
 */
#define GEPUB_META_LANG "language"

/**
 * GEPUB_META_ID:
 * The book id.
 */
#define GEPUB_META_ID "identifier"

/**
 * GEPUB_META_AUTHOR:
 * The book author.
 */
#define GEPUB_META_AUTHOR "creator"

/**
 * GEPUB_META_DESC:
 * The book description.
 */
#define GEPUB_META_DESC "description"

#endif /* __GEPUB_DOC_H__ */

