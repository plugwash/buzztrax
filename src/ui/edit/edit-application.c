/* $Id: edit-application.c,v 1.96 2007-03-17 22:50:18 ensonic Exp $
 *
 * Buzztard
 * Copyright (C) 2006 Buzztard team <buzztard-devel@lists.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION:bteditapplication
 * @short_description: class for a gtk based buzztard editor application
 * @see_also: #BtMainWindow
 *
 * Opens the #BtMainWindow and provide application level function like load,
 * save, run and exit.
 */

#define BT_EDIT
#define BT_EDIT_APPLICATION_C

#include "bt-edit.h"
#include <libbtcore/application-private.h>

//-- signal ids

//-- property ids

enum {
  EDIT_APPLICATION_SONG=1,
  EDIT_APPLICATION_MAIN_WINDOW,
  EDIT_APPLICATION_IC_REGISTRY
};

// this needs to be here because of gtk-doc and unit-tests
GST_DEBUG_CATEGORY(GST_CAT_DEFAULT);

struct _BtEditApplicationPrivate {
  /* used to validate if dispose has run */
  gboolean dispose_has_run;

  /* the currently loaded song */
  BtSong *song;
  /* shared ui ressources */
  BtUIRessources *ui_ressources;
  /* the top-level window of our app */
  BtMainWindow *main_window;

  /* remote playback controller */
  BtPlaybackControllerSocket *pb_controller;

  /* interaction controller registry */
  BtIcRegistry *ic_registry;
};

static BtApplicationClass *parent_class=NULL;

//-- event handler

static void on_songio_status_changed(BtSongIO *songio,GParamSpec *arg,gpointer user_data) {
  BtEditApplication *self=BT_EDIT_APPLICATION(user_data);
  BtMainStatusbar *statusbar;
  gchar *str;

  g_assert(BT_IS_SONG_IO(songio));
  g_assert(user_data);

  g_object_get(self->priv->main_window,"statusbar",&statusbar,NULL);

  /* @todo push loader status changes into the statusbar
   * - how to handle to push and pop stuff, first_acces=push_only, last_access=pop_only
   *   - str!=NULL old.pop & new.push
   *   - str==NULL old.pop & default.push
   */
  g_object_get(songio,"status",&str,NULL);
  GST_INFO("songio_status has changed : \"%s\"",safe_string(str));
  g_object_set(statusbar,"status",str,NULL);
  g_object_try_unref(statusbar);
  g_free(str);
}

//-- helper methods

/*
 * bt_edit_application_check_missing:
 * @self: the edit application
 *
 * Run gstreamer element checks. If elements are missing, if shows a dialog with
 * element-names and brief description what will not work.
 *
 * Returns: %TRUE is no critical elements are missing
 */
static gboolean bt_edit_application_check_missing(const BtEditApplication *self) {
  GList *missing_core_elements,*missing_edit_elements=NULL,*missing_elements;
  GList *edit_elements=NULL;
  gboolean res=TRUE,missing=FALSE;

  if((missing_core_elements=bt_gst_check_core_elements())) {
    missing=TRUE;res=FALSE;
  }
  // @todo check 'formats' -> rendering (core?)
  edit_elements=g_list_prepend(NULL,"level");
  if((missing_elements=bt_gst_check_elements(edit_elements))) {
    missing_edit_elements=g_list_concat(missing_edit_elements,g_list_copy(missing_elements));
    missing_edit_elements=g_list_append(missing_edit_elements,_("-> You will not see any level-meters."));
    missing=TRUE;
  }
  g_list_free(edit_elements);
  edit_elements=g_list_prepend(NULL,"spectrum");
  if((missing_elements=bt_gst_check_elements(edit_elements))) {
    missing_edit_elements=g_list_concat(missing_edit_elements,g_list_copy(missing_elements));
    missing_edit_elements=g_list_append(missing_edit_elements,_("-> You will not see the frequency spectrum in the analyzer window."));
    missing=TRUE;
  }
  g_list_free(edit_elements);
  // DEBUG test if it works
#if 0
  edit_elements=g_list_prepend(NULL,"ploink");
  if((missing_elements=bt_gst_check_elements(edit_elements))) {
    missing_edit_elements=g_list_concat(missing_edit_elements,g_list_copy(missing_elements));
    missing_edit_elements=g_list_append(missing_edit_elements,"-> You will not be able to ploink.");
    missing=TRUE;
  }
  g_list_free(edit_elements);
  edit_elements=g_list_prepend(NULL,"grummel");
  edit_elements=g_list_prepend(edit_elements,"groll");
  if((missing_elements=bt_gst_check_elements(edit_elements))) {
    missing_edit_elements=g_list_concat(missing_edit_elements,g_list_copy(missing_elements));
    missing_edit_elements=g_list_append(missing_edit_elements,"-> You will not be able to grummel and groll.");
    missing=TRUE;
  }
  g_list_free(edit_elements);
#endif
  // DEBUG
  // show missing dialog
  if(missing) {
    /* @todo add checkbox 'don't show again'
     * if only non-critical elements are missing
     */
    GtkWidget *label,*icon,*hbox,*vbox;
    gchar *str;
    GtkWidget *dialog;

    // @todo: move to new class
    dialog = gtk_dialog_new_with_buttons(_("Missing GStreamer elements"),
                                          GTK_WINDOW(self->priv->main_window),
                                          GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                          NULL);

    hbox=gtk_hbox_new(FALSE,12);
    gtk_container_set_border_width(GTK_CONTAINER(hbox),6);

    icon=gtk_image_new_from_stock(res?GTK_STOCK_DIALOG_WARNING:GTK_STOCK_DIALOG_ERROR,GTK_ICON_SIZE_DIALOG);
    gtk_container_add(GTK_CONTAINER(hbox),icon);

    vbox=gtk_vbox_new(FALSE,6);
    label=gtk_label_new(NULL);
    str=g_strdup_printf("<big><b>%s</b></big>",_("Missing GStreamer elemnts"));
    gtk_label_set_markup(GTK_LABEL(label),str);
    gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
    g_free(str);
    gtk_container_add(GTK_CONTAINER(vbox),label);
    if(missing_core_elements) {
      GList *node;
      GtkWidget *missing_list, *missing_list_view;
      gchar *missing_text,*ptr;
      gint length=0;

      label=gtk_label_new(_("The elements listed below are missing from you installation, but are required."));
      gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
      gtk_container_add(GTK_CONTAINER(vbox),label);

      for(node=missing_core_elements;node;node=g_list_next(node)) {
        length+=2+strlen((gchar *)(node->data));
      }
      ptr=missing_text=g_malloc(length);
      for(node=missing_core_elements;node;node=g_list_next(node)) {
        length=g_sprintf(ptr,"%s\n",(gchar *)(node->data));
        ptr=&ptr[length];
      }

      missing_list = gtk_text_view_new();
      gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(missing_list), FALSE);
      gtk_text_view_set_editable(GTK_TEXT_VIEW(missing_list), FALSE);
      gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(missing_list), GTK_WRAP_WORD);
      gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(missing_list)),missing_text,-1);
      gtk_widget_show(missing_list);
      g_free(missing_text);

      missing_list_view = gtk_scrolled_window_new(NULL, NULL);
      gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (missing_list_view), GTK_SHADOW_IN);
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (missing_list_view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_container_add(GTK_CONTAINER(missing_list_view), missing_list);
      gtk_widget_show(missing_list_view);
      gtk_container_add(GTK_CONTAINER(vbox),missing_list_view);
    }
    if(missing_edit_elements) {
      GList *node;
      GtkWidget *missing_list, *missing_list_view;
      gchar *missing_text,*ptr;
      gint length=0;

      label=gtk_label_new(_("The elements listed below are missing from you installation, but are recommended for full functionality."));
      gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
      gtk_container_add(GTK_CONTAINER(vbox),label);

      for(node=missing_edit_elements;node;node=g_list_next(node)) {
        length+=2+strlen((gchar *)(node->data));
      }
      ptr=missing_text=g_malloc(length);
      for(node=missing_edit_elements;node;node=g_list_next(node)) {
        length=g_sprintf(ptr,"%s\n",(gchar *)(node->data));
        ptr=&ptr[length];
      }

      missing_list = gtk_text_view_new();
      gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(missing_list), FALSE);
      gtk_text_view_set_editable(GTK_TEXT_VIEW(missing_list), FALSE);
      gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(missing_list), GTK_WRAP_WORD);
      gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(missing_list)),missing_text,-1);
      gtk_widget_show(missing_list);
      g_free(missing_text);

      missing_list_view = gtk_scrolled_window_new(NULL, NULL);
      gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (missing_list_view), GTK_SHADOW_IN);
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (missing_list_view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_container_add(GTK_CONTAINER(missing_list_view), missing_list);
      gtk_widget_show(missing_list_view);
      gtk_container_add(GTK_CONTAINER(vbox),missing_list_view);
    }
    gtk_container_add(GTK_CONTAINER(hbox),vbox);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
  g_list_free(missing_core_elements);
  g_list_free(missing_edit_elements);
  return(res);
}

/*
 * bt_edit_application_run_ui:
 * @self: the edit application
 *
 * Run the user interface. This checks if the users runs a new version for the
 * first time. In this case it show the about dialog. It also runs the gstreamer
 * element checks.
 *
 * Returns: %TRUE for success
 */
static gboolean bt_edit_application_run_ui(const BtEditApplication *self) {
  BtSettings *settings;
  guint version;
  gboolean res;

  g_assert(self);
  g_assert(self->priv->main_window);

  GST_INFO("application.run_ui launched");

  g_object_get(G_OBJECT(self),"settings",&settings,NULL);
  g_object_get(G_OBJECT(settings),"news-seen",&version,NULL);

  if(PACKAGE_VERSION_NUMBER>version) {
    // show about
    bt_edit_application_show_about(self);
    // store new version
    version=PACKAGE_VERSION_NUMBER;
    g_object_set(G_OBJECT(settings),"news-seen",version,NULL);
  }
  g_object_unref(settings);

  // check for missing elements
  if((res=bt_edit_application_check_missing(self))) {
    res=bt_main_window_run(self->priv->main_window);
  }

  GST_INFO("application.run_ui finished : %d",res);
  return(res);
}

static void on_about_dialog_url_clicked(GtkAboutDialog *about, const gchar *link, gpointer user_data) {
  gnome_vfs_url_show(link);
}

//-- constructor methods

/**
 * bt_edit_application_new:
 *
 * Create a new instance
 *
 * Returns: the new instance or %NULL in case of an error
 */
BtEditApplication *bt_edit_application_new(void) {
  BtEditApplication *self;

  if(!(self=BT_EDIT_APPLICATION(g_object_new(BT_TYPE_EDIT_APPLICATION,NULL)))) {
    goto Error;
  }
  if(!bt_application_new(BT_APPLICATION(self))) {
    goto Error;
  }
  // create or ref the shared ui ressources
  if(!(self->priv->ui_ressources=bt_ui_ressources_new())) {
    goto Error;
  }
  // create the playback controller
  self->priv->pb_controller=bt_playback_controller_socket_new(self);
  // create the interaction controller registry
  self->priv->ic_registry=btic_registry_new();
  // create main window
  GST_INFO("new edit app created, app->ref_ct=%d",G_OBJECT(self)->ref_count);
  if(!(self->priv->main_window=bt_main_window_new(self))) {
    goto Error;
  }
#ifdef USE_HILDON
  hildon_program_add_window(HILDON_PROGRAM(hildon_program_get_instance()),
    HILDON_WINDOW(self->priv->main_window));
#endif
  GST_INFO("new edit app window created, app->ref_ct=%d",G_OBJECT(self)->ref_count);
  return(self);
Error:
  GST_WARNING("new edit app failed");
  g_object_try_unref(self);
  return(NULL);
}

//-- methods

/**
 * bt_edit_application_new_song:
 * @self: the application instance to create a new song in
 *
 * Creates a new blank song instance. If there is a previous song instance it
 * will be freed.
 *
 * Returns: %TRUE for success
 */
gboolean bt_edit_application_new_song(const BtEditApplication *self) {
  gboolean res=FALSE;
  BtSong *song;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  // create new song
  if((song=bt_song_new(BT_APPLICATION(self)))) {
    BtSetup *setup;
    BtSequence *sequence;
    BtMachine *machine;
    gchar *id;

    // free previous song
    //g_object_set(G_OBJECT(self),"song",NULL,NULL);

    g_object_get(song,"setup",&setup,"sequence",&sequence,NULL);
    // add some initial timelines
    g_object_set(sequence,"length",SEQUENCE_ROW_ADDITION_INTERVAL,NULL);
    // add audiosink
    id=bt_setup_get_unique_machine_id(setup,"master");
    if((machine=BT_MACHINE(bt_sink_machine_new(song,id)))) {
      GHashTable *properties;

      GST_DEBUG("sink-machine-refs: %d",(G_OBJECT(machine))->ref_count);
      g_object_get(machine,"properties",&properties,NULL);
      if(properties) {
        gchar str[G_ASCII_DTOSTR_BUF_SIZE];
        g_hash_table_insert(properties,g_strdup("xpos"),g_strdup(g_ascii_dtostr(str,G_ASCII_DTOSTR_BUF_SIZE,0.0)));
        g_hash_table_insert(properties,g_strdup("ypos"),g_strdup(g_ascii_dtostr(str,G_ASCII_DTOSTR_BUF_SIZE,0.0)));
      }
      if(bt_machine_enable_input_level(machine) &&
        bt_machine_enable_input_gain(machine)
      ) {
        GST_DEBUG("sink-machine-refs: %d",(G_OBJECT(machine))->ref_count);
        // set new song in application
        bt_song_set_unsaved(song,FALSE);
        g_object_set(G_OBJECT(self),"song",song,NULL);
        res=TRUE;
      }
      else {
        GST_WARNING("Can't add input level/gain element in sink machine");
      }
      GST_DEBUG("sink-machine-refs: %d",(G_OBJECT(machine))->ref_count);
      g_object_unref(machine);
    }
    else {
      GST_WARNING("Can't create sink machine");
    }
    g_free(id);

    // release references
    g_object_try_unref(setup);
    g_object_try_unref(sequence);
    g_object_unref(song);
  }
  return(res);
}

/**
 * bt_edit_application_load_song:
 * @self: the application instance to load a new song in
  *@file_name: the song filename to load
 *
 * Loads a new song. If there is a previous song instance it will be freed.
 *
 * Returns: true for success
 */
gboolean bt_edit_application_load_song(const BtEditApplication *self,const char *file_name) {
  gboolean res=FALSE;
  BtSongIO *loader;
  BtSong *song;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("song name = %s",file_name);

  if((loader=bt_song_io_new(file_name))) {
    GdkCursor *cursor=gdk_cursor_new(GDK_WATCH);
    GdkWindow *window=GTK_WIDGET(self->priv->main_window)->window;

    gdk_window_set_cursor(window,cursor);
    gdk_cursor_unref(cursor);
    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),FALSE);

    g_signal_connect(G_OBJECT(loader),"notify::status",G_CALLBACK(on_songio_status_changed),(gpointer)self);
    while(gtk_events_pending()) gtk_main_iteration();

    // create new song
    if((song=bt_song_new(BT_APPLICATION(self)))) {

      // free previous song
      //g_object_set(G_OBJECT(self),"song",NULL,NULL);

      if(bt_song_io_load(loader,song)) {
        BtSetup *setup;
        BtWavetable *wavetable;
        BtMachine *machine;

        g_object_get(song,"setup",&setup,"wavetable",&wavetable,NULL);
        // get sink-machine
        if((machine=bt_setup_get_machine_by_type(setup,BT_TYPE_SINK_MACHINE))) {
          if(bt_machine_enable_input_level(machine) &&
            bt_machine_enable_input_gain(machine)
          ) {
            GList *missing_machines,*missing_waves;

            // DEBUG
            //bt_song_write_to_highlevel_dot_file(song);
            // DEBUG
            // set new song
            g_object_set(G_OBJECT(self),"song",song,NULL);
            res=TRUE;
            GST_INFO("new song activated");

            // get missing element info
            g_object_get(G_OBJECT(setup),"missing-machines",&missing_machines,NULL);
            g_object_get(G_OBJECT(wavetable),"missing-waves",&missing_waves,NULL);
            // tell about missing machines and/or missing waves
            if(missing_machines || missing_waves) {
              GtkWidget *label,*icon,*hbox,*vbox;
              gchar *str;
              GtkWidget *dialog;

              // @todo: move to new class
              dialog = gtk_dialog_new_with_buttons(_("Missing elements in song"),
                                                    GTK_WINDOW(self->priv->main_window),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                                    NULL);

              hbox=gtk_hbox_new(FALSE,12);
              gtk_container_set_border_width(GTK_CONTAINER(hbox),6);

              icon=gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING,GTK_ICON_SIZE_DIALOG);
              gtk_container_add(GTK_CONTAINER(hbox),icon);

              vbox=gtk_vbox_new(FALSE,6);
              label=gtk_label_new(NULL);
              str=g_strdup_printf("<big><b>%s</b></big>",_("Missing elements in song"));
              gtk_label_set_markup(GTK_LABEL(label),str);
              gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
              g_free(str);
              gtk_container_add(GTK_CONTAINER(vbox),label);
              if(missing_machines) {
                GList *node;
                GtkWidget *missing_list, *missing_list_view;
                gchar *missing_text,*ptr;
                gint length=0;

                label=gtk_label_new(_("The machines listed below are missing or failed to load."));
                gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
                gtk_container_add(GTK_CONTAINER(vbox),label);

                for(node=missing_machines;node;node=g_list_next(node)) {
                  length+=2+strlen((gchar *)(node->data));
                }
                ptr=missing_text=g_malloc(length);
                for(node=missing_machines;node;node=g_list_next(node)) {
                  length=g_sprintf(ptr,"%s\n",(gchar *)(node->data));
                  ptr=&ptr[length];
                }

                missing_list = gtk_text_view_new();
                gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_editable(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(missing_list), GTK_WRAP_WORD);
                gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(missing_list)),missing_text,-1);
                gtk_widget_show(missing_list);
                g_free(missing_text);

                missing_list_view = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (missing_list_view), GTK_SHADOW_IN);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (missing_list_view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                gtk_container_add(GTK_CONTAINER(missing_list_view), missing_list);
                gtk_widget_show(missing_list_view);
                gtk_container_add(GTK_CONTAINER(vbox),missing_list_view);
              }
              if(missing_waves) {
                GList *node;
                GtkWidget *missing_list, *missing_list_view;
                gchar *missing_text,*ptr;
                gint length=0;

                label=gtk_label_new(_("The waves listed below are missing or failed to load."));
                gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
                gtk_container_add(GTK_CONTAINER(vbox),label);

                for(node=missing_waves;node;node=g_list_next(node)) {
                  length+=2+strlen((gchar *)(node->data));
                }
                ptr=missing_text=g_malloc(length);
                for(node=missing_waves;node;node=g_list_next(node)) {
                  length=g_sprintf(ptr,"%s\n",(gchar *)(node->data));
                  ptr=&ptr[length];
                }

                missing_list = gtk_text_view_new();
                gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_editable(GTK_TEXT_VIEW(missing_list), FALSE);
                gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(missing_list), GTK_WRAP_WORD);
                gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(missing_list)),missing_text,-1);
                gtk_widget_show(missing_list);
                g_free(missing_text);

                missing_list_view = gtk_scrolled_window_new(NULL, NULL);
                gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (missing_list_view), GTK_SHADOW_IN);
                gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (missing_list_view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
                gtk_container_add(GTK_CONTAINER(missing_list_view), missing_list);
                gtk_widget_show(missing_list_view);
                gtk_container_add(GTK_CONTAINER(vbox),missing_list_view);
              }
              gtk_container_add(GTK_CONTAINER(hbox),vbox);
              gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),hbox);
              gtk_widget_show_all(dialog);

              gtk_dialog_run(GTK_DIALOG(dialog));
              gtk_widget_destroy(dialog);
            }
          }
          else {
            GST_WARNING("Can't add input level/gain element in sink machine");
          }
          GST_DEBUG("unreffing stuff after loading");
          g_object_unref(machine);
        }
        else {
          GST_WARNING("Can't look up sink machine");
        }
        g_object_try_unref(setup);
        g_object_unref(wavetable);
      }
      else {
        GST_ERROR("could not load song \"%s\"",file_name);
      }
      g_object_unref(song);
    }
    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),TRUE);
    gdk_window_set_cursor(window,NULL);
    g_object_unref(loader);
  }
  return(res);
}

/**
 * bt_edit_application_save_song:
 * @self: the application instance to save a song from
  *@file_name: the song filename to save
 *
 * Saves a song.
 *
 * Returns: true for success
 */
gboolean bt_edit_application_save_song(const BtEditApplication *self,const char *file_name) {
  gboolean res=FALSE;
  BtSongIO *saver;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("song name = %s",file_name);

  if((saver=bt_song_io_new(file_name))) {
    GdkCursor *cursor=gdk_cursor_new(GDK_WATCH);
    GdkWindow *window=GTK_WIDGET(self->priv->main_window)->window;

    gdk_window_set_cursor(window,cursor);
    gdk_cursor_unref(cursor);
    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),FALSE);

    g_signal_connect(G_OBJECT(saver),"notify::status",G_CALLBACK(on_songio_status_changed),(gpointer)self);
    while(gtk_events_pending()) gtk_main_iteration();
    if(bt_song_io_save(saver,self->priv->song)) {
      res=TRUE;
    }
    else {
      GST_ERROR("could not save song \"%s\"",file_name);
    }
    GST_INFO("saving done");

    gtk_widget_set_sensitive(GTK_WIDGET(self->priv->main_window),TRUE);
    gdk_window_set_cursor(window,NULL);
    g_object_unref(saver);
  }
  return(res);
}


/**
 * bt_edit_application_run:
 * @self: the application instance to run
 *
 * start the gtk based editor application
 *
 * Returns: true for success
 */
gboolean bt_edit_application_run(const BtEditApplication *self) {
  gboolean res=FALSE;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("application.run launched");

  if(bt_edit_application_new_song(self)) {
    res=bt_edit_application_run_ui(self);
  }
  GST_INFO("application.run finished");
  return(res);
}

/**
 * bt_edit_application_load_and_run:
 * @self: the application instance to run
 * @input_file_name: the file to load initially
 *
 * load the file of the supplied name and start the gtk based editor application
 *
 * Returns: true for success
 */
gboolean bt_edit_application_load_and_run(const BtEditApplication *self, const gchar *input_file_name) {
  gboolean res=FALSE;

  g_return_val_if_fail(BT_IS_EDIT_APPLICATION(self),FALSE);

  GST_INFO("application.load_and_run launched");

  if(bt_edit_application_load_song(self,input_file_name)) {
    res=bt_edit_application_run_ui(self);
  }
  else {
    GST_WARNING("loading song failed");
    // start normaly
    bt_edit_application_run(self);
    // @todo show error message
  }
  GST_INFO("application.load_and_run finished");
  return(res);
}

/**
 * bt_edit_application_show_about:
 * @self: the application instance
 *
 * Shows the applications about window
 */
void bt_edit_application_show_about(const BtEditApplication *self) {
  GtkWidget *dialog,*news,*news_view;
  static const gchar *authors[] = {
    "Stefan 'ensonic' Kost <ensonic@users.sf.net>",
    "Thomas 'waffel' Wabner <waffel@users.sf.net>",
    "Patric Schmitz  <berzerka@users.sf.net>",
    NULL
  };
  static const gchar *documenters[] = {
    "Stefan 'ensonic' Kost <ensonic@users.sf.net>",
    NULL
  };

  GST_INFO("menu about event occurred");

  /* we can get logo via icon name, so this here is just for educational purpose
  GdkPixbuf *logo;
  GError *error = NULL;
  logo=gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),"buzztard",48,0,&error);
  //logo = gdk_pixbuf_new_from_file(DATADIR""G_DIR_SEPARATOR_S"icons"G_DIR_SEPARATOR_S"hicolor"G_DIR_SEPARATOR_S"48x48"G_DIR_SEPARATOR_S"apps"G_DIR_SEPARATOR_S""PACKAGE".png",&error);
  if(!logo) {
    GST_WARNING("Couldn't load icon: %s", error->message);
    g_error_free(error);
  }
  */

  /* use GtkAboutDialog */
  dialog = gtk_about_dialog_new();
  g_object_set(dialog,
    "authors",authors,
    "comments",_("Music production environment"),
    "copyright",_("Copyright \xc2\xa9 2003-2007 Buzztard developer team"),
    "documenters", documenters,
    /* http://www.gnu.org/licenses/lgpl.html, http://www.gnu.de/lgpl-ger.html */
    "license", _(
      "This package is free software; you can redistribute it and/or "
      "modify it under the terms of the GNU Library General Public "
      "License as published by the Free Software Foundation; either "
      "version 2 of the License, or (at your option) any later version."
      "\n\n"
      "This package is distributed in the hope that it will be useful, "
      "but WITHOUT ANY WARRANTY; without even the implied warranty of "
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU "
      "Library General Public License for more details."
      "\n\n"
      "You should have received a copy of the GNU Library General Public "
      "License along with this package; if not, write to the "
      "Free Software Foundation, Inc., 59 Temple Place - Suite 330, "
      "Boston, MA 02111-1307, USA."
    ),
    "logo-icon-name",PACKAGE_NAME,
    "version", PACKAGE_VERSION,
    "website","http://www.buzztard.org",
    "wrap-license",TRUE,
    NULL);

  // install hooks for hyper-links
  gtk_about_dialog_set_email_hook(on_about_dialog_url_clicked, (gpointer)self, NULL);
  gtk_about_dialog_set_url_hook(on_about_dialog_url_clicked, (gpointer)self, NULL);

  // add the NEWS directly below copyright
  news = gtk_text_view_new();
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(news), FALSE);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(news), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(news), GTK_WRAP_WORD);
  gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(news)),
    _("This is a technical preview version. The application is not complete or end-user ready yet. "
      "The fileformat of the songs can still change.\n\n"
      "Nonetheless if you find bugs or have comments, please take your time to contact us."
    ),-1);
  gtk_widget_show(news);

  news_view = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (news_view), GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (news_view), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(news_view), news);
  gtk_widget_show(news_view);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), news_view, TRUE, TRUE, 0);

  // set parent relationship
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(self->priv->main_window));
  gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

//-- wrapper

//-- default signal handler

//-- class internals

/* returns a property for the given property_id for this object */
static void bt_edit_application_get_property(GObject      *object,
                               guint         property_id,
                               GValue       *value,
                               GParamSpec   *pspec)
{
  BtEditApplication *self = BT_EDIT_APPLICATION(object);
  return_if_disposed();
  switch (property_id) {
    case EDIT_APPLICATION_SONG: {
      g_value_set_object(value, self->priv->song);
    } break;
    case EDIT_APPLICATION_MAIN_WINDOW: {
      g_value_set_object(value, self->priv->main_window);
    } break;
    case EDIT_APPLICATION_IC_REGISTRY: {
      g_value_set_object(value, self->priv->ic_registry);
    } break;
    default: {
       G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    } break;
  }
}

/* sets the given properties for this object */
static void bt_edit_application_set_property(GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  BtEditApplication *self = BT_EDIT_APPLICATION(object);
  return_if_disposed();
  switch (property_id) {
    case EDIT_APPLICATION_SONG: {
      // DEBUG
      GstElement *bin;
      g_object_get(self,"bin",&bin,NULL);
      GST_INFO("bin->num_children=%d",GST_BIN_NUMCHILDREN(bin));
      // DEBUG

      if(self->priv->song) {
        GST_INFO("old song->ref_ct=%d",G_OBJECT(self->priv->song)->ref_count);
        g_object_unref(self->priv->song);
        // DEBUG - if new song is NULL, it should be empty now
#if 0
        {
          gint num=GST_BIN_NUMCHILDREN(bin);
          GList *node=GST_BIN_CHILDREN(bin);

          GST_INFO("bin->num_children=%d",num);
          for(;node;node=g_list_next(node)) {
            GST_INFO("  %p, ref_ct=%d, '%s'",node->data,G_OBJECT(node->data)->ref_count,GST_ELEMENT_NAME(node->data));
          }
          gst_object_unref(bin);
        }
#endif
        // DEBUG
      }

      self->priv->song=BT_SONG(g_value_dup_object(value));
      if(self->priv->song) GST_DEBUG("new song: %p, song->ref_ct=%d",self->priv->song,G_OBJECT(self->priv->song)->ref_count);
    } break;
    default: {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    } break;
  }
}

static void bt_edit_application_dispose(GObject *object) {
  BtEditApplication *self = BT_EDIT_APPLICATION(object);

  return_if_disposed();
  self->priv->dispose_has_run = TRUE;

  /* This should destory the window as this is a child of the app.
   * Problem 1: On the other hand, this *NEVER* gets called as long as the window keeps its
   * strong reference to the app.
   * Solution 1: Only use weak refs when reffing upstream objects
   */
  GST_DEBUG("!!!! self=%p, self->ref_ct=%d",self,G_OBJECT(self)->ref_count);

  if(self->priv->song) {
    GST_INFO("song->ref_ct=%d",G_OBJECT(self->priv->song)->ref_count);
    bt_song_stop(self->priv->song);
  }
  g_object_try_unref(self->priv->song);

  //if(self->priv->main_window)
    //GST_INFO("main_window->ref_ct=%d",G_OBJECT(self->priv->main_window)->ref_count);
  //g_object_try_unref(self->priv->main_window);

  g_object_try_unref(self->priv->ui_ressources);
  g_object_try_unref(self->priv->pb_controller);
  g_object_try_unref(self->priv->ic_registry);

  GST_DEBUG("  chaining up");
  G_OBJECT_CLASS(parent_class)->dispose(object);
  GST_DEBUG("  done");
}

static void bt_edit_application_finalize(GObject *object) {
  //BtEditApplication *self = BT_EDIT_APPLICATION(object);

  //GST_DEBUG("!!!! self=%p",self);

  G_OBJECT_CLASS(parent_class)->finalize(object);
  GST_DEBUG("  done");
}

static void bt_edit_application_init(GTypeInstance *instance, gpointer g_class) {
  BtEditApplication *self = BT_EDIT_APPLICATION(instance);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, BT_TYPE_EDIT_APPLICATION, BtEditApplicationPrivate);
}

static void bt_edit_application_class_init(BtEditApplicationClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  parent_class=g_type_class_peek_parent(klass);
  g_type_class_add_private(klass,sizeof(BtEditApplicationPrivate));

  gobject_class->set_property = bt_edit_application_set_property;
  gobject_class->get_property = bt_edit_application_get_property;
  gobject_class->dispose      = bt_edit_application_dispose;
  gobject_class->finalize     = bt_edit_application_finalize;

  klass->song_changed = NULL;

  g_object_class_install_property(gobject_class,EDIT_APPLICATION_SONG,
                                  g_param_spec_object("song",
                                     "song construct prop",
                                     "the song object, the wire belongs to",
                                     BT_TYPE_SONG, /* object type */
                                     G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class,EDIT_APPLICATION_MAIN_WINDOW,
                                  g_param_spec_object("main-window",
                                     "main window prop",
                                     "the main window of this application",
                                     BT_TYPE_MAIN_WINDOW, /* object type */
                                     G_PARAM_READABLE));

  g_object_class_install_property(gobject_class,EDIT_APPLICATION_IC_REGISTRY,
                                  g_param_spec_object("ic-registry",
                                     "ic registry prop",
                                     "the interaction controller registry of this application",
                                     BTIC_TYPE_REGISTRY, /* object type */
                                     G_PARAM_READABLE));
}

GType bt_edit_application_get_type(void) {
  static GType type = 0;
  if (G_UNLIKELY(type == 0)) {
    const GTypeInfo info = {
      G_STRUCT_SIZE(BtEditApplicationClass),
      NULL, // base_init
      NULL, // base_finalize
      (GClassInitFunc)bt_edit_application_class_init, // class_init
      NULL, // class_finalize
      NULL, // class_data
      G_STRUCT_SIZE(BtEditApplication),
      0,   // n_preallocs
      (GInstanceInitFunc)bt_edit_application_init, // instance_init
      NULL // value_table
    };
    type = g_type_register_static(BT_TYPE_APPLICATION,"BtEditApplication",&info,0);
  }
  return type;
}
