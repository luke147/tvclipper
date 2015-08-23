/*  port.h - portability definitions
    Copyright (c) 2007 Michael Riepe
 
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

/* $Id: port.h 104 2007-11-28 13:01:51Z too-tired $ */

#ifndef _TVCLIPPER_PORT_H
#define _TVCLIPPER_PORT_H

#include <sys/types.h>
#ifndef __WIN32__
#include <sys/wait.h>
#endif
#include <stdint.h>

typedef int64_t tvclipper_off_t;
#ifndef FORMAT_PRINT_TV_CLIPPER_OFF_T
#define PRINT_FORMAT_TV_CLIPPER_OFF_T PRId64
#endif

#endif /* _TVCLIPPER_PORT_H */
