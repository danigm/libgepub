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

#ifndef __GEPUB_UTILS_H__
#define __GEPUB_UTILS_H__

#include <glib.h>
#include <libxml/tree.h>

xmlNode * gepub_utils_get_element_by_tag  (xmlNode *node, const gchar *name);
xmlNode * gepub_utils_get_element_by_attr (xmlNode *node, const gchar *attr, const gchar *value);
GList *   gepub_utils_get_text_elements   (xmlNode *node);
GBytes *  gepub_utils_replace_resources   (GBytes *content, const gchar *path);
gchar *   gepub_utils_get_prop            (xmlNode *node, const gchar *prop);

#endif
