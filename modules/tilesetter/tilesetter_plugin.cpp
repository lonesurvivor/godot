#include "tilesetter_plugin.h"
#include "print_string.h"


void Tilesetter::_gui_input(const InputEvent &ev) {
    if(ev.type == InputEvent::KEY) {
        const InputEventKey &k = ev.key;

        if(k.pressed && k.scancode == KEY_Q) {
            print_line("hallo");
        }
    }

    print_line("hello");

}

void Tilesetter::_update_gui(bool all) {

    if(all) {
        while(texture_container->get_child_count() > 0) {
            memdelete(texture_container->get_child(0));
        }

        for(int i = 0;i<current_textures.size();i++) {
            TextureContainer *c = memnew(TextureContainer(current_textures[i], this));
            texture_container->add_child(c);
        }
    }

    TextureContainer *c = texture_container->get_current_tab_control()->cast_to<TextureContainer>();
    if(c) {
        c->update();
    }

    bool tileset_loaded = false;

    tile_list->clear();

    if(!current_tileset.is_null()) {
        List<int> tile_ids;
        current_tileset->get_tile_list(&tile_ids);

        for(List<int>::Element *E=tile_ids.front();E;E=E->next()) {
            tile_list->add_item(current_tileset->tile_get_name(E->get()));
            tile_list->set_item_metadata(tile_list->get_item_count()-1, E->get());
            if(E->get() == selected_tile)
                tile_list->select(tile_list->get_item_count()-1);
        }

        if(tile_ids.size() > 0)
            tileset_loaded = true;
    }

    main_menu_save_tileset->set_disabled(!tileset_loaded);
    main_menu_save_tileset_as->set_disabled(!tileset_loaded);
    main_menu_new_tileset->set_disabled(!tileset_loaded && texture_container->get_child_count() == 0);
    main_menu_delete_texture->set_disabled(texture_container->get_child_count() == 0);
    main_menu_new_tile->set_disabled(texture_container->get_child_count() == 0);

    if(tileset_loaded)
        set_title("Tilesetter - " + current_tileset_path);
    else
        set_title("Tilesetter");
}


void Tilesetter::_menu_action(int action) {

    switch(action) {

        case ACTION_NEW_TILESET:
            current_tileset.unref();
            current_tileset.instance();
            current_tileset_path = "";
            current_textures.clear();
            _update_gui(true);
            break;

        case ACTION_LOAD_TILESET:
            load_file_dialog->popup_centered();
            break;

        case ACTION_SAVE_TILESET:
            if(current_tileset_path != "") {
                _save_tileset(current_tileset_path);
                _update_gui(false);
            } else {
                save_file_dialog->popup_centered();
            }
            break;

        case ACTION_SAVE_TILESET_AS:
            save_file_dialog->popup_centered();
            break;

        case ACTION_ADD_TEXTURE:
            load_texture_dialog->popup_centered();
            break;

        case ACTION_DELETE_TEXTURE: {
            TextureContainer *c = texture_container->get_current_tab_control()->cast_to<TextureContainer>();
            if(c) {
                Ref<Texture> t = c->get_texture();
                current_textures.remove(current_textures.find(t));

                List<int> tile_ids;
                current_tileset->get_tile_list(&tile_ids);

                for(List<int>::Element *E=tile_ids.front();E;E=E->next()) {
                    if(current_tileset->tile_get_texture(E->get()) == t)
                        current_tileset->remove_tile(E->get());
                }
                _update_gui(true);
            }
            break; }

        case ACTION_NEW_TILE: {
            int tile_id = current_tileset->get_last_unused_tile_id();
            current_tileset->create_tile(tile_id);
            current_tileset->tile_set_name(tile_id, "Tile " + itos(tile_id));
            _update_gui(false);
            break; }

        case ACTION_DELETE_TILE:
            Vector<int> selected = tile_list->get_selected_items();
            for(int i=0;i<selected.size();i++) {
                current_tileset->remove_tile(tile_list->get_item_metadata(selected[i]));
            }
            _update_gui(false);
            break;
    }

}

void Tilesetter::_mode_select(int mode) {

    current_mode = mode;
    _update_gui();
}

void Tilesetter::_tile_select(int list_id) {

    if(current_tileset.is_null())
        return;

    selected_tile = tile_list->get_item_metadata(list_id);
    Ref<Texture> t = current_tileset->tile_get_texture(selected_tile);


    for(int i = 0; i<texture_container->get_child_count();i++){
        if(texture_container->get_child(i)->cast_to<TextureContainer>()->get_texture() == t)
            texture_container->set_current_tab(i);
    }

    _update_gui();
}

void Tilesetter::_load_tileset(String path) {

    current_tileset = ResourceLoader::load(path, "TileSet", true);

    if(current_tileset.is_null()) {
        message_dialog->set_text("Can't load TileSet");
        message_dialog->popup_centered_minsize();
        return;
    }

    current_tileset_path = path;

    List<int> tile_ids;
    current_tileset->get_tile_list(&tile_ids);

    current_textures.clear();

    for(List<int>::Element *E=tile_ids.front();E;E=E->next()) {
        Ref<Texture> t = current_tileset->tile_get_texture(E->get());

        if(t.is_null() || current_textures.find(t) != -1)
            continue;

        current_textures.push_back(t);
    }

    _update_gui(true);
}


void Tilesetter::_save_tileset(String path) {

    if(current_tileset.is_null())
        return;

    Error e = ResourceSaver::save(path, current_tileset);
    if(e) {
        message_dialog->set_text("Can't save TileSet");
        message_dialog->popup_centered_minsize();
    }

    current_tileset_path = path;

    _update_gui(false);
}

void Tilesetter::_load_texture(String path) {

    Ref<Texture> txt = ResourceLoader::load(path, "Texture");
    if(txt.is_null()) {
        message_dialog->set_text("Can't load Texture");
        message_dialog->popup_centered_minsize();
        return;
    }

    if(current_textures.find(txt) != -1) {
        message_dialog->set_text("Texture is already loaded");
        message_dialog->popup_centered_minsize();
        return;
    }

    current_textures.push_back(txt);

    _update_gui(true);
}


Tilesetter::Tilesetter() {

    Vector2 window_size(800,600);

    set_size(window_size);
    set_title("Tilesetter");

    // File Dialog

    load_file_dialog = memnew(EditorFileDialog);
    load_file_dialog->set_size(window_size);
    load_file_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
    load_file_dialog->set_mode(EditorFileDialog::MODE_OPEN_FILE);
    load_file_dialog->add_filter("*.tres");
    load_file_dialog->connect("file_selected", this, "_load_tileset");
    add_child(load_file_dialog);


    load_texture_dialog = memnew(EditorFileDialog);
    load_texture_dialog->set_size(window_size);
    load_texture_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
    load_texture_dialog->set_mode(EditorFileDialog::MODE_OPEN_FILE);
    load_texture_dialog->add_filter("*.png");
    load_texture_dialog->connect("file_selected", this, "_load_texture");
    add_child(load_texture_dialog);

    save_file_dialog = memnew(EditorFileDialog);
    save_file_dialog->set_size(window_size);
    save_file_dialog->set_access(EditorFileDialog::ACCESS_RESOURCES);
    save_file_dialog->set_mode(EditorFileDialog::MODE_SAVE_FILE);
    save_file_dialog->add_filter("*.tres");
    save_file_dialog->connect("file_selected", this, "_save_tileset");
    add_child(save_file_dialog);

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
    main_menu_new_tileset->set_text("Clear All");
    main_menu_new_tileset->connect("pressed", this, "_menu_action", varray(ACTION_NEW_TILESET));
    main_menu->add_child(main_menu_new_tileset);

    main_menu_load_tileset = memnew(Button);
    main_menu_load_tileset->set_text("Load Tileset");
    main_menu_load_tileset->connect("pressed", this, "_menu_action", varray(ACTION_LOAD_TILESET));
    main_menu->add_child(main_menu_load_tileset);

    main_menu_save_tileset = memnew(Button);
    main_menu_save_tileset->set_text("Save Tileset");
    main_menu_save_tileset->connect("pressed", this, "_menu_action", varray(ACTION_SAVE_TILESET));
    main_menu->add_child(main_menu_save_tileset);

    main_menu_save_tileset_as = memnew(Button);
    main_menu_save_tileset_as->set_text("Save Tileset As");
    main_menu_save_tileset_as->connect("pressed", this, "_menu_action", varray(ACTION_SAVE_TILESET_AS));
    main_menu->add_child(main_menu_save_tileset_as);

    main_menu_add_texture = memnew(Button);
    main_menu_add_texture->set_text("Add Texture");
    main_menu_add_texture->connect("pressed", this, "_menu_action", varray(ACTION_ADD_TEXTURE));
    main_menu->add_child(main_menu_add_texture);

    main_menu_delete_texture = memnew(Button);
    main_menu_delete_texture->set_text("Delete Texture");
    main_menu_delete_texture->connect("pressed", this, "_menu_action", varray(ACTION_DELETE_TEXTURE));
    main_menu->add_child(main_menu_delete_texture);

    main_menu_new_tile = memnew(Button);
    main_menu_new_tile->set_text("New Tile");
    main_menu_new_tile->connect("pressed", this, "_menu_action", varray(ACTION_NEW_TILE));
    main_menu->add_child(main_menu_new_tile);

    main_menu_delete_tile = memnew(Button);
    main_menu_delete_tile->set_text("Delete Tile");
    main_menu_delete_tile->connect("pressed", this, "_menu_action", varray(ACTION_DELETE_TILE));
    main_menu->add_child(main_menu_delete_tile);

    top_container->add_child(main_menu);

    // Other controls
    controls_container = memnew(HBoxContainer);

    edit_mode_selector = memnew (OptionButton);
    edit_mode_selector->add_item("Select Area", MODE_TEXTURE_AREA);
    edit_mode_selector->add_item("Set Pivot", MODE_PIVOT);
    edit_mode_selector->add_item("Set Collision Polygon", MODE_COLLISION_POLYGON);
    edit_mode_selector->connect("item_selected", this, "_mode_select");
    edit_mode_selector->select(MODE_TEXTURE_AREA);
    controls_container->add_child(edit_mode_selector);


    top_container->add_child(controls_container);

    // Bottom Container

    bottom_container = memnew(HSplitContainer);
    bottom_container->set_v_size_flags(SIZE_EXPAND_FILL);
    bottom_container->set_h_size_flags(SIZE_EXPAND_FILL);

    // Tile List

    tile_list = memnew (ItemList);
	tile_list->set_v_size_flags(SIZE_EXPAND_FILL);
	tile_list->set_custom_minimum_size(Size2(100,0));
	tile_list->set_max_columns(1);
	//tile_list->set_icon_mode(ItemList::ICON_MODE_TOP);
	//tile_list->set_max_text_lines(2);
	tile_list->connect("item_selected", this, "_tile_select");
    bottom_container->add_child(tile_list);

    // Editor

    texture_container = memnew (TabContainer);
    texture_container->set_tabs_visible(true);
    texture_container->set_h_size_flags(SIZE_EXPAND_FILL);
    texture_container->set_v_size_flags(SIZE_EXPAND_FILL);
    texture_container->set_custom_minimum_size(Size2(200,0));
    bottom_container->add_child(texture_container);

    top_container->add_child(bottom_container);

    current_tileset.instance();
    selected_tile = -1;
    _update_gui();
}

void Tilesetter::_bind_methods() {

	ClassDB::bind_method(_MD("_gui_input"),&Tilesetter::_gui_input);
	ClassDB::bind_method(_MD("_menu_action"),&Tilesetter::_menu_action);
	ClassDB::bind_method(_MD("_mode_select"),&Tilesetter::_mode_select);
	ClassDB::bind_method(_MD("_tile_select"),&Tilesetter::_tile_select);
	ClassDB::bind_method(_MD("_load_tileset"),&Tilesetter::_load_tileset);
	ClassDB::bind_method(_MD("_load_texture"),&Tilesetter::_load_texture);
	ClassDB::bind_method(_MD("_save_tileset"),&Tilesetter::_save_tileset);
}

/////////////////////////////////////////////////

void TextureContainer::_notification(int p_what) {
    switch(p_what) {
        case NOTIFICATION_ENTER_TREE:
        if(!is_connected("draw", this, "_draw_container"))
            connect("draw", this, "_draw_container");
    }
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
            if(texture_scale < 10) {
                texture_scale += 0.5;

                texture_rect->set_scale(Vector2(texture_scale, texture_scale));
            }

        } else if(mb.button_index == BUTTON_WHEEL_DOWN) {
            if(texture_scale > 0.6) {
                texture_scale -= 0.5;
                texture_rect->set_scale(Vector2(texture_scale, texture_scale));
            }
        } else if(mb.button_index == BUTTON_LEFT) {
            if(mb.pressed) {
                Point2 mouse_pos_on_texture = texture_rect->get_transform().affine_inverse().xform(Point2(mb.x, mb.y));

                if(tilesetter->current_mode == Tilesetter::MODE_TEXTURE_AREA) {
                    drawing_mouse_rect = true;
                    mouse_rect_start = mouse_pos_on_texture;
                    mouse_rect_start.x = Math::round(mouse_rect_start.x);
                    mouse_rect_start.y = Math::round(mouse_rect_start.y);

                    mouse_rect_end = mouse_rect_start;
                } else if(tilesetter->current_mode == Tilesetter::MODE_PIVOT) {
                    Rect2 tile_region = tilesetter->current_tileset->tile_get_region(tilesetter->selected_tile);
                    tilesetter->current_tileset->tile_set_texture_offset(tilesetter->selected_tile, tile_region.get_pos()-mouse_pos_on_texture);
                }

            } else {
                if(drawing_mouse_rect) {
                    drawing_mouse_rect = false;

                    Rect2 r(Vector2(Math::fast_ftoi(mouse_rect_start.x), Math::fast_ftoi(mouse_rect_start.y)), Size2(0,0));
                    r.expand_to(Vector2(Math::fast_ftoi(mouse_rect_end.x), Math::fast_ftoi(mouse_rect_end.y)));
                    tilesetter->current_tileset->tile_set_region(tilesetter->selected_tile, r);
                    tilesetter->current_tileset->tile_set_texture(tilesetter->selected_tile, texture_rect->get_texture());

                    mouse_rect_start = Point2(0,0);
                    mouse_rect_end = Point2(0,0);
                }
            }
        }

        update();
    }

    if(ev.type == InputEvent::MOUSE_MOTION) {
        const InputEventMouseMotion &mm = ev.mouse_motion;

        if(dragging) {
            texture_rect->set_pos(texture_rect->get_pos() + Vector2(mm.relative_x, mm.relative_y));
        } else if(drawing_mouse_rect) {
            mouse_rect_end = texture_rect->get_transform().affine_inverse().xform(Point2(mm.x, mm.y));
            mouse_rect_end.x = Math::round(mouse_rect_end.x);
            mouse_rect_end.y = Math::round(mouse_rect_end.y);
        }

        update();
    }
}

void TextureContainer::_draw_container() {

    // Grid

    Point2 pos = texture_rect->get_pos();

    for(real_t p = pos.x - Math::stepify(pos.x, 16 * texture_scale);p < get_size().width;p+=16 * texture_scale) {
        draw_line(Point2(p,0), Point2(p,get_size().height), Color(1,1,1,0.2));
    }

    for(real_t p = pos.y - Math::stepify(pos.y, 16 * texture_scale);p < get_size().height;p+=16 * texture_scale) {
        draw_line(Point2(0,p), Point2(get_size().width,p), Color(1,1,1,0.2));
    }

    // Region rect

    bool tile_uses_texture = tilesetter->current_tileset->tile_get_texture(tilesetter->selected_tile) == texture_rect->get_texture();
    Rect2 tile_region = tilesetter->current_tileset->tile_get_region(tilesetter->selected_tile);

    Rect2 r;
    if(drawing_mouse_rect) {
        r.set_pos(mouse_rect_start);
        r.expand_to(mouse_rect_end);
    } else if(tile_uses_texture) {

        r = tile_region;
    }

    if(!r.has_no_area()) {
        r = texture_rect->get_transform().xform(r);
        Color c(1,1,1);
        draw_line(r.get_pos(), Point2(r.get_pos().x + r.get_size().width, r.get_pos().y), c);
        draw_line(Point2(r.get_pos().x + r.get_size().width, r.get_pos().y), r.get_pos() + r.get_size(), c);
        draw_line(r.get_pos() + r.get_size(), Point2(r.get_pos().x, r.get_pos().y + r.get_size().height), c);
        draw_line(Point2(r.get_pos().x, r.get_pos().y + r.get_size().height), r.get_pos(), c);
    }

    // Pivot

    if(tile_uses_texture) {
        Vector2 pivot = -tilesetter->current_tileset->tile_get_texture_offset(tilesetter->selected_tile) + tile_region.get_pos();
        pivot = texture_rect->get_transform().xform(pivot);

        Color c(1,0,0);
        draw_line(pivot - Vector2(6,0), pivot + Vector2(5,0), c);
        draw_line(pivot - Vector2(0,5), pivot + Vector2(0,6), c);
    }

}

Ref<Texture> TextureContainer::get_texture() {
    return texture_rect->get_texture();
}

TextureContainer::TextureContainer(const Ref<Texture> &texture, Tilesetter *parent) {

    tilesetter = parent;
    texture_rect = memnew(TextureRect);

    set_clip_contents(true);
    set_name(texture->get_name());

    texture_rect->set_texture(texture);
    texture_rect->set_draw_behind_parent(true);
    add_child(texture_rect);

    dragging = false;
    drawing_mouse_rect = false;
    texture_scale = 1.0;
}


void TextureContainer::_bind_methods() {

	ClassDB::bind_method(_MD("_draw_container"),&TextureContainer::_draw_container);
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
