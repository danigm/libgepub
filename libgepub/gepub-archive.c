/* GepubArchive
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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <archive.h>
#include <archive_entry.h>

#include "gepub-archive.h"
#include "gepub-utils.h"

#define BUFZISE 1024

struct _GepubArchive {
    GObject parent;

    struct archive *archive;
    gchar *path;
};

struct _GepubArchiveClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (GepubArchive, gepub_archive, G_TYPE_OBJECT)

static gboolean
gepub_archive_open (GepubArchive *archive)
{
    int r;

    archive->archive = archive_read_new ();
    archive_read_support_format_zip (archive->archive);

    r = archive_read_open_filename (archive->archive, archive->path, 10240);

    if (r != ARCHIVE_OK) {
        return FALSE;
    }

    return TRUE;
}

static void
gepub_archive_close (GepubArchive *archive)
{
    if (!archive->archive)
        return;

    archive_read_free (archive->archive);
    archive->archive = NULL;
}

static void
gepub_archive_finalize (GObject *object)
{
    GepubArchive *archive = GEPUB_ARCHIVE (object);

    g_clear_pointer (&archive->path, g_free);

    gepub_archive_close (archive);

    G_OBJECT_CLASS (gepub_archive_parent_class)->finalize (object);
}

static void
gepub_archive_init (GepubArchive *archive)
{
}

static void
gepub_archive_class_init (GepubArchiveClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = gepub_archive_finalize;
}

GepubArchive *
gepub_archive_new (const gchar *path)
{
    GepubArchive *archive;

    archive = GEPUB_ARCHIVE (g_object_new (GEPUB_TYPE_ARCHIVE, NULL));
    archive->path = g_strdup (path);
    archive->archive = NULL;

    return archive;
}

/**
 * gepub_archive_list_files:
 * @archive: a #GepubArchive
 *
 * Returns: (element-type utf8) (transfer full): list of files in the archive
 */
GList *
gepub_archive_list_files (GepubArchive *archive)
{
    struct archive_entry *entry;
    GList *file_list = NULL;

    if (!gepub_archive_open (archive))
        return NULL;
    while (archive_read_next_header (archive->archive, &entry) == ARCHIVE_OK) {
        file_list = g_list_prepend (file_list, g_strdup (archive_entry_pathname (entry)));
        archive_read_data_skip (archive->archive);
    }
    gepub_archive_close (archive);

    return file_list;
}

GBytes *
gepub_archive_read_entry (GepubArchive *archive,
                          const gchar *path)
{
    struct archive_entry *entry;
    guchar *buffer;
    gint size;
    const gchar *_path;

    if (path[0] == '/') {
        _path = path + 1;
    }
    else {
        _path = path;
    }

    if (!gepub_archive_open (archive))
        return NULL;

    while (archive_read_next_header (archive->archive, &entry) == ARCHIVE_OK) {
        if (g_ascii_strcasecmp (_path, archive_entry_pathname (entry)) == 0)
            break;
        archive_read_data_skip (archive->archive);
    }

    size = archive_entry_size (entry);
    buffer = g_malloc0 (size);
    archive_read_data (archive->archive, buffer, size);

    gepub_archive_close (archive);
    return g_bytes_new_take (buffer, size);
}

gchar *
gepub_archive_get_root_file (GepubArchive *archive)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *root_node = NULL;
    GBytes *bytes;
    const gchar *buffer;
    gsize bufsize;
    gchar *root_file = NULL;

    // root file is in META-INF/container.xml
    bytes = gepub_archive_read_entry (archive, "META-INF/container.xml");
    if (!bytes)
        return NULL;

    buffer = g_bytes_get_data (bytes, &bufsize);
    doc = xmlRecoverMemory (buffer, bufsize);
    root_element = xmlDocGetRootElement (doc);
    root_node = gepub_utils_get_element_by_tag (root_element, "rootfile");
    root_file = gepub_utils_get_prop (root_node, "full-path");

    xmlFreeDoc (doc);
    g_bytes_unref (bytes);

    return root_file;
}
