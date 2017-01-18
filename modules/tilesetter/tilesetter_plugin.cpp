#include "tilesetter_plugin.h"
#include "print_string.h"




void Tilesetter::_update() {

    if(current_tileset.is_null())
        return;

    List<int> tile_ids;
    current_tileset->get_tile_list(&tile_ids);

    current_textures.clear();

    for(List<int>::Element *E=tile_ids.front();E;E=E->next()) {
        Ref<Texture> t = current_tileset->tile_get_texture(E->get());

        if(t.is_null() || current_textures.find(t))
            continue;

        current_textures.push_back(t);

        TextureContainer *c = memnew(TextureContainer(t));
        texture_list->add_child(c);
    }
}


void Tilesetter::_menu_action(int action) {

    switch(action) {
        case ACTION_LOAD_TILESET:
            file_dialog->popup_centered();
            break;
    }

}

void Tilesetter::_load_tileset(String path) {

    current_tileset = ResourceLoader::load(path, "TileSet");
    if(current_tileset.is_null()) {
        message_dialog->set_text("Can't load TileSet");
        message_dialog->popup_centered_minsize();
        return;
    }

    _update();

    file_dialog->hide();
}


Tilesetter::Tilesetter() {

    Vector2 window_size(800,600);

    set_size(window_size);
    set_title("Tilesetter");

    // File Dialog

    file_dialog = memnew(EditorFileDialog);
    file_dialog->set_size(window_size);
    file_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
    file_dialog->set_mode(EditorFileDialog::MODE_OPEN_FILE);
    file_dialog->add_filter("*.tres");
    file_dialog->set_hide_on_ok(false);
    file_dialog->connect("file_selected", this, "_load_tileset");
    add_child(file_dialog);

    // Message Dialog

    message_dialog = memnew(AcceptDialog);
    add_child(message_dialog);

    // Top Container

    top_container = memnew(VBoxContainer);
    top_container->set_size(window_size);
    add_child(top_container);

    // Menu
    main_menu = memnew(HBoxContainer);

    main_menu_new_tileset = memnew(Button);
    main_menu_new_tileset->set_text("New Tileset");
    main_menu_new_tileset->connect("pressed", this, "_menu_action", varray(ACTION_NEW_TILESET));
    main_menu->add_child(main_menu_new_tileset);

    main_menu_load_tileset = memnew(Button);
    main_menu_load_tileset->set_text("Load Tileset");
    main_menu_load_tileset->connect("pressed", this, "_menu_action", varray(ACTION_LOAD_TILESET));
    main_menu->add_child(main_menu_load_tileset);

    main_menu_save_tileset = memnew(Button);
    main_menu_save_tileset->set_text("Save Tileset");
    main_menu->add_child(main_menu_save_tileset);

    main_menu_load_image = memnew(Button);
    main_menu_load_image->set_text("Load Tileset");
    main_menu->add_child(main_menu_load_image);

    main_menu_delete_image = memnew(Button);
    main_menu_delete_image->set_text("Delete Image");
    main_menu->add_child(main_menu_delete_image);

    main_menu_new_tile = memnew(Button);
    main_menu_new_tile->set_text("New Tile");
    main_menu->add_child(main_menu_new_tile);

    main_menu_delete_tile = memnew(Button);
    main_menu_delete_tile->set_text("Delete Tile");
    main_menu->add_child(main_menu_delete_tile);

    top_container->add_child(main_menu);

    // Bottom Container

    bottom_container = memnew(HSplitContainer);
    bottom_container->set_v_size_flags(SIZE_EXPAND_FILL);
    bottom_container->set_h_size_flags(SIZE_EXPAND_FILL);

    // Tile List

    tile_list = memnew (ItemList);
	tile_list->set_v_size_flags(SIZE_EXPAND_FILL);
	tile_list->set_custom_minimum_size(Size2(100,0));
	tile_list->set_max_columns(0);
	tile_list->set_icon_mode(ItemList::ICON_MODE_TOP);
	tile_list->set_max_text_lines(2);
    bottom_container->add_child(tile_list);

    // Editor

    texture_list = memnew (TabContainer);
    texture_list->set_tabs_visible(true);
    texture_list->set_h_size_flags(SIZE_EXPAND_FILL);
    texture_list->set_v_size_flags(SIZE_EXPAND_FILL);
    texture_list->set_custom_minimum_size(Size2(200,0));
    bottom_container->add_child(texture_list);

    top_container->add_child(bottom_container);


}

void Tilesetter::_bind_methods() {

	ClassDB::bind_method(_MD("_menu_action"),&Tilesetter::_menu_action);
	ClassDB::bind_method(_MD("_load_tileset"),&Tilesetter::_load_tileset);
}

/////////////////////////////////////////////////

TextureContainer::TextureContainer(const Ref<Texture> &texture) {

    texture_rect = memnew(TextureRect);

    set_clip_contents(true);
    set_name(texture->get_name());

    texture_rect->set_texture(texture);
    add_child(texture_rect);

    dragging = false;
    zoom = 1.0;
}

void TextureContainer::_gui_input(const InputEvent& ev) {
    if(ev.type == InputEvent::MOUSE_BUTTON) {
        const InputEventMouseButton &mb = ev.mouse_button;

        if(mb.button_index == BUTTON_MIDDLE) {
            if(mb.pressed)
                dragging = true;
            else
                dragging = false;
        } else if(mb.button_index == BUTTON_WHEEL_UP) {
            if(zoom < 10) {
                zoom += 0.5;

                texture_rect->set_scale(Vector2(zoom, zoom));
            }

        } else if(mb.button_index == BUTTON_WHEEL_DOWN) {
            if(zoom > 0.6) {
                zoom -= 0.5;
                texture_rect->set_scale(Vector2(zoom, zoom));
            }
        }
    }

    if(ev.type == InputEvent::MOUSE_MOTION) {
        const InputEventMouseMotion &mm = ev.mouse_motion;

        if(dragging) {
            texture_rect->set_pos(texture_rect->get_pos() + Vector2(mm.relative_x, mm.relative_y));
        }
    }

    accept_event();
}


void TextureContainer::_bind_methods() {

	ClassDB::bind_method(_MD("_gui_input"),&TextureContainer::_gui_input);
}


/////////////////////////////////////////////////7


void TilesetterPlugin::_open_window() {
    main_window->popup_centered();
}

TilesetterPlugin::TilesetterPlugin(EditorNode *p_editor) {
    // Menu button (temporary)
    menubutton = memnew(Button);
    menubutton->set_text("Tilesetter");
    add_control_to_container(CONTAINER_TOOLBAR, menubutton);
    menubutton->connect("pressed", this, "_open_window");

    // Main window
    main_window = memnew(Tilesetter());
    p_editor->get_gui_base()->add_child(main_window);
}

TilesetterPlugin::TilesetterPlugin() {
    // Doesn't compile without
}

void TilesetterPlugin::_bind_methods() {

	ClassDB::bind_method(_MD("_open_window"),&TilesetterPlugin::_open_window);
}
