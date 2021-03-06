/* Buzztrax
 * Copyright (C) 2012 Stefan Sauer <ensonic@users.sf.net>
 *
 * wavereplay.c: wavetable player
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
/**
 * SECTION:wavereplay
 * @title: GstBtWaveReplay
 * @short_description: wavetable player
 *
 * Plays wavetable assets pre-loaded by the application. Unlike in tracker
 * machines, the wave is implicitly triggered at the start and one can seek in
 * the song without loosing the audio.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "plugin.h"
#include "wavereplay.h"

#define GST_CAT_DEFAULT bt_audio_debug
GST_DEBUG_CATEGORY_EXTERN (GST_CAT_DEFAULT);

enum
{
  // static class properties
  PROP_WAVE_CALLBACKS = 1,
  // dynamic class properties
  PROP_WAVE, PROP_WAVE_LEVEL,
  N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

#define PROP(name) properties[PROP_##name]

//-- the class

G_DEFINE_TYPE (GstBtWaveReplay, gstbt_wave_replay, GSTBT_TYPE_AUDIO_SYNTH);

//-- audiosynth vmethods

static void
gstbt_wave_replay_negotiate (GstBtAudioSynth * base, GstCaps * caps)
{
  GstBtWaveReplay *src = ((GstBtWaveReplay *) base);
  gint i, n = gst_caps_get_size (caps), c;

  gstbt_osc_wave_setup (src->osc);
  c = src->osc->channels;

  for (i = 0; i < n; i++) {
    gst_structure_fixate_field_nearest_int (gst_caps_get_structure (caps, i),
        "channels", c);
  }
}

static gboolean
gstbt_wave_replay_process (GstBtAudioSynth * base, GstBuffer * data,
    GstMapInfo * info)
{
  GstBtWaveReplay *src = ((GstBtWaveReplay *) base);

  if (src->osc->process) {
    gint16 *d = (gint16 *) info->data;
    guint ct = ((GstBtAudioSynth *) src)->generate_samples_per_buffer;
    guint64 off = gst_util_uint64_scale_round (GST_BUFFER_TIMESTAMP (data),
        base->info.rate, GST_SECOND);

    return src->osc->process (src->osc, off, ct, d);
  }
  return FALSE;
}

//-- interfaces

//-- gobject vmethods

static void
gstbt_wave_replay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstBtWaveReplay *src = GSTBT_WAVE_REPLAY (object);

  switch (prop_id) {
    case PROP_WAVE_CALLBACKS:
    case PROP_WAVE:
    case PROP_WAVE_LEVEL:
      g_object_set_property ((GObject *) (src->osc), pspec->name, value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gstbt_wave_replay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstBtWaveReplay *src = GSTBT_WAVE_REPLAY (object);

  switch (prop_id) {
    case PROP_WAVE:
    case PROP_WAVE_LEVEL:
      g_object_get_property ((GObject *) (src->osc), pspec->name, value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gstbt_wave_replay_dispose (GObject * object)
{
  GstBtWaveReplay *src = GSTBT_WAVE_REPLAY (object);

  g_clear_object (&src->osc);

  G_OBJECT_CLASS (gstbt_wave_replay_parent_class)->dispose (object);
}

//-- gobject type methods

static void
gstbt_wave_replay_init (GstBtWaveReplay * src)
{
  /* synth components */
  src->osc = gstbt_osc_wave_new ();
}

static void
gstbt_wave_replay_class_init (GstBtWaveReplayClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *element_class = (GstElementClass *) klass;
  GstBtAudioSynthClass *audio_synth_class = (GstBtAudioSynthClass *) klass;
  GObjectClass *component;

  audio_synth_class->process = gstbt_wave_replay_process;
  audio_synth_class->negotiate = gstbt_wave_replay_negotiate;

  gobject_class->set_property = gstbt_wave_replay_set_property;
  gobject_class->get_property = gstbt_wave_replay_get_property;
  gobject_class->dispose = gstbt_wave_replay_dispose;

  // describe us
  gst_element_class_set_static_metadata (element_class,
      "Wave Replay", "Source/Audio",
      "Wavetable player", "Stefan Sauer <ensonic@users.sf.net>");
  gst_element_class_add_metadata (element_class, GST_ELEMENT_METADATA_DOC_URI,
      "file://" DATADIR "" G_DIR_SEPARATOR_S "gtk-doc" G_DIR_SEPARATOR_S "html"
      G_DIR_SEPARATOR_S "" PACKAGE "-gst" G_DIR_SEPARATOR_S
      "GstBtWaveReplay.html");

  // register properties
  PROP (WAVE_CALLBACKS) = g_param_spec_pointer ("wave-callbacks",
      "Wavetable Callbacks", "The wave-table access callbacks",
      G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);

  component = g_type_class_ref (GSTBT_TYPE_OSC_WAVE);
  PROP (WAVE) = bt_g_param_spec_clone (component, "wave");
  PROP (WAVE_LEVEL) = bt_g_param_spec_clone (component, "wave-level");
  g_type_class_unref (component);

  g_object_class_install_properties (gobject_class, N_PROPERTIES, properties);
}
