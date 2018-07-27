#include <Urho3D/Urho3DAll.h>
#include "WeaponChoiceWindow.h"
#include "../../MyEvents.h"
#include "../../UI/NuklearUI.h"
#include "../../Audio/AudioManagerDefs.h"

static struct nk_image menu[6];
static struct nk_vec2 _rect;

/// Construct.
WeaponChoiceWindow::WeaponChoiceWindow(Context* context) :
    BaseWindow(context),
    _active(true)
{
    int width = 300;
    int height = 160;
    auto graphics = GetSubsystem<Graphics>();
    _rect.x = graphics->GetWidth() / 2;
    _rect.y = graphics->GetHeight() / 2;

    StringVector imageList = {
        "Textures/baseball-bat.png",
        "Textures/grenade.png",
        "Textures/luger.png",
        "Textures/slingshot.png",
        "Textures/tomahawk.png",
        "Textures/wood-club.png",
    };
    for (int i = 0; i < imageList.Size(); i++) {
        struct nk_image _image;
        auto nuklear = GetSubsystem<NuklearUI>();
        auto* cache = GetSubsystem<ResourceCache>();
        SharedPtr<Texture2D> imageTexture(cache->GetResource<Texture2D>(imageList.At(i)));
        if (imageTexture) {
            _image = nk_image_ptr((void*)imageTexture.Get());
        }
        menu[i] = _image;

        _imageTextures.Push(imageTexture);
    }

    Init();
}

WeaponChoiceWindow::~WeaponChoiceWindow()
{
}

void WeaponChoiceWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void WeaponChoiceWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void WeaponChoiceWindow::SubscribeToEvents()
{
}

void WeaponChoiceWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!_active) {
        return;
    }
    
    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();

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

    if (nk_begin(nuklear->GetNkContext(), "WeaponChoice", nk_rect(0, 0, 0, 0), NK_WINDOW_NO_SCROLLBAR)) {
        int ret = -1;
        struct nk_rect total_space;
        struct nk_rect bounds;
        int active_item = 0;

        /* pie menu popup */
        struct nk_vec2 pos{_rect.x, _rect.y};
        float radius = 200.0f;
        int item_count = 6;

        total_space  = nk_window_get_content_region(ctx);
        ctx->style.window.spacing = nk_vec2(0,0);
        ctx->style.window.padding = nk_vec2(0,0);
        ctx->style.window.popup_border_color = nk_rgba(0, 0, 0, 0);

        if (nk_popup_begin(ctx, NK_POPUP_STATIC, "piemenu", NK_WINDOW_NO_SCROLLBAR,
            nk_rect(pos.x - total_space.x - radius, pos.y - radius - total_space.y,
            2*radius,2*radius)))
        {
            int i = 0;
            struct nk_command_buffer* out = nk_window_get_canvas(ctx);
            const struct nk_input *in = &ctx->input;

            total_space = nk_window_get_content_region(ctx);
            ctx->style.window.spacing = nk_vec2(4,4);
            ctx->style.window.padding = nk_vec2(8,8);
            nk_layout_row_dynamic(ctx, total_space.h, 1);
            nk_widget(&bounds, ctx);

            /* outer circle */
            nk_fill_circle(out, bounds, nk_rgba(0,0,0, 0));
            {
                /* circle buttons */
                float step = (2 * M_PI) / (float)(Max(1,item_count));
                float a_min = 0; float a_max = step;

                struct nk_vec2 center = nk_vec2(bounds.x + bounds.w / 2.0f, bounds.y + bounds.h / 2.0f);
                struct nk_vec2 drag = nk_vec2(in->mouse.pos.x - center.x, in->mouse.pos.y - center.y);
                float angle = (float)atan2(drag.y, drag.x);
                if (angle < -0.0f) angle += 2.0f * 3.141592654f;
                active_item = (int)(angle/step);

                for (i = 0; i < item_count; ++i) {
                    struct nk_rect content;
                    float rx, ry, dx, dy, a;
                    nk_fill_arc(out, center.x, center.y, (bounds.w/2.0f),
                        a_min, a_max, (active_item == i) ? nk_rgba(45, 100, 255, 200): nk_rgba(60, 60, 60, 200));

                    /* separator line */
                    rx = bounds.w/2.0f; ry = 0;
                    dx = rx * (float)cos(a_min) - ry * (float)sin(a_min);
                    dy = rx * (float)sin(a_min) + ry * (float)cos(a_min);
                    nk_stroke_line(out, center.x, center.y,
                        center.x + dx, center.y + dy, 2.0f, nk_rgba(50, 50, 50, 200));

                    /* button content */
                    a = a_min + (a_max - a_min)/2.0f;
                    rx = bounds.w/2.5f; ry = 0;
                    content.w = 30; content.h = 30;
                    content.x = center.x + ((rx * (float)cos(a) - ry * (float)sin(a)) - content.w/2.0f);
                    content.y = center.y + (rx * (float)sin(a) + ry * (float)cos(a) - content.h/2.0f);
                    nk_draw_image(out, content, &menu[i], nk_rgba(255, 255, 255, 255));
                    a_min = a_max; a_max += step;
                }
            }
            {
                /* inner circle */
                struct nk_rect inner;
                inner.x = bounds.x + bounds.w/2 - bounds.w/4;
                inner.y = bounds.y + bounds.h/2 - bounds.h/4;
                inner.w = bounds.w/2; inner.h = bounds.h/2;
                nk_fill_circle(out, inner, nk_rgba(45, 45, 45, 255));

                /* active icon content */
                bounds.w = inner.w / 2.0f;
                bounds.h = inner.h / 2.0f;
                bounds.x = inner.x + inner.w/2 - bounds.w/2;
                bounds.y = inner.y + inner.h/2 - bounds.h/2;
                nk_draw_image(out, bounds, &menu[active_item], nk_rgba(255, 255, 255, 255));
            }
            nk_layout_space_end(ctx);
            if (!nk_input_is_mouse_down(&ctx->input, NK_BUTTON_RIGHT)) {
                nk_popup_close(ctx);
                ret = active_item;
            }
        } else ret = -2;
        ctx->style.window.spacing = nk_vec2(4,4);
        ctx->style.window.padding = nk_vec2(8,8);
        nk_popup_end(ctx);
    }
    nk_end(ctx);
}
