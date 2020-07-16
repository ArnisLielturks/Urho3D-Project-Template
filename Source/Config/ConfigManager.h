#pragma once
/**
  @brief ConfigManager for Urho3D
  @author 100espressos
  @author Thebluefish
  @license The MIT License (MIT)
  @copyleft
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

#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Math/Color.h>
#include <Urho3D/Math/Matrix3.h>
#include <Urho3D/Math/Matrix3x4.h>
#include <Urho3D/Math/Matrix4.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Math/Rect.h>
#include <Urho3D/Math/Vector2.h>
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Vector4.h>

class ConfigFile;

using SettingsMap = Urho3D::HashMap<Urho3D::String, Urho3D::Variant>;

class ConfigManager :
  public Urho3D::Object {

URHO3D_OBJECT(ConfigManager, Urho3D::Object);

public:
  explicit ConfigManager(Urho3D::Context* context, bool caseSensitive = false, bool saveDefaultParameters = false);

  ~ConfigManager() override = default;

  static void RegisterObject(Urho3D::Context* context);

  SettingsMap& GetMap() {
    return map_;
  }

  bool Has(const Urho3D::String& section, const Urho3D::String& parameter);

  /// Get the given parameter as a Variant.
  const Urho3D::Variant Get(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Variant& defaultValue = Urho3D::Variant::EMPTY);

  /// Get the given parameter as a string value.
  const Urho3D::String GetString(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::String& defaultValue = Urho3D::String::EMPTY);

  const int GetInt(const Urho3D::String& section, const Urho3D::String& parameter, const int defaultValue = 0);

  const unsigned int GetUInt(const Urho3D::String& section, const Urho3D::String& parameter, const unsigned int defaultValue = 0u);

  const bool GetBool(const Urho3D::String& section, const Urho3D::String& parameter, const bool defaultValue = false);

  const float GetFloat(const Urho3D::String& section, const Urho3D::String& parameter, const float defaultValue = 0.0f);

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

  void Set(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Variant& value);

  /// Clear all settings.
  void Clear();

  /// Load settings from file at stored filepath.
  bool Load(bool overwriteExisting = true) {
    return Load(filePath_, overwriteExisting);
  }

  /// Load settings from file with given filepath.
  bool Load(const Urho3D::String& filePath, bool overwriteExisting = true);

  /// Load settings from ConfigFile.
  bool Load(ConfigFile& configFile, bool overwriteExisting = true);

  /// Save settings to file at stored filepath.
  bool Save(bool preserveFormatting = true) {
    return Save(filePath_, preserveFormatting);
  }

  /// Save settings to file at given filepath.
  bool Save(const Urho3D::String& fileName, bool preserveFormatting = true);

  /// Save root SettingsMap to ConfigFile.
  bool Save(ConfigFile& configFile);

  /// Save submap to ConfigFile.
  void SaveSettingsMap(const Urho3D::String& section, SettingsMap& map, ConfigFile& configFile);

  /// Set stored filepath.
  void SetFilePath(const Urho3D::String& filePath) {
    filePath_ = filePath;
  }

  /// Get stored filepath.
  const Urho3D::String& GetFilePath() {
    return filePath_;
  }

protected:
  /// Get a reference to a section and optionally create it.
  SettingsMap* GetSection(const Urho3D::String& section, bool create = false);

protected:
  SettingsMap map_;
  Urho3D::String filePath_;
  bool saveDefaultParameters_;
  bool caseSensitive_;
};
