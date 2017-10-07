/*
 * geany_generic_sql_formatter.c - a Geany plugin to format SQL files
 *
 *  Copyright 2017 zhgzhg @ github.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <geanyplugin.h>
#include <document.h>
#include <gdk/gdkkeysyms.h> /* for the key bindings */

#include "fsqlf-1.0.0-prealpha-02/include/lib_fsqlf.h"

#ifdef HAVE_LOCALE_H
	#include <locale.h>
#endif

GeanyPlugin *geany_plugin;
struct GeanyKeyGroup *geany_key_group;
GeanyData *geany_data;

static gchar *plugin_config_path = NULL;

PLUGIN_VERSION_CHECK(224)

PLUGIN_SET_TRANSLATABLE_INFO(LOCALEDIR,
	GETTEXT_PACKAGE,
	_("Generic SQL Formatter"),
	_("SQL file formatter that covers the basic SQL syntax.\n\
https://github.com/zhgzhg/Geany-Generic-SQL-Formatter"),
	"1.0",
	"zhgzhg @@ github.com\n\
https://github.com/zhgzhg/Geany-Generic-SQL-Formatter"
);

static GtkWidget *main_menu_item = NULL;
static fsqlf_kwmap_t kwmap;

/* SQL formatter Code - parts of fsqlf cli tool used as an example */

static void my_sql_format(GeanyDocument *doc)
{
	gint text_len = 0;
	gchar *text_string = NULL;
	char *formatted_string = NULL;

	if (!doc) return;

	/* load the text and prepares it */

	gboolean workWithTextSelection = FALSE;

	/* first try to work only with a text selection (if any) */
	if (sci_has_selection(doc->editor->sci))
	{
		text_string = sci_get_selection_contents(doc->editor->sci);
		if (text_string != NULL)
		{
			workWithTextSelection = TRUE;
			text_len =
				sci_get_selected_text_length(doc->editor->sci) + 1;
		}
	}
	else
	{	/* Work with the entire file */
		text_len = sci_get_length(doc->editor->sci);
		if (text_len == 0) return;
		++text_len;
		text_string = sci_get_contents(doc->editor->sci, -1);
	}

	if (text_string == NULL) return;

	/* begin the formatting process */

    fsqlf_format_bytes(
		kwmap, (const char *)text_string, text_len, &formatted_string);

	{
		gint spos;
		gboolean insertNewLineAtTheEnd = FALSE;

		if (!workWithTextSelection)
		{
			sci_set_text(
				doc->editor->sci, (const gchar*) formatted_string);
		}
		else
		{
			spos = sci_get_selection_start(doc->editor->sci);

			sci_replace_sel(
				doc->editor->sci, (const gchar*) formatted_string);

			/* Insert additional new lines for user's ease */
			if (spos > 0)
			{
				sci_insert_text(doc->editor->sci, spos,
					editor_get_eol_char(doc->editor));
			}
		}

		// Change the cursor position to the start of the line
		// and scroll to there
		gint cursPos = sci_get_current_position(doc->editor->sci);
		gint colPos = sci_get_col_from_position(
			doc->editor->sci, cursPos);
		sci_set_current_position(
			doc->editor->sci, cursPos - colPos, TRUE);

		if (formatted_string != NULL)
		{ free(formatted_string); }
	}
}

/* Plugin settings */

void plugin_configure_single(GtkWidget *dialog)
{
	document_open_file(plugin_config_path, FALSE, NULL, NULL);
}


/* Geany plugin EP code */

static void item_activate_cb(GtkMenuItem *menuitem, gpointer gdata)
{
	my_sql_format(document_get_current());
}

static void kb_run_generic_sql_formatter(G_GNUC_UNUSED guint key_id)
{
	my_sql_format(document_get_current());
}

void plugin_init(GeanyData *data)
{
	guint i = 0;

	/* read & prepare configuration */
	gchar *config_dir = g_build_path(G_DIR_SEPARATOR_S,
		geany_data->app->configdir, "plugins", "genericsqlformatter",
		NULL);
	plugin_config_path = g_build_path(G_DIR_SEPARATOR_S, config_dir,
										"genericsqlformatter.conf",
										NULL);

	g_mkdir_with_parents(config_dir, S_IRUSR | S_IWUSR | S_IXUSR);
	g_free(config_dir);

	/* --- Init default configs. --- */
	fsqlf_kwmap_init(&kwmap);

	/* --- Read configs from file. --- */
    fsqlf_kwmap_conffile_read_default(kwmap);
	if (fsqlf_kwmap_conffile_read(
			kwmap, plugin_config_path) != FSQLF_OK) {
		/* Recreate the config file if the reading fails */
		fsqlf_kwmap_conffile_create(plugin_config_path);
	}

	/* ---------------------------- */

	main_menu_item = gtk_menu_item_new_with_mnemonic("SQL Formatter");
	gtk_widget_show(main_menu_item);
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu),
						main_menu_item);
	g_signal_connect(
		main_menu_item, "activate", G_CALLBACK(item_activate_cb), NULL);

	/* do not activate if there are do documents opened */
	ui_add_document_sensitive(main_menu_item);

	/* Register shortcut key group */
	geany_key_group = plugin_set_key_group(
					geany_plugin, _("generic_sql_formatter"), 1, NULL);


	/* Shift + Alt + q to format */
	keybindings_set_item(geany_key_group, 0,
						 kb_run_generic_sql_formatter,
                         GDK_q, GEANY_PRIMARY_MOD_MASK | GDK_SHIFT_MASK,
                         "generic_sql_formatter",
                         _("Run the Generic SQL Formatter"),
                         main_menu_item);
}


void plugin_cleanup(void)
{
	fsqlf_kwmap_destroy(kwmap);
	g_free(plugin_config_path);
	gtk_widget_destroy(main_menu_item);
}
