/* $Id: main-window.h,v 1.9 2006-08-31 19:57:57 ensonic Exp $
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

#ifndef BT_MAIN_WINDOW_H
#define BT_MAIN_WINDOW_H

#include <glib.h>
#include <glib-object.h>

#define BT_TYPE_MAIN_WINDOW             (bt_main_window_get_type ())
#define BT_MAIN_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), BT_TYPE_MAIN_WINDOW, BtMainWindow))
#define BT_MAIN_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), BT_TYPE_MAIN_WINDOW, BtMainWindowClass))
#define BT_IS_MAIN_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BT_TYPE_MAIN_WINDOW))
#define BT_IS_MAIN_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BT_TYPE_MAIN_WINDOW))
#define BT_MAIN_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BT_TYPE_MAIN_WINDOW, BtMainWindowClass))

/* type macros */

typedef struct _BtMainWindow BtMainWindow;
typedef struct _BtMainWindowClass BtMainWindowClass;
typedef struct _BtMainWindowPrivate BtMainWindowPrivate;

/**
 * BtMainWindow:
 *
 * the root window for the editor application
 */
struct _BtMainWindow {
  GtkWindow parent;
  
  /*< private >*/
  BtMainWindowPrivate *priv;
};
/* structure of the main-window class */
struct _BtMainWindowClass {
  GtkWindowClass parent;
  
};

/* used by MAIN_WINDOW_TYPE */
GType bt_main_window_get_type(void) G_GNUC_CONST;

#endif // BT_MAIN_WINDOW_H
