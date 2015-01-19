/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-01-19
 * Description : message box notification settings
 *
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DMESSAGEBOX_H
#define DMESSAGEBOX_H

// Qt includes

#include <QWidget>
#include <QString>
#include <QMessageBox>

// Local includes

#include "digikam_export.h"

class QDialog;
class QDialogButtonBox;

namespace Digikam
{

class DIGIKAM_EXPORT DMessageBox
{

public:

    /**
     * @return true if the corresponding message box should be shown.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, this method return false.
     * @param result is set to the result that was chosen the last
     * time the message box was shown.
     */
    static bool readMsgBoxShouldBeShown(const QString& dontShowAgainName);
    
    /**
     * Save the fact that the message box should not be shown again.
     * @param dontShowAgainName the name that identify the message box. If
     * empty, this method does nothing.
     * @param value the value chosen in the message box to show it again next time.
     */
    static void saveMsgBoxShouldBeShown(const QString& dontShowAgainName, bool value);

public:

    /** Show List of items into an informative message box.
     */
    static void showInformationList(QMessageBox::Icon icon,
                                    QWidget* const parent,
                                    const QString& caption,
                                    const QString& text,
                                    const QStringList& items,
                                    const QString& dontShowAgainName = QString());

public:

    /** Show a message box with Yes, No and Cancel buttons, and wait user feedback.
     *  Return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
     */
    static int showYesNoCancel(QMessageBox::Icon icon,
                               QWidget* const parent,
                               const QString& caption,
                               const QString& text,
                               const QString& dontAskAgainName = QString(),
                               bool showCancelButton = false);

    /** Show List of items to processs into a message box with Yes, No and Cancel buttons,
     *  and wait user feedback.
     *  Return QMessageBox::Yes, QMessageBox::No or QMessageBox::Cancel.
     */
    static int showYesNoCancelList(QMessageBox::Icon icon,
                                   QWidget* const parent,
                                   const QString& caption,
                                   const QString& text,
                                   const QStringList& items,
                                   const QString& dontAskAgainName = QString(),
                                   bool showCancelButton = false);

public:

    /** Show a message box with Continue and Cancel buttons, and wait user feedback.
     *  Return QMessageBox::Yes or QMessageBox::Cancel
     */
    static int showContinueCancel(QMessageBox::Icon icon,
                                  QWidget* const parent,
                                  const QString& caption,
                                  const QString& text,
                                  const QString& dontAskAgainName = QString());
   
    /** Show List of items to processs into a message box with Continue and Cancel buttons,
     *  and wait user feedback.
     *  Return QMessageBox::Yes or QMessageBox::Cancel
     */
    static int showContinueCancelList(QMessageBox::Icon icon,
                                      QWidget* const parent,
                                      const QString& caption,
                                      const QString& text,
                                      const QStringList& items,
                                      const QString& dontAskAgainName = QString());

private:

    static int createMessageBox(QDialog* const dialog,
                                QDialogButtonBox* const buttons,
                                const QIcon& icon,
                                const QString& text,
                                const QStringList& items,
                                const QString& ask,
                                bool* checkboxReturn
                               );
    
    static QIcon messageBoxIcon(QMessageBox::Icon icon);
};

}  // namespace Digikam

#endif  // DMESSAGEBOX_H
