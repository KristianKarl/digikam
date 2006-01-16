/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2004-2006 by Gilles Caulier
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

#include <config.h>

// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>

// Qt Includes.
 
#include <qlayout.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qhbuttongroup.h> 
#include <qpushbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ktabwidget.h>

// Local includes.

#include "dimg.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "navigatebarwidget.h"
#include "sharedloadsavethread.h"
#include "cietonguewidget.h"
#include "imagepropertiescolorstab.h"

#include LCMS_HEADER

namespace Digikam
{

class ImagePropertiesColorsTabPriv
{
public:

    ImagePropertiesColorsTabPriv()
    {
        imageLoaderThread = 0;
    }

    QComboBox             *channelCB;
    QComboBox             *colorsCB;
    QComboBox             *renderingCB;

    QHButtonGroup         *scaleBG;
    QHButtonGroup         *regionBG;
    
    QSpinBox              *minInterv;
    QSpinBox              *maxInterv;
    
    QLabel                *labelMeanValue;
    QLabel                *labelPixelsValue;
    QLabel                *labelStdDevValue;
    QLabel                *labelCountValue;
    QLabel                *labelMedianValue;
    QLabel                *labelPercentileValue;
    QLabel                *labelColorDepth;
    QLabel                *labelAlphaChannel;
    QLabel                *infoHeader;
    
    QString                currentFilePath;

    QRect                 *selectionArea;
        
    KSqueezedTextLabel    *labelICCName;
    KSqueezedTextLabel    *labelICCDescription;
    KSqueezedTextLabel    *labelICCCopyright;
    KSqueezedTextLabel    *labelICCIntent;
    KSqueezedTextLabel    *labelICCColorSpace;
    
    DImg                   image;
    DImg                   imageSelection;
    
    ColorGradientWidget   *hGradient;
    HistogramWidget       *histogramWidget;
    NavigateBarWidget     *navigateBar;
    SharedLoadSaveThread  *imageLoaderThread;
    CIETongueWidget       *cieTongue;
    
    QByteArray             embedded_profile;
};

ImagePropertiesColorsTab::ImagePropertiesColorsTab(QWidget* parent, QRect* selectionArea, bool navBar)
                           : QWidget(parent)
{
    d = new ImagePropertiesColorsTabPriv;
    
    d->selectionArea = selectionArea;
   
    QVBoxLayout *vLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    d->navigateBar       = new NavigateBarWidget(this, navBar);
    KTabWidget *tab      = new KTabWidget(this);
    vLayout->addWidget(d->navigateBar);
    vLayout->addWidget(tab);
       
    // Histogram tab area -----------------------------------------------------
       
    QWidget* histogramPage = new QWidget( tab );
    QGridLayout *topLayout = new QGridLayout(histogramPage, 8, 3, KDialog::marginHint(), KDialog::spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), histogramPage);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    d->channelCB = new QComboBox( false, histogramPage );
    d->channelCB->insertItem( i18n("Luminosity") );
    d->channelCB->insertItem( i18n("Red") );
    d->channelCB->insertItem( i18n("Green") );
    d->channelCB->insertItem( i18n("Blue") );
    d->channelCB->insertItem( i18n("Alpha") );
    d->channelCB->insertItem( i18n("Colors") );
    QWhatsThis::add( d->channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: drawing the image luminosity values.<p>"
                                       "<b>Red</b>: drawing the red image channel values.<p>"
                                       "<b>Green</b>: drawing the green image channel values.<p>"
                                       "<b>Blue</b>: drawing the blue image channel values.<p>"
                                       "<b>Alpha</b>: drawing the alpha image channel values. " 
                                       "This channel corresponding to the transparency value and "
                                       "is supported by some image formats such as PNG or GIF.<p>"
                                       "<b>Colors</b>: drawing all color channels values at the same time."));
    
    d->scaleBG = new QHButtonGroup(histogramPage);
    d->scaleBG->setExclusive(true);
    d->scaleBG->setFrameShape(QFrame::NoFrame);
    d->scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( d->scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the "
                                     "graph."));
    
    QPushButton *linHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    d->scaleBG->insert(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);
    
    QPushButton *logHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    d->scaleBG->insert(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);       
    
    QLabel *label10 = new QLabel(i18n("Colors:"), histogramPage);
    label10->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    d->colorsCB = new QComboBox( false, histogramPage );
    d->colorsCB->insertItem( i18n("Red") );
    d->colorsCB->insertItem( i18n("Green") );
    d->colorsCB->insertItem( i18n("Blue") );
    d->colorsCB->setEnabled( false );
    QWhatsThis::add( d->colorsCB, i18n("<p>Select here the main color displayed with Colors Channel mode:<p>"
                                      "<b>Red</b>: drawing the red image channel on the foreground.<p>"
                                      "<b>Green</b>: drawing the green image channel on the foreground.<p>"
                                      "<b>Blue</b>: drawing the blue image channel on the foreground.<p>"));
                                       
    d->regionBG = new QHButtonGroup(histogramPage);
    d->regionBG->setExclusive(true);
    d->regionBG->setFrameShape(QFrame::NoFrame);
    d->regionBG->setInsideMargin( 0 );
    d->regionBG->hide();
    QWhatsThis::add( d->regionBG, i18n("<p>Select here the histogram region computation:<p>"
                                      "<b>Full Image</b>: drawing histogram using the full image.<p>"
                                      "<b>Selection</b>: drawing histogram using the current image "
                                      "selection."));
    
    QPushButton *fullImageButton = new QPushButton( d->regionBG );
    QToolTip::add( fullImageButton, i18n( "<p>Full Image" ) );
    d->regionBG->insert(fullImageButton, Digikam::HistogramWidget::FullImageHistogram);
    KGlobal::dirs()->addResourceType("image-full", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-full", "image-full.png");
    fullImageButton->setPixmap( QPixmap( directory + "image-full.png" ) );
    fullImageButton->setToggleButton(true);
    
    QPushButton *SelectionImageButton = new QPushButton( d->regionBG );
    QToolTip::add( SelectionImageButton, i18n( "<p>Selection" ) );
    d->regionBG->insert(SelectionImageButton, Digikam::HistogramWidget::ImageSelectionHistogram);
    KGlobal::dirs()->addResourceType("image-selection", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("image-selection", "image-selection.png");
    SelectionImageButton->setPixmap( QPixmap( directory + "image-selection.png" ) );
    SelectionImageButton->setToggleButton(true);       
                                                                        
    topLayout->addMultiCellWidget(label1, 1, 1, 0, 0);
    topLayout->addMultiCellWidget(d->channelCB, 1, 1, 1, 1);
    topLayout->addMultiCellWidget(d->scaleBG, 1, 1, 2, 2);
    topLayout->addMultiCellWidget(label10, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(d->colorsCB, 2, 2, 1, 1);
    topLayout->addMultiCellWidget(d->regionBG, 2, 2, 2, 2);
    topLayout->setColStretch(3, 10);
    
    // -------------------------------------------------------------
    
    d->histogramWidget = new HistogramWidget(256, 140, histogramPage);
    QWhatsThis::add( d->histogramWidget, i18n("<p>This is the histogram drawing of the "
                                             "selected image channel"));
        
    d->hGradient = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histogramPage );
    d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    topLayout->addMultiCellWidget(d->histogramWidget, 3, 3, 0, 3);
    topLayout->addMultiCellWidget(d->hGradient, 4, 4, 0, 3);

    // -------------------------------------------------------------

    QHBoxLayout *hlay2 = new QHBoxLayout(KDialog::spacingHint());
    QLabel *label3 = new QLabel(i18n("Range:"), histogramPage);
    label3->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->minInterv = new QSpinBox(0, 255, 1, histogramPage);
    d->minInterv->setValue(0);
    QWhatsThis::add( d->minInterv, i18n("<p>Select here the minimal intensity "
                                       "value of the histogram selection."));    
    d->maxInterv = new QSpinBox(0, 255, 1, histogramPage);
    d->maxInterv->setValue(255);
    QWhatsThis::add( d->minInterv, i18n("<p>Select here the maximal intensity value "
                                       "of the histogram selection."));
    hlay2->addWidget(label3);
    hlay2->addWidget(d->minInterv);
    hlay2->addWidget(d->maxInterv);
    topLayout->addMultiCellLayout(hlay2, 5, 5, 0, 3);
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox = new QGroupBox(2, Qt::Horizontal, i18n("Statistics"), histogramPage);
    QWhatsThis::add( gbox, i18n("<p>Here you can see the statistic results calculated with the "
                                "selected histogram part. These values are available for all "
                                "channels."));
                                
    QLabel *label4 = new QLabel(i18n("Mean:"), gbox);
    label4->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelMeanValue = new QLabel(gbox);
    d->labelMeanValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label5 = new QLabel(i18n("Pixels:"), gbox);
    label5->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelPixelsValue = new QLabel(gbox);
    d->labelPixelsValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label6 = new QLabel(i18n("Standard deviation:"), gbox);
    label6->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelStdDevValue = new QLabel(gbox);
    d->labelStdDevValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *label7 = new QLabel(i18n("Count:"), gbox);
    label7->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelCountValue = new QLabel(gbox);
    d->labelCountValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label8 = new QLabel(i18n("Median:"), gbox);
    label8->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelMedianValue = new QLabel(gbox);
    d->labelMedianValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    QLabel *label9 = new QLabel(i18n("Percentile:"), gbox);
    label9->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelPercentileValue = new QLabel(gbox);
    d->labelPercentileValue->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    topLayout->addMultiCellWidget(gbox, 6, 6, 0, 3);

    // -------------------------------------------------------------

    QGroupBox *gbox2 = new QGroupBox(2, Qt::Horizontal, histogramPage);
    gbox2->setFrameStyle( QFrame::NoFrame );

    QLabel *label11     = new QLabel(i18n("Color depth:"), gbox2);
    label11->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelColorDepth   = new QLabel(gbox2);
    d->labelColorDepth->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *label12     = new QLabel(i18n("Alpha Channel:"), gbox2);
    label12->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    d->labelAlphaChannel = new QLabel(gbox2);
    d->labelAlphaChannel->setAlignment ( Qt::AlignLeft | Qt::AlignVCenter);
    
    topLayout->addMultiCellWidget(gbox2, 7, 7, 0, 3);

    topLayout->setRowStretch(8, 10);
    tab->addTab(histogramPage, i18n("Histogram") );

    // ICC Profiles tab area ---------------------------------------
    
    QWidget* iccprofilePage = new QWidget( tab );
    QGridLayout *iccLayout  = new QGridLayout(iccprofilePage, 9, 3, KDialog::marginHint(), KDialog::spacingHint());
    
    QGroupBox *iccbox = new QGroupBox(2, Qt::Vertical, iccprofilePage);
    iccbox->setFrameStyle (QFrame::NoFrame);

    d->infoHeader            = new QLabel(0, iccbox);
    QGroupBox *iccdetail     = new QGroupBox(2, Qt::Horizontal, iccprofilePage);
    QLabel *labelName        = new QLabel(i18n("Name: "), iccdetail);
    d->labelICCName          = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelDescription = new QLabel(i18n("Description: "), iccdetail);
    d->labelICCDescription   = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelCright      = new QLabel(i18n("Copyright: "), iccdetail);
    d->labelICCCopyright     = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelIntent      = new QLabel(i18n("Rendering Intent: "), iccdetail);
    d->labelICCIntent        = new KSqueezedTextLabel(0, iccdetail);
    QLabel *labelColor       = new QLabel(i18n("Color Space: "), iccdetail);
    d->labelICCColorSpace    = new KSqueezedTextLabel(0, iccdetail);

    d->cieTongue = new CIETongueWidget(256, 256, iccprofilePage);
    
    iccLayout->addMultiCellWidget(iccbox, 0, 0, 0, 2);
    iccLayout->addMultiCellWidget(iccdetail, 2, 7, 0, 2);
    iccLayout->addMultiCellWidget(d->cieTongue, 8, 8, 0, 2);
    iccLayout->setRowStretch(9, 10);

    tab->addTab(iccprofilePage, i18n("ICC profile") );

    // -------------------------------------------------------------

    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
                            
    // -------------------------------------------------------------
                                
    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));
    
    connect(d->scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));
    
    connect(d->colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));     
                   
    connect(d->regionBG, SIGNAL(released(int)),
            this, SLOT(slotRenderingChanged(int)));       
             
    connect(d->histogramWidget, SIGNAL(signalIntervalChanged( int, int )),
            this, SLOT(slotUpdateInterval(int, int)));
       
    connect(d->histogramWidget, SIGNAL(signalMaximumValueChanged( int )),
            this, SLOT(slotUpdateIntervRange(int)));

    connect(d->histogramWidget, SIGNAL(signalHistogramComputationDone(bool)),
            this, SLOT(slotRefreshOptions(bool)));

    connect(d->histogramWidget, SIGNAL(signalHistogramComputationFailed(void)),
            this, SLOT(slotHistogramComputationFailed(void)));
                        
    connect(d->minInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMinValueChanged(int)));

    connect(d->maxInterv, SIGNAL(valueChanged (int)),
            this, SLOT(slotMaxValueChanged(int)));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    d->channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", 0));    // Luminosity.
    d->scaleBG->setButton(config->readNumEntry("Histogram Scale", Digikam::HistogramWidget::LogScaleHistogram));
    d->colorsCB->setCurrentItem(config->readNumEntry("Histogram Color", 0));       // Red.
    d->regionBG->setButton(config->readNumEntry("Histogram Rendering", Digikam::HistogramWidget::FullImageHistogram));
}

ImagePropertiesColorsTab::~ImagePropertiesColorsTab()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the d->image data are deleted automaticly!
    d->histogramWidget->stopHistogramComputation();

    KConfig* config = kapp->config();
    config->setGroup("Image Properties SideBar");
    config->writeEntry("Histogram Channel", d->channelCB->currentItem());
    config->writeEntry("Histogram Scale", d->scaleBG->selectedId());
    config->writeEntry("Histogram Color", d->colorsCB->currentItem());
    config->writeEntry("Histogram Rendering", d->regionBG->selectedId());

    if (d->imageLoaderThread)
       delete d->imageLoaderThread;
    
    if ( d->histogramWidget )
       delete d->histogramWidget;
    
    if ( d->hGradient )
       delete d->hGradient;

    delete d;       
}

void ImagePropertiesColorsTab::setData(const KURL& url, QRect *selectionArea,
                                       DImg *img, int itemType)
{
    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.
    
    d->histogramWidget->stopHistogramComputation();
    
    if (url.isEmpty())
    {
       d->navigateBar->setFileName("");
       d->labelMeanValue->clear();
       d->labelPixelsValue->clear();
       d->labelStdDevValue->clear();
       d->labelCountValue->clear();
       d->labelMedianValue->clear();
       d->labelPercentileValue->clear();
       d->labelColorDepth->clear();
       d->labelAlphaChannel->clear();
       setEnabled(false);
       return;
    }

    setEnabled(true);
    
    d->navigateBar->setFileName(url.filename());
    d->navigateBar->setButtonsState(itemType);
    d->selectionArea = selectionArea;
    d->image.reset();
                
    if (!img)
    {
        loadImageFromUrl(url);
    }
    else 
    {
        d->image = img->copy();

        if ( !d->image.isNull() )
        {
            getICCData();

            // If a selection area is done in Image Editor and if the current image is the same 
            // in Image Editor, then compute too the histogram for this selection.
            if (d->selectionArea)
            {
                d->imageSelection = d->image.copy(*d->selectionArea);
                d->histogramWidget->updateData(d->image.bits(), d->image.width(), d->image.height(), d->image.sixteenBit(),
                                              d->imageSelection.bits(), d->imageSelection.width(),
                                              d->imageSelection.height());
                d->regionBG->show();
                updateInformations();
            }
            else 
            {
                d->histogramWidget->updateData(d->image.bits(), d->image.width(), d->image.height(), d->image.sixteenBit());
                d->regionBG->hide();
                updateInformations();
            }
        }
        else 
        {
            d->histogramWidget->setLoadingFailed();
            slotHistogramComputationFailed();
        }
    }
}

void ImagePropertiesColorsTab::loadImageFromUrl(const KURL& url)
{
    // create thread on demand
    if (!d->imageLoaderThread)
    {
        d->imageLoaderThread = new SharedLoadSaveThread();

        connect(d->imageLoaderThread, SIGNAL(signalImageLoaded(const QString&, const DImg&)),
                this, SLOT(slotLoadImageFromUrlComplete(const QString&, const DImg&)));
        //connect(d->imageLoaderThread, SIGNAL(signalImageStartedLoading(const QString&)),
          //      this, SLOT(slotStartedLoading(const QString&)));
    }

    d->currentFilePath = url.path();
    d->imageLoaderThread->load(d->currentFilePath, SharedLoadSaveThread::AccessModeRead,
                               SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    slotStartedLoading(d->currentFilePath);
}

void ImagePropertiesColorsTab::slotLoadImageFromUrlComplete(const QString& filePath, const DImg& img)
{
    // Discard any leftover messages from previous, possibly aborted loads
    if ( filePath != d->currentFilePath )
        return;

    if ( !img.isNull() )
    {
        d->histogramWidget->updateData(img.bits(), img.width(), img.height(),
                                       img.sixteenBit());
        // As a safety precaution, this must be changed only after updateData is called,
        // which stops computation because d->image.bits() is currently used by threaded histogram algorithm.
        d->image = img;
        d->regionBG->hide();
        updateInformations();
        getICCData();
    }
    else
    {
        d->histogramWidget->setLoadingFailed();
        slotHistogramComputationFailed();
    }
}

void ImagePropertiesColorsTab::slotStartedLoading(const QString& filePath)
{
    if ( filePath == d->currentFilePath )
        d->histogramWidget->setDataLoading();
}

void ImagePropertiesColorsTab::setSelection(QRect *selectionArea)
{
    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.
    
    d->histogramWidget->stopHistogramComputation();
    d->selectionArea = selectionArea;
        
    if (d->selectionArea)
    {
        d->imageSelection = d->image.copy(*d->selectionArea);
        d->histogramWidget->updateSelectionData(d->imageSelection.bits(), d->imageSelection.width(),
                                               d->imageSelection.height(), d->imageSelection.sixteenBit());
        d->regionBG->show();
    }
    else 
    {
        d->regionBG->hide();
        slotRenderingChanged(Digikam::HistogramWidget::FullImageHistogram);
    }
}

void ImagePropertiesColorsTab::slotRefreshOptions(bool sixteenBit)
{
    slotChannelChanged(d->channelCB->currentItem());
    slotScaleChanged(d->scaleBG->selectedId());
    slotColorsChanged(d->colorsCB->currentItem());
    
    if (d->selectionArea)
       slotRenderingChanged(d->regionBG->selectedId());
}

void ImagePropertiesColorsTab::slotHistogramComputationFailed()
{
    d->imageSelection.reset();
    d->image.reset();
}

void ImagePropertiesColorsTab::slotChannelChanged(int channel)
{
    switch(channel)
    {
    case RedChannel: 
        d->histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "red" ) );
        d->colorsCB->setEnabled(false);
        break;
       
    case GreenChannel:
        d->histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "green" ) );
        d->colorsCB->setEnabled(false);
        break;
          
    case BlueChannel:        
        d->histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
        d->colorsCB->setEnabled(false);
        break;

    case AlphaChannel:
        d->histogramWidget->m_channelType = Digikam::HistogramWidget::AlphaChannelHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        d->colorsCB->setEnabled(false);
        break;
          
    case ColorChannels:
        d->histogramWidget->m_channelType = Digikam::HistogramWidget::ColorChannelsHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        d->colorsCB->setEnabled(true);
        break;
                              
    default:          // Luminosity.
        d->histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
        d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
        d->colorsCB->setEnabled(false);
        break;
    }
   
    d->histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotScaleChanged(int scale)
{
    d->histogramWidget->m_scaleType = scale;
    d->histogramWidget->repaint(false);
}

void ImagePropertiesColorsTab::slotColorsChanged(int color)
{
    switch(color)
    {
    case AllColorsGreen:
        d->histogramWidget->m_colorType = Digikam::HistogramWidget::GreenColor;
        break;
       
    case AllColorsBlue:
        d->histogramWidget->m_colorType = Digikam::HistogramWidget::BlueColor;
        break;

    default:          // Red.
        d->histogramWidget->m_colorType = Digikam::HistogramWidget::RedColor;
        break;
    }

    d->histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotRenderingChanged(int rendering)
{
    d->histogramWidget->m_renderingType = rendering;
    d->histogramWidget->repaint(false);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotMinValueChanged(int min)
{
    // Called when user changes values of spin box.
    // Communicate the change to histogram widget.

    // make the one control "push" the other
    if (min == d->maxInterv->value()+1)
        d->maxInterv->setValue(min);
    d->maxInterv->setMinValue(min-1);
    d->histogramWidget->slotMinValueChanged(min);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotMaxValueChanged(int max)
{
    if (max == d->minInterv->value()-1)
        d->minInterv->setValue(max);
    d->minInterv->setMaxValue(max+1);
    d->histogramWidget->slotMaxValueChanged(max);
    updateStatistiques();
}

void ImagePropertiesColorsTab::slotUpdateInterval(int min, int max)
{
    // Called when value is set from within histogram widget.
    // Block signals to prevent slotMinValueChanged and
    // slotMaxValueChanged being called. 
    d->minInterv->blockSignals(true);
    d->minInterv->setMaxValue(max+1);
    d->minInterv->setValue(min);
    d->minInterv->blockSignals(false);

    d->maxInterv->blockSignals(true);
    d->maxInterv->setMinValue(min-1);
    d->maxInterv->setValue(max);
    d->maxInterv->blockSignals(false);

    updateStatistiques();
}

void ImagePropertiesColorsTab::slotUpdateIntervRange(int range)
{
    d->maxInterv->setMaxValue( range );
}

void ImagePropertiesColorsTab::updateInformations()
{
    d->labelColorDepth->setText(d->image.sixteenBit() ? i18n("16 bits") : i18n("8 bits"));
    d->labelAlphaChannel->setText(d->image.hasAlpha() ? i18n("Yes") : i18n("No"));
}

void ImagePropertiesColorsTab::updateStatistiques()
{
    QString value;
    int min = d->minInterv->value();
    int max = d->maxInterv->value();
    int channel = d->channelCB->currentItem();

    if ( channel == Digikam::HistogramWidget::ColorChannelsHistogram )
        channel = d->colorsCB->currentItem()+1;
               
    double mean = d->histogramWidget->m_imageHistogram->getMean(channel, min, max);
    d->labelMeanValue->setText(value.setNum(mean, 'f', 1));
    
    double pixels = d->histogramWidget->m_imageHistogram->getPixels();
    d->labelPixelsValue->setText(value.setNum((float)pixels, 'f', 0));
    
    double stddev = d->histogramWidget->m_imageHistogram->getStdDev(channel, min, max);
    d->labelStdDevValue->setText(value.setNum(stddev, 'f', 1));
      
    double counts = d->histogramWidget->m_imageHistogram->getCount(channel, min, max);
    d->labelCountValue->setText(value.setNum((float)counts, 'f', 0));
    
    double median = d->histogramWidget->m_imageHistogram->getMedian(channel, min, max);
    d->labelMedianValue->setText(value.setNum(median, 'f', 1));

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    d->labelPercentileValue->setText(value.setNum(percentile, 'f', 1));
}

void ImagePropertiesColorsTab::getICCData()
{
    if (d->image.getICCProfil().isNull())
    {
        d->infoHeader->setText(i18n("No embedded ICC profile available."));

        d->labelICCName->setText(i18n("N.A."));
        d->labelICCDescription->setText(i18n("N.A."));
        d->labelICCCopyright->setText(i18n("N.A."));
        d->labelICCIntent->setText(i18n("N.A."));
        d->labelICCColorSpace->setText(i18n("N.A."));
        d->cieTongue->setProfileData();
    }
    else
    {
        cmsHPROFILE embProfile=0;
        QString intent, colorSpace;
        d->infoHeader->setText(i18n("Embedded color profile info:"));
        
        d->embedded_profile = d->image.getICCProfil();
        d->cieTongue->setProfileData(&d->embedded_profile);
        embProfile = cmsOpenProfileFromMem(d->embedded_profile.data(),
                                          (DWORD)d->embedded_profile.size());
        d->labelICCName->setText(QString(cmsTakeProductName(embProfile)));
        d->labelICCDescription->setText(QString(cmsTakeProductDesc(embProfile)));
        d->labelICCCopyright->setText(QString(cmsTakeCopyright(embProfile)));

        switch (cmsTakeRenderingIntent(embProfile))
        {
            case 0:
                intent = i18n("Perceptual");
                break;
            case 1:
                intent = i18n("Relative Colorimetric");
                break;
            case 2:
                intent = i18n("Saturation");
                break;
            case 3:
                intent = i18n("Absolute Colorimetric");
                break;
        }

        switch (cmsGetColorSpace(embProfile))
        {
            case icSigLabData:
                colorSpace = i18n("Lab");
                break;
            case icSigLuvData:
                colorSpace = i18n("Luv");
                break;
            case icSigRgbData:
                colorSpace = i18n("RGB");
                break;
            case icSigGrayData:
                colorSpace = i18n("GRAY");
                break;
            case icSigHsvData:
                colorSpace = i18n("HSV");
                break;
            case icSigHlsData:
                colorSpace = i18n("HLS");
                break;
            case icSigCmykData:
                colorSpace = i18n("CMYK");
                break;
            case icSigCmyData:
                colorSpace= i18n("CMY");
                break;
            default:
                colorSpace = i18n("Other");
                break;
        }

        d->labelICCIntent->setText(intent);

        d->labelICCColorSpace->setText(colorSpace);

        cmsCloseProfile(embProfile);
    }
}

}  // NameSpace Digikam

#include "imagepropertiescolorstab.moc"
