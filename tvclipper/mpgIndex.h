/*  dvbcut
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

 *  tvclipper
    Copyright (c) 2015 Lukáš Vlček

    This file is part of TV Clipper.

    TV Clipper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TV Clipper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TV Clipper. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _TVCLIPPER_MPGINDEX_H
#define _TVCLIPPER_MPGINDEX_H

#include <stdint.h>
#include <byteswap.h>
#include <set>
#include <vector>
#include "types.h"
#include "pts.h"
#include "defines.h"

#define IDX_PICTYPE_I 1
#define IDX_PICTYPE_P 2
#define IDX_PICTYPE_B 3
#define IDX_PICTYPE ((char*)".IPB")

class mpgfile;
class logoutput;

/**
@author Sven Over
*/
class mpgIndex
  {
public:
  struct picture
    {
    filepos_t position;
    uint64_t pts; // additional information in the 24 most significant bits:
    // RRRQTTSSSSSSSSSSAAAAFFFF
    // R: resolution number (according to lookup table with WIDTHxHEIGHT at end of index)
    // Q: sequence header flag
    // T: picture type
    // S: picture sequence number
    // A: aspect ratio (mpeg code)
    // F: frame rate (mpeg code)

    picture(filepos_t pos, pts_t _pts, int framerate, int aspectratio,
            int sequencenumber, int picturetype, bool seqheader=false, int resolution=0) :
        position( pos ),
        pts((_pts&0xffffffffffll) | ((uint64_t)( (framerate&0xf)|((aspectratio&0xf)<<4)|
                                    ((sequencenumber&0x3ff)<<8)|((picturetype&0x3)<<18)|
                                    (seqheader?0x100000:0)|((resolution&0x7)<<21) )<<40))
      { }
    picture() : position(0), pts(0)
      {}

    filepos_t getpos() const
      {
      return position;
      }
    pts_t getpts() const
      {
      pts_t l=pts&0xffffffffffll;
      return (l ^ 0x8000000000ll) - 0x8000000000ll;
      }
    int getframerate() const
      {
      return int(pts>>40)&0xf;
      }
    int getaspectratio() const
      {
      return int(pts>>44)&0xf;
      }
    int getsequencenumber() const
      {
      return int(pts>>48)&0x3ff;
      }
    void setsequencenumber(int s)
      {
      pts=(pts&0xfc00ffffffffffffull)|(uint64_t(s&0x3ff)<<48);
      }
    int getpicturetype() const
      {
      return int(pts>>58)&0x3;
      }
    bool getseqheader() const
      {
      return pts&0x1000000000000000ll;
      }
    int getresolution() const
      {
      return int(pts>>61)&0x7;
      }
    bool isbframe() const
      {
      return getpicturetype()==IDX_PICTYPE_B;
      }
    bool ispframe() const
      {
      return getpicturetype()==IDX_PICTYPE_P;
      }
    bool isiframe() const
      {
      return getpicturetype()==IDX_PICTYPE_I;
      }

    bool operator<(picture &a) const
      {
      return getsequencenumber()<a.getsequencenumber();
      }

    };

protected:
  static picture zeroitem;
  mpgfile &mpg;
  picture *p;
  int pictures;
  int skipfirst;
  int realpictures;

  int check();
   
  // lookup-tables with found resolutions (stored at end of index file and coded in 3
  // most significant bits of picture structure, 0 for old type index files)
  std::vector<int> WIDTH, HEIGHT;  
  std::set<std::pair<int,int> > resolutions;

public:
  mpgIndex(mpgfile &m) : mpg(m),p(0),pictures(0)
    {}

  ~mpgIndex();

  int generate(const char *savefilename=0, std::string *errorstring=0, logoutput *log=0);
  int save(int fd, std::string *errorstring = 0, bool closeme = false);
  int save(const char *filename, std::string *errorstring=0);
  int load(const char *filename, std::string *errorstring=0);
  
  int getwidth(int res) const
  {
      return res>0 && res<=int(WIDTH.size()) ? WIDTH[res-1] : -1;
  }
  int getheight(int res) const
  {
      return res>0 && res<=int(HEIGHT.size()) ? HEIGHT[res-1] : -1;
  }

  int indexnr(int pic) const
    {
    pic+=skipfirst;
    int seq=pic;
    while (seq > 0 && !p[seq].getseqheader())
      --seq;
    pic-=seq;
    while(seq < pictures && p[seq].getsequencenumber()!=pic)
      ++seq;
    return seq;
    }
  int picturenr(int ind) const  // the reverse function
    {
    int pic=p[ind].getsequencenumber();
    while (ind > 0 && !p[ind].getseqheader())
      --ind;
    pic+=ind-skipfirst;
    return pic;
    }
  const picture &operator[](unsigned int i) const
    {
    return p[i];
    }
  int getpictures() const
    {
    return pictures;
    }
  int getrealpictures() const
    {
    return realpictures;
    }
  };

#endif // _TVCLIPPER_MPGINDEX_H
