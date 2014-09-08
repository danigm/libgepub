/* GepubDoc
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
#include <libxml/HTMLparser.h>
#include <string.h>

#include "gepub-utils.h"
#include "gepub-doc.h"
#include "gepub-archive.h"
#include "gepub-text-chunk.h"

static void g_epub_doc_fill_resources (GepubDoc *doc);
static void g_epub_doc_fill_spine (GepubDoc *doc);
static gboolean equal_strs (gchar *one, gchar *two);

void g_epub_resource_free (GepubResource *res)
{
    g_free (res->mime);
    g_free (res->uri);
    g_free (res);
}

struct _GepubDoc {
    GObject parent;

    GepubArchive *archive;
    guchar *content;
    gchar *content_base;
    GHashTable *resources;
    GList *spine;
};

struct _GepubDocClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (GepubDoc, gepub_doc, G_TYPE_OBJECT)

static void
gepub_doc_finalize (GObject *object)
{
    GepubDoc *doc = GEPUB_DOC (object);

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

    xmlCleanupParser ();

    G_OBJECT_CLASS (gepub_doc_parent_class)->finalize (object);
}

static void
gepub_doc_init (GepubDoc *doc)
{
}

static void
gepub_doc_class_init (GepubDocClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = gepub_doc_finalize;
}

/**
 * gepub_doc_new:
 * @path: the epub doc path
 *
 * Returns: (transfer full): the new GepubDoc created
 */
GepubDoc *
gepub_doc_new (const gchar *path)
{
    GepubDoc *doc;

    gchar *file;
    gsize bufsize = 0;
    gint i = 0, len;

    doc = GEPUB_DOC (g_object_new (GEPUB_TYPE_DOC, NULL));
    doc->archive = gepub_archive_new (path);

    file = gepub_archive_get_root_file (doc->archive);
    if (!file)
        return NULL;
    if (!gepub_archive_read_entry (doc->archive, file, &(doc->content), &bufsize))
        return NULL;

    len = strlen (file);
    while (file[i++] != '/' && i < len);
    doc->content_base = g_strndup (file, i);

    // doc resources hashtable:
    // id : (mime, path)
    doc->resources = g_hash_table_new_full (g_str_hash,
                                            (GEqualFunc)equal_strs,
                                            (GDestroyNotify)g_free,
                                            (GDestroyNotify)g_epub_resource_free);
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
g_epub_doc_fill_resources (GepubDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *mnode = NULL;
    xmlNode *item = NULL;
    gchar *id, *tmpuri, *uri;
    GepubResource *res;

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

        res = g_malloc (sizeof (GepubResource));
        res->mime = xmlGetProp (item, "media-type");
        res->uri = uri;
        g_hash_table_insert (doc->resources, id, res);
        item = item->next;
    }

    xmlFreeDoc (xdoc);
}

static void
g_epub_doc_fill_spine (GepubDoc *doc)
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

/**
 * gepub_doc_get_content:
 * @doc: a #GepubDoc
 *
 * Returns: (transfer none): the document content
 */
gchar *
gepub_doc_get_content (GepubDoc *doc)
{
    return doc->content;
}

/**
 * gepub_doc_get_metadata:
 * @doc: a #GepubDoc
 * @mdata: a metadata name string, GEPUB_META_TITLE for example
 *
 * Returns: (transfer full): metadata string
 */
gchar *
gepub_doc_get_metadata (GepubDoc *doc, gchar *mdata)
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

/**
 * gepub_doc_get_resources:
 * @doc: a #GepubDoc
 *
 * Returns: (element-type utf8 Gepub.Resource) (transfer none): doc resource table
 */
GHashTable *
gepub_doc_get_resources (GepubDoc *doc)
{
    return doc->resources;
}

/**
 * gepub_doc_get_resource:
 * @doc: a #GepubDoc
 * @id: the resource id
 * @bufsize: (out): location to store the length in bytes of the contents
 *
 * Returns: (transfer full): the resource content
 */
guchar *
gepub_doc_get_resource (GepubDoc *doc, gchar *id, gsize *bufsize)
{
    guchar *res = NULL;
    GepubResource *gres = g_hash_table_lookup (doc->resources, id);
    if (!gres) {
        // not found
        return NULL;
    }
    if (!gepub_archive_read_entry (doc->archive, gres->uri, &res, bufsize))
        return NULL;

    return res;
}

/**
 * gepub_doc_get_resource_v:
 * @doc: a #GepubDoc
 * @v: the resource path
 * @bufsize: (out): location to store length in bytes of the contents
 *
 * Returns: (transfer full): the resource content
 */
guchar *
gepub_doc_get_resource_v (GepubDoc *doc, gchar *v, gsize *bufsize)
{
    guchar *res = NULL;
    gchar *path = NULL;

    path = g_strdup_printf ("%s%s", doc->content_base, v);
    if (!gepub_archive_read_entry (doc->archive, path, &res, bufsize)) {
        g_free (path);
        return NULL;
    }
    g_free (path);

    return res;
}

/**
 * gepub_doc_get_resource_mime_id:
 * @doc: a #GepubDoc
 * @id: the resource id
 *
 * Returns: (transfer full): the resource content
 */
guchar *
gepub_doc_get_resource_mime_id (GepubDoc *doc, gchar *id)
{
    GepubResource *gres = g_hash_table_lookup (doc->resources, id);
    if (!gres) {
        // not found
        return NULL;
    }

    return gres->mime;
}

/**
 * gepub_doc_get_resource_mime:
 * @doc: a #GepubDoc
 * @v: the resource path
 *
 * Returns: (transfer none): the resource mime
 */
guchar *
gepub_doc_get_resource_mime (GepubDoc *doc, gchar *v)
{
    gchar *path = NULL;
    GepubResource *gres;
    GList *keys = g_hash_table_get_keys (doc->resources);

    path = g_strdup_printf ("%s%s", doc->content_base, v);

    while (keys) {
        gres = ((GepubResource*)g_hash_table_lookup (doc->resources, keys->data));
        if (!strcmp (gres->uri, path))
            break;
        keys = keys->next;
    }

    if (keys)
        return gres->mime;
    else
        return NULL;
}

/**
 * gepub_doc_get_spine:
 * @doc: a #GepubDoc
 *
 * Returns: (element-type utf8) (transfer none): the document spine
 */
GList *
gepub_doc_get_spine (GepubDoc *doc)
{
    return doc->spine;
}

/**
 * gepub_doc_get_current:
 * @doc: a #GepubDoc
 * @bufsize: (out): location to store the length in bytes of the contents
 *
 * Returns: (transfer full): the current chapter data
 */
guchar *
gepub_doc_get_current (GepubDoc *doc, gsize *bufsize)
{
    return gepub_doc_get_resource (doc, doc->spine->data, bufsize);
}

/**
 * gepub_doc_get_text:
 * @doc: a #GepubDoc
 *
 * Returns: (element-type Gepub.TextChunk) (transfer full): the list of text in the current chapter, Free with gepub_doc_free_text().
 */
GList *
gepub_doc_get_text (GepubDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    guchar *res = NULL;
    gsize size = 0;

    GList *texts = NULL;

    LIBXML_TEST_VERSION

    res = gepub_doc_get_current (doc, &size);
    if (!res) {
        return NULL;
    }
    xdoc = htmlReadDoc (res, "", NULL, HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    root_element = xmlDocGetRootElement (xdoc);
    texts = gepub_utils_get_text_elements (root_element);

    g_free (res);
    xmlFreeDoc (xdoc);

    return texts;
}

/**
 * gepub_doc_get_text_by_id:
 * @doc: a #GepubDoc
 * @id: the resource id
 *
 * Returns: (element-type Gepub.TextChunk) (transfer full): the list of text in the current chapter, Free with gepub_doc_free_text().
 */
GList *
gepub_doc_get_text_by_id (GepubDoc *doc, gchar *id)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    gsize size = 0;
    guchar *res = NULL;

    GList *texts = NULL;

    LIBXML_TEST_VERSION

    res = gepub_doc_get_resource (doc, id, &size);
    if (!res) {
        return NULL;
    }

    xdoc = htmlReadDoc (res, "", NULL, HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    root_element = xmlDocGetRootElement (xdoc);
    texts = gepub_utils_get_text_elements (root_element);

    g_free (res);
    xmlFreeDoc (xdoc);

    return texts;
}

/**
 * gepub_doc_free_text:
 * @doc: a #GList
 */
void
gepub_doc_free_text (GList *tlist)
{
    g_list_free_full (tlist, (GDestroyNotify)g_object_unref);
}

/**
 * gepub_doc_go_next:
 * @doc: a #GepubDoc
 */
void gepub_doc_go_next (GepubDoc *doc)
{
    if (doc->spine->next)
        doc->spine = doc->spine->next;
}

/**
 * gepub_doc_go_next:
 * @doc: a #GepubDoc
 */
void gepub_doc_go_prev (GepubDoc *doc)
{
    if (doc->spine->prev)
        doc->spine = doc->spine->prev;
}


/**
 * gepub_doc_get_cover:
 * @doc: a #GepubDoc
 *
 * Returns: (transfer full): cover file path to retrieve with
 * gepub_doc_get_resource
 */
gchar *
gepub_doc_get_cover (GepubDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *mnode = NULL;
    gchar *ret;
    xmlChar *text;

    LIBXML_TEST_VERSION

    xdoc = xmlRecoverDoc (doc->content);
    root_element = xmlDocGetRootElement (xdoc);
    mnode = gepub_utils_get_element_by_attr (root_element, "name", "cover");
    text = xmlGetProp(mnode, "content");

    ret = g_strdup (text);
    xmlFree (text);

    xmlFreeDoc (xdoc);
    xmlCleanupParser ();

    return ret;
}
