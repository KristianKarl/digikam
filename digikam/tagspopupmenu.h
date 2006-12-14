/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-07
 * Description : a pop-up menu implementation to diplay the 
 *               hierarchical view of digiKam tags.
 * 
 * Copyright 2004-2005 by Renchi Raju
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

#ifndef TAGSPOPUPMENU_H
#define TAGSPOPUPMENU_H

// Qt includes.

#include <qpopupmenu.h>
#include <qvaluelist.h>
#include <qpixmap.h>

namespace Digikam
{

class TAlbum;

class TagsPopupMenu : public QPopupMenu
{
    Q_OBJECT

public:

    enum Mode
    {
        ASSIGN = 0,
        REMOVE
    };
    
    TagsPopupMenu(const QValueList<Q_LLONG>& selectedImageIDs, int addToID, Mode mode);
    ~TagsPopupMenu();

signals:

    void signalTagActivated(int id);

private slots:

    void slotAboutToShow();
    void slotActivated(int id);

private:

    void        clearPopup();
    QPopupMenu* buildSubMenu(int tagid);
    void        iterateAndBuildMenu(QPopupMenu *menu, TAlbum *album);
    bool        showThisTag(int tagid);

private:

    int                 m_addToID;

    QPixmap             m_addTagPix;

    QValueList<int>     m_assignedTags;
    QValueList<Q_LLONG> m_selectedImageIDs;

    Mode                m_mode;
};

}  // namespace Digikam

#endif /* TAGSPOPUPMENU_H */
