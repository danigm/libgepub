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

#include <libxml/tree.h>
#include <stdarg.h>
#include <string.h>

#include "gepub-utils.h"
#include "gepub-text-chunk.h"

gboolean
gepub_utils_has_parent_tag (xmlNode *node, gchar *name, ...)
{
    va_list ap;
    int i;

    xmlNode *cur_node = NULL;
    GList *tags = NULL;
    GList *l = NULL;
    gchar *name2 = NULL;

    va_start (ap, name);

    for (name2 = name; name2 != NULL; name2 = va_arg(ap, gchar*)) {
        tags = g_list_append (tags, name2);
    }

    for (cur_node = node; cur_node; cur_node = cur_node->parent) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            for (l = tags; l; l = l->next) {
                gchar *nodetag = g_ascii_strup (cur_node->name, strlen (cur_node->name));
                name2 = g_ascii_strup (l->data, strlen (l->data));

                if (!strcmp (nodetag, name2))
                    return TRUE;
            }
        }
    }

    va_end (ap);

    return FALSE;
}

xmlNode *
gepub_utils_get_element_by_tag (xmlNode *node, gchar *name)
{
    xmlNode *cur_node = NULL;
    xmlNode *ret = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE ) {
            if (!strcmp (cur_node->name, name))
                return cur_node;
        }

        ret = gepub_utils_get_element_by_tag (cur_node->children, name);
        if (ret)
            return ret;
    }
    return ret;
}

GList *
gepub_utils_get_text_elements (xmlNode *node)
{
    GList *text_list = NULL;
    GList *sub_texts = NULL;

    xmlNode *cur_node = NULL;
    xmlNode *ret = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_TEXT_NODE) {
            GepubTextChunk *text_chunk = NULL;

            if (gepub_utils_has_parent_tag (cur_node, "b", "strong", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextBold, cur_node->content);
            } else if (gepub_utils_has_parent_tag (cur_node, "i", "em", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextItalic, cur_node->content);
            } else if (gepub_utils_has_parent_tag (cur_node, "h1", "h2", "h3", "h4", "h5", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextHeader, cur_node->content);
            } else if (gepub_utils_has_parent_tag (cur_node, "p", NULL)) {
                text_chunk = gepub_text_chunk_new (GEPUBTextNormal, cur_node->content);
            }

            if (text_chunk)
                text_list = g_list_append (text_list, text_chunk);
        }

        if (cur_node->type == XML_ELEMENT_NODE) {
            GepubTextChunk *text_chunk = NULL;
            gchar *nodetag = g_ascii_strup (cur_node->name, strlen (cur_node->name));
            if (text_list && (!strcmp (nodetag, "P") || !strcmp (nodetag, "BR"))) {
                gchar *old_text;
                text_chunk = (GepubTextChunk*)(g_list_last (text_list)->data);
                old_text = text_chunk->text;
                text_chunk->text = g_strdup_printf ("%s\n", old_text);
                g_free (old_text);
            }
        }

        // TODO add images to this list of objects

        sub_texts = gepub_utils_get_text_elements (cur_node->children);
        if (sub_texts)
            text_list = g_list_concat (text_list, sub_texts);
    }

    return text_list;
}
