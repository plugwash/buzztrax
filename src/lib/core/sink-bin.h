/* Buzztrax
 * Copyright (C) 2006 Buzztrax team <buzztrax-devel@buzztrax.org>
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
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BT_SINK_BIN_H
#define BT_SINK_BIN_H

#include <glib.h>
#include <glib-object.h>

#define BT_TYPE_SINK_BIN            (bt_sink_bin_get_type ())
#define BT_SINK_BIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BT_TYPE_SINK_BIN, BtSinkBin))
#define BT_SINK_BIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BT_TYPE_SINK_BIN, BtSinkBinClass))
#define BT_IS_SINK_BIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BT_TYPE_SINK_BIN))
#define BT_IS_SINK_BIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BT_TYPE_SINK_BIN))
#define BT_SINK_BIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BT_TYPE_SINK_BIN, BtSinkBinClass))

/* type macros */

typedef struct _BtSinkBin BtSinkBin;
typedef struct _BtSinkBinClass BtSinkBinClass;
typedef struct _BtSinkBinPrivate BtSinkBinPrivate;

/**
 * BtSinkBin:
 *
 * Sub-class of a #GstBin that implements a signal output
 * (a machine with inputs only).
 */
struct _BtSinkBin {
  const GstBin parent;

  /*< private >*/
  BtSinkBinPrivate *priv;
};
/* structure of the sink_bin class */
struct _BtSinkBinClass {
  const GstBinClass parent;
};

#define BT_TYPE_SINK_BIN_MODE       (bt_sink_bin_mode_get_type())

/**
 * BtSinkBinMode:
 * @BT_SINK_BIN_MODE_PLAY: play the song
 * @BT_SINK_BIN_MODE_RECORD: record to file
 * @BT_SINK_BIN_MODE_PLAY_AND_RECORD: play and record together
 * @BT_SINK_BIN_MODE_PASS_THRU: output audio on sometimes src pad
 *
 * #BtSinkBin supports several modes of operation. Playing is the default
 * mode. Passthru is only needed if the song is plugged in another pipeline.
 */
typedef enum {
  BT_SINK_BIN_MODE_PLAY=0,
  BT_SINK_BIN_MODE_RECORD,
  BT_SINK_BIN_MODE_PLAY_AND_RECORD,
  BT_SINK_BIN_MODE_PASS_THRU
} BtSinkBinMode;

#define BT_TYPE_SINK_BIN_RECORD_FORMAT (bt_sink_bin_record_format_get_type())

/**
 * BtSinkBinRecordFormat:
 * @BT_SINK_BIN_RECORD_FORMAT_OGG_VORBIS: ogg vorbis
 * @BT_SINK_BIN_RECORD_FORMAT_MP3: mp3
 * @BT_SINK_BIN_RECORD_FORMAT_WAV: wav
 * @BT_SINK_BIN_RECORD_FORMAT_OGG_FLAC: ogg flac
 * @BT_SINK_BIN_RECORD_FORMAT_MP4_AAC: mp4 aac
 * @BT_SINK_BIN_RECORD_FORMAT_RAW: raw
 * @BT_SINK_BIN_RECORD_FORMAT_FLAC: flac
 * @BT_SINK_BIN_RECORD_FORMAT_OGG_OPUS: ogg opus
 * @BT_SINK_BIN_RECORD_FORMAT_COUNT: number of formats
 *
 * #BtSinkMachine can record audio in several formats.
 */
typedef enum {
  BT_SINK_BIN_RECORD_FORMAT_OGG_VORBIS=0,
  BT_SINK_BIN_RECORD_FORMAT_MP3,
  BT_SINK_BIN_RECORD_FORMAT_WAV,
  BT_SINK_BIN_RECORD_FORMAT_OGG_FLAC,
  BT_SINK_BIN_RECORD_FORMAT_RAW,
  BT_SINK_BIN_RECORD_FORMAT_MP4_AAC,
  BT_SINK_BIN_RECORD_FORMAT_FLAC,
  BT_SINK_BIN_RECORD_FORMAT_OGG_OPUS,
  BT_SINK_BIN_RECORD_FORMAT_COUNT
} BtSinkBinRecordFormat;

gboolean bt_sink_bin_is_record_format_supported(BtSinkBinRecordFormat format);

GType bt_sink_bin_get_type(void) G_GNUC_CONST;
GType bt_sink_bin_mode_get_type(void) G_GNUC_CONST;
GType bt_sink_bin_record_format_get_type(void) G_GNUC_CONST;

#endif // BT_SINK_BIN_H
