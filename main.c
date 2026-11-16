#include <stdio.h>
#include <gtk/gtk.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *combo;
static GtkWidget *info;

/* read /sys/block/<dev>/<attr> into out & return 1 on success */
static int read_attr(const char *dev, const char *attr, char *out, size_t n)
{
    char path[256];
    snprintf(path, sizeof path, "/sys/block/%s/%s", dev, attr);
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    if (!fgets(out, (int)n, f))
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    size_t len = strspn(out, "\n");
    while (len && out[len - 1] == ' ') len--;
    out[len] = 0;
    return 1;
}
/**/

// scan drives
static void scan_drives(void)
{
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));

    DIR *d = opendir("/sys/block");
    if (!d) return;

    struct dirent *e;
    while((e = readdir(d)))
    {
        char rem[8], size[32], model[64];
        if (e->d_name[0] == '.') continue;
        if (!read_attr(e->d_name, "removable", rem, sizeof rem) || rem[0] != '1')
            continue;
        if (!read_attr(e->d_name, "size", size, sizeof size))
            continue;
        unsigned long long sectors = strtoull(size, NULL, 10);
        if (sectors == 0) continue;

        if (!read_attr(e->d_name, "device/model", model, sizeof model))
            strcpy(model, "Unknown");

        char line[200];
        snprintf(line, sizeof line, "/dev/%s  -  %s  -  %.1f GB", e->d_name, model, (double)sectors * 512.0 / 1e9);

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), e->d_name, line);

    }
    closedir(d);

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    if (gtk_combo_box_get_active(GTK_COMBO_BOX(combo)) < 0)
        gtk_label_set_text(GTK_LABEL(info), "No removable drive was found");
}

static void on_changed(GtkComboBox *c, gpointer data)
{
    (void)data;
    const char *id = gtk_combo_box_get_active_id(c);
    if (!id) return;
    char msg[64];
    snprintf(msg, sizeof msg, "Selected: /dev/%s", id);
    gtk_label_set_text(GTK_LABEL(info), msg);
}

static void on_refresh(GtkButton *b, gpointer data)
{
    (void)b;
    (void)data;
    scan_drives();
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "USBFS");
    gtk_window_set_default_size(GTK_WINDOW(win), 420, 140);
    gtk_container_set_border_width(GTK_CONTAINER(win), 12);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    combo = gtk_combo_box_text_new();
    info = gtk_label_new("");
    GtkWidget *refresh = gtk_button_new_with_label("Refresh");

    g_signal_connect(combo, "changed", G_CALLBACK(on_changed), NULL);
    g_signal_connect(refresh, "clicked", G_CALLBACK(on_refresh), NULL);

    gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), info, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), refresh, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(win), box);

    scan_drives();
    gtk_widget_show_all(win);
    gtk_main();


    return 0;
}
