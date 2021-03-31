#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <libgepub/gepub.h>

gchar *buf = NULL;
gchar *buf2 = NULL;
gchar *tmpbuf;

GtkTextBuffer *page_buffer;
GtkWidget *PAGE_LABEL;

#define PTEST1(...) { printf (__VA_ARGS__); }
#define PTEST2(...) { buf = g_strdup_printf (__VA_ARGS__);\
                      tmpbuf = buf2;\
                      buf2 = g_strdup_printf ("%s%s", buf2, buf);\
                      g_free (buf);\
                      g_free (tmpbuf);\
                    }
#define PTEST PTEST2

#define TEST(f,arg...) PTEST ("\n### TESTING " #f " ###\n\n"); f (arg); PTEST ("\n\n");

static void
reload_current_chapter (GepubWidget *widget)
{
    gchar *txt = g_strdup_printf ("%02.2f", gepub_widget_get_pos (widget));
    gtk_label_set_text (GTK_LABEL (PAGE_LABEL), txt);
    g_free (txt);
}

static void
update_text (GepubDoc *doc)
{
    GList *l, *chunks;
    GtkTextIter start, end;

    gtk_text_buffer_get_start_iter (page_buffer, &start);
    gtk_text_buffer_get_end_iter (page_buffer, &end);
    gtk_text_buffer_delete (page_buffer, &start, &end);

    chunks = gepub_doc_get_text (doc);

    for (l=chunks; l; l = l->next) {
        GepubTextChunk *chunk = GEPUB_TEXT_CHUNK (l->data);
        if (chunk->type == GEPUBTextHeader) {
            gtk_text_buffer_insert_at_cursor (page_buffer, "\n", -1);
            gtk_text_buffer_get_end_iter (page_buffer, &end);
            gtk_text_buffer_insert_with_tags_by_name (page_buffer, &end, chunk->text, -1, "head",  NULL);
            gtk_text_buffer_insert_at_cursor (page_buffer, "\n", -1);
        } else if (chunk->type == GEPUBTextNormal) {
            gtk_text_buffer_insert_at_cursor (page_buffer, "\n", -1);
            gtk_text_buffer_insert_at_cursor (page_buffer, chunk->text, -1);
            gtk_text_buffer_insert_at_cursor (page_buffer, "\n", -1);
        } else if (chunk->type == GEPUBTextItalic) {
            gtk_text_buffer_get_end_iter (page_buffer, &end);
            gtk_text_buffer_insert_with_tags_by_name (page_buffer, &end, chunk->text, -1, "italic",  NULL);
        } else if (chunk->type == GEPUBTextBold) {
            gtk_text_buffer_get_end_iter (page_buffer, &end);
            gtk_text_buffer_insert_with_tags_by_name (page_buffer, &end, chunk->text, -1, "bold",  NULL);
        }
    }
}

#if 0
static void
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
#endif

static void
button_pressed (GtkButton *button, GepubWidget *widget)
{
    GepubDoc *doc = gepub_widget_get_doc (widget);
    printf("CLICKED %s\n",  gtk_button_get_label (button));

    if (!strcmp (gtk_button_get_label (button), "< chapter")) {
        gepub_doc_go_prev (doc);
    } else if (!strcmp (gtk_button_get_label (button), "chapter >")) {
        gepub_doc_go_next (doc);
    } else if (!strcmp (gtk_button_get_label (button), "paginated")) {
        gboolean b = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
        gepub_widget_set_paginate (widget, b);
    } else if (!strcmp (gtk_button_get_label (button), "< page")) {
        gepub_widget_page_prev (widget);
    } else if (!strcmp (gtk_button_get_label (button), "page >")) {
        gepub_widget_page_next (widget);
    } else if (!strcmp (gtk_button_get_label (button), "margin +")) {
        gepub_widget_set_margin (widget, gepub_widget_get_margin (widget) + 20);
    } else if (!strcmp (gtk_button_get_label (button), "margin -")) {
        gepub_widget_set_margin (widget, gepub_widget_get_margin (widget) - 20);
    } else if (!strcmp (gtk_button_get_label (button), "font +")) {
        gint f = gepub_widget_get_fontsize (widget);
        f = f ? f : 12;
        printf ("font %d\n", f);
        gepub_widget_set_fontsize (widget, f + 2);
    } else if (!strcmp (gtk_button_get_label (button), "font -")) {
        gint f = gepub_widget_get_fontsize (widget);
        f = f ? f : 12;
        printf ("font %d\n", f);
        gepub_widget_set_fontsize (widget, f - 2);
    } else if (!strcmp (gtk_button_get_label (button), "line height +")) {
        gfloat l = gepub_widget_get_lineheight (widget);
        l = l ? l : 1.5;
        gepub_widget_set_lineheight (widget, l + 0.1);
    } else if (!strcmp (gtk_button_get_label (button), "line height -")) {
        gfloat l = gepub_widget_get_lineheight (widget);
        l = l ? l : 1.5;
        gepub_widget_set_lineheight (widget, l - 0.1);
    }
    update_text (doc);
    //print_replaced_text (doc);
}

static void
test_open (const char *path)
{
    GepubArchive *a;
    GList *list_files;
    gint i;
    gint size;

    a = gepub_archive_new (path);
    list_files = gepub_archive_list_files (a);
    if (!list_files) {
        PTEST ("ERROR: BAD epub file");
        g_object_unref (a);
        return;
    }

    size = g_list_length (list_files);
    PTEST ("%d\n", size);
    for (i = 0; i < size; i++) {
        PTEST ("file: %s\n", (char *)g_list_nth_data (list_files, i));
        g_free (g_list_nth_data (list_files, i));
    }

    g_list_free (list_files);

    g_object_unref (a);
}

static void
find_xhtml (gchar *key, GepubResource *value, gpointer data)
{
    gchar **d = (gchar **)data;
    if (g_strcmp0 (value->mime, "application/xhtml+xml") == 0) {
        *d = value->uri;
    }
}

static void
test_read (const char *path)
{
    GepubArchive *a;
    GList *list_files = NULL;
    const guchar *buffer;
    gchar *file = NULL;
    gsize bufsize;
    GBytes *bytes;
    GepubDoc *doc;
    GHashTable *ht;

    a = gepub_archive_new (path);

    doc = gepub_doc_new (path, NULL);
    ht = (GHashTable*)gepub_doc_get_resources (doc);
    g_hash_table_foreach (ht, (GHFunc)find_xhtml, &file);

    bytes = gepub_archive_read_entry (a, file);
    if (bytes) {
        buffer = g_bytes_get_data (bytes, &bufsize);
        PTEST ("doc:%s\n----\n%s\n-----\n", file, buffer);
        g_bytes_unref (bytes);
    }

    g_list_foreach (list_files, (GFunc)g_free, NULL);
    g_list_free (list_files);

    g_object_unref (a);
}

static void
test_root_file (const char *path)
{
    GepubArchive *a;
    gchar *root_file = NULL;

    a = gepub_archive_new (path);

    root_file = gepub_archive_get_root_file (a);
    PTEST ("root file: %s\n", root_file);
    if (root_file)
        g_free (root_file);

    g_object_unref (a);
}

static void
test_doc_name (const char *path)
{
    GepubDoc *doc = gepub_doc_new (path, NULL);
    gchar *title = gepub_doc_get_metadata (doc, GEPUB_META_TITLE);
    gchar *lang = gepub_doc_get_metadata (doc, GEPUB_META_LANG);
    gchar *id = gepub_doc_get_metadata (doc, GEPUB_META_ID);
    gchar *author = gepub_doc_get_metadata (doc, GEPUB_META_AUTHOR);
    gchar *description = gepub_doc_get_metadata (doc, GEPUB_META_DESC);
    gchar *cover = gepub_doc_get_cover (doc);
    gchar *cover_mime = NULL;

    if (cover)
        cover_mime = gepub_doc_get_resource_mime_by_id (doc, cover);

    PTEST ("title: %s\n", title);
    PTEST ("author: %s\n", author);
    PTEST ("id: %s\n", id);
    PTEST ("lang: %s\n", lang);
    PTEST ("desc: %s\n", description);
    if (cover)
        PTEST ("cover: %s\n", cover);
    if (cover_mime)
        PTEST ("cover mime: %s\n", cover_mime);

    g_free (title);
    g_free (lang);
    g_free (id);
    g_free (author);
    g_free (description);
    g_free (cover);
    g_object_unref (G_OBJECT (doc));
}

static void
pk (gchar *key, GepubResource *value, gpointer data)
{
    PTEST ("%s: %s, %s\n", key, value->mime, value->uri);
}

static void
test_doc_resources (const char *path)
{
    GepubDoc *doc;
    GHashTable *ht;
    GBytes *ncx;
    const guchar *data;
    gsize size;

    doc = gepub_doc_new (path, NULL);
    ht = (GHashTable*)gepub_doc_get_resources (doc);
    g_hash_table_foreach (ht, (GHFunc)pk, NULL);

    ncx = gepub_doc_get_resource_by_id (doc, "ncx");
    data = g_bytes_get_data (ncx, &size);
    PTEST ("ncx:\n%.*s\n", size, data);
    g_bytes_unref (ncx);

    g_object_unref (G_OBJECT (doc));
}

static void
test_doc_spine (const char *path)
{
    GepubDoc *doc = gepub_doc_new (path, NULL);
    int id = 0;

    do {
        PTEST ("%d: %s\n", id++, gepub_doc_get_current_id (doc));
    } while (gepub_doc_go_next (doc));

    g_object_unref (G_OBJECT (doc));
}

static void
test_doc_toc (const char *path)
{
    GepubDoc *doc = gepub_doc_new (path, NULL);

    GList *nav = gepub_doc_get_toc (doc);
    while (nav && nav->data) {
        GepubNavPoint *point = (GepubNavPoint*)nav->data;
        PTEST ("%02d: %s -> %s\n", (gint)point->playorder, point->label, point->content);
        PTEST (" -> Chapter: %d\n", gepub_doc_resource_uri_to_chapter (doc, point->content));
        nav = nav->next;
    }

    g_object_unref (G_OBJECT (doc));
}

static void
destroy_cb (GtkWidget *window,
            GtkWidget *view)
{
    g_signal_handlers_disconnect_by_func (G_OBJECT (view),
                                          reload_current_chapter,
                                          view);
    gtk_main_quit ();
}

int
main (int argc, char **argv)
{
    GtkWidget *window;
    GtkWidget *vpaned;
    GtkWidget *textview;
    GtkWidget *scrolled;

    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *hbox2;
    GtkWidget *b_next;
    GtkWidget *b_prev;

    GtkWidget *p_next;
    GtkWidget *p_prev;

    GtkWidget *margin1;
    GtkWidget *margin2;

    GtkWidget *font1;
    GtkWidget *font2;

    GtkWidget *lh1;
    GtkWidget *lh2;

    GtkWidget *paginate;

    GtkTextBuffer *buffer;

    GepubDoc *doc;
    GtkWidget *textview2;

    GtkWidget *widget;

    gtk_init (&argc, &argv);

    widget = gepub_widget_new ();

    webkit_settings_set_enable_developer_extras (
        webkit_web_view_get_settings (WEBKIT_WEB_VIEW (widget)), TRUE);

    if (argc < 2) {
        printf ("you should provide an .epub file\n");
        return 1;
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", G_CALLBACK(destroy_cb), widget);
    gtk_widget_set_size_request (GTK_WIDGET (window), 1200, 800);
    vpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add (GTK_CONTAINER (window), vpaned);

    // gepub widget
    doc = gepub_doc_new (argv[1], NULL);
    if (!doc) {
        perror ("BAD epub FILE");
        return -1;
    }

    gepub_widget_set_doc (GEPUB_WIDGET (widget), doc);

    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    textview2 = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview2), GTK_WRAP_WORD_CHAR);
    page_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview2));
    gtk_text_buffer_create_tag (page_buffer, "bold", "weight", PANGO_WEIGHT_BOLD, "foreground", "#ff0000", NULL);
    gtk_text_buffer_create_tag (page_buffer, "italic", "style", PANGO_STYLE_ITALIC, "foreground", "#005500", NULL);
    gtk_text_buffer_create_tag (page_buffer, "head", "size-points", 20.0, NULL);
    update_text (doc);
    gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (textview2));
    gtk_widget_set_size_request (GTK_WIDGET (textview2), 500, 300);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    hbox2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
    b_prev = gtk_button_new_with_label ("< chapter");
    g_signal_connect (b_prev, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));
    b_next = gtk_button_new_with_label ("chapter >");
    g_signal_connect (b_next, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));

    p_prev = gtk_button_new_with_label ("< page");
    g_signal_connect (p_prev, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));
    p_next = gtk_button_new_with_label ("page >");
    g_signal_connect (p_next, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));

    margin1 = gtk_button_new_with_label ("margin +");
    g_signal_connect (margin1, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));
    margin2 = gtk_button_new_with_label ("margin -");
    g_signal_connect (margin2, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));

    font1 = gtk_button_new_with_label ("font +");
    g_signal_connect (font1, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));
    font2 = gtk_button_new_with_label ("font -");
    g_signal_connect (font2, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));

    lh1 = gtk_button_new_with_label ("line height +");
    g_signal_connect (lh1, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));
    lh2 = gtk_button_new_with_label ("line height -");
    g_signal_connect (lh2, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));

    PAGE_LABEL = gtk_label_new ("0");

    g_signal_connect_swapped (widget, "notify",
                              G_CALLBACK (reload_current_chapter), widget);

    paginate = gtk_check_button_new_with_label ("paginated");
    g_signal_connect (paginate, "clicked", (GCallback)button_pressed, GEPUB_WIDGET (widget));

    gtk_container_add (GTK_CONTAINER (hbox), b_prev);
    gtk_container_add (GTK_CONTAINER (hbox), b_next);

    gtk_container_add (GTK_CONTAINER (hbox), p_prev);
    gtk_container_add (GTK_CONTAINER (hbox), p_next);

    gtk_container_add (GTK_CONTAINER (hbox), PAGE_LABEL);
    gtk_container_add (GTK_CONTAINER (hbox), paginate);

    gtk_container_add (GTK_CONTAINER (hbox2), margin1);
    gtk_container_add (GTK_CONTAINER (hbox2), margin2);

    gtk_container_add (GTK_CONTAINER (hbox2), font1);
    gtk_container_add (GTK_CONTAINER (hbox2), font2);

    gtk_container_add (GTK_CONTAINER (hbox2), lh1);
    gtk_container_add (GTK_CONTAINER (hbox2), lh2);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 5);
    gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 5);

    textview = gtk_text_view_new ();
    scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (scrolled), textview);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 5);

    gtk_widget_set_size_request (GTK_WIDGET (vbox), 600, 500);
    gtk_paned_add1 (GTK_PANED (vpaned), vbox);
    gtk_paned_add2 (GTK_PANED (vpaned), widget);

    gtk_widget_show_all (window);


    // Testing all
    TEST(test_open, argv[1])
    TEST(test_read, argv[1])
    TEST(test_root_file, argv[1])
    TEST(test_doc_name, argv[1])
    TEST(test_doc_resources, argv[1])
    TEST(test_doc_spine, argv[1])
    TEST(test_doc_toc, argv[1])

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
