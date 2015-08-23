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

/* $Id: stream.h 164 2009-06-27 14:43:01Z too-tired $ */

#ifndef TVCLIPPER_STREAM_H
#define TVCLIPPER_STREAM_H

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "types.h"

class stream
{
protected:
    int id; //avfid;
    streamtype::type type;
    std::string infostring;
    AVCodec *dec,*enc;
    AVCodecContext *avcc;

    stream() : id(-1),type(streamtype::unknown),dec(0),enc(0),avcc(0)
    {}

    ~stream();

    void freeavcc();
    void allocavcc();
    void setvideoencodingparameters(bool interlaced=false);

    friend class mpgfile;
    friend class tsfile;
    friend class psfile;
public:
    const std::string& getinfo() const;
};

#endif
