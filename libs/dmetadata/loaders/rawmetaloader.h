/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : RAW file metadata loader
 *
 * Copyright 2006 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */
 
#ifndef RAWMETALOADER_H
#define RAWMETALOADER_H

// C ANSI includes.

extern "C"
{
#include <stdint.h> 
}

// Qt includes.

#include <qstring.h>

// Exiv2 includes.

#include <exiv2/exif.hpp>
#include <exiv2/value.hpp>

// Local includes.

#include "dmetaloader.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RAWMetaLoader : public DMetaLoader
{

public:

    RAWMetaLoader(DMetadata* metadata);
    ~RAWMetaLoader();

    bool load(const QString& filePath);
    bool save(const QString& filePath);

    bool isReadOnly() const { return true; };
    
// -- MRW RAW file parser ----------------------------------------

public:     

    typedef struct 
    {
        const char *id ; 
        const char *model ; 
    } prd_version_t; 
    
private:
    
    typedef enum 
    {
        PRD_BAYER_RGGB = 0x0001 ,   /*   RGRGRG...
                                    *   GBGBGB...
                                    *   RGRGRG...
                                    *   GBGBGB...
                                    *   ...
                                    */
        PRD_BAYER_GBRG = 0x0004 ,   /*   GBGBGB...
                                    *   RGRGRG...
                                    *   GBGBGB...
                                    *   RGRGRG...
                                    *   ...
                                    */
    } prd_bayer_t ; 
    
    typedef enum 
    {
        PRD_STORAGE_UNPACKED = 0x52 , 
        PRD_STORAGE_PACKED   = 0x59 , 
    } prd_storage_t ; 
    
    typedef enum 
    {
        TIFF_TYPE_NONE       = 0,    
        TIFF_TYPE_BYTE       = 1,    /* 8bit unsigned */
        TIFF_TYPE_ASCII      = 2,    /* Ascii string (terminated by \0) */
        TIFF_TYPE_SHORT      = 3,    /* 16bit unsigned  */
        TIFF_TYPE_LONG       = 4,    /* 32bit unsigned  */
        TIFF_TYPE_RATIONAL   = 5,    /* 32bit/32bit unsigned  */
        TIFF_TYPE_SBYTE      = 6,    /* 8bit signed */
        TIFF_TYPE_UNDEFINED  = 7,    /* undefined (depend of tag) */
        TIFF_TYPE_SSHORT     = 8,    /* 16bit signed*/
        TIFF_TYPE_SLONG      = 9,    /* 32bit signed  */
        TIFF_TYPE_SRATIONAL  = 10,   /* 32bit/32bit signed */
        TIFF_TYPE_FLOAT      = 11,   /* 32-bit IEEE float */
        TIFF_TYPE_DOUBLE     = 12,   /* 64-bit IEEE float */
    } TIFF_TYPE_t;
    
    typedef struct 
    {
        int                   valid ;             /* Set to non-zero after loading the PRD block*/
        
        char                  versionString[9] ;  /* An ascii string of exactly 8 digits describing a version number */
        const prd_version_t * version_info ;      /* Interpreted version[]  */
        
        uint16_t               sensorHeight ;
        uint16_t               sensorWidth ;
        uint16_t               imageHeight ;
        uint16_t               imageWidth ;
        uint8_t                dataSize ;
        uint8_t                pixelSize ;
        uint8_t                storageMethod ;
        uint8_t                unknown19 ;
        uint8_t                unknown20 ;
        uint8_t                unknown21 ;
        uint8_t                unknown22 ;
        uint8_t                bayerPattern ;
    } prd_block_t ;

    const char * rawfile ; 
    
    off_t        rawsize ;                  /* Size of the raw file */
    uchar *      rawdata ;                  /* The raw file data (exactly rawsize bytes) */
    
    off_t        image_start ;              /* The position in the raw file where the image data starts */
    
    int          err_counter; 
    int          warn_counter; 
    
    off_t        tiff_base;
    int32_t      tiff_size;
    int          tiff_msb;
    
    uint32_t     exif_start;                /* start of EXIF related to tiff_base */
    uint32_t     maker_note;                /* start of MakerNote */
    
    uint32_t     camera_settings_pos_1;     /* start of CameraSettings model 1 */
    uint32_t     camera_settings_size_1;    /* size of CameraSettings model 1 */
    
    uint32_t     camera_settings_pos_2;     /* start of CameraSettings model 2 */
    uint32_t     camera_settings_size_2;    /* size of CameraSettings model 2 */
    
    uint32_t     camera_settings_pos_3;     /* start of CameraSettings model 3 */
    uint32_t     camera_settings_size_3;    /* size of CameraSettings model 3 */
    
    uint32_t     camera_settings_pos_4;     /* start of CameraSettings model 4 (Maxxum 7D)*/
    uint32_t     camera_settings_size_4;    /* size of CameraSettings model 4 */
    
    uint32_t     camera_settings_pos_0114;  /* start of CameraSettings model 0114 (Maxxum 5D) */
    uint32_t     camera_settings_size_0114; /* size of CameraSettings model 0114 */
    
    prd_block_t  prd ;                      /* The interpreted content of the PRD block */
    
    Exiv2::ExifData m_exifMetadata;    
    
private: 
    
    bool    mrw_parseMRW(const QString& filePath);
    void    mrw_load_file(const char *fname);
    void    mrw_check_valid(off_t pos, int count);
    
    char    mrw_get_8(off_t pos);
    int16_t mrw_get_16_l(off_t pos);
    int32_t mrw_get_32_l(off_t pos);
    int16_t mrw_get_16_m(off_t pos);
    int32_t mrw_get_32_m(off_t pos);
    
    void    mrw_parse_wbg_block( off_t pos, uint32_t sz );
    void    mrw_parse_rif_block( off_t pos, uint32_t sz );
    void    mrw_parse_prd_block( off_t pos, uint32_t sz );
    
    void    mrw_check_valid_tiff(off_t pos, int count);
    char    mrw_get_8_tiff(off_t pos);
    int16_t mrw_get_16_tiff(off_t pos);
    int32_t mrw_get_32_tiff(off_t pos);

    QByteArray mrw_get_ttf_tag_value(off_t pos);
    
    void    mrw_dump_ttf_tag(off_t pos);
    void    mrw_dump_exif_tag(off_t pos);
    void    mrw_dump_maker_note_tag(off_t pos);

    void    mrw_dump_camera_settings_32bit(off_t pos, uint32_t size ,int mode );
    void    mrw_dump_camera_settings_16bit(off_t pos, uint32_t size ,int mode );
    void    mrw_dump_camera_settings_4(off_t pos, uint32_t size);
    void    mrw_dump_camera_settings_0114(off_t pos, uint32_t size);
    
    void    mrw_parse_ttf_block(off_t pos, uint32_t sz);
    bool    mrw_parse_mrm_block();
};

}  // NameSpace Digikam

#endif /* RAWMETALOADER_H */
