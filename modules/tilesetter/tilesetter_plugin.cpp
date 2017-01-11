#include "tilesetter_plugin.h"

void TilesetterPlugin::_open_window() {
    main_window->popup_centered();
}

void TilesetterPlugin::_menu_action(int action) {
    switch(action) {
        break;
    }

    main_window->hide();
}

void TilesetterPlugin::_update() {

}

TilesetterPlugin::TilesetterPlugin(EditorNode *p_editor) {
    // Menu button (temporary)
    menubutton = memnew(Button);
    menubutton->set_text("Tilesetter");
    add_control_to_container(CONTAINER_TOOLBAR, menubutton);
    menubutton->connect("pressed", this, "_open_window");

    // Main window
    main_window = memnew(WindowDialog);
    main_window->set_size(Vector2(800,600));
    add_child(main_window);
    main_window->set_title("Tilesetter");

    // Menu
    main_window->add_child(&main_menu);

    main_menu_new_tileset.set_text("New Tileset");
    main_menu.add_child(&main_menu_new_tileset);
    main_menu_new_tileset.connect("pressed", this, "_menu_action", varray(ACTION_NEW_TILESET));

    main_menu_load_tileset.set_text("Load Tileset");
    main_menu.add_child(&main_menu_load_tileset);

    main_menu_save_tileset.set_text("Save Tileset");
    main_menu.add_child(&main_menu_save_tileset);

    main_menu_load_image.set_text("Load Tileset");
    main_menu.add_child(&main_menu_load_image);

    main_menu_delete_image.set_text("Delete Image");
    main_menu.add_child(&main_menu_delete_image);

    main_menu_new_tile.set_text("New Tile");
    main_menu.add_child(&main_menu_new_tile);

    main_menu_delete_tile.set_text("Delete Tile");
    main_menu.add_child(&main_menu_delete_tile);

}

TilesetterPlugin::TilesetterPlugin() {
    // Doesn't compile without
}

void TilesetterPlugin::_bind_methods() {

	ClassDB::bind_method(_MD("_open_window"),&TilesetterPlugin::_open_window);
	ClassDB::bind_method(_MD("_menu_action"),&TilesetterPlugin::_menu_action);
}
