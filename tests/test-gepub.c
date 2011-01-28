#include <stdio.h>
#include <libgepub/gepub.h>

void
test_open (const char *path)
{
    GEPUBArchive *a;
    a = gepub_archive_new (path);
    gepub_archive_list_files (a);
    g_object_unref (a);
}

int
main (int argc, char **argv)
{
    g_type_init ();

    if (argc < 2) {
        printf ("you should provide an .epub file\n");
        return 1;
    }

    test_open (argv[1]);

    return 0;
}
