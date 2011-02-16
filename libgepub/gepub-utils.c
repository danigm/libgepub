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

#include "gepub-utils.h"

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
