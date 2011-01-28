/* GEPUBArchive
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

#ifndef __GEPUB_ARCHIVE_H__
#define __GEPUB_ARCHIVE_H__

#include <glib-object.h>
#include <gio/gio.h>
#include <archive.h>

G_BEGIN_DECLS

#define GEPUB_TYPE_ARCHIVE           (gepub_archive_get_type ())
#define GEPUB_ARCHIVE(obj)           (G_TYPE_CHECK_INSTANCE_CAST (obj, GEPUB_TYPE_ARCHIVE, GEPUBArchive))
#define GEPUB_ARCHIVE_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST (cls, GEPUB_TYPE_ARCHIVE, GEPUBArchiveClass))
#define GEPUB_IS_ARCHIVE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE (obj, GEPUB_TYPE_ARCHIVE))
#define GEPUB_IS_ARCHIVE_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE (obj, GEPUB_TYPE_ARCHIVE))
#define GEPUB_ARCHIVE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GEPUB_TYPE_ARCHIVE, GEPUBArchiveClass))

typedef struct _GEPUBArchive      GEPUBArchive;
typedef struct _GEPUBArchiveClass GEPUBArchiveClass;

GType             gepub_archive_get_type       (void) G_GNUC_CONST;

GEPUBArchive     *gepub_archive_new            (const gchar  *path);
void              gepub_archive_list_files     (GEPUBArchive *archive);

G_END_DECLS

#endif /* __GEPUB_ARCHIVE_H__ */
