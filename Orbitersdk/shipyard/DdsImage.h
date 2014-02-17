/* DdsImage.cpp -- Library for reading DDS Images

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

#ifndef DDSIMAGE_H
#define DDSIMAGE_H

#include <iostream>
#include <fstream>

#include <irrlicht.h>

#include "DdsHeader.h"

using namespace irr;
using namespace irr::core;
using namespace irr::video;

using std::ifstream;
using std::ios;
using std::cout;
using std::endl;
using std::hex;

namespace irrutils
{
    class DdsImage
    {
        private:
        unsigned char *m_RawBytes;
        unsigned int m_RawLen;
        IVideoDriver *m_Driver;
        IrrlichtDevice *m_Device;
        DdsHeader *m_DdsHeader;

        bool decodeDdsHeader();
        bool decodePixels( IImage *destination );

        public:
        DdsImage( const char *fileName, IVideoDriver *driver = 0 );
        ~DdsImage();
        IImage * getImage();
    };
}

#endif // DDSIMAGE_H
