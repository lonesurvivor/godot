/* register_types.cpp */

#include "register_types.h"
#include "class_db.h"
#include "tilesetter_plugin.h"

void register_tilesetter_types() {
#ifdef TOOLS_ENABLED
    //ClassDB::register_class<TilesetterPlugin>();
    EditorPlugins::add_by_type<TilesetterPlugin>();
#endif
}

void unregister_tilesetter_types() {
   //nothing to do here
}
