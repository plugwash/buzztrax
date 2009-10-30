/* $Id$
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
 * SECTION:btedit
 * @short_description: buzztard graphical editor application
 * @see_also: #BtEditApplication
 *
 * Implements the body of the buzztard GUI editor.
 * 
 * You can try to run the uninstalled program via
 * <informalexample><programlisting>
 *   libtool --mode=execute buzztard-edit
 * </programlisting></informalexample>
 * to enable debug output add:
 * <informalexample><programlisting>
 *  --gst-debug="*:2,bt-*:3" for not-so-much-logdata or
 *  --gst-debug="*:2,bt-*:4" for a-lot-logdata
 * </programlisting></informalexample>
 *
 * Example songs can be found in <filename>./test/songs/</filename>.
 */

#define BT_EDIT
#define BT_EDIT_C

#include "bt-edit.h"

static void usage(int argc, char **argv, GOptionContext *ctx) {
#if GLIB_CHECK_VERSION(2,14,0)
// @idea: show as dialog?
  gchar *help=g_option_context_get_help(ctx, TRUE, NULL);
  puts(help);
  g_free(help);
#endif
}

// see if(arg_version) comment in main() below
static gboolean parse_goption_arg (const gchar * opt, const gchar * arg, gpointer data, GError ** err)
{
  if (!strcmp (opt, "--version")) {
    g_printf("%s from "PACKAGE_STRING"\n", (gchar *)data);
    exit(0);
  }
  return(TRUE);
}

int main(int argc, char **argv) {
  gboolean res=FALSE;
  //gboolean arg_version=FALSE;
  gchar *command=NULL,*input_file_name=NULL;
  BtEditApplication *app;
  GOptionContext *ctx=NULL;
  GOptionGroup *group;
  GError *err=NULL;
  
#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif /* ENABLE_NLS */

  GOptionEntry options[] = {
    {"version",     '\0', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (gpointer)parse_goption_arg,     N_("Print application version"),    NULL },
    {"command",     '\0', 0,                    G_OPTION_ARG_STRING,   &command,         N_("Command name"),    "{load}" },
    {"input-file",  '\0', 0,                    G_OPTION_ARG_FILENAME, &input_file_name, N_("Input file name"), N_("<songfile>") },
    {NULL}
  };
  
  // initialize as soon as possible
  if(!g_thread_supported()) {
    g_thread_init(NULL);
  }

  // load our custom gtk-theming
#ifndef USE_HILDON
  gtk_rc_parse(DATADIR""G_DIR_SEPARATOR_S""PACKAGE""G_DIR_SEPARATOR_S"bt-edit.gtkrc");
#else
  gtk_rc_parse(DATADIR""G_DIR_SEPARATOR_S""PACKAGE""G_DIR_SEPARATOR_S"bt-edit.hildon.gtkrc");
#endif

  // init libraries
  ctx=g_option_context_new(NULL);
  //g_option_context_add_main_entries (ctx, options, PACKAGE_NAME);
  group=g_option_group_new("main", _("buzztard-edit options"),_("Show buzztard-edit options"), argv[0], NULL);
  g_option_group_add_entries(group, options);
  g_option_group_set_translation_domain(group, PACKAGE_NAME);
  g_option_context_set_main_group(ctx, group);
  
  bt_init_add_option_groups(ctx);
  g_option_context_add_group(ctx, btic_init_get_option_group());
  g_option_context_add_group(ctx, gtk_get_option_group(TRUE));

  if(!g_option_context_parse(ctx, &argc, &argv, &err)) {
    g_print("Error initializing: %s\n", safe_string(err->message));
    g_option_context_free(ctx);
    exit(1);
  }
  // if we use this, all other libs are initialized and their options are processed
  // this causes #1777461 (fail if we don't have X)
  //if(arg_version) {
  //  g_printf("%s from "PACKAGE_STRING"\n",argv[0]);
  //  res=TRUE;
  //  goto DONE;
  //}
  GST_DEBUG_CATEGORY_INIT(GST_CAT_DEFAULT, "bt-edit", 0, "music production environment / editor ui");
  
  add_pixmap_directory(DATADIR""G_DIR_SEPARATOR_S""PACKAGE""G_DIR_SEPARATOR_S"pixmaps"G_DIR_SEPARATOR_S);

  // give some global context info
  g_set_application_name("Buzztard");
  gtk_window_set_default_icon_name("buzztard");
  g_setenv("PULSE_PROP_media.role", "production", TRUE);

  
#if GST_CHECK_VERSION(0,10,16)
  extern gboolean bt_memory_audio_src_plugin_init (GstPlugin * const plugin);
  gst_plugin_register_static(GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "memoryaudiosrc",
    "Plays audio from memory",
    bt_memory_audio_src_plugin_init,
    VERSION, "LGPL", PACKAGE, PACKAGE_NAME, "http://www.buzztard.org");
#endif

#if !GTK_CHECK_VERSION(2,14,0)
  GST_DEBUG("init gnome-vfs"); // move to UI if we have
  //-- initialize gnome-vfs
  if (!gnome_vfs_init ()) {
    GST_WARNING("gnome vfs failed to initialize");
    goto Error;
  }
#endif

  GST_INFO("starting: thread=%p",g_thread_self());

  app=bt_edit_application_new();
  
  if(command) {
    // depending on the popt options call the correct method
    if(!strncmp(command,"load",4)) {
      if(!input_file_name) {
        usage(argc, argv, ctx);
        // if commandline options where wrong, just start
        res=bt_edit_application_run(app);
      }
      else {
        res=bt_edit_application_load_and_run(app,input_file_name);
      }
    }
    else {
      usage(argc, argv, ctx);
      // if commandline options where wrong, just start
      res=bt_edit_application_run(app);
    }
  }
  else {
    res=bt_edit_application_run(app);
  }

  // free application
  GST_INFO("app->ref_ct=%d",G_OBJECT(app)->ref_count);
  g_object_unref(app);

//Done:
  g_free(command);
  g_free(input_file_name);
  g_option_context_free(ctx);
  return(!res);
}
