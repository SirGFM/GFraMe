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

#include "gfmExporter.h"

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"

#include <QFile>

#if QT_VERSION >= 0x050100
#define HAS_QSAVEFILE_SUPPORT
#endif

#ifdef HAS_QSAVEFILE_SUPPORT
#include <QSaveFile>
#endif

using namespace Tiled;
using namespace GFMExporter;

/**
 * Write a tilemap layer to the output file
 * 
 * @param  file      The output file
 * @param  tileLayer The layer to be outputed
 */
#ifdef HAS_QSAVEFILE_SUPPORT
static void gfm_writeTilemap(QSaveFile &file, const TileLayer *tileLayer);
#else
static void gfm_writeTilemap(QFile &file, const TileLayer *tileLayer);
#endif

/**
 * Write a object layer to the output file
 * 
 * @param  file        The output file
 * @param  objectLayer The layer to be outputed
 */
#ifdef HAS_QSAVEFILE_SUPPORT
static void gfm_writeObjects(QSaveFile &file, const ObjectGroup *objectLayer);
#else
static void gfm_writeObjects(QFile &file, const ObjectGroup *objectLayer);
#endif

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
    const TileLayer *tileLayer;
    
    // Open the output file
#ifdef HAS_QSAVEFILE_SUPPORT
    QSaveFile file(fileName);
#else
    QFile file(fileName);
#endif
    
    // Open the file for writing
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    // First iteration, check that there's at most a single tilemap and store it
    tileLayer = 0;
    foreach (const Layer *layer, map->layers()) {
        // Check that the layer is visible
        if (!layer->isVisible())
            continue;
        
        // Check that the layer is a tilemap
        if (layer->layerType() == Layer::TileLayerType) {
            // Check that there's only a single tilemap to be exported
            if (tileLayer != 0 &&
                        tileLayer != (static_cast<const TileLayer*>(layer))) {
                mError = tr("Found more than one visible layers, but the "
                        "plugin can only handle a single layer at a time");
                return false;
            }
            
            // Retrieve the current tilemap (it will be output later)
            tileLayer = static_cast<const TileLayer*>(layer);
        }
        else if (layer->layerType() == Layer::ObjectGroupType) {
            // Do nothing on this iteration, as it must appear after the tilemap
            continue;
        }
        else {
            // TODO Any other possible layer type?
            continue;
        }
        
    }
    // Output any found tilemap at the begining of the file
    if (tileLayer) {
        // Output to the file: 'type <terrain_name> <tile_index>'
        //                     '...'
        //                     'map <width_in_tiles> <height_in_tiles>\n'
        //                     '  tile_0 tile_1 ...\n'
        //                     '  ...\n'
        //                     '  tile_0 tile_1 ... last_tile'
        gfm_writeTilemap(file, tileLayer);
    }
    
    // Second iteration, now objects and areas are exported
    foreach (const Layer *layer, map->layers()) {
        // Check that the layer is visible
        if (!layer->isVisible())
            continue;
        
        // Check that the layer is a tilemap
        if (layer->layerType() == Layer::TileLayerType) {
            // It was checked that there's at most one tilemap, so ignore this
            continue;
        }
        else if (layer->layerType() == Layer::ObjectGroupType) {
            const ObjectGroup *groupLayer;
            
            // Actually export the object layer
            groupLayer = static_cast<const ObjectGroup*>(layer);
            gfm_writeObjects(file, groupLayer);
        }
        else {
            // TODO Any other possible layer type?
            continue;
        }
    }
    
    if (file.error() != QFile::NoError) {
        mError = file.errorString();
        return false;
    }

#ifdef HAS_QSAVEFILE_SUPPORT
    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }
#endif

    file.close();
    return true;
}

/** Return the plugin's description and file type */
QString GFMExporterPlugin::nameFilter() const
{
    return tr("GFraMe tilemap (*.gfm)");
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
#ifdef HAS_QSAVEFILE_SUPPORT
static void gfm_writeTilemap(QSaveFile &file, const TileLayer *tileLayer) {
#else
static void gfm_writeTilemap(QFile &file, const TileLayer *tileLayer) {
#endif
    QSet<Tileset*>::iterator tileset;
    QSet<Tileset*> tilesets = tileLayer->usedTilesets();

    // First, export it's terrain data
    // Loop through the tilemap's iterators
    tileset = tilesets.begin();
    while (tileset != tilesets.end()) {
        QList<Tile*>::iterator tile;
        QList<Tile*> tiles = (*tileset)->tiles();

        // Loop through all of it's tiles and print the terrain info
        tile = tiles.begin();
        while (tile != tiles.end()) {
            Terrain *terrain;

            // Retrieve the terrain's index (since we always set tiles to a
            // single type, simply retrieve one of the corners)
            terrain = (*tile)->terrainAtCorner(0);
            // If we actually found a terrain, output it
            if (terrain) {
                // Output to the file: 'area <terrain_name> <tile_id>'
                file.write("type ");
                file.write(terrain->name().toLatin1());
                file.write(" ");
                file.write(QByteArray::number((*tile)->id()));
                file.write("\n");
            }

            // Iterate to the next node
            tile++;
        }

        // Iterate to the next node
        tileset++;
    }

    // Write a tilemap 'header'
    file.write("map ");
    file.write(QByteArray::number(tileLayer->width()));
    file.write(" ");
    file.write(QByteArray::number(tileLayer->height()));
    file.write("\n");

    // Write the buffer data
    for (int y = 0; y < tileLayer->height(); y++) {
        file.write("  ");
        for (int x = 0; x < tileLayer->width(); x++) {
            const Cell &cell = tileLayer->cellAt(x, y);
            const Tile *tile = cell.tile;
            const int id = tile ? tile->id() : -1;

            // Write the curren tile (or -1, if there's none)
            file.write(QByteArray::number(id));
            // Don't write a comma after the last tile
            if (y != tileLayer->height() - 1 || x < tileLayer->width() -1) {
                file.write(" ", 1);
            }
        }

        file.write("\n", 1);
    }
}

#ifdef HAS_QSAVEFILE_SUPPORT
static void gfm_writeObjects(QSaveFile &file, const ObjectGroup *objectLayer) {
#else
static void gfm_writeObjects(QFile &file, const ObjectGroup *objectLayer) {
#endif
    foreach (const MapObject *pObj, objectLayer->objects()) {
        if (pObj->shape() != MapObject::Rectangle) {
            // TODO Add suport for non-rectangulaer shapes?
            continue;
        }
        if (pObj->type().isEmpty()) {
            // TODO Do something to warn that there's an area without type?
            continue;
        }
        
        if (pObj->cell().isEmpty() && pObj->properties().isEmpty()) {
            // Output a area
            file.write("area ");
            file.write(pObj->type().toLatin1());
            file.write(" ");
            file.write(QByteArray::number(pObj->x()));
            file.write(" ");
            file.write(QByteArray::number(pObj->y()));
            file.write(" ");
            file.write(QByteArray::number(pObj->width()));
            file.write(" ");
            file.write(QByteArray::number(pObj->height()));
            file.write("\n");
        }
        else {
            QMap<QString, QString>::const_iterator it;
            
            // Output an object
            file.write("obj ");
            file.write(pObj->type().toLatin1());
            file.write(" ");
            file.write(QByteArray::number(pObj->x()));
            file.write(" ");
            file.write(QByteArray::number(pObj->y()));
            file.write(" ");
            file.write(QByteArray::number(pObj->width()));
            file.write(" ");
            file.write(QByteArray::number(pObj->height()));
            // Output all of its properties
            it = pObj->properties().begin();
            while (it != pObj->properties().end()) {
                file.write(" [ ");
                file.write(it.key().toLatin1());
                file.write(" , ");
                file.write(it.value().toLatin1());
                file.write(" ]");
                
                it++;
            }
            file.write("\n");
        }
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Gfm, GFMExporterPlugin)
#endif

