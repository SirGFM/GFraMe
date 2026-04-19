/**
 * Tiled plugin for the GFraMe library
 * 
 * Based on the "CSV Tiled Plugin"
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "mapformat.h"

#include "gfmExporter_global.h"

namespace GFMExporter {

class GFMEXPORTER_SHARED_EXPORT GFMExporterPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    GFMExporterPlugin();

    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;
    QString errorString() const override;
    QStringList outputFiles(const Tiled::Map *map, const QString &fileName) const override;

    QString shortName() const override;

protected:
    QString nameFilter() const override;
}; /** class GFMExporterPlugin */

} /** namespace GFMExporter */
