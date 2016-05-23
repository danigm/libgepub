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
    archive->archive = archive_read_new ();
    archive_read_support_format_zip (archive->archive);
    int r;

    r = archive_read_open_filename (archive->archive, archive->path, 10240);

    if (r != ARCHIVE_OK) {
        archive_read_free (archive->archive);
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

    if (archive->path) {
        g_free (archive->path);
        archive->path = NULL;
    }

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

gboolean
gepub_archive_read_entry (GepubArchive *archive,
                          const gchar *path,
                          guchar **buffer,
                          gsize *bufsize)
{
    struct archive_entry *entry;
    gint size;

    if (!gepub_archive_open (archive))
        return FALSE;

    while (archive_read_next_header (archive->archive, &entry) == ARCHIVE_OK) {
        if (g_ascii_strcasecmp (path, archive_entry_pathname (entry)) == 0)
            break;
        archive_read_data_skip (archive->archive);
    }

    *bufsize = archive_entry_size (entry) + 1;
    size = (*bufsize) - 1;
    *buffer = g_malloc (*bufsize);
    archive_read_data (archive->archive, *buffer, size);
    (*buffer)[size] = '\0';

    gepub_archive_close (archive);
    return TRUE;
}

gchar *
gepub_archive_get_root_file (GepubArchive *archive)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *root_node = NULL;
    guchar *buffer;
    gsize bufsize;
    gchar *root_file = NULL;

    LIBXML_TEST_VERSION

    // root file is in META-INF/container.xml
    if (!gepub_archive_read_entry (archive, "META-INF/container.xml", &buffer, &bufsize))
        return NULL;

    doc = xmlRecoverDoc (buffer);
    root_element = xmlDocGetRootElement (doc);
    root_node = gepub_utils_get_element_by_tag (root_element, "rootfile");
    root_file = xmlGetProp (root_node, "full-path");

    xmlFreeDoc (doc);
    g_free (buffer);

    return root_file;
}
