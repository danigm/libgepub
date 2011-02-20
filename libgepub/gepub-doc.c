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
#include <string.h>

#include "gepub-utils.h"
#include "gepub-doc.h"
#include "gepub-archive.h"

static void g_epub_doc_fill_resources (GEPUBDoc *doc);
static void g_epub_doc_fill_spine (GEPUBDoc *doc);
static gboolean equal_strs (gchar *one, gchar *two);

struct _GEPUBDoc {
    GObject parent;

    GEPUBArchive *archive;
    guchar *content;
    gchar *content_base;
    GHashTable *resources;
    GList *spine;
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
    if (doc->content_base)
        g_free (doc->content_base);

    if (doc->resources) {
        g_hash_table_destroy (doc->resources);
        doc->resources = NULL;
    }

    if (doc->spine) {
        g_list_foreach (doc->spine, (GFunc)g_free, NULL);
        g_list_free (doc->spine);
        doc->spine = NULL;
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
    gint i = 0, len;

    doc = GEPUB_DOC (g_object_new (GEPUB_TYPE_DOC, NULL));
    doc->archive = gepub_archive_new (path);

    file = gepub_archive_get_root_file (doc->archive);
    gepub_archive_read_entry (doc->archive, file, &(doc->content), &bufsize);

    len = strlen (file);
    while (file[i++] != '/' && i < len);
    doc->content_base = g_strndup (file, i);

    // doc resources hashtable:
    // id : path
    doc->resources = g_hash_table_new_full (g_str_hash,
                                            (GEqualFunc)equal_strs,
                                            (GDestroyNotify)g_free,
                                            (GDestroyNotify)g_free);
    g_epub_doc_fill_resources (doc);
    doc->spine = NULL;
    g_epub_doc_fill_spine (doc);

    if (file)
        g_free (file);

    return doc;
}

static gboolean
equal_strs (gchar *one, gchar *two)
{
    if (strcmp (one, two))
        return FALSE;
    return TRUE;
}

static void
g_epub_doc_fill_resources (GEPUBDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *mnode = NULL;
    xmlNode *item = NULL;
    gchar *id, *tmpuri, *uri;

    LIBXML_TEST_VERSION

    xdoc = xmlRecoverDoc (doc->content);
    root_element = xmlDocGetRootElement (xdoc);
    mnode = gepub_utils_get_element_by_tag (root_element, "manifest");

    item = mnode->children;
    while (item) {
        if (item->type != XML_ELEMENT_NODE ) {
            item = item->next;
            continue;
        }

        id = xmlGetProp (item, "id");
        tmpuri = xmlGetProp (item, "href");
        uri = g_strdup_printf ("%s%s", doc->content_base, tmpuri);
        g_free (tmpuri);

        g_hash_table_insert (doc->resources, id, uri);
        item = item->next;
    }

    xmlFreeDoc (xdoc);
    xmlCleanupParser ();
}

static void
g_epub_doc_fill_spine (GEPUBDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *snode = NULL;
    xmlNode *item = NULL;
    gchar *id;

    LIBXML_TEST_VERSION

    xdoc = xmlRecoverDoc (doc->content);
    root_element = xmlDocGetRootElement (xdoc);
    snode = gepub_utils_get_element_by_tag (root_element, "spine");

    item = snode->children;
    while (item) {
        if (item->type != XML_ELEMENT_NODE ) {
            item = item->next;
            continue;
        }

        id = xmlGetProp (item, "idref");

        doc->spine = g_list_append (doc->spine, id);
        item = item->next;
    }

    xmlFreeDoc (xdoc);
    xmlCleanupParser ();
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

GHashTable *
gepub_doc_get_resources (GEPUBDoc *doc)
{
    return doc->resources;
}

guchar *
gepub_doc_get_resource (GEPUBDoc *doc, gchar *id)
{
    guchar *res = NULL;
    gchar *path = NULL;
    gint bufsize = 0;
    path = g_hash_table_lookup (doc->resources, id);
    if (!path) {
        // not found
        return NULL;
    }
    gepub_archive_read_entry (doc->archive, g_hash_table_lookup (doc->resources, id), &res, &bufsize);

    return res;
}

GList *
gepub_doc_get_spine (GEPUBDoc *doc)
{
    return doc->spine;
}

guchar *
gepub_doc_get_current (GEPUBDoc *doc)
{
    return gepub_doc_get_resource (doc, doc->spine->data);
}

void gepub_doc_go_next (GEPUBDoc *doc)
{
    if (doc->spine->next)
        doc->spine = doc->spine->next;
}

void gepub_doc_go_prev (GEPUBDoc *doc)
{
    if (doc->spine->prev)
        doc->spine = doc->spine->prev;
}
