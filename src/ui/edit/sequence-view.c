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
 * SECTION:btsequenceview
 * @short_description: the editor main sequence table view
 * @see_also: #BtMainPageSequence
 *
 * This widget derives from the #GtkTreeView to additionaly draw loop- and
 * play-position bars.
 */

#define BT_EDIT
#define BT_SEQUENCE_VIEW_C

#include "bt-edit.h"

enum {
  SEQUENCE_VIEW_PLAY_POSITION=1,
  SEQUENCE_VIEW_LOOP_START,
  SEQUENCE_VIEW_LOOP_END,
  SEQUENCE_VIEW_VISIBLE_ROWS
};


struct _BtSequenceViewPrivate {
  /* used to validate if dispose has run */
  gboolean dispose_has_run;

  /* the application */
  BtEditApplication *app;

  /* position of playing pointer from 0.0 ... 1.0 */
  gdouble play_pos;

  /* position of loop range from 0.0 ... 1.0 */
  gdouble loop_start,loop_end;

  /* number of visible rows, the height of one row */
  gulong visible_rows,row_height;

  /* cache some ressources */
  GdkWindow *window;
  GdkGC *play_pos_gc,*loop_pos_gc,*end_pos_gc;
};

static GtkTreeViewClass *parent_class=NULL;

static gint8 loop_pos_dash_list[]= {4};

#define AREA_REDRAW 1

//-- event handler

//-- helper methods

static void bt_sequence_view_invalidate(const BtSequenceView *self, gdouble old_pos, gdouble new_pos) {
  gdouble h=(gdouble)(self->priv->visible_rows*self->priv->row_height);
  GdkRectangle vr;
  gint y;
  
  gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(self),&vr);

  y=(gint)(old_pos*h)-vr.y;
  gtk_widget_queue_draw_area(GTK_WIDGET(self),0,y-1,GTK_WIDGET(self)->allocation.width,2);
  y=(gint)(new_pos*h)-vr.y;
  gtk_widget_queue_draw_area(GTK_WIDGET(self),0,y-1,GTK_WIDGET(self)->allocation.width,2);
}

//-- constructor methods

/**
 * bt_sequence_view_new:
 *
 * Create a new instance
 *
 * Returns: the new instance
 */
BtSequenceView *bt_sequence_view_new(void) {
  BtSequenceView *self;

  self=BT_SEQUENCE_VIEW(g_object_new(BT_TYPE_SEQUENCE_VIEW,NULL));
  return(self);
}

//-- methods

//-- wrapper

//-- class internals

static void bt_sequence_view_realize(GtkWidget *widget) {
  BtSequenceView *self = BT_SEQUENCE_VIEW(widget);

  // first let the parent realize itslf
  if(GTK_WIDGET_CLASS(parent_class)->realize) {
    (GTK_WIDGET_CLASS(parent_class)->realize)(widget);
  }
  self->priv->window=gtk_tree_view_get_bin_window(GTK_TREE_VIEW(self));

  // allocation graphical contexts for drawing the overlay lines
  self->priv->play_pos_gc=gdk_gc_new(self->priv->window);
  gdk_gc_set_rgb_fg_color(self->priv->play_pos_gc,bt_ui_resources_get_gdk_color(BT_UI_RES_COLOR_PLAYLINE));
  gdk_gc_set_line_attributes(self->priv->play_pos_gc,2,GDK_LINE_SOLID,GDK_CAP_BUTT,GDK_JOIN_MITER);

  self->priv->loop_pos_gc=gdk_gc_new(self->priv->window);
  gdk_gc_set_rgb_fg_color(self->priv->loop_pos_gc,bt_ui_resources_get_gdk_color(BT_UI_RES_COLOR_LOOPLINE));
  gdk_gc_set_line_attributes(self->priv->loop_pos_gc,2,GDK_LINE_ON_OFF_DASH,GDK_CAP_BUTT,GDK_JOIN_MITER);
  gdk_gc_set_dashes(self->priv->loop_pos_gc,0,loop_pos_dash_list,1);

  self->priv->end_pos_gc=gdk_gc_new(self->priv->window);
  gdk_gc_set_rgb_fg_color(self->priv->end_pos_gc,bt_ui_resources_get_gdk_color(BT_UI_RES_COLOR_ENDLINE));
  gdk_gc_set_line_attributes(self->priv->end_pos_gc,2,GDK_LINE_SOLID,GDK_CAP_BUTT,GDK_JOIN_MITER);
}

static void bt_sequence_view_unrealize(GtkWidget *widget) {
  BtSequenceView *self = BT_SEQUENCE_VIEW(widget);

  // first let the parent unrealize itslf
  if(GTK_WIDGET_CLASS(parent_class)->unrealize) {
    (GTK_WIDGET_CLASS(parent_class)->unrealize)(widget);
  }

  g_object_unref(self->priv->play_pos_gc);
  self->priv->play_pos_gc=NULL;

  g_object_unref(self->priv->loop_pos_gc);
  self->priv->loop_pos_gc=NULL;
}

static gboolean bt_sequence_view_expose_event(GtkWidget *widget,GdkEventExpose *event) {
  BtSequenceView *self = BT_SEQUENCE_VIEW(widget);

  //GST_INFO("!!!! self=%p",self);

  // let the parent handle its expose
  if(GTK_WIDGET_CLASS(parent_class)->expose_event) {
    (GTK_WIDGET_CLASS(parent_class)->expose_event)(widget,event);
  }

  /* We need to check to make sure that the expose event is actually occuring on
   * the window where the table data is being drawn.  If we don't do this check,
   * row zero spanners can be drawn on top of the column headers.
   */
  if(self->priv->window==event->window) {
    gint w,y;
    gdouble h;
    GdkRectangle vr;

    if(G_UNLIKELY(!self->priv->row_height)) {
      GtkTreePath *path;
      GdkRectangle br;

      // determine row height
      path=gtk_tree_path_new_from_indices(0,-1);
      gtk_tree_view_get_background_area(GTK_TREE_VIEW(widget),path,NULL,&br);
      self->priv->row_height=br.height;
      gtk_tree_path_free(path);
      GST_INFO("view=%p, cell background rect: %d x %d, %d x %d",widget,br.x,br.y,br.width,br.height);
    }

    gtk_tree_view_get_visible_rect(GTK_TREE_VIEW(widget),&vr);
    GST_DEBUG("view=%p, visible rect: %d x %d, %d x %d, alloc: %d x %d",
      widget,vr.x,vr.y,vr.width,vr.height,widget->allocation.width,widget->allocation.height);

    //h=(gint)(self->priv->play_pos*(double)widget->allocation.height);
    //w=vr.width;
    //h=(gint)(self->priv->play_pos*(double)vr.height);

    w=widget->allocation.width;
    h=(gdouble)(self->priv->visible_rows*self->priv->row_height);

    // draw play-pos
    y=(gint)(self->priv->play_pos*h)-vr.y;
    if((y>=0) && (y<vr.height)) {
      gdk_draw_line(self->priv->window,self->priv->play_pos_gc,vr.x+0,y,vr.x+w,y);
    }

    // draw song-end
    y=(gint)(h)-(1+vr.y);
    if((y>=0) && (y<vr.height)) {
      gdk_draw_line(self->priv->window,self->priv->end_pos_gc,vr.x+0,y,vr.x+w,y);
    }

    // draw loop-start/-end
    // draw these always from 0 as they are dashed and we can't adjust the start of the dash pattern
    y=(gint)(self->priv->loop_start*h)-vr.y;
    if((y>=0) && (y<vr.height)) {
      //gdk_draw_line(self->priv->window,self->priv->loop_pos_gc,vr.x+0,y,vr.x+w,y);
      gdk_draw_line(self->priv->window,self->priv->loop_pos_gc,0,y,vr.x+w,y);
    }
    y=(gint)(self->priv->loop_end*h)-(1+vr.y);
    if((y>=0) && (y<vr.height)) {
      //gdk_draw_line(self->priv->window,self->priv->loop_pos_gc,vr.x+0,y,vr.x+w,y);
      gdk_draw_line(self->priv->window,self->priv->loop_pos_gc,0,y,vr.x+w,y);
    }
  }
  return(FALSE);
}

static void bt_sequence_view_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
  BtSequenceView *self = BT_SEQUENCE_VIEW(object);

  return_if_disposed();
  switch (property_id) {
    case SEQUENCE_VIEW_PLAY_POSITION: {
      gdouble old_pos = self->priv->play_pos;

      self->priv->play_pos = g_value_get_double(value);
      if(GTK_WIDGET_REALIZED(GTK_WIDGET(self))) {
        bt_sequence_view_invalidate(self,old_pos,self->priv->play_pos);
      }
    } break;
    case SEQUENCE_VIEW_LOOP_START: {
      gdouble old_pos = self->priv->loop_start;

      self->priv->loop_start = g_value_get_double(value);
      if(GTK_WIDGET_REALIZED(GTK_WIDGET(self))) {
        bt_sequence_view_invalidate(self,old_pos,self->priv->loop_start);
      }
    } break;
    case SEQUENCE_VIEW_LOOP_END: {
      gdouble  old_pos = self->priv->loop_end;

      self->priv->loop_end = g_value_get_double(value);
      if(GTK_WIDGET_REALIZED(GTK_WIDGET(self))) {
        bt_sequence_view_invalidate(self,old_pos,self->priv->loop_end);
      }
    } break;
    case SEQUENCE_VIEW_VISIBLE_ROWS: {
      self->priv->visible_rows = g_value_get_ulong(value);
      GST_INFO("visible-rows = %lu",self->priv->visible_rows);
      if(GTK_WIDGET_REALIZED(GTK_WIDGET(self))) {
	      gtk_widget_queue_draw(GTK_WIDGET(self));
      }
    } break;
    default: {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object,property_id,pspec);
    } break;
  }
}

static void bt_sequence_view_dispose(GObject *object) {
  BtSequenceView *self = BT_SEQUENCE_VIEW(object);
  return_if_disposed();
  self->priv->dispose_has_run = TRUE;

  GST_DEBUG("!!!! self=%p",self);
  g_object_try_unref(self->priv->play_pos_gc);
  g_object_try_unref(self->priv->loop_pos_gc);
  g_object_try_unref(self->priv->end_pos_gc);
  g_object_unref(self->priv->app);

  G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void bt_sequence_view_init(GTypeInstance *instance, gpointer g_class) {
  BtSequenceView *self = BT_SEQUENCE_VIEW(instance);

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, BT_TYPE_SEQUENCE_VIEW, BtSequenceViewPrivate);
  self->priv->app = bt_edit_application_new();
}

static void bt_sequence_view_class_init(BtSequenceViewClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS(klass);

  parent_class=g_type_class_peek_parent(klass);
  g_type_class_add_private(klass,sizeof(BtSequenceViewPrivate));

  gobject_class->set_property = bt_sequence_view_set_property;
  gobject_class->dispose      = bt_sequence_view_dispose;

  gtkwidget_class->realize = bt_sequence_view_realize;
  gtkwidget_class->unrealize = bt_sequence_view_unrealize;
  gtkwidget_class->expose_event = bt_sequence_view_expose_event;


  g_object_class_install_property(gobject_class,SEQUENCE_VIEW_PLAY_POSITION,
                                  g_param_spec_double("play-position",
                                     "play position prop.",
                                     "The current playing position as a fraction",
                                     0.0,
                                     1.0,
                                     0.0,
                                     G_PARAM_WRITABLE|G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class,SEQUENCE_VIEW_LOOP_START,
                                  g_param_spec_double("loop-start",
                                     "loop start position prop.",
                                     "The start position of the loop range as a fraction",
                                     0.0,
                                     1.0,
                                     0.0,
                                     G_PARAM_WRITABLE|G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class,SEQUENCE_VIEW_LOOP_END,
                                  g_param_spec_double("loop-end",
                                     "loop end position prop.",
                                     "The end position of the loop range as a fraction",
                                     0.0,
                                     1.0,
                                     1.0,
                                     G_PARAM_WRITABLE|G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(gobject_class,SEQUENCE_VIEW_VISIBLE_ROWS,
                                  g_param_spec_ulong("visible-rows",
                                     "visible rows prop.",
                                     "The number of currntly visible sequence rows",
                                     0,
                                     G_MAXULONG,
                                     0,
                                     G_PARAM_WRITABLE|G_PARAM_STATIC_STRINGS));
}

GType bt_sequence_view_get_type(void) {
  static GType type = 0;
  if (G_UNLIKELY(type == 0)) {
    const GTypeInfo info = {
      sizeof(BtSequenceViewClass),
      NULL, // base_init
      NULL, // base_finalize
      (GClassInitFunc)bt_sequence_view_class_init, // class_init
      NULL, // class_finalize
      NULL, // class_data
      sizeof(BtSequenceView),
      0,   // n_preallocs
      (GInstanceInitFunc)bt_sequence_view_init, // instance_init
      NULL // value_table
    };
    type = g_type_register_static(GTK_TYPE_TREE_VIEW,"BtSequenceView",&info,0);
  }
  return type;
}
