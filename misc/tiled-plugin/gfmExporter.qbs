import qbs 1.0

TiledPlugin {
    cpp.defines: ["GFRAME_EXPORTER"]

    files: [
        "gfmExporter_global.h"
        "gfmExporter.cpp",
        "gfmExporter.h",
    ]
}
