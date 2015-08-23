/*  tvclipper
    Copyright (c) 2005 Sven Over <svenover@svenover.de>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* $Id: playaudio.h 58 2007-07-23 07:03:07Z too-tired $ */

#ifndef _TVCLIPPER_PLAYAUDIO_H
#define _TVCLIPPER_PLAYAUDIO_H

#include <stdint.h>
void playaudio_ac3(const void *data, uint32_t len);
void playaudio_mp2(const void *data, uint32_t len);

#endif