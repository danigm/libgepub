/*
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

#include <libsoup/soup.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <stdarg.h>
#include <string.h>

#include "gepub-utils.h"
#include "gepub-text-chunk.h"


/* Replaces the attr value with epub:/// prefix for the tagname. This
 * function also makes the resource absolute based on the epub root
 */
static void
set_epub_uri (xmlNode *node,
              const gchar *path,
              const gchar *tagname,
              const gchar *attr,
              const gchar *ns)
{
    xmlNode *cur_node = NULL;
    xmlChar *text = NULL;

    gchar *attrname = NULL;

    SoupURI *baseURI;
    gchar *basepath = g_strdup_printf ("epub:///%s/", path);

    baseURI = soup_uri_new (basepath);
    g_free (basepath);

    if (ns) {
        attrname = g_strdup_printf ("%s:%s", ns, attr);
    } else {
        attrname = g_strdup (attr);
    }

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE ) {
            text = xmlGetProp (cur_node, BAD_CAST (attr));

            if (!strcmp ((const char *) cur_node->name, tagname) && text) {
                SoupURI *uri = soup_uri_new_with_base (baseURI, (const char *) text);
                gchar *value = soup_uri_to_string (uri, FALSE);

                xmlSetProp (cur_node, BAD_CAST (attrname), BAD_CAST (value));

                soup_uri_free (uri);
                g_free (value);
            }
            if (text) {
                xmlFree (text);
                text = NULL;
            }
        }

        if (cur_node->children)
            set_epub_uri (cur_node->children, path, tagname, attr, ns);
    }

    g_free (attrname);

    soup_uri_free (baseURI);
}

static gboolean
gepub_utils_has_parent_tag (xmlNode *node, const char *name, ...)
{
    va_list ap;

    xmlNode *cur_node = NULL;
    GList *tags = NULL;
    GList *l = NULL;
    const char *name2 = NULL;
    gboolean ret = FALSE;

    va_start (ap, name);

    for (name2 = name; name2 != NULL; name2 = va_arg(ap, gchar*)) {
        tags = g_list_append (tags, g_ascii_strdown (name2, -1));
    }

    for (cur_node = node; cur_node; cur_node = cur_node->parent) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            for (l = tags; l; l = l->next) {
                gchar *nodetag = g_ascii_strdown ((char *) cur_node->name, -1);
                name2 = l->data;

                if (!strcmp (nodetag, name2))
                    ret = TRUE;

                g_free (nodetag);

                if (ret == TRUE)
                  goto out;
            }
        }
    }

    va_end (ap);

out:
    g_list_free_full (tags, g_free);

    return ret;
}

/**
 * gepub_utils_get_element_by_tag: (skip):
 * @node: an #xmlNode
 * @name: the tag name
 *
 * Returns: the tag matching @name.
 */
xmlNode *
gepub_utils_get_element_by_tag (xmlNode *node, const gchar *name)
{
    xmlNode *cur_node = NULL;
    xmlNode *ret = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE ) {
            if (!strcmp ((const char *) cur_node->name, name))
                return cur_node;
        }

        ret = gepub_utils_get_element_by_tag (cur_node->children, name);
        if (ret)
            return ret;
    }
    return ret;
}

/**
 * gepub_utils_get_element_by_attr: (skip):
 * @node: an #xmlNode
 * @attr: the attribute
 * @value: the value
 *
 * Returns: the element matching @attr and @value.
 */
xmlNode *
gepub_utils_get_element_by_attr (xmlNode *node, const gchar *attr, const gchar *value)
{
    xmlNode *cur_node = NULL;
    xmlNode *ret = NULL;
    xmlChar *text = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE ) {
            text = xmlGetProp (cur_node, BAD_CAST (attr));
            if (text && !strcmp ((const char *) text, value)) {
                xmlFree (text);
                return cur_node;
            }
            if (text) {
                xmlFree (text);
                text = NULL;
            }
        }

        if (cur_node->children)
            ret = gepub_utils_get_element_by_attr (cur_node->children, attr, value);

        if (ret)
            return ret;
    }
    return ret;
}

/**
 * gepub_utils_get_text_elements:
 * @node: an #xmlNode
 *
 * Returns: (element-type Gepub.TextChunk) (transfer full): the text elements
 *  of @node.
 */
GList *
gepub_utils_get_text_elements (xmlNode *node)
{
    GList *text_list = NULL;
    GList *sub_texts = NULL;

    xmlNode *cur_node = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_TEXT_NODE) {
            GepubTextChunk *text_chunk = NULL;

            if (gepub_utils_has_parent_tag (cur_node, "b", "strong", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextBold, (char *) cur_node->content);
            } else if (gepub_utils_has_parent_tag (cur_node, "i", "em", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextItalic, (char *) cur_node->content);
            } else if (gepub_utils_has_parent_tag (cur_node, "h1", "h2", "h3", "h4", "h5", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextHeader, (char *) cur_node->content);
            } else if (gepub_utils_has_parent_tag (cur_node, "p", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextNormal, (char *) cur_node->content);
            }

            if (text_chunk)
                text_list = g_list_append (text_list, text_chunk);
        }

        if (cur_node->type == XML_ELEMENT_NODE) {
            GepubTextChunk *text_chunk = NULL;
            gchar *nodetag = g_ascii_strup ((const char *) cur_node->name, -1);
            if (text_list && (!strcmp (nodetag, "P") || !strcmp (nodetag, "BR"))) {
                gchar *old_text;
                text_chunk = (GepubTextChunk*)(g_list_last (text_list)->data);
                old_text = text_chunk->text;
                text_chunk->text = g_strdup_printf ("%s\n", old_text);
                g_free (old_text);
            }
            g_free (nodetag);
        }

        // TODO add images to this list of objects

        sub_texts = gepub_utils_get_text_elements (cur_node->children);
        if (sub_texts)
            text_list = g_list_concat (text_list, sub_texts);
    }

    return text_list;
}

/**
 * gepub_utils_replace_resources:
 * @content: a #GBytes containing the XML data
 * @path: The path to replace
 *
 * Replacing epub media paths, for css, image and svg files, to be
 * able to provide these files to webkit from the epub file.
 *
 * Returns: a new #GBytes containing the updated XML data
 */
GBytes *
gepub_utils_replace_resources (GBytes *content, const gchar *path)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    guchar *buffer;
    const gchar *data;
    gsize bufsize;

    data = g_bytes_get_data (content, &bufsize);
    doc = xmlReadMemory (data, bufsize, "", NULL, XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    root_element = xmlDocGetRootElement (doc);

    // replacing css resources
    set_epub_uri (root_element, path, "link", "href", NULL);
    // replacing images resources
    set_epub_uri (root_element, path, "img", "src", NULL);
    // replacing svg images resources
    set_epub_uri (root_element, path, "image", "href", "xlink");
    // replacing crosslinks
    set_epub_uri (root_element, path, "a", "href", NULL);

    xmlDocDumpFormatMemory (doc, (xmlChar**)&buffer, (int*)&bufsize, 1);
    xmlFreeDoc (doc);

    return g_bytes_new_take (buffer, bufsize);
}


/**
 * gepub_utils_get_prop:
 * @node: an #xmlNode
 * @prop: a property
 *
 * Returns: a string with the property contained in @prop
 */
gchar *
gepub_utils_get_prop (xmlNode *node, const gchar *prop)
{
    xmlChar *p = NULL;
    gchar *ret = NULL;

    p = xmlGetProp (node, (const xmlChar *) prop);
    if (p) {
        ret = g_strdup ((char *) p);
        xmlFree (p);
    }

    return ret;
}
