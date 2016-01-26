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

/**
 * NOTE ABOUT THE EXPORTED DATA
 *
 * - If a layer (either an object layers or a tilemap) has any of "x", "y",
 * "width" or "height" properties, the layer will be offset by that amount. For
 * tilemaps, it means that only a sub-area will be exported; for object layers,
 * it means that all objects will have its position subtracted by the layer's
 * offset.
 *
 * - Still on the 'offset' topic: objects should be offset in pixels, while tile
 * layers should be offset in tiles
 *
 * - Last note on the topic: to make it easier for the user, 'width' and
 * 'height' should be specified as the last tile's position. Unfortunately, this
 * should be somewhat confusing...
 *
 * - Object's position is the exact pixel its found on the map. It doesn't get
 * automatically aligned to the grid!
 */

#include "gfmExporter.h"

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <string.h>

using namespace Tiled;
using namespace GFMExporter;

struct stBoundary {
    int height;
    int width;
    int x;
    int y;
};
typedef struct stBoundary boundary;

/**
 * Write a tilemap layer to the output file
 * 
 * @param  file      The output file
 * @param  tileLayer The layer to be outputed
 */
static void gfm_writeTilemap(QSaveFile &file, const TileLayer *tileLayer,
        boundary &b);

/**
 * Write a object layer to the output file
 * 
 * @param  file        The output file
 * @param  objectLayer The layer to be outputed
 */
static void gfm_writeObjects(QSaveFile &file, const ObjectGroup *objectLayer,
        boundary &b);

/** Constructor... that does nothing */
GFMExporterPlugin::GFMExporterPlugin()
{
}

/**
 * Plugin's actual entry point
 * 
 * @param  map      The tilemap to be exported
 * @param  filename The exported filename
 */
bool GFMExporterPlugin::write(const Map *map, const QString &fileName)
{
    boundary b;

    /* Get file paths for each layer */
    QStringList layerPaths = outputFiles(map, fileName);

    /* Export all layers (object and tile alike) */
    uint currentLayer = 0u;
    foreach (const Layer *layer, map->layers()) {
        QMap<QString, QString>::const_iterator it;

        /* Check if we know how to parse this map and if it's visible */
        if ((layer->layerType() != Layer::TileLayerType &&
                layer->layerType() != Layer::ObjectGroupType) ||
                !layer->isVisible())
            continue;

        /* Try to get the map's boundaries from its custom properties */
        memset(&b, 0x0, sizeof(boundary));
        it = layer->properties().begin();
        while (it != layer->properties().end()) {
            int *pVal;

            pVal = 0;
            if (it.key().toLatin1() == "x") {
                pVal = &(b.x);
            }
            else if (it.key().toLatin1() == "y") {
                pVal = &(b.y);
            }
            else if (it.key().toLatin1() == "width") {
                pVal = &(b.width);
            }
            else if (it.key().toLatin1() == "height") {
                pVal = &(b.height);
            }

            if (pVal) {
                bool ok;

                /* Retrieve the key's value in its own base (e.g., if it starts
                 * with "0x", its parsed as an hexadecimal) */
                *pVal = it.value().toInt(&ok, 0);

                if (ok == false) {
                    mError = "Got invalid property when parsing layer \"" +
                            layer->name() +"\"";
                    return false;
                }

                it++;
            }
        }

        /* Open a file for with the desired name appended to the layer's name */
        QSaveFile file(layerPaths.at(currentLayer));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            mError = tr("Could not open file for writing.");
            return false;
        }


        if (layer->layerType() == Layer::TileLayerType) {
            /* Export it as a tilemap */
            const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);

            gfm_writeTilemap(file, tileLayer, b);
        }
        else {
            /* Export it as an object layer */
            const ObjectGroup *groupLayer =
                    static_cast<const ObjectGroup*>(layer);
            
            gfm_writeObjects(file, groupLayer, b);
        }

        if (file.error() != QFile::NoError) {
            mError = file.errorString();
            return false;
        }

        if (!file.commit()) {
            mError = file.errorString();
            return false;
        }

        currentLayer++;
    }

    return true;
}

/** Return the plugin's description and file type */
QString GFMExporterPlugin::nameFilter() const
{
    return tr("GFraMe tilemap (*.gfm)");
}

QStringList GFMExporterPlugin::outputFiles(const Tiled::Map *map, const QString &fileName) const
{
    QStringList result;

    /* Extract file name without extension and path */
    QFileInfo fileInfo(fileName);
    const QString base = fileInfo.completeBaseName() + QLatin1String("_");
    const QString path = fileInfo.path();

    /* Loop layers to calculate the path for the exported file */
    foreach (const Layer *layer, map->layers()) {
        if ((layer->layerType() != Layer::TileLayerType &&
                layer->layerType() != Layer::ObjectGroupType) ||
                !layer->isVisible())
            continue;

        /* Get the output file name for this layer */
        const QString layerName = layer->name();
        const QString layerFileName = base + layerName + QLatin1String(".gfm");
        const QString layerFilePath = QDir(path).filePath(layerFileName);

        result.append(layerFilePath);
    }

    /* If there was only one tile layer, there's no need to change the name
     * (also keeps behavior backwards compatible) */
    if (result.size() == 1)
        result[0] = fileName;

    return result;
}

/** Return the occurred error */
QString GFMExporterPlugin::errorString() const
{
    return mError;
}

/**
 * Write a tilemap layer to the output file
 * 
 * @param  file      The output file
 * @param  tileLayer The layer to be outputed
 */
static void gfm_writeTilemap(QSaveFile &file, const TileLayer *tileLayer,
        boundary &b) {
    QSet<QSharedPointer<Tileset> >::iterator tileset;
    QSet<QSharedPointer<Tileset> > tilesets = tileLayer->usedTilesets();

    /* First, export it's terrain data */
    /* Loop through the tilemap's iterators */
    tileset = tilesets.begin();
    while (tileset != tilesets.end()) {
        QMap<int, Tile*>::iterator tile;
        QMap<int, Tile*> tiles = (*tileset)->tiles();

        /* Loop through all of it's tiles and print the terrain info */
        tile = tiles.begin();
        while (tile != tiles.end()) {
            Terrain *terrain;

            /* Retrieve the terrain's index (since we always set tiles to a
             * single type, simply retrieve one of the corners) */
            terrain = (*tile)->terrainAtCorner(0);
            /* If we actually found a terrain, output it */
            if (terrain) {
                /* Output to the file: 'area <terrain_name> <tile_id>' */
                file.write("type ");
                file.write(terrain->name().toLatin1());
                file.write(" ");
                file.write(QByteArray::number((*tile)->id()));
                file.write("\n");
            }

            /* Iterate to the next node */
            tile++;
        }

        /* Iterate to the next node */
        tileset++;
    }

    /* Fix the tilemap's dimension */
    if (b.width == 0) {
        b.width = tileLayer->width() - 1;
    }
    if (b.height == 0) {
        b.height = tileLayer->height() - 1;
    }

    /* Write a tilemap 'header' */
    file.write("map ");
    file.write(QByteArray::number(b.width - b.x));
    file.write(" ");
    file.write(QByteArray::number(b.height - b.y));
    file.write("\n");

    /* Write the buffer data */
    for (int y = b.y; y <= b.height; y++) {
        file.write("  ");
        for (int x = b.x; x <= b.width; x++) {
            const Cell &cell = tileLayer->cellAt(x, y);
            const Tile *tile = cell.tile;
            const int id = tile ? tile->id() : -1;

            /* Write the curren tile (or -1, if there's none) */
            file.write(QByteArray::number(id));
            /* Don't write a comma after the last tile */
            if (y != tileLayer->height() - 1 || x < tileLayer->width() -1) {
                file.write(" ", 1);
            }
        }

        file.write("\n", 1);
    }
}

static void gfm_writeObjects(QSaveFile &file, const ObjectGroup *objectLayer,
        boundary &b) {
    foreach (const MapObject *pObj, objectLayer->objects()) {
        if (pObj->shape() != MapObject::Rectangle) {
            /* TODO Add suport for non-rectangulaer shapes? */
            continue;
        }
        if (pObj->type().isEmpty()) {
            /* TODO Do something to warn that there's an area without type? */
            continue;
        }
        
        if (pObj->cell().isEmpty() && pObj->properties().isEmpty()) {
            /* Output a area */
            file.write("area ");
        }
        else {
            file.write("obj ");
        }
        file.write(pObj->type().toLatin1());
        file.write(" ");
        file.write(QByteArray::number(((int)pObj->x()) - b.x));
        file.write(" ");
        file.write(QByteArray::number(((int)pObj->y()) - b.y));
        file.write(" ");
        file.write(QByteArray::number((int)pObj->width()));
        file.write(" ");
        file.write(QByteArray::number((int)pObj->height()));
        if (!pObj->cell().isEmpty() || pObj->properties().isEmpty()) {
            QMap<QString, QString>::const_iterator it;
            
            /* Output all of its properties */
            it = pObj->properties().begin();
            while (it != pObj->properties().end()) {
                file.write(" [ ");
                file.write(it.key().toLatin1());
                file.write(" , ");
                file.write(it.value().toLatin1());
                file.write(" ]");
                
                it++;
            }
        }
        file.write("\n");
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Gfm, GFMExporterPlugin)
#endif

