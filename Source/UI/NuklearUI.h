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
#pragma once

#define NK_INCLUDE_FONT_BAKING
//#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FIXED_TYPES

#include <Urho3D/Core/Object.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Texture2D.h>
#include "nuklear.h"

class NuklearUI
    : public Urho3D::Object
{
    URHO3D_OBJECT(NuklearUI, Urho3D::Object);
public:
    NuklearUI(Urho3D::Context* ctx);
    virtual ~NuklearUI();

    nk_context* GetNkContext() { return &_nk_ctx; }
    operator nk_context*() { return &_nk_ctx; }
    nk_font_atlas* GetFontAtlas() { return &_atlas; }
    void FinalizeFonts();

protected:
    void OnInputBegin(Urho3D::StringHash, Urho3D::VariantMap&);
    void OnRawEvent(Urho3D::StringHash, Urho3D::VariantMap& args);
    void OnInputEnd(Urho3D::StringHash, Urho3D::VariantMap&);
    void OnEndRendering(Urho3D::StringHash, Urho3D::VariantMap&);

    static void ClipboardCopy(nk_handle usr, const char* text, int len);
    static void ClipboardPaste(nk_handle usr, struct nk_text_edit *edit);

    /*const int MAX_VERTEX_MEMORY = 2048 * 1024;
    const int MAX_ELEMENT_MEMORY = 512 * 1024;*/

    const int MAX_VERTEX_MEMORY = 2048 * 128;
    const int MAX_ELEMENT_MEMORY = 512 * 128;

    Urho3D::Graphics* _graphics = 0;
    nk_context _nk_ctx;
    struct nk_font_atlas _atlas;
    struct nk_buffer _commands;
    struct nk_draw_null_texture _null_texture;
    Urho3D::VertexBuffer* _vertex_buffer;
    Urho3D::IndexBuffer* _index_buffer;
    Urho3D::Vector<Urho3D::SharedPtr<Urho3D::Texture2D>> _font_textures;
    float _uiScale = 1.0f;
    struct nk_convert_config _config;
};


