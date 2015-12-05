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

#ifndef __GFMEXPORTER_H__
#define __GFMEXPORTER_H__

#include "mapformat.h"

#include "gfmExporter_global.h"

namespace GFMExporter {

/** Declare the plugin's class as a QObject */
class GFMEXPORTER_SHARED_EXPORT GFMExporterPlugin : public
        Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    /** Constructor... that does nothing */
    GFMExporterPlugin();

    /**
     * Plugin's actual entry point
     * 
     * @param  map      The tilemap to be exported
     * @param  filename The exported filename
     */
    bool write(const Tiled::Map *map, const QString &fileName) override;
    /** Return the occurred error */
    QString errorString() const override;
    /** ??? */
    QStringList outputFiles(const Tiled::Map *map, const QString &fileName) const override;

protected:
    /** Return the plugin's description and file type */
    QString nameFilter() const override;

private:
    /** String that constains message on error */
    QString mError;
}; /** class GFMExporterPlugin */

} /** namespace GFMExporter */

#endif /** __GFMEXPORTER_H__ */

