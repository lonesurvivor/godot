#ifndef TILESETTERPLUGIN_H
#define TILESETTERPLUGIN_H

#include "tools/editor/editor_plugin.h"
#include "tools/editor/editor_node.h"
#include "tools/editor/editor_file_dialog.h"

#include "scene/gui/button.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/scroll_bar.h"
#include "scene/gui/texture_rect.h"
#include "scene/resources/tile_set.h"

class Tilesetter : public WindowDialog {
    GDCLASS(Tilesetter, WindowDialog);

    public:
        Tilesetter();

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

        // GUI

        WindowDialog* main_window;
        EditorFileDialog *file_dialog;
        AcceptDialog *message_dialog;

        VBoxContainer *top_container;
        HSplitContainer *bottom_container;

        HBoxContainer *main_menu;
        Button *main_menu_new_tileset;
        Button *main_menu_load_tileset;
        Button *main_menu_save_tileset;
        Button *main_menu_load_image;
        Button *main_menu_delete_image;
        Button *main_menu_new_tile;
        Button *main_menu_delete_tile;

        ItemList *tile_list;

        TabContainer *texture_list;

        // internal stuff

        Ref<TileSet> current_tileset;
        List<Ref<Texture>> current_textures;

        void _update();

        void _menu_action(int action);
        void _load_tileset(String path);
};

class TextureContainer : public Control {
    GDCLASS(TextureContainer, Control);

    public:
        TextureContainer(const Ref<Texture> &texture);
    protected:
        static void _bind_methods();
    private:
        TextureRect *texture_rect;
        bool dragging;
        real_t zoom;

        virtual void _gui_input(const InputEvent& ev);
};

class TilesetterPlugin : public EditorPlugin
{
    GDCLASS(TilesetterPlugin, EditorPlugin);

    public:
        TilesetterPlugin(EditorNode *p_editor);
        TilesetterPlugin();  // Doesn't compile without
    protected:
        static void _bind_methods();
    private:
        Tilesetter *main_window;
        Button* menubutton;

        void _open_window();
};

#endif // TILESETTERPLUGIN_H
