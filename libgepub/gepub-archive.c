/* GEPUBArchive
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

#include "gepub-archive.h"

struct _GEPUBArchive {
    GObject parent;

    struct archive *archive;
    gchar *path;
};

struct _GEPUBArchiveClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (GEPUBArchive, gepub_archive, G_TYPE_OBJECT)

static void
gepub_archive_finalize (GObject *object)
{
    GEPUBArchive *archive = GEPUB_ARCHIVE (object);

    if (archive->path) {
        g_free (archive->path);
        archive->path = NULL;
    }
    if (archive->archive) {
        archive_read_finish (archive->archive);
    }

    G_OBJECT_CLASS (gepub_archive_parent_class)->finalize (object);
}

static void
gepub_archive_init (GEPUBArchive *archive)
{
}

static void
gepub_archive_class_init (GEPUBArchiveClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = gepub_archive_finalize;
}

GEPUBArchive *
gepub_archive_new (const gchar *path)
{
    GEPUBArchive *archive;
    struct archive *a;
    int r;

    a = archive_read_new ();
    archive_read_support_format_zip (a);
    r = archive_read_open_filename (a, path, 10240);

    if (r != ARCHIVE_OK) {
        archive_read_finish (a);
        return NULL;
    }

    archive = GEPUB_ARCHIVE (g_object_new (GEPUB_TYPE_ARCHIVE, NULL));
    archive->path = g_strdup (path);
    archive->archive = a;

    return archive;
}

void
gepub_archive_list_files (GEPUBArchive *archive)
{
    struct archive_entry *entry;

    while (archive_read_next_header (archive->archive, &entry) == ARCHIVE_OK) {
        printf ("%s\n",archive_entry_pathname (entry));
        archive_read_data_skip (archive->archive);
    }
}
