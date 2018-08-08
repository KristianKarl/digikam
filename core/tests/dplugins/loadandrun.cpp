/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : stand alone test application for plugin loader.
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

// Qt Includes

#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QIcon>
#include <QDebug>

// Local includes

#include "dmetainfoiface.h"
#include "metaengine.h"
#include "dpluginloader.h"
#include "dplugin.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.setApplicationDescription(QLatin1String("Test application to run digiKam plugins as stand alone"));

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("list"),      QLatin1String("List all available plugins")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("n"),         QLatin1String("Name of plugin to use"),     QLatin1String("Plugin Name")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("a"),         QLatin1String("Plugin action name to run"), QLatin1String("Action Name")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("w"),         QLatin1String("Wait until plugin non-modal dialog is closed")));

    parser.process(app);

    DPluginLoader dpl;
    bool found = false;

    if (parser.isSet(QString::fromLatin1("list")))
    {
        foreach (DPlugin* const p, dpl.allPlugins())
        {
            qDebug() << "--------------------------------------------";
            qDebug() << "Id     :" << p->id();
            qDebug() << "Name   :" << p->name();
            qDebug() << "Version:" << p->version();
            qDebug() << "Desc   :" << p->description();

            QString authors;

            foreach (const DPluginAuthor& au, p->authors())
            {
                authors.append(au.toString());
                authors.append(QLatin1String(" ; "));
            }

            qDebug() << "Authors:" << authors;

            p->setup();
            QString actions;

            foreach (DPluginAction* const ac, p->actions())
            {
                actions.append(ac->toString());
                actions.append(QLatin1String(" ; "));
            }

            qDebug() << "Actions:" << actions;
        }

        return 0;
    }
    else if (parser.isSet(QString::fromLatin1("n")))
    {
        const QString name = parser.value(QString::fromLatin1("n"));
        QString action;

        if (parser.isSet(QString::fromLatin1("a")))
        {
            action = parser.value(QString::fromLatin1("a"));
        }
        else
        {
            qDebug() << "Plugin action name to run is missing...";
            qDebug() << "Use --help option for details.";
            return -1;
        }

        MetaEngine::initializeExiv2();

        foreach (DPlugin* const p, dpl.allPlugins())
        {
            if (p->id() == name)
            {
                found = true;
                p->setup();
                DPluginAction* const ac = p->findActionByName(action);

                if (ac)
                {
                    ac->trigger();

                    if (parser.isSet(QString::fromLatin1("w")))
                    {
                        app.exec();
                    }
                }
                else
                {
                    qDebug() << action << "action not found in plugin!";
                }

                break;
            }
        }

        MetaEngine::cleanupExiv2();
    }
    else
    {
        qDebug() << "Command line option not recognized...";
        qDebug() << "Use --help option for details.";
        return -1;
    }

    if (!found)
    {
        qDebug() << "Plugin not found!";
        return -1;
    }

    return 0;
}
