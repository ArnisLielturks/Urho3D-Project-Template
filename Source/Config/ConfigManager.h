/**
  @author Justin Miller (carnalis.j at gmail.com)
  @author Thebluefish
  @license The MIT License (MIT)
  @copyright
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "ConfigFile.h"

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Resource/Resource.h>

typedef Urho3D::HashMap<Urho3D::String, Urho3D::Variant> SettingsMap;

class State;

class ConfigManager : public Urho3D::Object {
URHO3D_OBJECT(ConfigManager, Urho3D::Object);

public:

    ConfigManager(Urho3D::Context* context, const Urho3D::String& defaultFileName = "Data/Config/config.cfg", bool caseSensitive = false, bool saveDefaultParameters = true);
    ~ConfigManager() override = default;;

    static void RegisterObject(Urho3D::Context* context);

    // Gets the settings map
    SettingsMap& GetMap() {
        return map_;
    }

    // Check if value exists
    bool Has(const Urho3D::String& section, const Urho3D::String& parameter);

    // Set value
    void Set(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Variant& value);

    // Get value
    const Urho3D::Variant Get(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Variant& defaultValue = Urho3D::Variant::EMPTY);
    const Urho3D::String GetString(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::String& defaultValue = Urho3D::String::EMPTY);
    const int GetInt(const Urho3D::String& section, const Urho3D::String& parameter, const int defaultValue = 0);
    const int GetUInt(const Urho3D::String& section, const Urho3D::String& parameter, const unsigned defaultValue = 0);
    const bool GetBool(const Urho3D::String& section, const Urho3D::String& parameter, const bool defaultValue = false);
    const float GetFloat(const Urho3D::String& section, const Urho3D::String& parameter, const float defaultValue = 0.f);
    const Urho3D::Vector2 GetVector2(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Vector2& defaultValue = Urho3D::Vector2::ZERO);
    const Urho3D::Vector3 GetVector3(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Vector3& defaultValue = Urho3D::Vector3::ZERO);
    const Urho3D::Vector4 GetVector4(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Vector4& defaultValue = Urho3D::Vector4::ZERO);
    const Urho3D::Quaternion GetQuaternion(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Quaternion& defaultValue = Urho3D::Quaternion::IDENTITY);
    const Urho3D::Color GetColor(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Color& defaultValue = Urho3D::Color::WHITE);
    const Urho3D::IntRect GetIntRect(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::IntRect& defaultValue = Urho3D::IntRect::ZERO);
    const Urho3D::IntVector2 GetIntVector2(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::IntVector2& defaultValue = Urho3D::IntVector2::ZERO);
    const Urho3D::Matrix3 GetMatrix3(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Matrix3& defaultValue = Urho3D::Matrix3::IDENTITY);
    const Urho3D::Matrix3x4 GetMatrix3x4(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Matrix3x4& defaultValue = Urho3D::Matrix3x4::IDENTITY);
    const Urho3D::Matrix4 GetMatrix4(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Matrix4& defaultValue = Urho3D::Matrix4::IDENTITY);


    // Clears all settings
    void Clear();

    // Load settings from file
    bool Load(bool overwriteExisting = true) {
        return Load(defaultFileName_, overwriteExisting);
    }
    bool Load(const Urho3D::String& fileName, bool overwriteExisting = true);
    bool Load(ConfigFile& configFile, bool overwriteExisting = true);

    // Save settings to file
    bool Save(bool smartSave = true) {
        return Save(defaultFileName_, smartSave);
    }
    bool Save(const Urho3D::String& fileName, bool smartSave = true);
    bool Save(ConfigFile& configFile);
    void SaveSettingsMap(Urho3D::String section, SettingsMap& map, ConfigFile& configFile);

protected:

    SettingsMap* GetSection(const Urho3D::String& section, bool create = false);

protected:

    bool saveDefaultParameters_;
    bool caseSensitive_;

    Urho3D::String defaultFileName_;

    SettingsMap map_;
};
