#include <stdio.h>
#include <gtk/gtk.h>
#include <libgepub/gepub.h>
#include <webkit/webkit.h>

#define TEST(f,arg) printf ("\n### TESTING " #f " ###\n\n"); f (arg); printf ("\n\n");

void
test_open (const char *path)
{
    GEPUBArchive *a;
    GList *list_files;
    gint i;
    gint size;

    a = gepub_archive_new (path);
    list_files = gepub_archive_list_files (a);
    size = g_list_length (list_files);
    printf ("%d\n", size);
    for (i = 0; i < size; i++) {
        printf ("file: %s\n", (char *)g_list_nth_data (list_files, i));
        g_free (g_list_nth_data (list_files, i));
    }

    g_list_free (list_files);

    g_object_unref (a);
}

void
test_read (const char *path)
{
    GEPUBArchive *a;
    GList *list_files;
    gint i;
    gint size;
    guchar *buffer;
    gchar *file;
    int bufsize;

    a = gepub_archive_new (path);
    list_files = gepub_archive_list_files (a);
    size = g_list_length (list_files);
    if (!size)
        return;

    file = g_list_nth_data (list_files, 0);
    gepub_archive_read_entry (a, file, &buffer, &bufsize);
    if (bufsize)
        g_printf ("doc:%s\n----\n%s\n-----\n", file, buffer);

    g_free (buffer);

    g_list_foreach (list_files, (GFunc)g_free, NULL);
    g_list_free (list_files);

    g_object_unref (a);
}

void
test_root_file (const char *path)
{
    GEPUBArchive *a;
    gchar *root_file = NULL;

    a = gepub_archive_new (path);

    root_file = gepub_archive_get_root_file (a);
    printf ("root file: %s\n", root_file);
    if (root_file)
        g_free (root_file);

    g_object_unref (a);
}

void
test_webkit ()
{
}

void
test_doc_name (const char *path)
{
    GEPUBDoc *doc = gepub_doc_new (path);
    gchar *title = gepub_doc_get_metadata (doc, META_TITLE);
    gchar *lang = gepub_doc_get_metadata (doc, META_LANG);
    gchar *id = gepub_doc_get_metadata (doc, META_ID);
    gchar *author = gepub_doc_get_metadata (doc, META_AUTHOR);
    gchar *description = gepub_doc_get_metadata (doc, META_DESC);

    printf ("title: %s\n", title);
    printf ("author: %s\n", author);
    printf ("id: %s\n", id);
    printf ("lang: %s\n", lang);
    printf ("desc: %s\n", description);

    g_free (title);
    g_free (lang);
    g_free (id);
    g_free (author);
    g_free (description);
    g_object_unref (doc);
}

void
pk (gchar *key, gchar *value, gpointer data)
{
    printf ("%s: %s\n", key, value);
}

void
test_doc_resources (const char *path)
{
    GEPUBDoc *doc = gepub_doc_new (path);
    GHashTable *ht = gepub_doc_get_resources (doc);
    g_hash_table_foreach (ht, pk, NULL);
    guchar *ncx;

    ncx = gepub_doc_get_resource (doc, "ncx");
    printf ("ncx:\n%s\n", ncx);
    g_free (ncx);

    g_object_unref (doc);
}

int
main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    GtkWidget *window;
    GtkWidget *wview;

    if (argc < 2) {
        printf ("you should provide an .epub file\n");
        return 1;
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //wview = webkit_web_view_new ();

    //gtk_container_add (GTK_CONTAINER (window), wview);
    //gtk_widget_show_all (window);

    //webkit_web_view_open (WEBKIT_WEB_VIEW (wview), "http://google.es");

    TEST(test_open, argv[1])
    TEST(test_read, argv[1])
    TEST(test_root_file, argv[1])
    TEST(test_doc_name, argv[1])
    TEST(test_doc_resources, argv[1])

    gtk_main ();

    return 0;
}
