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


static GQuark
gepub_error_quark (void)
{
    static GQuark q = 0;
    if (q == 0)
        q = g_quark_from_string ("gepub-quark");
    return q;
}

/**
 * GepubDocError:
 * @GEPUB_ERROR_INVALID: Invalid file
 *
 * Common errors that may be reported by GepubDoc.
 */
typedef enum {
    GEPUB_ERROR_INVALID = 0,  /*< nick=Invalid >*/
} GepubDocError;



static void gepub_doc_fill_resources (GepubDoc *doc);
static void gepub_doc_fill_spine (GepubDoc *doc);
static void gepub_doc_fill_toc (GepubDoc *doc, gchar *toc_id);
static void gepub_doc_initable_iface_init (GInitableIface *iface);
static gint navpoint_compare (GepubNavPoint *a, GepubNavPoint *b);

struct _GepubDoc {
    GObject parent;

    GepubArchive *archive;
    GBytes *content;
    gchar *content_base;
    gchar *path;
    GHashTable *resources;

    GList *spine;
    GList *chapter;
    GList *toc;
};

struct _GepubDocClass {
    GObjectClass parent_class;
};

enum {
    PROP_0,
    PROP_PATH,
    PROP_CHAPTER,
    NUM_PROPS
};

static GParamSpec *properties[NUM_PROPS] = { NULL, };

G_DEFINE_TYPE_WITH_CODE (GepubDoc, gepub_doc, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, gepub_doc_initable_iface_init))

static void
gepub_resource_free (GepubResource *res)
{
    g_free (res->mime);
    g_free (res->uri);
    g_free (res);
}

static void
gepub_doc_finalize (GObject *object)
{
    GepubDoc *doc = GEPUB_DOC (object);

    g_clear_object (&doc->archive);
    g_clear_pointer (&doc->content, g_bytes_unref);
    g_clear_pointer (&doc->path, g_free);
    g_clear_pointer (&doc->resources, g_hash_table_destroy);

    if (doc->spine) {
        g_list_foreach (doc->spine, (GFunc)g_free, NULL);
        g_clear_pointer (&doc->spine, g_list_free);
        g_list_foreach (doc->toc, (GFunc)g_free, NULL);
        g_clear_pointer (&doc->toc, g_list_free);
    }

    G_OBJECT_CLASS (gepub_doc_parent_class)->finalize (object);
}

static void
gepub_doc_set_property (GObject      *object,
			guint         prop_id,
			const GValue *value,
			GParamSpec   *pspec)
{
    GepubDoc *doc = GEPUB_DOC (object);

    switch (prop_id) {
    case PROP_PATH:
        doc->path = g_value_dup_string (value);
        break;
    case PROP_CHAPTER:
        gepub_doc_set_chapter (doc, g_value_get_int (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gepub_doc_get_property (GObject    *object,
			guint       prop_id,
			GValue     *value,
			GParamSpec *pspec)
{
    GepubDoc *doc = GEPUB_DOC (object);

    switch (prop_id) {
    case PROP_PATH:
        g_value_set_string (value, doc->path);
        break;
    case PROP_CHAPTER:
        g_value_set_int (value, gepub_doc_get_chapter (doc));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gepub_doc_init (GepubDoc *doc)
{
    /* doc resources hashtable:
     * id : (mime, path)
     */
    doc->resources = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            (GDestroyNotify)g_free,
                                            (GDestroyNotify)gepub_resource_free);
}

static void
gepub_doc_class_init (GepubDocClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = gepub_doc_finalize;
    object_class->set_property = gepub_doc_set_property;
    object_class->get_property = gepub_doc_get_property;

    properties[PROP_PATH] =
        g_param_spec_string ("path",
                             "Path",
                             "Path to the EPUB document",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);
    properties[PROP_CHAPTER] =
        g_param_spec_int ("chapter",
                          "Current chapter",
                          "The current chapter index",
                          -1, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, NUM_PROPS, properties);
}

static gboolean
gepub_doc_initable_init (GInitable     *initable,
                         GCancellable  *cancellable,
                         GError       **error)
{
    GepubDoc *doc = GEPUB_DOC (initable);
    gchar *file;
    gint i = 0, len;
    g_autofree gchar *unescaped = NULL;

    g_assert (doc->path != NULL);

    doc->archive = gepub_archive_new (doc->path);
    file = gepub_archive_get_root_file (doc->archive);
    if (!file) {
        if (error != NULL) {
            g_set_error (error, gepub_error_quark (), GEPUB_ERROR_INVALID,
                         "Invalid epub file: %s", doc->path);
        }
        return FALSE;
    }
    unescaped = g_uri_unescape_string (file, NULL);
    doc->content = gepub_archive_read_entry (doc->archive, unescaped);
    if (!doc->content) {
        if (error != NULL) {
            g_set_error (error, gepub_error_quark (), GEPUB_ERROR_INVALID,
                         "Invalid epub file: %s", doc->path);
        }
        return FALSE;
    }

    len = strlen (file);
    doc->content_base = g_strdup ("");
    for (i=0; i<len; i++) {
        if (file[i] == '/') {
            g_free (doc->content_base);
            doc->content_base = g_strndup (file, i+1);
            break;
        }
    }

    gepub_doc_fill_resources (doc);
    gepub_doc_fill_spine (doc);

    g_free (file);

    return TRUE;
}

static void
gepub_doc_initable_iface_init (GInitableIface *iface)
{
    iface->init = gepub_doc_initable_init;
}

/**
 * gepub_doc_new:
 * @path: the epub doc path
 * @error: (nullable): Error
 *
 * Returns: (transfer full): the new GepubDoc created
 */
GepubDoc *
gepub_doc_new (const gchar *path, GError **error)
{
    return g_initable_new (GEPUB_TYPE_DOC,
                           NULL, error,
                           "path", path,
                           NULL);
}

static void
gepub_doc_fill_resources (GepubDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *mnode = NULL;
    xmlNode *item = NULL;
    gchar *id, *tmpuri, *uri;
    GepubResource *res;
    const char *data;
    gsize size;

    data = g_bytes_get_data (doc->content, &size);
    xdoc = xmlRecoverMemory (data, size);
    root_element = xmlDocGetRootElement (xdoc);
    mnode = gepub_utils_get_element_by_tag (root_element, "manifest");

    item = mnode->children;
    while (item) {
        if (item->type != XML_ELEMENT_NODE ) {
            item = item->next;
            continue;
        }

        id = gepub_utils_get_prop (item, "id");
        tmpuri = gepub_utils_get_prop (item, "href");
        uri = g_strdup_printf ("%s%s", doc->content_base, tmpuri);
        g_free (tmpuri);

        res = g_malloc (sizeof (GepubResource));
        res->mime = gepub_utils_get_prop (item, "media-type");
        res->uri = uri;
        g_hash_table_insert (doc->resources, id, res);
        item = item->next;
    }

    xmlFreeDoc (xdoc);
}

static void
gepub_doc_fill_spine (GepubDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *snode = NULL;
    xmlNode *item = NULL;
    gchar *id;
    const char *data;
    gsize size;
    GList *spine = NULL;
    gchar *toc = NULL;

    data = g_bytes_get_data (doc->content, &size);
    xdoc = xmlRecoverMemory (data, size);
    root_element = xmlDocGetRootElement (xdoc);
    snode = gepub_utils_get_element_by_tag (root_element, "spine");

    toc = gepub_utils_get_prop (snode, "toc");
    if (toc) {
        gepub_doc_fill_toc (doc, toc);
        g_free (toc);
    }

    item = snode->children;
    while (item) {
        if (item->type != XML_ELEMENT_NODE ) {
            item = item->next;
            continue;
        }

        id = gepub_utils_get_prop (item, "idref");

        spine = g_list_prepend (spine, id);
        item = item->next;
    }

    doc->spine = g_list_reverse (spine);
    doc->chapter = doc->spine;

    xmlFreeDoc (xdoc);
}

static gint
navpoint_compare (GepubNavPoint *a, GepubNavPoint *b)
{
    return a->playorder - b->playorder;
}

static void
gepub_doc_fill_toc (GepubDoc *doc, gchar *toc_id)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *mapnode = NULL;
    xmlNode *item = NULL;
    const char *data;
    gsize size;
    GList *toc = NULL;
    GBytes *toc_data = NULL;

    doc->toc = toc;

    toc_data = gepub_doc_get_resource_by_id (doc, toc_id);
    if (!toc_data) {
        return;
    }

    data = g_bytes_get_data (toc_data, &size);
    xdoc = xmlRecoverMemory (data, size);
    root_element = xmlDocGetRootElement (xdoc);
    mapnode = gepub_utils_get_element_by_tag (root_element, "navMap");

    // TODO: get docTitle
    // TODO: parse metadata (dtb:totalPageCount, dtb:depth, dtb:maxPageNumber)

    item = mapnode->children;
    while (item) {
        GepubNavPoint *navpoint = NULL;
        gchar *order;
        xmlNode *navchilds = NULL;

        if (item->type != XML_ELEMENT_NODE ||
            g_strcmp0 ((const gchar *)item->name, "navPoint")) {
            item = item->next;
            continue;
        }

        navpoint = g_malloc0 (sizeof (GepubNavPoint));

        order = gepub_utils_get_prop (item, "playOrder");
        if (order) {
            g_ascii_string_to_unsigned (order, 10, 0, INT_MAX,
                                        &navpoint->playorder, NULL);
            g_free (order);
        }

        // parsing navPoint->navLabel->text and navPoint->content
        navchilds = item->children;
        while (navchilds) {
            if (item->type != XML_ELEMENT_NODE) {
                navchilds = navchilds->next;
                continue;
            }

            if (!g_strcmp0 ((const gchar *)navchilds->name, "content")) {
                gchar **split;
                gchar *tmpuri;
                tmpuri = gepub_utils_get_prop (navchilds, "src");
                // removing # params. Maybe we should store the # params in the
                // navpoint to use in the future if the doc references to a position
                // inside the chapter
                split = g_strsplit (tmpuri, "#", -1);

                // adding the base path
                navpoint->content = g_strdup_printf ("%s%s", doc->content_base, split[0]);

                g_strfreev (split);
                g_free (tmpuri);
            }

            if (!g_strcmp0 ((const gchar *)navchilds->name, "navLabel")) {
                xmlNode *text = gepub_utils_get_element_by_tag (navchilds, "text");
                if (text->children && text->children->type == XML_TEXT_NODE) {
                  navpoint->label = g_strdup ((gchar *)text->children->content);
                }
            }

            navchilds = navchilds->next;
        }

        toc = g_list_prepend (toc, navpoint);
        item = item->next;
    }

    doc->toc = g_list_sort (toc, (GCompareFunc) navpoint_compare);

    xmlFreeDoc (xdoc);
    g_bytes_unref (toc_data);
}

/**
 * gepub_doc_get_content:
 * @doc: a #GepubDoc
 *
 * Returns: (transfer none): the document content
 */
GBytes *
gepub_doc_get_content (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);

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
gepub_doc_get_metadata (GepubDoc *doc, const gchar *mdata)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *mnode = NULL;
    xmlNode *mdata_node = NULL;
    gchar *ret;
    xmlChar *text;
    const char *data;
    gsize size;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (mdata != NULL, NULL);

    data = g_bytes_get_data (doc->content, &size);
    xdoc = xmlRecoverMemory (data, size);
    root_element = xmlDocGetRootElement (xdoc);
    mnode = gepub_utils_get_element_by_tag (root_element, "metadata");
    mdata_node = gepub_utils_get_element_by_tag (mnode, mdata);

    text = xmlNodeGetContent (mdata_node);
    ret = g_strdup ((const char *) text);
    xmlFree (text);

    xmlFreeDoc (xdoc);

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
    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);

    return doc->resources;
}

/**
 * gepub_doc_get_resource_by_id:
 * @doc: a #GepubDoc
 * @id: the resource id
 *
 * Returns: (transfer full): the resource content
 */
GBytes *
gepub_doc_get_resource_by_id (GepubDoc *doc, const gchar *id)
{
    GepubResource *gres;
    g_autofree gchar *unescaped = NULL;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    gres = g_hash_table_lookup (doc->resources, id);
    if (!gres) {
        // not found
        return NULL;
    }

    unescaped = g_uri_unescape_string (gres->uri, NULL);
    return gepub_archive_read_entry (doc->archive, unescaped);
}

/**
 * gepub_doc_get_resource:
 * @doc: a #GepubDoc
 * @path: the resource path
 *
 * Returns: (transfer full): the resource content
 */
GBytes *
gepub_doc_get_resource (GepubDoc *doc, const gchar *path)
{
    g_autofree gchar *unescaped = NULL;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (path != NULL, NULL);

    // we need to decode the path because we can get URL encoded paths
    // like "some%20text.jpg"
    unescaped = g_uri_unescape_string (path, NULL);

    return gepub_archive_read_entry (doc->archive, unescaped);
}

/**
 * gepub_doc_get_resource_mime_by_id:
 * @doc: a #GepubDoc
 * @id: the resource id
 *
 * Returns: (transfer full): the resource content
 */
gchar *
gepub_doc_get_resource_mime_by_id (GepubDoc *doc, const gchar *id)
{
    GepubResource *gres;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    gres = g_hash_table_lookup (doc->resources, id);
    if (!gres) {
        // not found
        return NULL;
    }

    return g_strdup (gres->mime);
}

/**
 * gepub_doc_get_resource_mime:
 * @doc: a #GepubDoc
 * @path: the resource path
 *
 * Returns: (transfer full): the resource mime
 */
gchar *
gepub_doc_get_resource_mime (GepubDoc *doc, const gchar *path)
{
    GepubResource *gres;
    GList *keys;
    const gchar *_path;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (path != NULL, NULL);

    if (path[0] == '/') {
        _path = path + 1;
    }
    else {
        _path = path;
    }

    keys = g_hash_table_get_keys (doc->resources);

    while (keys) {
        gres = ((GepubResource*)g_hash_table_lookup (doc->resources, keys->data));
        if (!strcmp (gres->uri, _path))
            break;
        keys = keys->next;
    }

    if (keys)
        return g_strdup (gres->mime);
    else
        return NULL;
}

/**
 * gepub_doc_get_current_mime:
 * @doc: a #GepubDoc
 *
 * Returns: (transfer full): the current resource mime
 */
gchar *
gepub_doc_get_current_mime (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (doc->chapter != NULL, NULL);

    return gepub_doc_get_resource_mime_by_id (doc, doc->chapter->data);
}

/**
 * gepub_doc_get_current:
 * @doc: a #GepubDoc
 *
 * Returns: (transfer full): the current chapter data
 */
GBytes *
gepub_doc_get_current (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (doc->chapter != NULL, NULL);

    return gepub_doc_get_resource_by_id (doc, doc->chapter->data);
}

/**
 * gepub_doc_get_current_with_epub_uris:
 * @doc: a #GepubDoc
 *
 * Returns: (transfer full): the current chapter
 * data, with resource uris renamed so they have the epub:/// prefix and all
 * are relative to the root file
 */
GBytes *
gepub_doc_get_current_with_epub_uris (GepubDoc *doc)
{
    GBytes *content, *replaced;
    gchar *path, *base;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);

    content = gepub_doc_get_current (doc);
    path = gepub_doc_get_current_path (doc);
    // getting the basepath of the current xhtml loaded
    base = g_path_get_dirname (path);

    replaced = gepub_utils_replace_resources (content, base);

    g_free (path);
    g_bytes_unref (content);

    return replaced;
}

/**
 * gepub_doc_get_text:
 * @doc: a #GepubDoc
 *
 * Returns: (element-type Gepub.TextChunk) (transfer full): the list of text in the current chapter.
 */
GList *
gepub_doc_get_text (GepubDoc *doc)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    GBytes *current;
    const gchar *data;
    gsize size;

    GList *texts = NULL;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);

    current = gepub_doc_get_current (doc);
    if (!current) {
        return NULL;
    }
    data = g_bytes_get_data (current, &size);
    xdoc = htmlReadMemory (data, size, "", NULL, HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    root_element = xmlDocGetRootElement (xdoc);
    texts = gepub_utils_get_text_elements (root_element);

    g_bytes_unref (current);
    xmlFreeDoc (xdoc);

    return texts;
}

/**
 * gepub_doc_get_text_by_id:
 * @doc: a #GepubDoc
 * @id: the resource id
 *
 * Returns: (element-type Gepub.TextChunk) (transfer full): the list of text in the current chapter.
 */
GList *
gepub_doc_get_text_by_id (GepubDoc *doc, const gchar *id)
{
    xmlDoc *xdoc = NULL;
    xmlNode *root_element = NULL;
    gsize size;
    const gchar *res;
    GBytes *contents;

    GList *texts = NULL;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    contents = gepub_doc_get_resource_by_id (doc, id);
    if (!contents) {
        return NULL;
    }

    res = g_bytes_get_data (contents, &size);
    xdoc = htmlReadMemory (res, size, "", NULL, HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    root_element = xmlDocGetRootElement (xdoc);
    texts = gepub_utils_get_text_elements (root_element);

    g_bytes_unref (contents);
    xmlFreeDoc (xdoc);

    return texts;
}

static gboolean
gepub_doc_set_chapter_internal (GepubDoc *doc,
                                GList    *chapter)
{
    if (!chapter || doc->chapter == chapter)
        return FALSE;

    doc->chapter = chapter;
    g_object_notify_by_pspec (G_OBJECT (doc), properties[PROP_CHAPTER]);

    return TRUE;
}

/**
 * gepub_doc_go_next:
 * @doc: a #GepubDoc
 *
 * Returns: TRUE on success, FALSE if there's no next chapter
 */
gboolean
gepub_doc_go_next (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), FALSE);
    g_return_val_if_fail (doc->chapter != NULL, FALSE);

    return gepub_doc_set_chapter_internal (doc, doc->chapter->next);
}

/**
 * gepub_doc_go_prev:
 * @doc: a #GepubDoc
 *
 * Returns: TRUE on success, FALSE if there's no previous chapter
 */
gboolean
gepub_doc_go_prev (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), FALSE);
    g_return_val_if_fail (doc->chapter != NULL, FALSE);

    return gepub_doc_set_chapter_internal (doc, doc->chapter->prev);
}

/**
 * gepub_doc_get_n_chapters:
 * @doc: a #GepubDoc
 *
 * Returns: the number of chapters in the document
 */
int
gepub_doc_get_n_chapters (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), 0);

    return g_list_length (doc->spine);
}

/**
 * gepub_doc_get_chapter:
 * @doc: a #GepubDoc
 *
 * Returns: the current chapter index, starting from 0
 */
int
gepub_doc_get_chapter (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), 0);
    g_return_val_if_fail (doc->spine != NULL, 0);
    g_return_val_if_fail (doc->chapter != NULL, 0);

    return g_list_position (doc->spine, doc->chapter);
}

/**
 * gepub_doc_set_chapter:
 * @doc: a #GepubDoc
 * @index: the index of the new chapter
 *
 * Sets the document current chapter to @index.
 */
void
gepub_doc_set_chapter (GepubDoc *doc,
                    gint      index)
{
    GList *chapter;

    g_return_if_fail (GEPUB_IS_DOC (doc));

    g_return_if_fail (index >= 0 && index <= gepub_doc_get_n_chapters (doc));

    chapter = g_list_nth (doc->spine, index);
    gepub_doc_set_chapter_internal (doc, chapter);
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
    const char *data;
    gsize size;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (doc->content != NULL, NULL);

    data = g_bytes_get_data (doc->content, &size);
    xdoc = xmlRecoverMemory (data, size);
    root_element = xmlDocGetRootElement (xdoc);
    mnode = gepub_utils_get_element_by_attr (root_element, "name", "cover");
    ret = gepub_utils_get_prop (mnode, "content");

    xmlFreeDoc (xdoc);

    return ret;
}

/**
 * gepub_doc_get_resource_path:
 * @doc: a #GepubDoc
 * @id: the resource id
 *
 * Returns: (transfer full): the resource path
 */
gchar *
gepub_doc_get_resource_path (GepubDoc *doc, const gchar *id)
{
    GepubResource *gres;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (id != NULL, NULL);

    gres = g_hash_table_lookup (doc->resources, id);
    if (!gres) {
        // not found
        return NULL;
    }

    return g_strdup (gres->uri);
}

/**
 * gepub_doc_get_current_path:
 * @doc: a #GepubDoc
 *
 * Returns: (transfer full): the current resource path
 */
gchar *
gepub_doc_get_current_path (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (doc->chapter != NULL, NULL);

    return gepub_doc_get_resource_path (doc, doc->chapter->data);
}

/**
 * gepub_doc_get_current_id:
 * @doc: a #GepubDoc
 *

 * Returns: (transfer none): the current resource id
 */
const gchar *
gepub_doc_get_current_id (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    g_return_val_if_fail (doc->chapter != NULL, NULL);

    return doc->chapter->data;
}

/**
 * gepub_doc_get_toc:
 * @doc: a #GepubDoc
 *

 * Returns: (element-type Gepub.NavPoint) (transfer none): the navigation list in order
 */
GList *
gepub_doc_get_toc (GepubDoc *doc)
{
    g_return_val_if_fail (GEPUB_IS_DOC (doc), NULL);
    return doc->toc;
}

/**
 * gepub_doc_resource_uri_to_chapter:
 * @doc: a #GepubDoc
 * @uri: The resource path
 *
 * This method tries to find the resource by path in the doc spine and
 * will return the index in that list. If the resourse isn't there this method
 * will return -1.

 * Returns: the chapter index to use with gepub_doc_set_chapter or -1 if the
 * resource isn't found
 */
gint
gepub_doc_resource_uri_to_chapter (GepubDoc *doc,
                                   const gchar *uri)
{
    GHashTableIter iter;
    gchar *key;
    GepubResource *res;
    gchar *id = NULL;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), -1);
    g_return_val_if_fail (doc->spine != NULL, -1);

    g_hash_table_iter_init (&iter, doc->resources);
    while (g_hash_table_iter_next (&iter, (gpointer *)&key, (gpointer *)&res)) {
        if (!g_strcmp0 (res->uri, uri)) {
            id = key;
            break;
        }
    }

    if (!id) {
        return -1;
    }

    return gepub_doc_resource_id_to_chapter (doc, id);
}

/**
 * gepub_doc_resource_id_to_chapter:
 * @doc: a #GepubDoc
 * @id: The resource id
 *
 * This method tries to find the resource by id in the doc spine and
 * will return the index in that list. If the resourse isn't there this method
 * will return -1.

 * Returns: the chapter index to use with gepub_doc_set_chapter or -1 if the
 * resource isn't found
 */
gint
gepub_doc_resource_id_to_chapter (GepubDoc *doc,
                                  const gchar *id)
{
    GList *spine;
    gint chapter = 0;

    g_return_val_if_fail (GEPUB_IS_DOC (doc), -1);
    g_return_val_if_fail (doc->spine != NULL, -1);

    spine = g_list_first (doc->spine);
    while (spine && spine->data) {
        if (!g_strcmp0 (spine->data, id)) {
            return chapter;
        }
        chapter++;
        spine = spine->next;
    }

    return -1;
}

