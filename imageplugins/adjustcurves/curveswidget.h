/* ============================================================
 * File  : curveswidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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

#ifndef CURVESWIDGET_H
#define CURVESWIDGET_H

// Qt includes.

#include <qwidget.h>

class QCustomEvent;

namespace Digikam
{
class ImageHistogram;
}

namespace DigikamAdjustCurvesImagesPlugin
{

class CurvesWidget : public QWidget
{
Q_OBJECT

public:

enum HistogramType
{
    ValueHistogram = 0,       // Luminosity.
    RedChannelHistogram,      // Red channel.
    GreenChannelHistogram,    // Green channel.
    BlueChannelHistogram,     // Blue channel.
    AlphaChannelHistogram,    // Alpha channel.
    LinScaleHistogram,        // Linear scale.
    LogScaleHistogram         // Logarithmic scale.
};
    
private:

enum RepaintType
{
    HistogramNone = 0,        // No current histogram values calculation.
    HistogramStarted,         // Histogram values calculation started.
    HistogramCompleted,       // Histogram values calculation completed.
    HistogramFailed           // Histogram values calculation failed.
};    

public:

    CurvesWidget(int w, int h,                              // Widget size.
                 uint *i_data, uint i_w, uint i_h,          // Full image info.
                 QWidget *parent=0);
                 
    ~CurvesWidget();

    // Stop current histogram computations.
    void stopHistogramComputation(void);
    
    int   m_channelType;     // Channel type to draw.
    int   m_scaleType;       // Scale to use for drawing.

    class Digikam::ImageHistogram *m_imageHistogram;          

signals:
    
    void signalMousePressed( int );
    void signalMouseReleased( int );
    void signalHistogramComputationDone(void);
    void signalHistogramComputationFailed(void);

protected slots:
    
    void slotBlinkTimerDone( void );
            
protected:

    void paintEvent( QPaintEvent * );
    void mousePressEvent ( QMouseEvent * e );
    void mouseReleaseEvent ( QMouseEvent * e );
    void mouseMoveEvent ( QMouseEvent * e );
    
private:

    // Current selection informations.
    int     m_xmin;
    int     m_xminOrg; 
    int     m_xmax;
    int     m_clearFlag;          // Clear drawing zone with message.
    
    bool    m_blinkFlag;         
    
    QTimer *m_blinkTimer;
    
    void customEvent(QCustomEvent *event);
};

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#endif /* CURVESWIDGET_H */
