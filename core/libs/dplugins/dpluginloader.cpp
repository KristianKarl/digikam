/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpluginloader.h"
#include "dpluginloader_p.h"

// Qt includes

#include <QStringList>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

DPluginLoader::DPluginLoader(QObject* const parent)
    : QObject(parent),
      d(new Private(this))
{
    d->loadPlugins();
}

DPluginLoader::~DPluginLoader()
{
    delete d;
}

QList<DPlugin*> DPluginLoader::allPlugins() const
{
    return d->allPlugins;
}

void DPluginLoader::appendPluginToBlackList(const QString& filename)
{
    d->blacklist << QLatin1String(DIGIKAM_SHARED_LIBRARY_PREFIX) + filename;
}

void DPluginLoader::appendPluginToWhiteList(const QString& filename)
{
    d->whitelist << QLatin1String(DIGIKAM_SHARED_LIBRARY_PREFIX) + filename;
}

} // namepace Digikam
