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

#include "core/io/resource_saver.h"
#include "os/keyboard.h"
#include "os/input.h"

class Tilesetter : public WindowDialog {
    GDCLASS(Tilesetter, WindowDialog);

    friend class TextureContainer;

    public:
        Tilesetter();

    protected:
        static void _bind_methods();
    private:
        enum Actions {
            ACTION_NEW_TILESET,
            ACTION_LOAD_TILESET,
            ACTION_SAVE_TILESET,
            ACTION_SAVE_TILESET_AS,
            ACTION_ADD_TEXTURE,
            ACTION_DELETE_TEXTURE,
            ACTION_NEW_TILE,
            ACTION_DELETE_TILE
        };

        enum EditModes {
            MODE_TEXTURE_AREA,
            MODE_PIVOT,
            MODE_COLLISION_POLYGON
        };

        // GUI

        WindowDialog* main_window;
        EditorFileDialog *load_file_dialog;
        EditorFileDialog *load_texture_dialog;
        EditorFileDialog *save_file_dialog;
        AcceptDialog *message_dialog;

        VBoxContainer *top_container;

        HBoxContainer *controls_container;
        OptionButton *edit_mode_selector;

        HBoxContainer *main_menu;
        Button *main_menu_new_tileset;
        Button *main_menu_load_tileset;
        Button *main_menu_save_tileset;
        Button *main_menu_save_tileset_as;
        Button *main_menu_add_texture;
        Button *main_menu_delete_texture;
        Button *main_menu_new_tile;
        Button *main_menu_delete_tile;

        HSplitContainer *bottom_container;
        ItemList *tile_list;
        TabContainer *texture_container;

        // internal stuff

        Ref<TileSet> current_tileset;
        Vector<Ref<Texture>> current_textures;
        String current_tileset_path;
        int current_mode;
        int selected_tile;

        void _update_gui(bool all = false);

        void _gui_input(const InputEvent &ev);
        void _menu_action(int action);
        void _mode_select(int mode);
        void _tile_select(int list_id);
        void _load_tileset(String path);
        void _load_texture(String path);
        void _save_tileset(String path);
};

class TextureContainer : public Control {
    GDCLASS(TextureContainer, Control);

    public:
        TextureContainer(const Ref<Texture> &texture, Tilesetter *parent);
        Ref<Texture> get_texture();
    protected:
        static void _bind_methods();
    private:
        Tilesetter *tilesetter;
        TextureRect *texture_rect;
        bool drawing_mouse_rect;
        Point2 mouse_rect_start;
        Point2 mouse_rect_end;

        bool dragging;
        real_t texture_scale;

        void _notification(int p_what);

        virtual void _gui_input(const InputEvent& ev);
        void _draw_container();
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
