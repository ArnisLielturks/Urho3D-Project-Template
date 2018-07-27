//
// Copyright (c) 2016 Rokas Kupstys
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#define NK_IMPLEMENTATION 1
#include <string.h>
#include <SDL/SDL.h>
#include <Urho3D/Urho3DAll.h>
#include "NuklearUI.h"
#undef NK_IMPLEMENTATION

using namespace Urho3D;

struct nk_sdl_vertex
{
    float position[2];
    float uv[2];
    nk_byte col[4];
};

void NuklearUI::ClipboardCopy(nk_handle usr, const char* text, int len)
{
    String str(text, (unsigned int)len);
    SDL_SetClipboardText(str.CString());
}

void NuklearUI::ClipboardPaste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = SDL_GetClipboardText();
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

NuklearUI::NuklearUI(Context* ctx)
    : Object(ctx)
    , _graphics(GetSubsystem<Graphics>())
{
    nk_init_default(&_nk_ctx, 0);
    _nk_ctx.clip.copy = &ClipboardCopy;
    _nk_ctx.clip.paste = &ClipboardPaste;
    _nk_ctx.clip.userdata = nk_handle_ptr(0);

    nk_buffer_init_default(&_commands);
    _index_buffer = new IndexBuffer(_graphics->GetContext());
    _vertex_buffer = new VertexBuffer(_graphics->GetContext());
    Texture2D* nullTex = new Texture2D(_graphics->GetContext());
    nullTex->SetNumLevels(1);
    unsigned whiteOpaque = 0xffffffff;
    nullTex->SetSize(1, 1, Graphics::GetRGBAFormat());
    nullTex->SetData(0, 0, 0, 1, 1, &whiteOpaque);
    _null_texture.texture.ptr = nullTex;

    PODVector< VertexElement > elems;
    elems.Push(VertexElement(TYPE_VECTOR2, SEM_POSITION));
    elems.Push(VertexElement(TYPE_VECTOR2, SEM_TEXCOORD));
    elems.Push(VertexElement(TYPE_UBYTE4_NORM, SEM_COLOR));
    _vertex_buffer->SetSize(MAX_VERTEX_MEMORY / sizeof(nk_sdl_vertex), elems, true);
    _index_buffer->SetSize(MAX_ELEMENT_MEMORY / sizeof(unsigned short), false, true);

    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT,    NK_OFFSETOF(struct nk_sdl_vertex, position)},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT,    NK_OFFSETOF(struct nk_sdl_vertex, uv)},
        {NK_VERTEX_COLOR,    NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_sdl_vertex, col)},
        {NK_VERTEX_LAYOUT_END}};
    NK_MEMSET(&_config, 0, sizeof(_config));
    _config.vertex_layout = vertex_layout;
    _config.vertex_size = sizeof(struct nk_sdl_vertex);
    _config.vertex_alignment = NK_ALIGNOF(struct nk_sdl_vertex);
    _config.null = _null_texture;
    _config.circle_segment_count = 22;
    _config.curve_segment_count = 22;
    _config.arc_segment_count = 22;
    _config.global_alpha = 1.0f;
    _config.shape_AA = NK_ANTI_ALIASING_ON;
    _config.line_AA = NK_ANTI_ALIASING_ON;

    SubscribeToEvent(E_INPUTBEGIN, URHO3D_HANDLER(NuklearUI, OnInputBegin));
    SubscribeToEvent(E_SDLRAWINPUT, URHO3D_HANDLER(NuklearUI, OnRawEvent));
    SubscribeToEvent(E_INPUTEND, URHO3D_HANDLER(NuklearUI, OnInputEnd));
    SubscribeToEvent(E_ENDRENDERING, URHO3D_HANDLER(NuklearUI, OnEndRendering));

    nk_font_atlas_init_default(&_atlas);
    nk_font_atlas_begin(&_atlas);
}

void NuklearUI::FinalizeFonts()
{
    const void* image;
    int w, h;
    image = nk_font_atlas_bake(&_atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

    SharedPtr<Texture2D> fontTex(new Texture2D(_graphics->GetContext()));
    fontTex->SetNumLevels(1);
    fontTex->SetSize(w, h, Graphics::GetRGBAFormat());
    fontTex->SetData(0, 0, 0, w, h, image);

    // Remember the created texture for cleanup
    _font_textures.Push(fontTex);

    nk_font_atlas_end(&_atlas, nk_handle_ptr(fontTex), &_null_texture);
    if (_atlas.default_font)
        nk_style_set_font(&_nk_ctx, &_atlas.default_font->handle);
}

NuklearUI::~NuklearUI()
{
    nk_font_atlas_clear(&_atlas);
    nk_free(&_nk_ctx);
}

void NuklearUI::OnInputBegin(StringHash, VariantMap&)
{
    nk_input_begin(&_nk_ctx);
}

void NuklearUI::OnRawEvent(StringHash, VariantMap& args)
{
    auto evt = static_cast<SDL_Event*>(args[SDLRawInput::P_SDLEVENT].Get<void*>());
    auto ctx = &_nk_ctx;
    if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN)
    {
        /* key events */
        int down = evt->type == SDL_KEYDOWN;
        const Uint8* state = SDL_GetKeyboardState(0);
        SDL_Keycode sym = evt->key.keysym.sym;
        if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
            nk_input_key(ctx, NK_KEY_SHIFT, down);
        else if (sym == SDLK_DELETE)
            nk_input_key(ctx, NK_KEY_DEL, down);
        else if (sym == SDLK_RETURN)
            nk_input_key(ctx, NK_KEY_ENTER, down);
        else if (sym == SDLK_TAB)
            nk_input_key(ctx, NK_KEY_TAB, down);
        else if (sym == SDLK_BACKSPACE)
            nk_input_key(ctx, NK_KEY_BACKSPACE, down);
        else if (sym == SDLK_HOME)
        {
            nk_input_key(ctx, NK_KEY_TEXT_START, down);
            nk_input_key(ctx, NK_KEY_SCROLL_START, down);
        }
        else if (sym == SDLK_END)
        {
            nk_input_key(ctx, NK_KEY_TEXT_END, down);
            nk_input_key(ctx, NK_KEY_SCROLL_END, down);
        }
        else if (sym == SDLK_PAGEDOWN)
            nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
        else if (sym == SDLK_PAGEUP)
            nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
        else if (sym == SDLK_z)
            nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_r)
            nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_c)
            nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_v)
            nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_x)
            nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_b)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_e)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_UP)
            nk_input_key(ctx, NK_KEY_UP, down);
        else if (sym == SDLK_DOWN)
            nk_input_key(ctx, NK_KEY_DOWN, down);
        else if (sym == SDLK_LEFT)
        {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else
                nk_input_key(ctx, NK_KEY_LEFT, down);
        }
        else if (sym == SDLK_RIGHT)
        {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else
                nk_input_key(ctx, NK_KEY_RIGHT, down);
        }
    }
    else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP)
    {
        Input* input = GetSubsystem<Input>();
        if (!input->IsMouseVisible()) {
            return;
        }
        /* mouse button */
        int down = evt->type == SDL_MOUSEBUTTONDOWN;
        const int x = evt->button.x, y = evt->button.y;
        if (evt->button.button == SDL_BUTTON_LEFT)
            nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
        if (evt->button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
        if (evt->button.button == SDL_BUTTON_RIGHT)
            nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
    }
    else if (evt->type == SDL_MOUSEMOTION)
    {
        Input* input = GetSubsystem<Input>();
        if (!input->IsMouseVisible()) {
            return;
        }
        if (ctx->input.mouse.grabbed)
        {
            int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
            nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
        }
        else
            nk_input_motion(ctx, evt->motion.x, evt->motion.y);
    }
    else if (evt->type == SDL_TEXTINPUT)
    {
        nk_glyph glyph = {};
        memcpy(glyph, evt->text.text, NK_UTF_SIZE);
        nk_input_glyph(ctx, glyph);
    }
    else if (evt->type == SDL_MOUSEWHEEL) {
        Input* input = GetSubsystem<Input>();
        if (!input->IsMouseVisible()) {
            return;
        }
        nk_input_scroll(ctx, {0, (float)evt->wheel.y});
    }
}

void NuklearUI::OnInputEnd(StringHash, VariantMap&)
{
    nk_input_end(&_nk_ctx);
}

void NuklearUI::OnEndRendering(StringHash, VariantMap&)
{
    URHO3D_PROFILE(NuklearUIRendering);
    // Engine does not render when window is closed or device is lost
    assert(_graphics && _graphics->IsInitialized() && !_graphics->IsDeviceLost());

    // Max. vertex / index count is not assumed to change later
    void* vertexData = _vertex_buffer->Lock(0, _vertex_buffer->GetVertexCount(), true);
    void* indexData = _index_buffer->Lock(0, _index_buffer->GetIndexCount(), true);
    if (vertexData && indexData)
    {
        struct nk_buffer vbuf, ebuf;
        nk_buffer_init_fixed(&vbuf, vertexData, (nk_size)MAX_VERTEX_MEMORY);
        nk_buffer_init_fixed(&ebuf, indexData, (nk_size)MAX_ELEMENT_MEMORY);
        nk_convert(&_nk_ctx, &_commands, &vbuf, &ebuf, &_config);

#if defined(_WIN32) && !defined(URHO3D_D3D11) && !defined(URHO3D_OPENGL)
        for (int i = 0; i < _vertex_buffer->GetVertexCount(); i++)
        {
            nk_sdl_vertex* v = (nk_sdl_vertex*)vertexData + i;
            v->position[0] += 0.5f;
            v->position[1] += 0.5f;
        }
#endif

        IntVector2 viewSize = _graphics->GetViewport().Size();
        Vector2 invScreenSize(1.0f / (float)viewSize.x_, 1.0f / (float)viewSize.y_);
        Vector2 scale(2.0f * invScreenSize.x_, -2.0f * invScreenSize.y_);
        Vector2 offset(-1.0f, 1.0f);

        Matrix4 projection(Matrix4::IDENTITY);
        projection.m00_ = scale.x_ * _uiScale;
        projection.m03_ = offset.x_;
        projection.m11_ = scale.y_ * _uiScale;
        projection.m13_ = offset.y_;
        projection.m22_ = 1.0f;
        projection.m23_ = 0.0f;
        projection.m33_ = 1.0f;

        _graphics->ClearParameterSources();
        _graphics->SetColorWrite(true);
        _graphics->SetCullMode(CULL_NONE);
        _graphics->SetDepthTest(CMP_ALWAYS);
        _graphics->SetDepthWrite(false);
        _graphics->SetFillMode(FILL_SOLID);
        _graphics->SetStencilTest(false);
        _graphics->SetVertexBuffer(_vertex_buffer);
        _graphics->SetIndexBuffer(_index_buffer);
        _vertex_buffer->Unlock();
        _index_buffer->Unlock();

        unsigned index = 0;
        const struct nk_draw_command* cmd;
        nk_draw_foreach(cmd, &_nk_ctx, &_commands)
        {
            if (!cmd->elem_count)
                continue;

            ShaderVariation* ps;
            ShaderVariation* vs;

            Texture2D* texture = static_cast<Texture2D*>(cmd->texture.ptr);
            if (!texture)
            {
                ps = _graphics->GetShader(PS, "Basic", "VERTEXCOLOR");
                vs = _graphics->GetShader(VS, "Basic", "VERTEXCOLOR");
            }
            else
            {
                // If texture contains only an alpha channel, use alpha shader (for fonts)
                vs = _graphics->GetShader(VS, "Basic", "DIFFMAP VERTEXCOLOR");
                if (texture->GetFormat() == Graphics::GetAlphaFormat())
                    ps = _graphics->GetShader(PS, "Basic", "ALPHAMAP VERTEXCOLOR");
                else
                    ps = _graphics->GetShader(PS, "Basic", "DIFFMAP VERTEXCOLOR");
            }

            _graphics->SetShaders(vs, ps);
            if (_graphics->NeedParameterUpdate(SP_OBJECT, this))
                _graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
            if (_graphics->NeedParameterUpdate(SP_CAMERA, this))
                _graphics->SetShaderParameter(VSP_VIEWPROJ, projection);
            if (_graphics->NeedParameterUpdate(SP_MATERIAL, this))
                _graphics->SetShaderParameter(PSP_MATDIFFCOLOR, Color(1.0f, 1.0f, 1.0f, 1.0f));

            float elapsedTime = GetSubsystem<Time>()->GetElapsedTime();
            _graphics->SetShaderParameter(VSP_ELAPSEDTIME, elapsedTime);
            _graphics->SetShaderParameter(PSP_ELAPSEDTIME, elapsedTime);

            IntRect scissor = IntRect((int)cmd->clip_rect.x, (int)cmd->clip_rect.y,
                                      (int)(cmd->clip_rect.x + cmd->clip_rect.w),
                                      (int)(cmd->clip_rect.y + cmd->clip_rect.h));
            scissor.left_ = (int)(scissor.left_ * _uiScale);
            scissor.top_ = (int)(scissor.top_ * _uiScale);
            scissor.right_ = (int)(scissor.right_ * _uiScale);
            scissor.bottom_ = (int)(scissor.bottom_ * _uiScale);

            _graphics->SetBlendMode(BLEND_ALPHA);
            _graphics->SetScissorTest(true, scissor);
            _graphics->SetTexture(0, (Texture2D*)cmd->texture.ptr);
            _graphics->Draw(TRIANGLE_LIST, index, cmd->elem_count, 0, 0, _vertex_buffer->GetVertexCount());
            index += cmd->elem_count;
        }
        nk_clear(&_nk_ctx);
    }
    _graphics->SetScissorTest(false);
}
