/* ============================================================
 * File  : imageeffect_oilpaint.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-25
 * Description : a digiKam image editor plugin to simulate 
 *               an oil painting.
 * 
 * Copyright 2004-2005 by Gilles Caulier
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

#ifndef IMAGEEFFECT_OILPAINT_H
#define IMAGEEFFECT_OILPAINT_H

// KDE include.

#include <kdialogbase.h>

class QPushButton;
class QTimer;

class KIntNumInput;

namespace Digikam
{
class ImagePreviewWidget;
}

namespace DigikamOilPaintImagesPlugin
{
class OilPaint;

class ImageEffect_OilPaint : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_OilPaint(QWidget* parent);
    ~ImageEffect_OilPaint();

protected:

    void closeEvent(QCloseEvent *e);
    
private:
    
    enum RunningMode
    {
    NoneRendering=0,
    PreviewRendering,
    FinalRendering
    };
        
    int           m_currentRenderingMode;
    
    QTimer       *m_timer;
    
    QWidget      *m_parent;
    
    QPushButton  *m_helpButton;
    
    KIntNumInput *m_brushSizeInput;
    KIntNumInput *m_smoothInput;

    OilPaint     *m_oilpaintFilter;
                
    Digikam::ImagePreviewWidget *m_imagePreviewWidget;

private:
    
    void abortPreview(void);
    void customEvent(QCustomEvent *event);
                    
private slots:

    void slotHelp();
    void slotEffect();
    void slotOk();
    void slotCancel();
    void slotUser1();
    void slotTimer(); 
    
};

}  // NameSpace DigikamOilPaintImagesPlugin

#endif /* IMAGEEFFECT_OILPAINT_H */
