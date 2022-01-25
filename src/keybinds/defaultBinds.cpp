#include <Geode>
#include <InternalMod.hpp>

USE_GEODE_NAMESPACE();
using namespace std::literals::string_literals;

#define CLICKED(...) if (down) { __VA_ARGS__ }
#define UI(...) auto ui = as<EditorUI*>(context); __VA_ARGS__
#define NO_PLAY(...) if (ui->m_editorLayer->m_playbackMode != PlaybackMode::Playing) { __VA_ARGS__ }
#define ACTION(...) __VA_ARGS__
#define EDIT_ACTION(...) CLICKED( UI( NO_PLAY( __VA_ARGS__ ) ) )
#define BINDS(...) { __VA_ARGS__ }
#define BIND(...) { { __VA_ARGS__ } }
#define KB(_mod_, _key_) { KEY_##_key_, Keybind::km##_mod_ }
#define KBS(_mod_, _key_) { { KEY_##_key_, Keybind::km##_mod_ } }

#define ADD_EDITOR_KB(_name_, _id_, _sub_, _func_, _desc_, _binds_)    \
    this->addKeybindAction(owner, TriggerableAction {   \
        _name_, _id_, KB_EDITOR_CATEGORY, _sub_,                \
        [](CCNode* context, bool down) -> bool {                \
            _func_;                                             \
            return false;                                       \
        }, _desc_ }, _binds_ );                                 \


void KeybindManager::loadDefaultKeybinds() {
    auto owner = InternalMod::get();

    this->setCategoryName(KB_PLAY_CATEGORY,   "Play");
    this->setCategoryName(KB_EDITOR_CATEGORY, "Editor");
    this->setCategoryName(KB_GLOBAL_CATEGORY, "Global");

    this->addKeybindAction(owner, TriggerableAction {
        "Pause",
        "gd.play.pause",
        { KB_PLAY_CATEGORY, KB_EDITOR_CATEGORY },
        [](CCNode* context, keybind_category_id const& category, bool push) -> bool {
            if (!push) return false;
            switch (category) {
                case hash(KB_PLAY_CATEGORY):
                    as<PlayLayer*>(context)->m_UILayer->onPause(nullptr);
                    break;
                case hash(KB_EDITOR_CATEGORY):
                    as<EditorUI*>(context)->onPause(nullptr);
                    break;
            }
            return false;
        },
        "Pause ingame / in the editor"s
    }, {{ KEY_Escape, 0 }});

    this->addKeybindAction(owner, TriggerableAction {
        "Jump P1",
        "gd.play.jump_p1",
        { KB_PLAY_CATEGORY, KB_EDITOR_CATEGORY },
        [](CCNode* context, keybind_category_id const& category, bool push) -> bool {
            GJBaseGameLayer* layer = as<GJBaseGameLayer*>(context);
            if (category == KB_EDITOR_CATEGORY) {
                layer = as<EditorUI*>(context)->m_editorLayer;
            }
            if (push) {
                layer->pushButton(0, true);
            } else {
                layer->releaseButton(0, true);
            }
            return false;
        },
        "Player 1 Jump"s
    }, {{ KEY_Space, 0 }});

    this->addKeybindAction(owner, TriggerableAction {
        "Jump P2",
        "gd.play.jump_p2",
        { KB_PLAY_CATEGORY, KB_EDITOR_CATEGORY },
        [](CCNode* context, keybind_category_id const& category, bool push) -> bool {
            GJBaseGameLayer* layer = as<GJBaseGameLayer*>(context);
            if (category == KB_EDITOR_CATEGORY) {
                layer = as<EditorUI*>(context)->m_editorLayer;
            }
            if (layer) {
                if (push) {
                    layer->pushButton(0, false);
                } else {
                    layer->releaseButton(0, false);
                }
            }
            return false;
        },
        "Player 2 Jump"s
    }, {{ KEY_Up, 0 }});

    this->addKeybindAction(owner, RepeatableAction {
        "Place Checkpoint",
        "gd.play.place_checkpoint",
        KB_PLAY_CATEGORY,
        [](CCNode* context, bool push) -> bool {
            if (push) {
                auto pl = as<PlayLayer*>(context);
                if (pl->m_isPracticeMode) {
                    pl->m_UILayer->onCheck(nullptr);
                }
            }
            return false;
        },
        "Place a Checkpoint in Practice Mode"s
    }, {{ KEY_Z, 0 }});

    this->addKeybindAction(owner, RepeatableAction {
        "Delete Checkpoint",
        "gd.play.delete_checkpoint",
        KB_PLAY_CATEGORY,
        [](CCNode* context, bool push) -> bool {
            if (push) {
                auto pl = as<PlayLayer*>(context);
                if (pl->m_isPracticeMode) {
                    pl->m_UILayer->onDeleteCheck(nullptr);
                }
            }
            return false;
        },
        "Delete the last Checkpoint in Practice Mode"s
    }, {{ KEY_X, 0 }});

    ////////////////////////

    { ADD_EDITOR_KB( "Build Mode",
        "gd.edit.build_mode",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->toggleMode(ui->m_buildModeBtn);
        ),
        "Toggle the Build Tab",
        BIND( KEY_One, 0 )
    ); }

    { ADD_EDITOR_KB( "Edit Mode",
        "gd.edit.edit_mode",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->toggleMode(ui->m_editModeBtn);
        ),
        "Toggle the Edit Tab",
        BIND( KEY_Two, 0 )
    ); }

    { ADD_EDITOR_KB( "Delete Mode",
        "gd.edit.delete_mode",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->toggleMode(ui->m_deleteModeBtn);
        ),
        "Toggle the Delete Tab",
        BIND( KEY_Three, 0 )
    ); }

    // this looks ugly as shit, but it's so when i fold it
    // in vs code i can see what keybind it is without
    // needing to unfold
    this->addKeybindAction(owner, KeybindModifier { "Swipe Modifier",
        "gd.edit.swipe_modifier",
        KB_EDITOR_CATEGORY,
        KB_SUBCATEGORY_GLOBAL,
        "When the Swipe Modifier is enabled, clicking anywhere "
        "in the editor enables swipe until the mouse is released"
    }, {{ Keybind::kmShift }});

    this->addKeybindAction(owner, KeybindModifier { "Move Modifier",
        "gd.edit.move_modifier",
        KB_EDITOR_CATEGORY,
        KB_SUBCATEGORY_GLOBAL,
        "If Swipe is enabled, pressing the Move Modifier lets you "
        "move the screen around"
    }, {{ KEY_Space, 0 }});

    this->addKeybindAction(owner, KeybindModifier { "Free Move Modifier",
        "gd.edit.free_move_modifier",
        KB_EDITOR_CATEGORY,
        KB_SUBCATEGORY_GLOBAL,
        "When you press with the mouse, Free Move is enabled "
        "until the mouse button is released"
    }, {{ Keybind::kmControl }});

    this->addKeybindAction(owner, KeybindModifier { "Copy Modifier",
        "gd.edit.duplicate_modifier",
        KB_EDITOR_CATEGORY,
        KB_SUBCATEGORY_GLOBAL,
        "When you press with the mouse, the selected object(s) are"
        "duplicated and Free Move is enabled until the mouse "
        "button is released"
    }, {{ Keybind::kmControl | Keybind::kmAlt }});

    { ADD_EDITOR_KB( "Rotate CCW",
        "gd.edit.rotate_ccw",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->transformObjectCall(EditCommand::RotateCCW);
        ),
        "Rotate Object Counter-Clockwise",
        BIND( KEY_Q, 0 )
    ); }

    { ADD_EDITOR_KB( "Rotate CW",
        "gd.edit.rotate_cw",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->transformObjectCall(EditCommand::RotateCW);
        ),
        "Rotate Object Clockwise",
        BIND( KEY_E, 0 )
    ); }

    { ADD_EDITOR_KB( "Flip X",
        "gd.edit.flip_x",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->transformObjectCall(EditCommand::FlipX);
        ),
        "Flip Object Along the X-axis",
        KBS(Alt, Q)
    ); }

    { ADD_EDITOR_KB( "Flip Y",
        "gd.edit.flip_y",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->transformObjectCall(EditCommand::FlipY);
        ),
        "Flip Object Along the Y-axis",
        KBS(Alt, E)
    ); }

    { ADD_EDITOR_KB( "Delete Selected",
        "gd.edit.delete_selected",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->onDeleteSelected(nullptr);
        ),
        "Delete Selected Object(s)",
        BIND( KEY_Delete, 0 )
    ); }

    { ADD_EDITOR_KB( "Undo",
        "gd.edit.undo",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->undoLastAction(nullptr);
        ),
        "Undo Last Action",
        BIND( KEY_Z, Keybind::kmControl )
    ); }

    { ADD_EDITOR_KB( "Redo",
        "gd.edit.redo",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->redoLastAction(nullptr);
        ),
        "Redo Last Action",
        BIND( KEY_Z, Keybind::kmControl | Keybind::kmShift )
    ); }

    { ADD_EDITOR_KB( "Deselect All",
        "gd.edit.deselect",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->deselectAll();
        ),
        "Deselect All Selected Objects",
        BIND( KEY_D, Keybind::kmAlt )
    ); }

    { ADD_EDITOR_KB( "Copy",
        "gd.edit.copy",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->onCopy(nullptr);
        ),
        "Copy Selected Objects",
        BIND( KEY_C, Keybind::kmControl )
    ); }

    { ADD_EDITOR_KB( "Paste",
        "gd.edit.paste",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->onPaste(nullptr);
        ),
        "Paste Copied Objects",
        BIND( KEY_V, Keybind::kmControl )
    ); }

    { ADD_EDITOR_KB( "Duplicate",
        "gd.edit.copy_and_paste",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->onDuplicate(nullptr);
        ),
        "Copy + Paste Selected Objects",
        BIND( KEY_D, Keybind::kmControl )
    ); }

    { ADD_EDITOR_KB( "Rotate",
        "gd.edit.toggle_rotate",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->toggleEnableRotate(nullptr);
        ),
        "Toggle Rotate Control",
        BIND( KEY_R, 0 )
    ); }

    { ADD_EDITOR_KB( "Free Move",
        "gd.edit.toggle_free_move",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->toggleFreeMove(nullptr);
        ),
        "Toggle Free Move",
        BIND( KEY_F, 0 )
    ); }

    { ADD_EDITOR_KB( "Swipe",
        "gd.edit.toggle_swipe",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->toggleSwipe(nullptr);
        ),
        "Toggle Swipe",
        BIND( KEY_T, 0 )
    ); }

    { ADD_EDITOR_KB( "Snap",
        "gd.edit.toggle_snap",
        KB_SUBCATEGORY_MODIFY,
        EDIT_ACTION(
            ui->toggleSnap(nullptr);
        ),
        "Toggle Snap",
        BIND( KEY_G, 0 )
    ); }

    { ADD_EDITOR_KB( "Playtest",
        "gd.edit.playtest",
        KB_SUBCATEGORY_MODIFY,
        CLICKED(UI(
            if (ui->m_editorLayer->m_playbackMode == PlaybackMode::Playing) {
                ui->onStopPlaytest(nullptr);
            } else {
                ui->onPlaytest(nullptr);
            }
        )),
        "Begin / Stop Playtest",
        BIND( KEY_Enter, 0 )
    ); }

    { ADD_EDITOR_KB( "Playback Music",
        "gd.edit.playback_music",
        KB_SUBCATEGORY_MODIFY,
        CLICKED(UI(
            if (ui->m_editorLayer->m_playbackMode == PlaybackMode::Playing) {
                ui->onPlayback(nullptr);
            }
        )),
        "Begin / Stop Playing the Level's Music",
        BIND( KEY_Enter, Keybind::kmControl )
    ); }

    { ADD_EDITOR_KB( "Previous Build Tab",
        "gd.edit.prev_build_tab",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            auto t = ui->m_selectedTab - 1;
            if (t < 0)
                t = ui->m_tabsArray->count() - 1;
            ui->selectBuildTab(t);
        ),
        "Previous Build Tab",
        BIND( KEY_F1, 0 )
    ); }

    { ADD_EDITOR_KB( "Next Build Tab",
        "gd.edit.next_build_tab",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            auto t = ui->m_selectedTab + 1;
            if (t > static_cast<int>(ui->m_tabsArray->count() - 1))
                t = 0;
            ui->selectBuildTab(t);
        ),
        "Next Build Tab",
        BIND( KEY_F2, 0 )
    ); }

    { ADD_EDITOR_KB( "Next Layer",
        "gd.edit.next_layer",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->onGroupUp(nullptr);
        ),
        "Go to Next Editor Layer (Named \"Next Group\" in Vanilla GD)",
        BIND( KEY_Right, 0 )
    ); }

    { ADD_EDITOR_KB( "Previous Layer",
        "gd.edit.prev_layer",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->onGroupDown(nullptr);
        ),
        "Go to Next Previous Layer (Named \"Previous Group\" in Vanilla GD)",
        BIND( KEY_Left, 0 )
    ); }

    { ADD_EDITOR_KB( "Scroll Up",
        "gd.edit.scroll_up",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->moveGameLayer({ 0.0f, 10.0f });
        ),
        "Scroll the Editor Up",
        BIND( KEY_OEMPlus, 0 )
    ); }

    { ADD_EDITOR_KB( "Scroll Down",
        "gd.edit.scroll_down",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->moveGameLayer({ 0.0f, -10.0f });
        ),
        "Scroll the Editor Down",
        BIND( KEY_OEMMinus, 0 )
    ); }

    { ADD_EDITOR_KB( "Zoom In",
        "gd.edit.zoom_in",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->zoomIn(nullptr);
        ),
        "Zoom In",
        BIND( KEY_OEMPlus, Keybind::kmShift )
    ); }

    { ADD_EDITOR_KB( "Zoom Out",
        "gd.edit.zoom_out",
        KB_SUBCATEGORY_UI,
        EDIT_ACTION(
            ui->zoomOut(nullptr);
        ),
        "Zoom Out",
        BIND( KEY_OEMMinus, Keybind::kmShift )
    ); }

    { ADD_EDITOR_KB( "Object Left",
        "gd.edit.move_obj_left",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::Left);
        ),
        "Move Object Left 1 Block (30 Units)",
        BIND( KEY_A, 0 )
    ); }

    { ADD_EDITOR_KB( "Object Right",
        "gd.edit.move_obj_right",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::Right);
        ),
        "Move Object Right 1 Block (30 Units)",
        BIND( KEY_D, 0 )
    ); }

    { ADD_EDITOR_KB( "Object Up",
        "gd.edit.move_obj_up",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::Up);
        ),
        "Move Object Up 1 Block (30 Units)",
        BIND( KEY_W, 0 )
    ); }

    { ADD_EDITOR_KB( "Object Down",
        "gd.edit.move_obj_down",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::Down);
        ),
        "Move Object Down 1 Block (30 Units)",
        BIND( KEY_S, 0 )
    ); }

    { ADD_EDITOR_KB( "Object Left Small",
        "gd.edit.move_obj_left_small",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::SmallLeft);
        ),
        "Move Object Left 2 Units",
        BIND( KEY_A, Keybind::kmShift )
    ); }

    { ADD_EDITOR_KB( "Object Right Small",
        "gd.edit.move_obj_right_small",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::SmallRight);
        ),
        "Move Object Right 2 Units",
        BIND( KEY_D, Keybind::kmShift )
    ); }

    { ADD_EDITOR_KB( "Object Up Small",
        "gd.edit.move_obj_up_small",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::SmallUp);
        ),
        "Move Object Up 2 Units",
        BIND( KEY_W, Keybind::kmShift )
    ); }

    { ADD_EDITOR_KB( "Object Down Small",
        "gd.edit.move_obj_down_small",
        KB_SUBCATEGORY_MOVE,
        EDIT_ACTION(
            ui->moveObjectCall(EditCommand::SmallDown);
        ),
        "Move Object Down 2 Units",
        BIND( KEY_S, Keybind::kmShift )
    ); }
}
