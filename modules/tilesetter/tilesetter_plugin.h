#ifndef TILESETTERPLUGIN_H
#define TILESETTERPLUGIN_H

#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_node.h"

#include "scene/gui/button.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/container.h"

class TilesetterPlugin : public EditorPlugin
{
    GDCLASS(TilesetterPlugin, EditorPlugin);

    public:
        TilesetterPlugin(EditorNode *p_editor);
        TilesetterPlugin();  // Doesn't compile without
    protected:
        static void _bind_methods();
    private:
        enum Actions {
            ACTION_NEW_TILESET,
            ACTION_LOAD_TILESET,
            ACTION_SAVE_TILESET,
            ACTION_LOAD_IMAGE,
            ACTION_DELETE_IMAGE,
            ACTION_NEW_TILE,
            ACTION_DELETE_TILE
        };

        Button* menubutton;
        WindowDialog* main_window;

        VBoxContainer main_menu;
        Button main_menu_new_tileset;
        Button main_menu_load_tileset;
        Button main_menu_save_tileset;
        Button main_menu_load_image;
        Button main_menu_delete_image;
        Button main_menu_new_tile;
        Button main_menu_delete_tile;

        void _update();


        void _open_window();
        void _menu_action(int action);
};

#endif // TILESETTERPLUGIN_H
