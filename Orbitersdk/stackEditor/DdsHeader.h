//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
/* DdsHeader.h -- Definition of the DDS File header

 Copyright (C) 2007 by Egon A. Rath

 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the author be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgement in the product documentation would be
    appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 
*/

#ifndef DDSHEADER_H
#define DDSHEADER_H

#ifndef MAKEFOURCC
#define MAKEFOURCC( ch0, ch1, ch2, ch3 ) \
    (( u32 ) ( unsigned char ) ( ch0 ) | (( u32 ) ( unsigned char ) ( ch1 ) << 8 ) | \
    (( u32 ) ( unsigned char ) ( ch2 ) << 16 ) | (( u32 ) ( unsigned char )( ch3 ) << 24 ))
#endif

#include <irrlicht.h>

using irr::u32;

namespace irrutils
{
    enum DDS_FLAGS
    {
        DDS_CAPS          = 0x00000001,
        DDS_HEIGHT        = 0x00000002,
        DDS_WIDTH         = 0x00000004,
        DDS_PITCH         = 0x00000008,
        DDS_PIXELFORMAT   = 0x00001000,
        DDS_MIPMAPCOUNT   = 0x00002000,
        DDS_LINEARSIZE    = 0x00008000,
        DDS_DEPTH         = 0x00800000,

        DDS_FOURCC        = 0x00000004,
        DDS_ALPHAPIXELS   = 0x00000001
    };

    enum DDS_FOURCC
    {
        DDS_FOURCC_DXT1   = MAKEFOURCC( 'D', 'X', 'T', '1' ),
        DDS_FOURCC_DXT3   = MAKEFOURCC( 'D', 'X', 'T', '3' )
    };

    struct PixelFormat
    {
        u32 Size;
        u32 Flags;
        u32 FourCC;
        u32 RGBBitCount;
        u32 RBitMask;
        u32 GBitMask;
        u32 BBitMask;
        u32 ABitMask;
    };

    struct DdsHeader
    {
        u32 Magic;
        u32 Size;
        u32 Flags;
        u32 Height;
        u32 Width;
        u32 PitchOrLinearSize;
        u32 Depth;
        u32 MipMapCount;
        u32 Reserved1[11];
        PixelFormat PF;
        u32 Caps1;
        u32 Caps2;
        u32 Reserved2[3];
    };
}

#endif // DDSHEADER_H
