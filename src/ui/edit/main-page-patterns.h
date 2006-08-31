/* $Id: main-page-patterns.h,v 1.9 2006-08-31 19:57:57 ensonic Exp $
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

#ifndef BT_MAIN_PAGE_PATTERNS_H
#define BT_MAIN_PAGE_PATTERNS_H

#include <glib.h>
#include <glib-object.h>

#define BT_TYPE_MAIN_PAGE_PATTERNS            (bt_main_page_patterns_get_type ())
#define BT_MAIN_PAGE_PATTERNS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BT_TYPE_MAIN_PAGE_PATTERNS, BtMainPagePatterns))
#define BT_MAIN_PAGE_PATTERNS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BT_TYPE_MAIN_PAGE_PATTERNS, BtMainPagePatternsClass))
#define BT_IS_MAIN_PAGE_PATTERNS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BT_TYPE_MAIN_PAGE_PATTERNS))
#define BT_IS_MAIN_PAGE_PATTERNS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BT_TYPE_MAIN_PAGE_PATTERNS))
#define BT_MAIN_PAGE_PATTERNS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BT_TYPE_MAIN_PAGE_PATTERNS, BtMainPagePatternsClass))

/* type macros */

typedef struct _BtMainPagePatterns BtMainPagePatterns;
typedef struct _BtMainPagePatternsClass BtMainPagePatternsClass;
typedef struct _BtMainPagePatternsPrivate BtMainPagePatternsPrivate;

/**
 * BtMainPagePatterns:
 *
 * the pattern page for the editor application
 */
struct _BtMainPagePatterns {
  GtkVBox parent;
  
  /*< private >*/
  BtMainPagePatternsPrivate *priv;
};
/* structure of the main-page-patterns class */
struct _BtMainPagePatternsClass {
  GtkVBoxClass parent;
  
};

/* used by MAIN_PAGE_PATTERNS_TYPE */
GType bt_main_page_patterns_get_type(void) G_GNUC_CONST;

#endif // BT_MAIN_PAGE_PATTERNS_H
