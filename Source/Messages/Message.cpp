#include <Urho3D/Urho3DAll.h>
#include "Message.h"
#include "../Audio/AudioManagerDefs.h"
#include "../MyEvents.h"
#include "../UI/NuklearUI.h"

static struct nk_rect _outerRect;
static struct nk_rect _innerRect;

/// Construct.
Message::Message(Context* context) :
    Object(context),
    _messageTime(0)
{
    Init();
    int width = 300;
    int height = 160;
    auto graphics = GetSubsystem<Graphics>();
    _innerRect.x = graphics->GetWidth() / 2 - width / 2;
    _innerRect.w = width;
    _innerRect.y = graphics->GetHeight() / 2 - height / 2;
    _innerRect.h = height;

    _outerRect.x = 0;
    _outerRect.w = graphics->GetWidth();
    _outerRect.y = 0;
    _outerRect.h = graphics->GetHeight();
}

Message::~Message()
{
}

void Message::Init()
{
    SubscribeToEvents();
}

bool Message::Create()
{
    SubscribeToEvents();
    return true;
}

void Message::SubscribeToEvents()
{
    SubscribeToEvent("ShowAlertMessage", URHO3D_HANDLER(Message, HandleShowMessage));
}

void Message::HandleShowMessage(StringHash eventType, VariantMap& eventData)
{
    _title = eventData["Title"].GetString();
    _message = eventData["Message"].GetString();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Message, HandleUpdate));
    _messageTime = 0.0f;
}

void Message::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    _messageTime += timeStep * 200;

    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    if (nuklear && ctx) {

        nk_style_default(ctx);

        int alpha = Sin(_messageTime) * 50 + 150;
        URHO3D_LOGINFO("Alpha " + String(alpha));
        ctx->style.window.background = nk_rgba(100, 50, 50, alpha);
        ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(100, 50, 50, alpha));
        ctx->style.window.border_color = nk_rgb(255, 165, 0);
        ctx->style.window.combo_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.contextual_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.menu_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.group_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.tooltip_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.scrollbar_size = nk_vec2(16, 16);
        ctx->style.window.border_color = nk_rgba(0, 0, 0, 0);
        ctx->style.window.border = 1;

        if (nk_begin(nuklear->GetNkContext(), "PopUpMessageOuter", _outerRect, NK_WINDOW_NO_SCROLLBAR)) {
            nk_style_default(ctx);
            //if (nk_begin(nuklear->GetNkContext(), "PopUpMessageInner", _innerRect, NK_WINDOW_NO_SCROLLBAR)) {
            if (nk_popup_begin(ctx, NK_POPUP_STATIC, "piemenu", NK_WINDOW_NO_SCROLLBAR, _innerRect))
            {
                nk_layout_row_dynamic(ctx, 1, 1);
                nk_spacing(nuklear->GetNkContext(), 1);
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(nuklear->GetNkContext(), _title.CString(), NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 2, 1);
                nk_spacing(nuklear->GetNkContext(), 1);

                nk_layout_row_dynamic(ctx, 60, 1);
                nk_label_wrap(nuklear->GetNkContext(), "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged.");

                nk_layout_row_dynamic(ctx, 30, 3);
                nk_spacing(nuklear->GetNkContext(), 1);
                if (nk_button_label(nuklear->GetNkContext(), "Ok")) {
                    UnsubscribeFromEvent(E_UPDATE);
                }
            }
            nk_popup_end(ctx);
        }
        nk_end(ctx);
    }
}