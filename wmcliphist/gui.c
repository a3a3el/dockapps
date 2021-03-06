#include "wmcliphist.h"
#include <gdk/gdkkeysyms.h>


/* color of locked item */
gchar		locked_color_str[32] = DEF_LOCKED_COLOR;
GdkRGBA		locked_color;

/* Exec on middle click? */
int exec_middleclick = 1;

/* main window widget */
GtkWidget	*main_window;

/* dock icon widget */
GtkWidget	*dock_app;

/* clipboard history menu */
GtkWidget	*menu_hist;
GtkWidget	*menu_title;
gint		submenu_count = 0;

/* application menu */
GtkWidget	*menu_app;
GtkWidget	*menu_app_clip_lock;
GtkWidget	*menu_app_clip_ignore;
GtkWidget	*menu_app_exit;
GtkWidget	*menu_app_save;

/* event box */
GtkWidget *event;

/* button */
GtkWidget	*button;

/* pixmap */
GtkWidget	*pixmap;

/* which clipboard to use */
gchar		clipboard_str[32] = DEF_CLIPBOARD_STR;
GdkAtom		clipboard;

/* ==========================================================================
 *                                                     clipboard history menu
 */

/*
 * history menu item button click function
 */
static gboolean
menu_item_button_released(GtkWidget *widget,
		GdkEvent *event,
		gpointer user_data)
{
	GdkEventButton	*bevent = (GdkEventButton *)event;
	HISTORY_ITEM	*data = user_data;

	begin_func("menu_item_button_released");

	/* button 2 or 3 - exec or (un)lock item respectively */
	if (bevent->button == 2) {
		if (exec_middleclick) {
			gtk_menu_popdown(GTK_MENU(menu_hist));
			exec_item(data->content, NULL);
		}
		return_val(TRUE);
	} else if (bevent->button == 3) {
		if (data->locked == 0) {

			/* cannot lock all records */
			if (locked_count == num_items_to_keep - 1) {
				show_message("There must remain at least one "
					"unlocked item\nwhen menu is full!",
					"Warning", "OK", NULL, NULL);
				return_val(TRUE);
			}

			gtk_widget_override_color(
				gtk_bin_get_child(GTK_BIN(data->menu_item)),
				GTK_STATE_FLAG_NORMAL, &locked_color);
			data->locked = 1;
			locked_count++;

		} else {
			gtk_widget_override_color(
				gtk_bin_get_child(GTK_BIN(data->menu_item)),
				GTK_STATE_FLAG_NORMAL, NULL);
			data->locked = 0;
			locked_count--;
		}
	} else {
		return_val(FALSE);
	}

	return_val(TRUE);
}


/*
 * history menu item left click or keypress function
 */
static gboolean
menu_item_activated(GtkWidget *widget, gpointer user_data)
{
	move_item_to_begin((HISTORY_ITEM *) user_data);
	return_val(TRUE);
}


/*
 * checks, if there is already such item in menu,
 * in which case it moves it to the begining
 */
HISTORY_ITEM *
menu_item_exists(gchar *content, GtkWidget *submenu)
{
	HISTORY_ITEM	*hist_item;
	GList		*list_node;

	begin_func("menu_item_exists");

	list_node = g_list_last(history_items);
	while (list_node) {
		hist_item = (HISTORY_ITEM *)list_node->data;
		if (hist_item->menu == submenu
				&& g_utf8_collate(hist_item->content, content)
				== 0) {
			gtk_menu_reorder_child(GTK_MENU(hist_item->menu),
					hist_item->menu_item, 1);
			history_items = g_list_remove_link(history_items,
					list_node);
			history_items = g_list_concat(list_node, history_items);

			return_val(hist_item);
		}
		list_node = g_list_previous(list_node);
	}

	return_val(NULL);
}


/*
 * add new item to menu
 */
HISTORY_ITEM *
menu_item_add(gchar *content, gint locked, GtkWidget *target_menu)
{
	GList		*list_node;
	gint		i;
	gchar		*menu_item_name;
	gchar		*fixed_menu_item_name;
	gsize		length;
	HISTORY_ITEM	*hist_item;

	begin_func("menu_item_add");

	hist_item = menu_item_exists(content, target_menu);
	if (hist_item != NULL) {
		dump_history_list("reorder");
		return_val(hist_item);
	}

	if (num_items == num_items_to_keep) {
		/* max item limit reached, destroy oldest one */
		list_node = g_list_last(history_items);
		while (1) {
			hist_item = (HISTORY_ITEM*)
				list_node->data;
			if (hist_item->locked == 0)
				break;
			list_node = g_list_previous(list_node);
			g_assert((list_node != NULL));
		}

		history_items = g_list_remove_link(history_items, list_node);
/*		gtk_container_remove(GTK_CONTAINER(hist_item->menu),
				hist_item->menu_item); */
		gtk_widget_destroy(hist_item->menu_item);
		g_free(hist_item->content);
		g_free(hist_item);
		g_list_free_1(list_node);
		num_items--;
		dump_history_list("remove oldest");
	}

	/* prepare menu item name */
	menu_item_name = g_new0(char, MAX_ITEM_LENGTH * 2 + 1);
	memset(menu_item_name, 0, MAX_ITEM_LENGTH * 2 + 1);
	length = g_utf8_strlen(content, -1);
	if (length > (size_t) (MAX_ITEM_LENGTH)) {
		g_utf8_strncpy(menu_item_name, content, MAX_ITEM_LENGTH - 4);
		strcat(menu_item_name, "...");
	} else {
		strcpy(menu_item_name, content);
	}

	/* do some menu item name cleanups */
	fixed_menu_item_name = g_new0(char, strlen(menu_item_name) + 1);
	for (i = 0; i < g_utf8_strlen(menu_item_name, -1); i++) {
		gchar *uchar_ptr = g_utf8_offset_to_pointer(menu_item_name, i);
		gunichar uchar = g_utf8_get_char(uchar_ptr);
		if (g_unichar_isprint(uchar)) {
			gchar *decoded_char = g_ucs4_to_utf8(&uchar, 1, NULL,
					NULL, NULL);
			strcat(fixed_menu_item_name, decoded_char);
			g_free(decoded_char);
		} else  {
			strcat(fixed_menu_item_name, "_");
		}
	}
	g_free(menu_item_name);
	menu_item_name = fixed_menu_item_name;

	/* create menu item */
	hist_item = g_new0(HISTORY_ITEM, 1);
	hist_item->menu_item = gtk_menu_item_new_with_label(menu_item_name);
	hist_item->content = g_strdup(content);
	hist_item->locked = locked;
	hist_item->menu = target_menu;

	if (locked == 1) {
		gtk_widget_override_color(
				gtk_bin_get_child(GTK_BIN(hist_item->menu_item)),
				GTK_STATE_FLAG_NORMAL, &locked_color);
		locked_count++;
	}

	/* add to menu */
	gtk_menu_shell_insert(GTK_MENU_SHELL(hist_item->menu), hist_item->menu_item, 1);


	/* connect actions to signals */
	g_signal_connect(G_OBJECT(hist_item->menu_item),
			"button-release-event",
			G_CALLBACK(menu_item_button_released),
			(gpointer)hist_item);

	g_signal_connect(G_OBJECT(hist_item->menu_item),
			"activate",
			G_CALLBACK(menu_item_activated),
			(gpointer)hist_item);

	gtk_widget_show(hist_item->menu_item);

	history_items = g_list_insert(history_items, hist_item, 0);

	num_items++;

	return_val(hist_item);
}



/* ==========================================================================
 *                                                           application menu
 */

/*
 * application main menu handler
 */
gboolean
menu_app_item_click(GtkWidget *menuitem, gpointer data)
{
	gint	button;

	begin_func("menu_app_item_click");

	switch (GPOINTER_TO_INT(data)) {
		/* save history menu */
		case 0:
			if (history_save() != 0) {
				button = show_message("History was NOT saved.\n",
						"Warning", "OK", NULL, NULL);
			}
			return_val(TRUE);

		/* exit menu */
		case 1:
			if (history_save() != 0) {
				button = show_message("History was NOT saved.\n"
						"Do you really want to exit?",
						"Error", "Yes", "No", NULL);
				if (button != 0)
					return_val(TRUE);
			}
			history_free();
			rcconfig_free();

			gtk_main_quit();
			exit(0);
			return_val(TRUE);
	}
	return_val(FALSE);
}



/* ==========================================================================
 *                                                          dock button press
 */

/*
 * dock button click response
 */
gboolean
button_press(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	begin_func("button_press");

	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton	*bevent = (GdkEventButton *)event;

		switch (bevent->button) {
			case 1:
				/* popup history menu */
				gtk_menu_popup(GTK_MENU(menu_hist),
						NULL, NULL,
						NULL, NULL,
						bevent->button,
						bevent->time);
				return_val(TRUE);

			case 3:
				/* popup application menu */
				gtk_menu_popup(GTK_MENU(menu_app),
						NULL, NULL,
						NULL, NULL,
						bevent->button,
						bevent->time);
				return_val(TRUE);
		}
	}

	return_val(FALSE);
}


/* ==========================================================================
 *                                                            message dialogs
 */


/*
 * open dialog with specified message and buttons
 * and return number of button pressed
 */
gint
show_message(gchar *message, char *title,
		char *b0_text, char *b1_text, char *b2_text)
{
	GtkWidget	*dialog,
			*label,
			*content_area;
	gint		result;
	gint		button_pressed = 1;

	begin_func("show_message");

	/* create the main widgets */
	dialog = gtk_dialog_new();

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	/* Show the message */
	label = gtk_label_new(message);
	gtk_container_add(GTK_CONTAINER (content_area), label);
	gtk_widget_show(label);

	/* add the button */
	gtk_dialog_add_button(GTK_DIALOG (dialog), b0_text, 0);
	if(b1_text != NULL) gtk_dialog_add_button(GTK_DIALOG (dialog), b1_text, 1);
	if(b2_text != NULL) gtk_dialog_add_button(GTK_DIALOG (dialog), b2_text, 2);

	gtk_widget_show_all(dialog);

	/* set the dialog title */
	gtk_window_set_title(GTK_WINDOW(dialog), title);

	/* set the dialog modal and transient */
/*	gtk_window_set_modal(GTK_WINDOW (dialog), TRUE);*/
	gtk_window_set_transient_for(GTK_WINDOW (dialog), GTK_WINDOW (dock_app));

	/*wait for the user response */
	result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result)
	{
		case 0:
			button_pressed = 0;
			break;
		case 1:
			button_pressed = 1;
			break;
		case 2:
			button_pressed = 2;
			break;
		default:
			break;
	}

	/*destroy the dialog box, when the user responds */
	gtk_widget_destroy(dialog);

	return_val(button_pressed);
}
