/* $Id: pattern-methods.h,v 1.6 2004-12-07 20:18:47 waffel Exp $
 * defines all public methods of the pattern class
 */

#ifndef BT_PATTERN_METHODS_H
#define BT_PATTERN_METHODS_H

#include "pattern.h"

extern BtPattern *bt_pattern_new(const BtSong *song, const gchar *id, const gchar *name, glong length, glong voices,const BtMachine *machine);

extern GValue *bt_pattern_get_global_event_data(const BtPattern *self, gulong tick, glong param);
extern GValue *bt_pattern_get_voice_event_data(const BtPattern *self, gulong tick, gulong voice, glong param);

extern gulong bt_pattern_get_global_dparam_index(const BtPattern *self, const gchar *name, GError **error);
extern gulong bt_pattern_get_voice_dparam_index(const BtPattern *self, const gchar *name, GError **error);

extern void bt_pattern_init_global_event(const BtPattern *self, GValue *event, gulong param);
extern void bt_pattern_init_voice_event(const BtPattern *self, GValue *event, gulong param);

extern gboolean bt_pattern_set_event(const BtPattern *self, GValue *event, const gchar *value);

extern void bt_pattern_play_tick(const BtPattern *self, gulong index);

#endif // BT_PATTERN_METHDOS_H
