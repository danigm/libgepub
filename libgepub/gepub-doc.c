/* GEPUBDoc
 *
 * Copyright (C) 2011 Daniel Garcia <danigm@wadobo.com>
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
#include <libxml/tree.h>

#include "gepub-utils.h"
#include "gepub-doc.h"
#include "gepub-archive.h"

struct _GEPUBDoc {
    GObject parent;

    GEPUBArchive *archive;
    guchar *content;
};

struct _GEPUBDocClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (GEPUBDoc, gepub_doc, G_TYPE_OBJECT)

static void
gepub_doc_finalize (GObject *object)
{
    GEPUBDoc *doc = GEPUB_DOC (object);

    if (doc->archive) {
        g_object_unref (doc->archive);
        doc->archive = NULL;
    }
    if (doc->content) {
        g_free (doc->content);
        doc->content = NULL;
    }

    G_OBJECT_CLASS (gepub_doc_parent_class)->finalize (object);
}

static void
gepub_doc_init (GEPUBDoc *doc)
{
}

static void
gepub_doc_class_init (GEPUBDocClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = gepub_doc_finalize;
}

GEPUBDoc *
gepub_doc_new (const gchar *path)
{
    GEPUBDoc *doc;

    gchar *file;
    gint bufsize;

    doc = GEPUB_DOC (g_object_new (GEPUB_TYPE_DOC, NULL));
    doc->archive = gepub_archive_new (path);

    file = gepub_archive_get_root_file (doc->archive);
    gepub_archive_read_entry (doc->archive, file, &(doc->content), &bufsize);

    if (file)
        g_free (file);

    return doc;
}

gchar *
gepub_doc_get_content (GEPUBDoc *doc)
{
    return doc->content;
}

gchar *
gepub_doc_get_metadata (GEPUBDoc *doc, gchar *mdata)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *mnode = NULL;
    xmlNode *mdata_node = NULL;
    gchar *ret;
    xmlChar *text;

    LIBXML_TEST_VERSION

    xdoc = xmlRecoverDoc (doc->content);
    root_element = xmlDocGetRootElement (xdoc);
    mnode = gepub_utils_get_element_by_tag (root_element, "metadata");
    mdata_node = gepub_utils_get_element_by_tag (mnode, mdata);

    text = xmlNodeGetContent (mdata_node);
    ret = g_strdup (text);
    xmlFree (text);

    xmlFreeDoc (xdoc);
    xmlCleanupParser ();

    return ret;
}
