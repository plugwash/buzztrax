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

#ifndef BT_WAVELEVEL_METHODS_H
#define BT_WAVELEVEL_METHODS_H

#include "wavelevel.h"

extern BtWavelevel *bt_wavelevel_new(const BtSong * const song, const BtWave * const wave, const guchar root_note, const gulong length, const glong loop_start, const glong loop_end, const gulong rate, gconstpointer sample);

#endif // BT_WAVELEVEL_METHDOS_H
