/** $Id: song-io.c,v 1.1 2004-05-06 18:26:58 ensonic Exp $
 * base class for song input and output
 */
 
#define BT_CORE
#define BT_SONG_IO_C

#include <libbtcore/core.h>
#include <libbtcore/song-io-native.h>

struct _BtSongIOPrivate {
  /* used to validate if dispose has run */
  gboolean dispose_has_run;
};

//-- helper methods

static void bt_song_io_register_all() {
	/** @todo implement bt_song_io_register_all() */
}

GType bt_song_io_detect(const gchar *filename) {
	return(bt_song_io_native_get_type());
}

//-- methods

gboolean bt_song_io_real_load(const BtSongIO *self, const BtSong *song, const gchar *filename) {
	GST_ERROR("virtual method bt_song_io_real_load(self,song,\"%s\") called",filename);
	return(FALSE);	// this is a base class that can't load anything
}

//-- wrapper

gboolean bt_song_io_load(const BtSongIO *self, const BtSong *song, const gchar *filename) {
	return(BT_SONG_IO_GET_CLASS(self)->load(self,song,filename));
}

//-- class internals

/* returns a property for the given property_id for this object */
static void bt_song_io_get_property(GObject      *object,
                               guint         property_id,
                               GValue       *value,
                               GParamSpec   *pspec)
{
  BtSongIO *self = BT_SONG_IO(object);
  return_if_disposed();
  switch (property_id) {
    default: {
      g_assert(FALSE);
      break;
    }
  }
}

/* sets the given properties for this object */
static void bt_song_io_set_property(GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  BtSongIO *self = BT_SONG_IO(object);
  return_if_disposed();
  switch (property_id) {
    default: {
      g_assert(FALSE);
      break;
    }
  }
}

static void bt_song_io_dispose(GObject *object) {
  BtSongIO *self = BT_SONG_IO(object);
	return_if_disposed();
  self->private->dispose_has_run = TRUE;
}

static void bt_song_io_finalize(GObject *object) {
  BtSongIO *self = BT_SONG_IO(object);
  g_free(self->private);
}

static void bt_song_io_init(GTypeInstance *instance, gpointer g_class) {
  BtSongIO *self = BT_SONG_IO(instance);
  self->private = g_new0(BtSongIOPrivate,1);
  self->private->dispose_has_run = FALSE;
}

static void bt_song_io_class_init(BtSongIOClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  
  gobject_class->set_property = bt_song_io_set_property;
  gobject_class->get_property = bt_song_io_get_property;
  gobject_class->dispose      = bt_song_io_dispose;
  gobject_class->finalize     = bt_song_io_finalize;
	
	klass->load       = bt_song_io_real_load;
}

GType bt_song_io_get_type(void) {
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (BtSongIOClass),
      NULL, // base_init
      NULL, // base_finalize
      (GClassInitFunc)bt_song_io_class_init, // class_init
      NULL, // class_finalize
      NULL, // class_data
      sizeof (BtSongIO),
      0,   // n_preallocs
	    (GInstanceInitFunc)bt_song_io_init, // instance_init
    };
		type = g_type_register_static(G_TYPE_OBJECT,"BtSongIOType",&info,0);
  }
  return type;
}

