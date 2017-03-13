#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <libgepub/gepub.h>

gchar *buf = NULL;
gchar *buf2 = NULL;
gchar *tmpbuf;

GtkTextBuffer *page_buffer;

#define PTEST1(...) printf (__VA_ARGS__)
#define PTEST2(...) buf = g_strdup_printf (__VA_ARGS__);\
                    tmpbuf = buf2;\
                    buf2 = g_strdup_printf ("%s%s", buf2, buf);\
                    g_free (buf);\
                    g_free (tmpbuf)
#define PTEST PTEST2

#define TEST(f,arg...) PTEST ("\n### TESTING " #f " ###\n\n"); f (arg); PTEST ("\n\n");

void
print_replaced_text (GepubDoc *doc)
{
    GBytes *content;
    gsize s;
    const guchar *data;
    content = gepub_doc_get_current_with_epub_uris (doc);

    data = g_bytes_get_data (content, &s);
    printf ("\n\nREPLACED:\n%s\n", data);
    g_bytes_unref (content);
}

void
button_pressed (GtkButton *button, GepubDoc *doc)
{
    if (!strcmp (gtk_button_get_label (button), "prev")) {
        gepub_doc_go_prev (doc);
    } else {
        gepub_doc_go_next (doc);
    }
    printf ("CURRENT: %d\n", gepub_doc_get_page (doc));
    printf ("CURRENT: %s\n", gepub_doc_get_current_id (doc));
    printf ("CURRENT: %s\n", gepub_doc_get_current_path (doc));
    //print_replaced_text (doc);
}

void
test_doc_name (const char *path)
{
    GepubDoc *doc = gepub_doc_new (path);
    gchar *title = gepub_doc_get_metadata (doc, GEPUB_META_TITLE);
    gchar *lang = gepub_doc_get_metadata (doc, GEPUB_META_LANG);
    gchar *id = gepub_doc_get_metadata (doc, GEPUB_META_ID);
    gchar *author = gepub_doc_get_metadata (doc, GEPUB_META_AUTHOR);
    gchar *description = gepub_doc_get_metadata (doc, GEPUB_META_DESC);
    //gchar *cover = gepub_doc_get_cover (doc);
    //gchar *cover_mime = gepub_doc_get_resource_mime_by_id (doc, cover);

    PTEST ("title: %s\n", title);
    PTEST ("author: %s\n", author);
    PTEST ("id: %s\n", id);
    PTEST ("lang: %s\n", lang);
    PTEST ("desc: %s\n", description);
    //PTEST ("cover: %s\n", cover);
    //PTEST ("cover mime: %s\n", cover_mime);

    g_free (title);
    g_free (lang);
    g_free (id);
    g_free (author);
    g_free (description);
    //g_free (cover);
    //g_free (cover_mime);
    g_object_unref (G_OBJECT (doc));
}

int
main (int argc, char **argv)
{
    gtk_init (&argc, &argv);

    GtkWidget *window;
    GtkWidget *vpaned;
    GtkWidget *textview;
    GtkWidget *scrolled;

    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *b_next;
    GtkWidget *b_prev;

    GtkTextBuffer *buffer;

    GError *error = NULL;

    GepubDoc *doc;
    GtkWidget *widget = gepub_widget_new ();

    if (argc < 2) {
        printf ("you should provide an .epub file\n");
        return 1;
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", (GCallback)gtk_main_quit, NULL);
    gtk_widget_set_size_request (GTK_WIDGET (window), 800, 500);
    vpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add (GTK_CONTAINER (window), vpaned);

    // gepub widget
    GFile *file = g_file_new_for_path (argv[1]);
    doc = g_initable_new (gepub_doc_get_type (), NULL, &error,
                          "file", file,
                          NULL);
    g_object_unref (file);
    if (!doc) {
        perror ("BAD epub FILE");
        return -1;
    }

    gepub_widget_set_doc (GEPUB_WIDGET (widget), doc);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    b_prev = gtk_button_new_with_label ("prev");
    g_signal_connect (b_prev, "clicked", (GCallback)button_pressed, GEPUB_DOC (doc));
    b_next = gtk_button_new_with_label ("next");
    g_signal_connect (b_next, "clicked", (GCallback)button_pressed, GEPUB_DOC (doc));
    gtk_container_add (GTK_CONTAINER (hbox), b_prev);
    gtk_container_add (GTK_CONTAINER (hbox), b_next);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);

    textview = gtk_text_view_new ();
    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (scrolled), textview);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 5);

    gtk_widget_set_size_request (GTK_WIDGET (vbox), 400, 500);
    gtk_paned_add1 (GTK_PANED (vpaned), vbox);
    gtk_paned_add2 (GTK_PANED (vpaned), widget);

    gtk_widget_show_all (window);


    // Testing all
    TEST(test_doc_name, argv[1])

    // Freeing the mallocs :P
    if (buf2) {
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
        gtk_text_buffer_set_text (buffer, buf2, strlen (buf2));
        g_free (buf2);
    }

    gtk_main ();

    g_object_unref (doc);

    return 0;
}
