//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
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

#include "DdsImage.h"
#include "s3tc.h"

using namespace irrutils;

bool DdsImage::decodeDdsHeader()
{
    m_DdsHeader = ( DdsHeader * ) m_RawBytes;

    if( m_DdsHeader->Magic != MAKEFOURCC( 'D', 'D', 'S', ' ' ) )
        return false;

    return true;
}

bool DdsImage::decodePixels( IImage *destination )
{
    uint8_t *dest = ( uint8_t * ) destination->lock();
    uint8_t *src = ( uint8_t * ) m_RawBytes + 128;

	if (m_DdsHeader->PF.FourCC == DDS_FOURCC_DXT1)
        ff_decode_dxt1( src, dest, m_DdsHeader->Width, m_DdsHeader->Height, m_DdsHeader->Width * 4 );
    else if( m_DdsHeader->PF.FourCC == DDS_FOURCC_DXT3 )
        ff_decode_dxt3( src, dest, m_DdsHeader->Width, m_DdsHeader->Height, m_DdsHeader->Width * 4 );
	else
		ff_read_uncompressed(src, dest, m_DdsHeader->Width, m_DdsHeader->Height);
	

    destination->unlock();

    return true;
}

DdsImage::DdsImage( const char *fileName, IVideoDriver *driver )
{
    if( driver == 0 )
    {
        m_Device = createDevice( EDT_NULL, core::dimension2d<u32>(), 32, false, false, false, 0 );
        m_Driver = m_Device->getVideoDriver();
    }
    else
    {
        m_Device = 0;
        m_Driver = driver;
    }

    // Read file content
    ifstream input( fileName, ios::binary );
    input.seekg( 0, ios::end );
    m_RawLen = (unsigned int)input.tellg();
    m_RawBytes = new unsigned char[m_RawLen];
    input.seekg( 0, ios::beg );

    input.read(( char * ) m_RawBytes, m_RawLen );
    input.close();

    if( ! decodeDdsHeader() )
        throw( "failed to decode DDS image. Maybe not a DDS file or not DXT1/3 encoded" );
}

DdsImage::~DdsImage()
{
    if( m_Device != 0 )
        m_Device->drop();

    delete[] m_RawBytes;
}

IImage * DdsImage::getImage()
{
    IImage *image = m_Driver->createImage( ECF_A8R8G8B8, core::dimension2d<u32>( m_DdsHeader->Width, m_DdsHeader->Height ));
    decodePixels( image );

    return image;
}

