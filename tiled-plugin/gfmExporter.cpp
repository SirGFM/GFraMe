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
    int i, foundTilemap, foundTerrain, terrainTilesetindex;
    
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
    
    // Flag that we've yet to find a terrain
    foundTerrain = 0;
    // Terrain information is stored on the tileset, so iterate over them and
    // check get how many of those have terrain (only one can have!)
    i = 0;
    while (i < map->tilesetCount()) {
        Tileset *pTset;
        
        pTset = map->tilesetAt(i);
        // Check if it has any terrain
        if (pTset->terrainCount() > 0) {
            // This tileset has a terrain, so no other can
            if (foundTerrain) {
                // If another has, set the error string and exit
                mError = tr("Found more than one tileset with terrain "
                        "information.\n For now, only a single tileset can have"
                        "it.");
                return false;
            }
            // Flag it as found
            foundTerrain = 1;
            // Also, store the index of the tileset with the terrain
            terrainTilesetindex = i;
        }
        
        i++;
    }
    
    // If we found a terrain, we must add that info to the tilemap
    if (foundTerrain) {
        Tileset *pTset;
        
        pTset = map->tilesetAt(terrainTilesetindex );
        
        // Loop through all terrains and loop their info
        i = 0;
        while (i < pTset->terrainCount()) {
            Terrain *pTerr;
            Tile *pTile;
            
            // Retrieve the current terrain
            pTerr = pTset->terrain(i);
            // Retrieve it's assigned tile
            pTile = pTerr->imageTile();
            
            // Output to the file: 'area <terrain_name> <tile_id>
            file.write("area ");
            file.write(pTerr->name().toLatin1());
            file.write(" ");
            file.write(QByteArray::number(pTile->id()));
            file.write("\n");
            
            i++;
        }
    }
    
    // Write every layer
    foundTilemap = 0;
    foreach (const Layer *layer, map->layers()) {
        const TileLayer *tileLayer;
        
        // Check that the layer is visible
        if (!layer->isVisible())
            continue;
        // Check that the layer is a tilemap
        if (layer->layerType() != Layer::TileLayerType)
            continue;
        
        // Check that there's only a single tilemap to be exported
        if (foundTilemap) {
            mError = tr("Found more than one visible layers, but the plugin can"
                    "only handle a single layer at a time");
            return false;
        }
        
        // Output the current layer
        tileLayer = static_cast<const TileLayer*>(layer);
        gfm_writeTilemap(file, tileLayer);
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
                file.write(",", 1);
            }
        }
        
        file.write("\n", 1);
    }
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Gfm, GFMExporterPlugin)
#endif

