#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <libgepub/gepub.h>
#include <webkit/webkit.h>

gchar *buf = NULL;
gchar *buf2 = NULL;
gchar *tmpbuf;

#define PTEST1(...) printf (__VA_ARGS__)
#define PTEST2(...) buf = g_strdup_printf (__VA_ARGS__);\
                    tmpbuf = buf2;\
                    buf2 = g_strdup_printf ("%s%s", buf2, buf);\
                    g_free (buf);\
                    g_free (tmpbuf)
#define PTEST PTEST2

#define TEST(f,arg) PTEST ("\n### TESTING " #f " ###\n\n"); f (arg); PTEST ("\n\n");

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
    PTEST ("%d\n", size);
    for (i = 0; i < size; i++) {
        PTEST ("file: %s\n", (char *)g_list_nth_data (list_files, i));
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
        PTEST ("doc:%s\n----\n%s\n-----\n", file, buffer);

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
    PTEST ("root file: %s\n", root_file);
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

    PTEST ("title: %s\n", title);
    PTEST ("author: %s\n", author);
    PTEST ("id: %s\n", id);
    PTEST ("lang: %s\n", lang);
    PTEST ("desc: %s\n", description);

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
    PTEST ("%s: %s\n", key, value);
}

void
test_doc_resources (const char *path)
{
    GEPUBDoc *doc = gepub_doc_new (path);
    GHashTable *ht = (GHashTable*)gepub_doc_get_resources (doc);
    g_hash_table_foreach (ht, (GHFunc)pk, NULL);
    guchar *ncx;

    ncx = gepub_doc_get_resource (doc, "ncx");
    PTEST ("ncx:\n%s\n", ncx);
    g_free (ncx);

    g_object_unref (doc);
}

void
p (gchar *value, gpointer data)
{
    static int id = 0;
    PTEST ("%d: %s\n", id++, value);
}

void
test_doc_spine (const char *path)
{
    GEPUBDoc *doc = gepub_doc_new (path);

    GList *spine = gepub_doc_get_spine (doc);
    g_list_foreach (spine, (GFunc)p, NULL);

    g_object_unref (doc);
}

int
main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    GtkWidget *window;
    GtkWidget *vpaned;
    GtkWidget *textview;
    GtkWidget *wview;
    GtkWidget *scrolled;

    GtkTextBuffer *buffer;

    if (argc < 2) {
        printf ("you should provide an .epub file\n");
        return 1;
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", (GCallback)gtk_exit, NULL);
    vpaned = gtk_hpaned_new ();
    gtk_container_add (GTK_CONTAINER (window), vpaned);

    wview = webkit_web_view_new ();
    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (scrolled), wview);
    gtk_widget_set_size_request (wview, 500, 300);
    gtk_paned_add1 (GTK_PANED (vpaned), scrolled);

    textview = gtk_text_view_new ();
    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (scrolled), textview);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_add2 (GTK_PANED (vpaned), scrolled);

    gtk_widget_show_all (window);

    webkit_web_view_open (WEBKIT_WEB_VIEW (wview), "http://google.es");

    TEST(test_open, argv[1])
    TEST(test_read, argv[1])
    TEST(test_root_file, argv[1])
    TEST(test_doc_name, argv[1])
    TEST(test_doc_resources, argv[1])
    TEST(test_doc_spine, argv[1])

    if (buf2) {
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        gtk_text_buffer_set_text (buffer, buf2, strlen (buf2));
        g_free (buf2);
    }

    gtk_main ();

    return 0;
}
