#include <Urho3D/Urho3DAll.h>
#include "MainMenu.h"
#include "../MyEvents.h"
#include "../Audio/AudioManagerDefs.h"
#include "../UI/NuklearUI.h"
#include <ctime>

using namespace Levels;

    /// Construct.
MainMenu::MainMenu(Context* context) :
    BaseLevel(context),
    _showGUI(true),
    _active(true)
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::Init()
{
    if (data_.Contains("Message")) {
        //SharedPtr<Urho3D::MessageBox> messageBox(new Urho3D::MessageBox(context_, data_["Message"].GetString(), "Oh crap!"));
        VariantMap data = GetEventDataMap();
        data["Title"] = "Error!";
        data["Message"] = data_["Message"].GetString();
        SendEvent("ShowAlertMessage", data);
    }
    BaseLevel::Init();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();
    VariantMap data = GetEventDataMap();
    data["Message"] = "Entered menu!";
    SendEvent("NewAchievement", data);

  /*  data["Title"] = "Hey!";
    data["Message"] = "Seems like everything is ok!";
    SendEvent("ShowAlertMessage", data);*/
}

void MainMenu::CreateScene()
{
    
}

void MainMenu::CreateUI()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void MainMenu::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MainMenu, HandleUpdate));
}

void MainMenu::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    draw();
}

void MainMenu::draw()
{
    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();

    nk_style_default(ctx);

    ctx->style.window.background = nk_rgba(0,0,0,0);
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0,0,0,0));
    ctx->style.window.border_color = nk_rgb(255,165,0);
    ctx->style.window.combo_border_color = nk_rgb(255,165,0);
    ctx->style.window.contextual_border_color = nk_rgb(255,165,0);
    ctx->style.window.menu_border_color = nk_rgb(255,165,0);
    ctx->style.window.group_border_color = nk_rgb(255,165,0);
    ctx->style.window.tooltip_border_color = nk_rgb(255,165,0);
    ctx->style.window.scrollbar_size = nk_vec2(16,16);
    ctx->style.window.border_color = nk_rgba(0,0,0,0);
    ctx->style.window.border = 1;

    if (nk_begin(nuklear->GetNkContext(), "Menu", nk_rect(graphics->GetWidth() - 200, graphics->GetHeight() - 200, 190, 190), NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(nuklear->GetNkContext(), "Start")) {
            if (_active) {
                {
                    using namespace AudioDefs;
                    using namespace MyEvents::PlaySound;
                    VariantMap data = GetEventDataMap();
                    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
                    data[P_TYPE] = SOUND_EFFECT;
                    SendEvent(MyEvents::E_PLAY_SOUND, data);
                }
                VariantMap& levelEventData = GetEventDataMap();
                levelEventData["Name"] = "Loading";
                SendEvent(MyEvents::E_SET_LEVEL, levelEventData);
                _active = false;
            }
        }
        nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(nuklear->GetNkContext(), "Settings")) {
            if (_active) {
                {
                    using namespace AudioDefs;
                    using namespace MyEvents::PlaySound;
                    VariantMap data = GetEventDataMap();
                    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
                    data[P_TYPE] = SOUND_EFFECT;
                    SendEvent(MyEvents::E_PLAY_SOUND, data);
                }
                VariantMap data = GetEventDataMap();
                data["Name"] = "SettingsWindow";
                SendEvent(MyEvents::E_OPEN_WINDOW, data);
            }
        }
        nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(nuklear->GetNkContext(), "Credits")) {
            if (_active) {
                {
                    using namespace AudioDefs;
                    using namespace MyEvents::PlaySound;
                    VariantMap data = GetEventDataMap();
                    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
                    data[P_TYPE] = SOUND_EFFECT;
                    SendEvent(MyEvents::E_PLAY_SOUND, data);
                }
                VariantMap& levelEventData = GetEventDataMap();
                levelEventData["Name"] = "Credits";
                SendEvent(MyEvents::E_SET_LEVEL, levelEventData);
                _active = false;
            }
        }
        nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(nuklear->GetNkContext(), "Exit")) {
            if (_active) {
                {
                    using namespace AudioDefs;
                    using namespace MyEvents::PlaySound;
                    VariantMap data = GetEventDataMap();
                    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
                    data[P_TYPE] = SOUND_EFFECT;
                    SendEvent(MyEvents::E_PLAY_SOUND, data);
                }
                VariantMap& levelEventData = GetEventDataMap();
                levelEventData["Name"] = "ExitGame";
                SendEvent(MyEvents::E_SET_LEVEL, levelEventData);
                _active = false;
            }
        }
        nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
    }
    nk_end(ctx);
    
    // enum {EASY, HARD};
    // static int op = EASY;
    // static float value = 0.6f;
    // static int i =  20;
    // static size_t progress = 1;
    // enum options {A,B,C};
    // static int checkbox;
    // static int option;

    // // Get logo texture
    // SharedPtr< ResourceCache > const cache(GetSubsystem< ResourceCache >());
    // SharedPtr< Texture2D > const LogoTexture(cache->GetResource< Texture2D >("Textures/UrhoIcon.png"));
    // struct nk_image logo;
    // logo = nk_image_ptr((void*)LogoTexture.Get());
    // if (_showGUI && nk_begin(nuklear->GetNkContext(), "DemoWindow", nk_rect(50, 50, 400, 400), NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE)) {
    //     /* fixed widget pixel width */
    //     nk_layout_row_static(nuklear->GetNkContext(), 30, 80, 1);
    //     if (nk_button_label(nuklear->GetNkContext(), "button")) {
    //         /* event handling */
    //     }

    //     /* fixed widget window ratio width */
    //     nk_layout_row_dynamic(nuklear->GetNkContext(), 30, 2);
    //     if (nk_option_label(nuklear->GetNkContext(), "easy", op == EASY)) op = EASY;
    //     if (nk_option_label(nuklear->GetNkContext(), "hard", op == HARD)) op = HARD;

    //     /* custom widget pixel width */
    //     nk_layout_row_begin(nuklear->GetNkContext(), NK_STATIC, 30, 2);
    //     {
    //         nk_layout_row_push(nuklear->GetNkContext(), 50);
    //         nk_label(nuklear->GetNkContext(), "Volume:", NK_TEXT_LEFT);
    //         nk_layout_row_push(nuklear->GetNkContext(), 110);
    //         nk_slider_float(nuklear->GetNkContext(), 0, &value, 1.0f, 0.05f);
    //     }
    //     nk_layout_row_end(nuklear->GetNkContext());

    //     nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
    //     {
    //         nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
    //         nk_progress(nuklear->GetNkContext(), &progress, 1000, NK_MODIFIABLE);
    //     }
    //     nk_layout_row_end(nuklear->GetNkContext());

    //     nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 150, 1);
    //     {
    //         nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
    //         nk_label_wrap(nuklear->GetNkContext(), "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged.");
    //     }
    //     nk_layout_row_end(nuklear->GetNkContext());

    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Text", NK_MINIMIZED))
    //     {
    //         /* Text Widgets */
    //         nk_layout_row_dynamic(nuklear->GetNkContext(), 20, 1);
    //         nk_label(nuklear->GetNkContext(), "Label aligned left", NK_TEXT_LEFT);
    //         nk_label(nuklear->GetNkContext(), "Label aligned centered", NK_TEXT_CENTERED);
    //         nk_label(nuklear->GetNkContext(), "Label aligned right", NK_TEXT_RIGHT);
    //         nk_label_colored(nuklear->GetNkContext(), "Blue text", NK_TEXT_LEFT, nk_rgb(0,0,255));
    //         nk_label_colored(nuklear->GetNkContext(), "Yellow text", NK_TEXT_LEFT, nk_rgb(255,255,0));
    //         nk_text(nuklear->GetNkContext(), "Text without /0", 15, NK_TEXT_RIGHT);

    //         nk_layout_row_static(nuklear->GetNkContext(), 100, 200, 1);
    //         nk_label_wrap(nuklear->GetNkContext(), "This is a very long line to hopefully get this text to be wrapped into multiple lines to show line wrapping");
    //         nk_layout_row_dynamic(nuklear->GetNkContext(), 100, 1);
    //         nk_label_wrap(nuklear->GetNkContext(), "This is another long text to show dynamic window changes on multiline text");
    //         nk_tree_pop(nuklear->GetNkContext());
    //     }
    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Image", NK_MINIMIZED))
    //         {
    //         nk_layout_row_dynamic(nuklear->GetNkContext(), 70, 1);
    //         if (logo.handle.ptr == NULL)
    //             {

    //         }
    //         else {
    //             nk_button_image_label(nuklear->GetNkContext(), logo, "logo", NK_TEXT_CENTERED);
    //         }
    //         nk_tree_pop(nuklear->GetNkContext());
    //     }
    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Button", NK_MINIMIZED))
    //     {
    //         /* Buttons Widgets */
    //         nk_layout_row_static(nuklear->GetNkContext(), 30, 100, 3);
    //         if (nk_button_label(nuklear->GetNkContext(), "Button"))
    //             fprintf(stdout, "Button pressed!\n");
    //         nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_REPEATER);
    //         if (nk_button_label(nuklear->GetNkContext(), "Repeater"))
    //             fprintf(stdout, "Repeater is being pressed!\n");
    //         nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
    //         nk_button_color(nuklear->GetNkContext(), nk_rgb(0,0,255));

    //         nk_layout_row_static(nuklear->GetNkContext(), 25, 25, 8);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_CIRCLE_SOLID);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_CIRCLE_OUTLINE);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_RECT_SOLID);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_RECT_OUTLINE);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_UP);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_DOWN);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_LEFT);
    //         nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_RIGHT);

    //         nk_layout_row_static(nuklear->GetNkContext(), 30, 100, 2);
    //         nk_button_symbol_label(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT);
    //         nk_button_symbol_label(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT);
    //         nk_tree_pop(nuklear->GetNkContext());
    //     }
    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Basic", NK_MINIMIZED))
    //     {
    //         /* Basic widgets */
    //         static int int_slider = 5;
    //         static float float_slider = 2.5f;
    //         static size_t prog_value = 40;
    //         static float property_float = 2;
    //         static int property_int = 10;
    //         static int property_neg = 10;

    //         static float range_float_min = 0;
    //         static float range_float_max = 100;
    //         static float range_float_value = 50;
    //         static int range_int_min = 0;
    //         static int range_int_value = 2048;
    //         static int range_int_max = 4096;
    //         static const float ratio[] = {120, 150};

    //         nk_layout_row_static(nuklear->GetNkContext(), 30, 100, 1);
    //         nk_checkbox_label(nuklear->GetNkContext(), "Checkbox", &checkbox);

    //         nk_layout_row_static(nuklear->GetNkContext(), 30, 80, 3);
    //         option = nk_option_label(nuklear->GetNkContext(), "optionA", option == A) ? A : option;
    //         option = nk_option_label(nuklear->GetNkContext(), "optionB", option == B) ? B : option;
    //         option = nk_option_label(nuklear->GetNkContext(), "optionC", option == C) ? C : option;

    //         nk_layout_row(nuklear->GetNkContext(), NK_STATIC, 30, 2, ratio);
    //         nk_labelf(nuklear->GetNkContext(), NK_TEXT_LEFT, "Slider int");
    //         nk_slider_int(nuklear->GetNkContext(), 0, &int_slider, 10, 1);

    //         nk_label(nuklear->GetNkContext(), "Slider float", NK_TEXT_LEFT);
    //         nk_slider_float(nuklear->GetNkContext(), 0, &float_slider, 5.0, 0.5f);
    //         nk_labelf(nuklear->GetNkContext(), NK_TEXT_LEFT, "Progressbar: %zu" , prog_value);
    //         nk_progress(nuklear->GetNkContext(), &prog_value, 100, NK_MODIFIABLE);

    //         nk_layout_row(nuklear->GetNkContext(), NK_STATIC, 25, 2, ratio);
    //         nk_label(nuklear->GetNkContext(), "Property float:", NK_TEXT_LEFT);
    //         nk_property_float(nuklear->GetNkContext(), "Float:", 0, &property_float, 64.0f, 0.1f, 0.2f);
    //         nk_label(nuklear->GetNkContext(), "Property int:", NK_TEXT_LEFT);
    //         nk_property_int(nuklear->GetNkContext(), "Int:", 0, &property_int, 100.0f, 1, 1);
    //         nk_label(nuklear->GetNkContext(), "Property neg:", NK_TEXT_LEFT);
    //         nk_property_int(nuklear->GetNkContext(), "Neg:", -10, &property_neg, 10, 1, 1);

    //         nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 1);
    //         nk_label(nuklear->GetNkContext(), "Range:", NK_TEXT_LEFT);
    //         nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 3);
    //         nk_property_float(nuklear->GetNkContext(), "#min:", 0, &range_float_min, range_float_max, 1.0f, 0.2f);
    //         nk_property_float(nuklear->GetNkContext(), "#float:", range_float_min, &range_float_value, range_float_max, 1.0f, 0.2f);
    //         nk_property_float(nuklear->GetNkContext(), "#max:", range_float_min, &range_float_max, 100, 1.0f, 0.2f);

    //         nk_property_int(nuklear->GetNkContext(), "#min:", INT_MIN, &range_int_min, range_int_max, 1, 10);
    //         nk_property_int(nuklear->GetNkContext(), "#neg:", range_int_min, &range_int_value, range_int_max, 1, 10);
    //         nk_property_int(nuklear->GetNkContext(), "#max:", range_int_min, &range_int_max, INT_MAX, 1, 10);

    //         nk_tree_pop(nuklear->GetNkContext());
    //     }

    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Inactive", NK_MINIMIZED))
    //     {
    //         static int inactive = 1;
    //         nk_layout_row_dynamic(nuklear->GetNkContext(), 30, 1);
    //         nk_checkbox_label(nuklear->GetNkContext(), "Inactive", &inactive);

    //         nk_layout_row_static(nuklear->GetNkContext(), 30, 80, 1);
    //         if (inactive) {
    //             struct nk_style_button button;
    //             button = nuklear->GetNkContext()->style.button;
    //             nuklear->GetNkContext()->style.button.normal = nk_style_item_color(nk_rgb(40,40,40));
    //             nuklear->GetNkContext()->style.button.hover = nk_style_item_color(nk_rgb(40,40,40));
    //             nuklear->GetNkContext()->style.button.active = nk_style_item_color(nk_rgb(40,40,40));
    //             nuklear->GetNkContext()->style.button.border_color = nk_rgb(60,60,60);
    //             nuklear->GetNkContext()->style.button.text_background = nk_rgb(60,60,60);
    //             nuklear->GetNkContext()->style.button.text_normal = nk_rgb(60,60,60);
    //             nuklear->GetNkContext()->style.button.text_hover = nk_rgb(60,60,60);
    //             nuklear->GetNkContext()->style.button.text_active = nk_rgb(60,60,60);
    //             nk_button_label(nuklear->GetNkContext(), "button");
    //             nuklear->GetNkContext()->style.button = button;
    //         } else if (nk_button_label(nuklear->GetNkContext(), "button"))
    //             fprintf(stdout, "button pressed\n");
    //         nk_tree_pop(nuklear->GetNkContext());
    //     }


    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Selectable", NK_MINIMIZED))
    //     {
    //         if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "List", NK_MINIMIZED))
    //         {
    //             static int selected[4] = {nk_false, nk_false, nk_true, nk_false};
    //             nk_layout_row_static(nuklear->GetNkContext(), 18, 100, 1);
    //             nk_selectable_label(nuklear->GetNkContext(), "Selectable", NK_TEXT_LEFT, &selected[0]);
    //             nk_selectable_label(nuklear->GetNkContext(), "Selectable", NK_TEXT_LEFT, &selected[1]);
    //             nk_label(nuklear->GetNkContext(), "Not Selectable", NK_TEXT_LEFT);
    //             nk_selectable_label(nuklear->GetNkContext(), "Selectable", NK_TEXT_LEFT, &selected[2]);
    //             nk_selectable_label(nuklear->GetNkContext(), "Selectable", NK_TEXT_LEFT, &selected[3]);
    //             nk_tree_pop(nuklear->GetNkContext());
    //         }
    //         if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Grid", NK_MINIMIZED))
    //         {
    //             int i;
    //             static int selected[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    //             nk_layout_row_static(nuklear->GetNkContext(), 50, 50, 4);
    //             for (i = 0; i < 16; ++i) {
    //                 if (nk_selectable_label(nuklear->GetNkContext(), "Z", NK_TEXT_CENTERED, &selected[i])) {
    //                     int x = (i % 4), y = i / 4;
    //                     if (x > 0) selected[i - 1] ^= 1;
    //                     if (x < 3) selected[i + 1] ^= 1;
    //                     if (y > 0) selected[i - 4] ^= 1;
    //                     if (y < 3) selected[i + 4] ^= 1;
    //                 }
    //             }
    //             nk_tree_pop(nuklear->GetNkContext());
    //         }
    //         nk_tree_pop(nuklear->GetNkContext());
    //     }

    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Combo", NK_MINIMIZED))
    //     {
    //         /* Combobox Widgets
    //             * In this library comboboxes are not limited to being a popup
    //             * list of selectable text. Instead it is a abstract concept of
    //             * having something that is *selected* or displayed, a popup window
    //             * which opens if something needs to be modified and the content
    //             * of the popup which causes the *selected* or displayed value to
    //             * change or if wanted close the combobox.
    //             *
    //             * While strange at first handling comboboxes in a abstract way
    //             * solves the problem of overloaded window content. For example
    //             * changing a color value requires 4 value modifier (slider, property,...)
    //             * for RGBA then you need a label and ways to display the current color.
    //             * If you want to go fancy you even add rgb and hsv ratio boxes.
    //             * While fine for one color if you have a lot of them it because
    //             * tedious to look at and quite wasteful in space. You could add
    //             * a popup which modifies the color but this does not solve the
    //             * fact that it still requires a lot of cluttered space to do.
    //             *
    //             * In these kind of instance abstract comboboxes are quite handy. All
    //             * value modifiers are hidden inside the combobox popup and only
    //             * the color is shown if not open. This combines the clarity of the
    //             * popup with the ease of use of just using the space for modifiers.
    //             *
    //             * Other instances are for example time and especially date picker,
    //             * which only show the currently activated time/data and hide the
    //             * selection logic inside the combobox popup.
    //             */
    //         static float chart_selection = 8.0f;
    //         static int current_weapon = 0;
    //         static int check_values[5];
    //         static float position[3];
    //         static struct nk_color combo_color = {130, 50, 50, 255};
    //         static struct nk_colorf combo_color2 = {0.509f, 0.705f, 0.2f, 1.0f};
    //         static size_t prog_a =  20, prog_b = 40, prog_c = 10, prog_d = 90;
    //         static const char *weapons[] = {"Fist","Pistol","Shotgun","Plasma","BFG"};

    //         char buffer[64];
    //         size_t sum = 0;

    //         /* default combobox */
    //         nk_layout_row_static(nuklear->GetNkContext(), 25, 200, 1);
    //         current_weapon = nk_combo(nuklear->GetNkContext(), weapons, NK_LEN(weapons), current_weapon, 25, nk_vec2(200,200));

    //         /* slider color combobox */
    //         if (nk_combo_begin_color(nuklear->GetNkContext(), combo_color, nk_vec2(200,200))) {
    //             float ratios[] = {0.15f, 0.85f};
    //             nk_layout_row(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2, ratios);
    //             nk_label(nuklear->GetNkContext(), "R:", NK_TEXT_LEFT);
    //             combo_color.r = (nk_byte)nk_slide_int(nuklear->GetNkContext(), 0, combo_color.r, 255, 5);
    //             nk_label(nuklear->GetNkContext(), "G:", NK_TEXT_LEFT);
    //             combo_color.g = (nk_byte)nk_slide_int(nuklear->GetNkContext(), 0, combo_color.g, 255, 5);
    //             nk_label(nuklear->GetNkContext(), "B:", NK_TEXT_LEFT);
    //             combo_color.b = (nk_byte)nk_slide_int(nuklear->GetNkContext(), 0, combo_color.b, 255, 5);
    //             nk_label(nuklear->GetNkContext(), "A:", NK_TEXT_LEFT);
    //             combo_color.a = (nk_byte)nk_slide_int(nuklear->GetNkContext(), 0, combo_color.a , 255, 5);
    //             nk_combo_end(nuklear->GetNkContext());
    //         }
    //         /* complex color combobox */
    //         if (nk_combo_begin_color(nuklear->GetNkContext(), nk_rgb_cf(combo_color2), nk_vec2(200,400))) {
    //             enum color_mode {COL_RGB, COL_HSV};
    //             static int col_mode = COL_RGB;
    //             #ifndef DEMO_DO_NOT_USE_COLOR_PICKER
    //             nk_layout_row_dynamic(nuklear->GetNkContext(), 120, 1);
    //             combo_color2 = nk_color_picker(nuklear->GetNkContext(), combo_color2, NK_RGBA);
    //             #endif

    //             nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
    //             col_mode = nk_option_label(nuklear->GetNkContext(), "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
    //             col_mode = nk_option_label(nuklear->GetNkContext(), "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;

    //             nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 1);
    //             if (col_mode == COL_RGB) {
    //                 combo_color2.r = nk_propertyf(nuklear->GetNkContext(), "#R:", 0, combo_color2.r, 1.0f, 0.01f,0.005f);
    //                 combo_color2.g = nk_propertyf(nuklear->GetNkContext(), "#G:", 0, combo_color2.g, 1.0f, 0.01f,0.005f);
    //                 combo_color2.b = nk_propertyf(nuklear->GetNkContext(), "#B:", 0, combo_color2.b, 1.0f, 0.01f,0.005f);
    //                 combo_color2.a = nk_propertyf(nuklear->GetNkContext(), "#A:", 0, combo_color2.a, 1.0f, 0.01f,0.005f);
    //             } else {
    //                 float hsva[4];
    //                 nk_colorf_hsva_fv(hsva, combo_color2);
    //                 hsva[0] = nk_propertyf(nuklear->GetNkContext(), "#H:", 0, hsva[0], 1.0f, 0.01f,0.05f);
    //                 hsva[1] = nk_propertyf(nuklear->GetNkContext(), "#S:", 0, hsva[1], 1.0f, 0.01f,0.05f);
    //                 hsva[2] = nk_propertyf(nuklear->GetNkContext(), "#V:", 0, hsva[2], 1.0f, 0.01f,0.05f);
    //                 hsva[3] = nk_propertyf(nuklear->GetNkContext(), "#A:", 0, hsva[3], 1.0f, 0.01f,0.05f);
    //                 combo_color2 = nk_hsva_colorfv(hsva);
    //             }
    //             nk_combo_end(nuklear->GetNkContext());
    //         }
    //         /* progressbar combobox */
    //         sum = prog_a + prog_b + prog_c + prog_d;
    //         sprintf(buffer, "%lu", sum);
    //         if (nk_combo_begin_label(nuklear->GetNkContext(), buffer, nk_vec2(200,200))) {
    //             nk_layout_row_dynamic(nuklear->GetNkContext(), 30, 1);
    //             nk_progress(nuklear->GetNkContext(), &prog_a, 100, NK_MODIFIABLE);
    //             nk_progress(nuklear->GetNkContext(), &prog_b, 100, NK_MODIFIABLE);
    //             nk_progress(nuklear->GetNkContext(), &prog_c, 100, NK_MODIFIABLE);
    //             nk_progress(nuklear->GetNkContext(), &prog_d, 100, NK_MODIFIABLE);
    //             nk_combo_end(nuklear->GetNkContext());
    //         }

    //         /* checkbox combobox */
    //         sum = (size_t)(check_values[0] + check_values[1] + check_values[2] + check_values[3] + check_values[4]);
    //         sprintf(buffer, "%lu", sum);
    //         if (nk_combo_begin_label(nuklear->GetNkContext(), buffer, nk_vec2(200,200))) {
    //             nk_layout_row_dynamic(nuklear->GetNkContext(), 30, 1);
    //             nk_checkbox_label(nuklear->GetNkContext(), weapons[0], &check_values[0]);
    //             nk_checkbox_label(nuklear->GetNkContext(), weapons[1], &check_values[1]);
    //             nk_checkbox_label(nuklear->GetNkContext(), weapons[2], &check_values[2]);
    //             nk_checkbox_label(nuklear->GetNkContext(), weapons[3], &check_values[3]);
    //             nk_combo_end(nuklear->GetNkContext());
    //         }

    //         /* complex text combobox */
    //         sprintf(buffer, "%.2f, %.2f, %.2f", position[0], position[1],position[2]);
    //         if (nk_combo_begin_label(nuklear->GetNkContext(), buffer, nk_vec2(200,200))) {
    //             nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 1);
    //             nk_property_float(nuklear->GetNkContext(), "#X:", -1024.0f, &position[0], 1024.0f, 1,0.5f);
    //             nk_property_float(nuklear->GetNkContext(), "#Y:", -1024.0f, &position[1], 1024.0f, 1,0.5f);
    //             nk_property_float(nuklear->GetNkContext(), "#Z:", -1024.0f, &position[2], 1024.0f, 1,0.5f);
    //             nk_combo_end(nuklear->GetNkContext());
    //         }

    //         /* chart combobox */
    //         sprintf(buffer, "%.1f", chart_selection);
    //         if (nk_combo_begin_label(nuklear->GetNkContext(), buffer, nk_vec2(200,250))) {
    //             size_t i = 0;
    //             static const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f, 12.0f, 8.0f, 22.0f, 28.0f, 5.0f};
    //             nk_layout_row_dynamic(nuklear->GetNkContext(), 150, 1);
    //             nk_chart_begin(nuklear->GetNkContext(), NK_CHART_COLUMN, NK_LEN(values), 0, 50);
    //             for (i = 0; i < NK_LEN(values); ++i) {
    //                 nk_flags res = nk_chart_push(nuklear->GetNkContext(), values[i]);
    //                 if (res & NK_CHART_CLICKED) {
    //                     chart_selection = values[i];
    //                     nk_combo_close(nuklear->GetNkContext());
    //                 }
    //             }
    //             nk_chart_end(nuklear->GetNkContext());
    //             nk_combo_end(nuklear->GetNkContext());
    //         }

    //         {
    //             static int time_selected = 0;
    //             static int date_selected = 0;
    //             static struct tm sel_time;
    //             static struct tm sel_date;
    //             if (!time_selected || !date_selected) {
    //                 /* keep time and date updated if nothing is selected */
    //                 time_t cur_time = time(0);
    //                 struct tm *n = localtime(&cur_time);
    //                 if (!time_selected)
    //                     memcpy(&sel_time, n, sizeof(struct tm));
    //                 if (!date_selected)
    //                     memcpy(&sel_date, n, sizeof(struct tm));
    //             }

    //             /* time combobox */
    //             sprintf(buffer, "%02d:%02d:%02d", sel_time.tm_hour, sel_time.tm_min, sel_time.tm_sec);
    //             if (nk_combo_begin_label(nuklear->GetNkContext(), buffer, nk_vec2(200,250))) {
    //                 time_selected = 1;
    //                 nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 1);
    //                 sel_time.tm_sec = nk_propertyi(nuklear->GetNkContext(), "#S:", 0, sel_time.tm_sec, 60, 1, 1);
    //                 sel_time.tm_min = nk_propertyi(nuklear->GetNkContext(), "#M:", 0, sel_time.tm_min, 60, 1, 1);
    //                 sel_time.tm_hour = nk_propertyi(nuklear->GetNkContext(), "#H:", 0, sel_time.tm_hour, 23, 1, 1);
    //                 nk_combo_end(nuklear->GetNkContext());
    //             }

    //             /* date combobox */
    //             sprintf(buffer, "%02d-%02d-%02d", sel_date.tm_mday, sel_date.tm_mon+1, sel_date.tm_year+1900);
    //             if (nk_combo_begin_label(nuklear->GetNkContext(), buffer, nk_vec2(350,400)))
    //             {
    //                 int i = 0;
    //                 const char *month[] = {"January", "February", "March",
    //                     "April", "May", "June", "July", "August", "September",
    //                     "October", "November", "December"};
    //                 const char *week_days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    //                 const int month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    //                 int year = sel_date.tm_year+1900;
    //                 int leap_year = (!(year % 4) && ((year % 100))) || !(year % 400);
    //                 int days = (sel_date.tm_mon == 1) ?
    //                     month_days[sel_date.tm_mon] + leap_year:
    //                     month_days[sel_date.tm_mon];

    //                 /* header with month and year */
    //                 date_selected = 1;
    //                 nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 20, 3);
    //                 nk_layout_row_push(nuklear->GetNkContext(), 0.05f);
    //                 if (nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_LEFT)) {
    //                     if (sel_date.tm_mon == 0) {
    //                         sel_date.tm_mon = 11;
    //                         sel_date.tm_year = NK_MAX(0, sel_date.tm_year-1);
    //                     } else sel_date.tm_mon--;
    //                 }
    //                 nk_layout_row_push(nuklear->GetNkContext(), 0.9f);
    //                 sprintf(buffer, "%s %d", month[sel_date.tm_mon], year);
    //                 nk_label(nuklear->GetNkContext(), buffer, NK_TEXT_CENTERED);
    //                 nk_layout_row_push(nuklear->GetNkContext(), 0.05f);
    //                 if (nk_button_symbol(nuklear->GetNkContext(), NK_SYMBOL_TRIANGLE_RIGHT)) {
    //                     if (sel_date.tm_mon == 11) {
    //                         sel_date.tm_mon = 0;
    //                         sel_date.tm_year++;
    //                     } else sel_date.tm_mon++;
    //                 }
    //                 nk_layout_row_end(nuklear->GetNkContext());

    //                 /* good old week day formula (double because precision) */
    //                 {int year_n = (sel_date.tm_mon < 2) ? year-1: year;
    //                 int y = year_n % 100;
    //                 int c = year_n / 100;
    //                 int y4 = (int)((float)y / 4);
    //                 int c4 = (int)((float)c / 4);
    //                 int m = (int)(2.6 * (double)(((sel_date.tm_mon + 10) % 12) + 1) - 0.2);
    //                 int week_day = (((1 + m + y + y4 + c4 - 2 * c) % 7) + 7) % 7;

    //                 /* weekdays  */
    //                 nk_layout_row_dynamic(nuklear->GetNkContext(), 35, 7);
    //                 for (i = 0; i < (int)NK_LEN(week_days); ++i)
    //                     nk_label(nuklear->GetNkContext(), week_days[i], NK_TEXT_CENTERED);

    //                 /* days  */
    //                 if (week_day > 0) nk_spacing(nuklear->GetNkContext(), week_day);
    //                 for (i = 1; i <= days; ++i) {
    //                     sprintf(buffer, "%d", i);
    //                     if (nk_button_label(nuklear->GetNkContext(), buffer)) {
    //                         sel_date.tm_mday = i;
    //                         nk_combo_close(nuklear->GetNkContext());
    //                     }
    //                 }}
    //                 nk_combo_end(nuklear->GetNkContext());
    //             }
    //         }

    //         nk_tree_pop(nuklear->GetNkContext());
    //     }

    //     if (nk_tree_push(nuklear->GetNkContext(), NK_TREE_NODE, "Input", NK_MINIMIZED))
    //     {
    //         static const float ratio[] = {120, 150};
    //         static char field_buffer[64];
    //         static char text[9][64];
    //         static int text_len[9];
    //         static char box_buffer[512];
    //         static int field_len;
    //         static int box_len;
    //         nk_flags active;

    //         nk_layout_row(nuklear->GetNkContext(), NK_STATIC, 25, 2, ratio);
    //         nk_label(nuklear->GetNkContext(), "Default:", NK_TEXT_LEFT);

    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, text[0], &text_len[0], 64, nk_filter_default);
    //         nk_label(nuklear->GetNkContext(), "Int:", NK_TEXT_LEFT);
    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, text[1], &text_len[1], 64, nk_filter_decimal);
    //         nk_label(nuklear->GetNkContext(), "Float:", NK_TEXT_LEFT);
    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, text[2], &text_len[2], 64, nk_filter_float);
    //         nk_label(nuklear->GetNkContext(), "Hex:", NK_TEXT_LEFT);
    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, text[4], &text_len[4], 64, nk_filter_hex);
    //         nk_label(nuklear->GetNkContext(), "Octal:", NK_TEXT_LEFT);
    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, text[5], &text_len[5], 64, nk_filter_oct);
    //         nk_label(nuklear->GetNkContext(), "Binary:", NK_TEXT_LEFT);
    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_SIMPLE, text[6], &text_len[6], 64, nk_filter_binary);

    //         nk_label(nuklear->GetNkContext(), "Password:", NK_TEXT_LEFT);
    //         {
    //             int i = 0;
    //             int old_len = text_len[8];
    //             char buffer[64];
    //             for (i = 0; i < text_len[8]; ++i) buffer[i] = '*';
    //             nk_edit_string(nuklear->GetNkContext(), NK_EDIT_FIELD, buffer, &text_len[8], 64, nk_filter_default);
    //             if (old_len < text_len[8])
    //                 memcpy(&text[8][old_len], &buffer[old_len], (nk_size)(text_len[8] - old_len));
    //         }

    //         nk_label(nuklear->GetNkContext(), "Field:", NK_TEXT_LEFT);
    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_FIELD, field_buffer, &field_len, 64, nk_filter_default);

    //         nk_label(nuklear->GetNkContext(), "Box:", NK_TEXT_LEFT);
    //         nk_layout_row_static(nuklear->GetNkContext(), 180, 278, 1);
    //         nk_edit_string(nuklear->GetNkContext(), NK_EDIT_BOX, box_buffer, &box_len, 512, nk_filter_default);

    //         nk_layout_row(nuklear->GetNkContext(), NK_STATIC, 25, 2, ratio);
    //         active = nk_edit_string(nuklear->GetNkContext(), NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, text[7], &text_len[7], 64,  nk_filter_ascii);
    //         if (nk_button_label(nuklear->GetNkContext(), "Submit") ||
    //             (active & NK_EDIT_COMMITED))
    //         {
    //             text[7][text_len[7]] = '\n';
    //             text_len[7]++;
    //             memcpy(&box_buffer[box_len], &text[7], (nk_size)text_len[7]);
    //             box_len += text_len[7];
    //             text_len[7] = 0;
    //         }
    //         nk_tree_pop(nuklear->GetNkContext());
    //     }
    //     progress += 1;
    //     if (progress >= 1000) {
    //         progress = 0;
    //     }
    // }

    // if (_showGUI) {
    //     nk_end(nuklear->GetNkContext());
    // }

    // if (nk_window_is_hidden(nuklear->GetNkContext(), "DemoWindow")) {
    //     _showGUI = false;
    //     nk_window_close(nuklear->GetNkContext(), "DemoWindow");
    // }
}
