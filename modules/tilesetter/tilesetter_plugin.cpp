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

void Tilesetter::_update_gui() {

    tile_list->clear();

    bool tileset_loaded = false;

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

void Tilesetter::_update_texture_container() {
    TextureContainer *c = texture_container->get_current_tab_control()->cast_to<TextureContainer>();
    if(c)
        c->update();
}

void Tilesetter::_update_texture_containers() {
    while(texture_container->get_child_count() > 0) {
        memdelete(texture_container->get_child(0));
    }

    for(int i = 0;i<current_textures.size();i++) {
        TextureContainer *c = memnew(TextureContainer(current_textures[i], this));
        texture_container->add_child(c);
    }
}



void Tilesetter::_menu_action(int action) {

    switch(action) {

        case ACTION_NEW_TILESET:
            current_tileset.unref();
            current_tileset.instance();
            current_tileset_path = "";
            current_textures.clear();

            _update_texture_containers();
            _update_gui();
            break;

        case ACTION_LOAD_TILESET:
            load_file_dialog->popup_centered();
            break;

        case ACTION_SAVE_TILESET:
            if(current_tileset_path != "") {
                _save_tileset(current_tileset_path);
                _update_gui();
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
                _update_texture_containers();
                _update_gui();
            }
            break; }

        case ACTION_NEW_TILE: {
            int tile_id = current_tileset->get_last_unused_tile_id();
            current_tileset->create_tile(tile_id);
            current_tileset->tile_set_name(tile_id, "Tile " + itos(tile_id));
            _update_gui();
            break; }

        case ACTION_DELETE_TILE:
            Vector<int> selected = tile_list->get_selected_items();
            for(int i=0;i<selected.size();i++) {
                current_tileset->remove_tile(tile_list->get_item_metadata(selected[i]));
            }
            _update_texture_container();
            _update_gui();
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

    _update_texture_container();
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
    selected_tile = -1;

    List<int> tile_ids;
    current_tileset->get_tile_list(&tile_ids);

    current_textures.clear();

    for(List<int>::Element *E=tile_ids.front();E;E=E->next()) {
        Ref<Texture> t = current_tileset->tile_get_texture(E->get());

        if(t.is_null() || current_textures.find(t) != -1)
            continue;

        current_textures.push_back(t);
    }

    _update_texture_containers();
    _update_gui();
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

    _update_gui();
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

    _update_texture_containers();
    _update_gui();
}


void Tilesetter::_grid_step_changed(float value, bool x) {
    if(x)
        grid_step.x = value;
    else
        grid_step.y = value;

    _update_texture_container();
}

void Tilesetter::_grid_show_set(bool show) {
    grid_show_enabled = show;
    _update_texture_container();
}

void Tilesetter::_grid_snap_set(bool snap) {
    grid_snap_enabled = snap;
    _update_texture_container();
}

void Tilesetter::_pivot_snap_mode_set(int mode) {
    pivot_snap_mode = mode;
}

void Tilesetter::_pivot_default_mode_set(int mode) {
    pivot_default_mode = mode;
}

void Tilesetter::_pivot_keep_offset_set(bool keep_offset) {
    pivot_keep_offset_enabled = keep_offset;
}



void Tilesetter::_notification(int what) {
    switch(what) {
    case NOTIFICATION_READY:
        grid_show_enabler->set_icon( get_icon("Grid", "EditorIcons"));
        grid_snap_enabler->set_icon( get_icon("Snap", "EditorIcons"));
        break;
    }
}


Tilesetter::Tilesetter() {

    Vector2 window_size(1280,800);

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
    edit_mode_selector->add_item("Select Area", EDIT_MODE_TEXTURE_AREA);
    edit_mode_selector->add_item("Set Pivot", EDIT_MODE_PIVOT);
    edit_mode_selector->add_item("Set Collision Polygon", EDIT_MODE_COLLISION_POLYGON);
    edit_mode_selector->connect("item_selected", this, "_mode_select");
    edit_mode_selector->select(EDIT_MODE_TEXTURE_AREA);
    controls_container->add_child(edit_mode_selector);

    controls_container->add_child( memnew( VSeparator ));


	grid_show_enabled = true;
	grid_show_enabler = memnew( ToolButton );
	grid_show_enabler->set_text(TTR("Show Grid"));
	grid_show_enabler->set_focus_mode(FOCUS_NONE);
	grid_show_enabler->set_toggle_mode(true);
	grid_show_enabler->set_pressed(grid_show_enabled);
	grid_show_enabler->connect("toggled",this,"_grid_show_set");
	controls_container->add_child(grid_show_enabler);


	grid_snap_enabled = true;
	grid_snap_enabler = memnew( ToolButton );
	grid_snap_enabler->set_text(TTR("Grid Snap"));
	grid_snap_enabler->set_focus_mode(FOCUS_NONE);
	grid_snap_enabler->set_toggle_mode(true);
	grid_snap_enabler->set_pressed(grid_snap_enabled);
	grid_snap_enabler->connect("toggled",this,"_grid_snap_set");
	controls_container->add_child(grid_snap_enabler);


	controls_container->add_child( memnew( Label(TTR("Grid Step:")) ) );


	grid_step = Vector2(16,16);
    grid_step_x_selector = memnew(SpinBox);
    grid_step_x_selector->set_min(1);
    grid_step_x_selector->set_step(1);
    grid_step_x_selector->set_suffix("px");
	grid_step_x_selector->connect("value_changed", this, "_grid_step_changed", varray(true));
    grid_step_x_selector->set_value(grid_step.x);
    controls_container->add_child(grid_step_x_selector);

    grid_step_y_selector = memnew(SpinBox);
    grid_step_y_selector->set_min(1);
    grid_step_y_selector->set_step(1);
    grid_step_y_selector->set_suffix("px");
	grid_step_y_selector->connect("value_changed", this, "_grid_step_changed", varray(false));
    grid_step_y_selector->set_value(grid_step.y);
    controls_container->add_child(grid_step_y_selector);



    controls_container->add_child( memnew( VSeparator ));

	controls_container->add_child( memnew( Label(TTR("Pivot Snap:")) ) );

	pivot_snap_mode = SNAP_MODE_PIXEL;
    pivot_snap_mode_selector = memnew(OptionButton);
    pivot_snap_mode_selector->add_item("None", SNAP_MODE_NONE);
    pivot_snap_mode_selector->add_item("Pixel Snap", SNAP_MODE_PIXEL);
    pivot_snap_mode_selector->add_item("Grid Snap", SNAP_MODE_GRID);
    pivot_snap_mode_selector->connect("item_selected", this, "_pivot_snap_mode_set");
    pivot_snap_mode_selector->select(pivot_snap_mode);
    controls_container->add_child(pivot_snap_mode_selector);

    controls_container->add_child( memnew( Label(TTR("Default Pivot Position:")) ) );

    pivot_default_mode = DEFAULT_PIVOT_MODE_BOTTOM_LEFT;
    default_pivot_mode_selector = memnew(OptionButton);
    default_pivot_mode_selector->add_item("Bottom Left", DEFAULT_PIVOT_MODE_BOTTOM_LEFT);
    default_pivot_mode_selector->add_item("Bottom Right", DEFAULT_PIVOT_MODE_BOTTOM_RIGHT);
    default_pivot_mode_selector->add_item("Top Left", DEFAULT_PIVOT_MODE_TOP_LEFT);
    default_pivot_mode_selector->add_item("Top Right", DEFAULT_PIVOT_MODE_TOP_RIGHT);
    default_pivot_mode_selector->connect("item_selected", this, "_pivot_default_mode_set");
    default_pivot_mode_selector->select(pivot_default_mode);
    controls_container->add_child(default_pivot_mode_selector);


	pivot_keep_offset_enabled = true;
	pivot_keep_offset_enabler = memnew( ToolButton );
	pivot_keep_offset_enabler->set_text(TTR("Keep Offset"));
	pivot_keep_offset_enabler->set_focus_mode(FOCUS_NONE);
	pivot_keep_offset_enabler->set_toggle_mode(true);
	pivot_keep_offset_enabler->set_pressed(pivot_keep_offset_enabled);
	pivot_keep_offset_enabler->connect("toggled",this,"_pivot_keep_offset_set");
	controls_container->add_child(pivot_keep_offset_enabler);



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
	ClassDB::bind_method(_MD("_grid_step_changed"),&Tilesetter::_grid_step_changed);
	ClassDB::bind_method(_MD("_grid_show_set"),&Tilesetter::_grid_show_set);
	ClassDB::bind_method(_MD("_grid_snap_set"),&Tilesetter::_grid_snap_set);
	ClassDB::bind_method(_MD("_pivot_snap_mode_set"),&Tilesetter::_pivot_snap_mode_set);
	ClassDB::bind_method(_MD("_pivot_default_mode_set"),&Tilesetter::_pivot_default_mode_set);
	ClassDB::bind_method(_MD("_pivot_keep_offset_set"),&Tilesetter::_pivot_keep_offset_set);
}

/////////////////////////////////////////////////


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

            if(tilesetter->selected_tile == -1)
                return;

            if(mb.pressed) {
                const Point2 &mouse_pos_on_texture = texture_rect->get_transform().affine_inverse().xform(Point2(mb.x, mb.y));

                if(tilesetter->current_mode == Tilesetter::EDIT_MODE_TEXTURE_AREA) {
                    drawing_mouse_rect = true;
                    mouse_rect_start = mouse_pos_on_texture;
                    if(tilesetter->grid_snap_enabled) {
                        mouse_rect_start.x = Math::stepify(mouse_rect_start.x, tilesetter->grid_step.x);
                        mouse_rect_start.y = Math::stepify(mouse_rect_start.y, tilesetter->grid_step.y);

                    } else {
                        mouse_rect_start.x = Math::round(mouse_rect_start.x);
                        mouse_rect_start.y = Math::round(mouse_rect_start.y);
                    }

                    mouse_rect_end = mouse_rect_start;

                } else if(tilesetter->current_mode == Tilesetter::EDIT_MODE_PIVOT) {

                    Point2 mp = mouse_pos_on_texture;
                    if(tilesetter->pivot_snap_mode == Tilesetter::SNAP_MODE_PIXEL) {
                        mp.x = Math::stepify(mp.x, 1);
                        mp.y = Math::stepify(mp.y, 1);
                    } else if(tilesetter->pivot_snap_mode == Tilesetter::SNAP_MODE_GRID) {
                        mp.x = Math::stepify(mp.x, tilesetter->grid_step.x);
                        mp.y = Math::stepify(mp.y, tilesetter->grid_step.x);
                    }


                    Rect2 tile_region = tilesetter->current_tileset->tile_get_region(tilesetter->selected_tile);
                    tilesetter->current_tileset->tile_set_texture_offset(tilesetter->selected_tile, tile_region.get_pos()-mp);
                }

            } else {
                if(drawing_mouse_rect) {
                    drawing_mouse_rect = false;

                    Rect2 r(Vector2(Math::fast_ftoi(mouse_rect_start.x), Math::fast_ftoi(mouse_rect_start.y)), Size2(0,0));
                    r.expand_to(Vector2(Math::fast_ftoi(mouse_rect_end.x), Math::fast_ftoi(mouse_rect_end.y)));

                    Rect2 old_r = tilesetter->current_tileset->tile_get_region(tilesetter->selected_tile);

                    tilesetter->current_tileset->tile_set_region(tilesetter->selected_tile, r);
                    tilesetter->current_tileset->tile_set_texture(tilesetter->selected_tile, texture_rect->get_texture());

                    Vector2 texture_offset(0,0);
                    Vector2 old_offset = tilesetter->current_tileset->tile_get_texture_offset(tilesetter->selected_tile);

                    switch(tilesetter->pivot_default_mode) {
                        case Tilesetter::DEFAULT_PIVOT_MODE_TOP_LEFT:
                            texture_offset = tilesetter->pivot_keep_offset_enabled ? old_offset : Vector2(0,0);
                            break;
                        case Tilesetter::DEFAULT_PIVOT_MODE_TOP_RIGHT:
                            texture_offset = tilesetter->pivot_keep_offset_enabled ? old_offset - Vector2(r.get_size().x - old_r.get_size().x, 0)  : -Vector2(r.get_size().x, 0);
                            break;
                        case Tilesetter::DEFAULT_PIVOT_MODE_BOTTOM_LEFT:
                            texture_offset = tilesetter->pivot_keep_offset_enabled ? old_offset - Vector2(0, r.get_size().y - old_r.get_size().y)  : -Vector2(0, r.get_size().y);
                            break;
                        case Tilesetter::DEFAULT_PIVOT_MODE_BOTTOM_RIGHT:
                            texture_offset = tilesetter->pivot_keep_offset_enabled ? old_offset - (old_r.get_size()-r.get_size()) : -r.get_size();
                            break;
                    }

                    tilesetter->current_tileset->tile_set_texture_offset(tilesetter->selected_tile, texture_offset);

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

            if(tilesetter->grid_snap_enabled) {
                mouse_rect_end.x = Math::stepify(mouse_rect_end.x, tilesetter->grid_step.x);
                mouse_rect_end.y = Math::stepify(mouse_rect_end.y, tilesetter->grid_step.y);

            } else {
                mouse_rect_end.x = Math::round(mouse_rect_end.x);
                mouse_rect_end.y = Math::round(mouse_rect_end.y);
            }
        }

        update();
    }
}

void TextureContainer::_draw_container() {

    // Grid

    const Point2 &pos = texture_rect->get_pos();

    if(tilesetter->grid_show_enabled) {

        for(real_t p = pos.x - Math::stepify(pos.x, tilesetter->grid_step.x * texture_scale);p < get_size().width;p+=tilesetter->grid_step.x * texture_scale) {
            draw_line(Point2(p,0), Point2(p,get_size().height), Color(1,1,1,0.2));
        }

        for(real_t p = pos.y - Math::stepify(pos.y, tilesetter->grid_step.y * texture_scale);p < get_size().height;p+=tilesetter->grid_step.y * texture_scale) {
            draw_line(Point2(0,p), Point2(get_size().width,p), Color(1,1,1,0.2));
        }
    }


    if(tilesetter->selected_tile == -1)
        return;

    // Region rect

    bool tile_uses_texture = tilesetter->current_tileset->tile_get_texture(tilesetter->selected_tile) == texture_rect->get_texture();
    const Rect2 &tile_region = tilesetter->current_tileset->tile_get_region(tilesetter->selected_tile);

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

void TextureContainer::_notification(int what) {
    switch(what) {
        case NOTIFICATION_ENTER_TREE:
        if(!is_connected("draw", this, "_draw_container"))
            connect("draw", this, "_draw_container");
    }
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
