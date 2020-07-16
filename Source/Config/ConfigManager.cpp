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

#include "ConfigFile.h"
#include "ConfigManager.h"
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>

using namespace Urho3D;

ConfigManager::ConfigManager(Context* context, bool caseSensitive /* = false */, bool saveDefaultParameters /* = false */) :
  Object(context)
  , saveDefaultParameters_(saveDefaultParameters)
  , caseSensitive_(caseSensitive) {
  Load();
}

void ConfigManager::RegisterObject(Context* context) {
  context->RegisterFactory<ConfigManager>();
}

bool ConfigManager::Has(const String& section, const String& parameter) {
  return Get(section, parameter) != Variant::EMPTY;
}

void ConfigManager::Set(const String& section, const String& parameter, const Variant& value) {
  SettingsMap* sectionMap(GetSection(section, true));

  sectionMap->operator[](parameter) = value;
}

const Variant ConfigManager::Get(const String& section, const String& parameter, const Variant& defaultValue /* = Urho3D::Variant::EMPTY */) {
  SettingsMap* sectionMap(GetSection(section));

  // Section doesn't exist.
  if (!sectionMap) {
    return defaultValue;
  }

  // Section exists; parameter does not.
  if (sectionMap->Find(parameter) == sectionMap->End()) {
    return defaultValue;
  }

  // Section exists; parameter exists.
  return (*sectionMap)[parameter];
}

const String ConfigManager::GetString(const String& section, const String& parameter, const String& defaultValue /* = Urho3D::String::EMPTY */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_STRING) {
    return value.GetString();
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const int ConfigManager::GetInt(const String& section, const String& parameter, const int defaultValue /* = 0 */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_INT && value.GetInt() != defaultValue) {
    return value.GetInt();
  }

  if (value.GetType() == VAR_STRING) {
    return ToInt(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const unsigned int ConfigManager::GetUInt(const String& section, const String& parameter, const unsigned int defaultValue /* = 0u */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_INT && value.GetUInt() != defaultValue) {
    return value.GetUInt();
  }

  if (value.GetType() == VAR_STRING) {
    return ToUInt(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const bool ConfigManager::GetBool(const String& section, const String& parameter, const bool defaultValue /* = false */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_BOOL) {
    return value.GetBool();
  }

  if (value.GetType() == VAR_STRING) {
    return ToBool(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const float ConfigManager::GetFloat(const String& section, const String& parameter, const float defaultValue /* = 0.0f */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_FLOAT) {
    return value.GetFloat();
  }

  if (value.GetType() == VAR_STRING) {
    return ToFloat(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    //
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Vector2 ConfigManager::GetVector2(const String& section, const String& parameter, const Vector2& defaultValue /* = Urho3D::Vector2::ZERO */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_VECTOR2) {
    return value.GetVector2();
  }

  if (value.GetType() == VAR_STRING) {
    return ToVector2(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Vector3 ConfigManager::GetVector3(const String& section, const String& parameter, const Vector3& defaultValue /* = Urho3D::Vector3::ZERO */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_VECTOR3) {
    return value.GetVector3();
  }

  if (value.GetType() == VAR_STRING) {
    return ToVector3(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Vector4 ConfigManager::GetVector4(const String& section, const String& parameter, const Vector4& defaultValue /* = Urho3D::Vector4::ZERO */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_VECTOR4) {
    return value.GetVector4();
  }

  if (value.GetType() == VAR_STRING) {
    return ToVector4(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Quaternion ConfigManager::GetQuaternion(const String& section, const String& parameter, const Quaternion& defaultValue /* = Urho3D::Quaternion::IDENTITY */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_QUATERNION) {
    return value.GetQuaternion();
  }

  if (value.GetType() == VAR_STRING) {
    return ToQuaternion(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Color ConfigManager::GetColor(const String& section, const String& parameter, const Color& defaultValue /* = Urho3D::Color::WHITE */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_COLOR) {
    return value.GetColor();
  }

  if (value.GetType() == VAR_STRING) {
    return ToColor(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const IntRect ConfigManager::GetIntRect(const String& section, const String& parameter, const IntRect& defaultValue /* = Urho3D::IntRect::ZERO */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_INTRECT) {
    return value.GetIntRect();
  }

  if (value.GetType() == VAR_STRING) {
    return ToIntRect(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const IntVector2 ConfigManager::GetIntVector2(const String& section, const String& parameter, const IntVector2& defaultValue /* = Urho3D::IntVector2::ZERO */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_INTVECTOR2) {
    return value.GetIntVector2();
  }

  if (value.GetType() == VAR_STRING) {
    return ToIntVector2(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Matrix3 ConfigManager::GetMatrix3(const String& section, const String& parameter, const Matrix3& defaultValue /* = Urho3D::Matrix3::IDENTITY */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_MATRIX3) {
    return value.GetMatrix3();
  }

  if (value.GetType() == VAR_STRING) {
    return ToMatrix3(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Matrix3x4 ConfigManager::GetMatrix3x4(const String& section, const String& parameter, const Matrix3x4& defaultValue /* = Urho3D::Matrix3x4::IDENTITY */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_MATRIX3X4) {
    return value.GetMatrix3x4();
  }

  if (value.GetType() == VAR_STRING) {
    return ToMatrix3x4(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

const Matrix4 ConfigManager::GetMatrix4(const String& section, const String& parameter, const Matrix4& defaultValue /* = Urho3D::Matrix4::IDENTITY */) {
  const Variant value(Get(section, parameter));

  if (value.GetType() == VAR_MATRIX4) {
    return value.GetMatrix4();
  }

  if (value.GetType() == VAR_STRING) {
    return ToMatrix4(value.GetString());
  }

  // If parameter does not exist or is a different type, set back to default.
  if (saveDefaultParameters_) {
    Set(section, parameter, defaultValue);
  }

  return defaultValue;
}

void ConfigManager::Clear() {
  map_.Clear();
}

bool ConfigManager::Load(const String& filePath, bool overwriteExisting /* = true */) {
  const FileSystem* fileSystem(context_->GetSubsystem<FileSystem>());

  ConfigFile configFile(context_);

  // Check if file exists.
  if (!fileSystem->FileExists(filePath)) {
    return false;
  }

  File file(context_, filePath, FILE_READ);
  configFile.BeginLoad(file);

  return Load(configFile, overwriteExisting);
}

bool ConfigManager::Load(ConfigFile& configFile, bool overwriteExisting /* = true */) {
  const ConfigSectionMap* map(configFile.GetMap());

  for (auto& section : *map) {
    if (section.Begin() == section.End()) {
      continue;
    }

    String header(String::EMPTY);

    if (section != *map->Begin()) {
      header = *(section.Begin());
      header = ConfigFile::ParseHeader(header);
    }

    for (Vector<String>::ConstIterator section_itr(++section.Begin()); section_itr != section.End(); ++section_itr) {
      const String& line(*section_itr);

      String parameter;
      String value;

      ConfigFile::ParseProperty(line, parameter, value);

      if (parameter != String::EMPTY && value != String::EMPTY) {
        Set(header, parameter, value);
      }
    }
  }

  return true;
}

bool ConfigManager::Save(const String& fileName, bool preserveFormatting /* = true */) {
  SharedPtr<ConfigFile> configFile(new ConfigFile(context_));

  if (preserveFormatting) {
    SharedPtr<File> file(new File(context_, fileName, FILE_READ));

    // Ensure file is open.
    if (file->IsOpen()) {
      configFile->BeginLoad(*file);
    }
  }

  // Attempt to save the file.
  if (Save(*configFile)) {
    SharedPtr<File> file(new File(context_, fileName, FILE_WRITE));

    // Ensure file is open.
    if (!file->IsOpen()) {
      return false;
    }

    configFile->Save(*file, preserveFormatting);
  }

  return true;
}

bool ConfigManager::Save(ConfigFile& configFile) {
  SaveSettingsMap("", map_, configFile);

  return true;
}

void ConfigManager::SaveSettingsMap(const String& section, SettingsMap& map, ConfigFile& configFile) {
  // Save out parameters.
  for (auto& itr : map) {
    // Skip over sub-sections.
    if (itr.second_.GetType() == VAR_VOIDPTR) {
      continue;
    }

    String value(itr.first_);

    if (itr.second_.GetType() == VAR_STRING) {
      configFile.Set(section, value, itr.second_.GetString());
    }

    if (itr.second_.GetType() == VAR_INT) {
      configFile.Set(section, value, String(itr.second_.GetInt()));
    }

    if (itr.second_.GetType() == VAR_BOOL) {
      configFile.Set(section, value, String(itr.second_.GetBool()));
    }

    if (itr.second_.GetType() == VAR_FLOAT) {
      configFile.Set(section, value, String(itr.second_.GetFloat()));
    }

    if (itr.second_.GetType() == VAR_VECTOR2) {
      configFile.Set(section, value, String(itr.second_.GetVector2()));
    }

    if (itr.second_.GetType() == VAR_VECTOR3) {
      configFile.Set(section, value, String(itr.second_.GetVector3()));
    }

    if (itr.second_.GetType() == VAR_VECTOR4) {
      configFile.Set(section, value, String(itr.second_.GetVector4()));
    }

    if (itr.second_.GetType() == VAR_QUATERNION) {
      configFile.Set(section, value, String(itr.second_.GetQuaternion()));
    }

    if (itr.second_.GetType() == VAR_COLOR) {
      configFile.Set(section, value, String(itr.second_.GetColor()));
    }

    if (itr.second_.GetType() == VAR_INTRECT) {
      configFile.Set(section, value, String(itr.second_.GetIntRect()));
    }

    if (itr.second_.GetType() == VAR_INTVECTOR2) {
      configFile.Set(section, value, String(itr.second_.GetIntVector2()));
    }

    if (itr.second_.GetType() == VAR_MATRIX3) {
      configFile.Set(section, value, String(itr.second_.GetMatrix3()));
    }

    if (itr.second_.GetType() == VAR_MATRIX3X4) {
      configFile.Set(section, value, String(itr.second_.GetMatrix3x4()));
    }

    if (itr.second_.GetType() == VAR_MATRIX4) {
      configFile.Set(section, value, String(itr.second_.GetMatrix4()));
    }
  }

  // Save out sub-sections.
  for (auto& itr : map) {
    // Skip over parameter.
    if (itr.second_.GetType() != VAR_VOIDPTR) {
      continue;
    }

    String path(section);
    path.Append(itr.first_);

    SettingsMap* value(static_cast<SettingsMap*>(itr.second_.GetVoidPtr()));

    if (value) {
      // Save sub-section.
      SaveSettingsMap(path, *value, configFile);
    }
  }
}

SettingsMap* ConfigManager::GetSection(const String& section, bool create /* = false */) {
  // Empty section gets assigned to main map.
  if (section == String::EMPTY) {
    return &map_;
  }

  // Split section into submaps.
  Vector<String> split;

  unsigned int splitPos(0);

  if (ConfigFile::ParseHeader(section).Empty()) {
    return &map_;
  }

  // Split sections by '.' or '/'. Comments will ignore splits behind them.
  while (splitPos != String::NPOS) {
    // Find next comment split.
    unsigned int commentSplitPos(splitPos);
    unsigned int hashPos(section.Find("#", commentSplitPos));
    unsigned int slashesPos(section.Find("//", commentSplitPos));
    commentSplitPos = (hashPos < slashesPos) ? hashPos : slashesPos;

    // Find next split.
    unsigned int lastSplitPos(splitPos);
    unsigned int dotPos(section.Find(".", lastSplitPos));
    unsigned int slashPos(section.Find("/", lastSplitPos));
    splitPos = (dotPos < slashPos) ? dotPos : slashPos;

    // Ignore splits after comments.
    splitPos = (commentSplitPos <= splitPos) ? String::NPOS : splitPos;

    unsigned int length(splitPos - lastSplitPos);
    if (splitPos == String::NPOS) {
      length = section.Length() - lastSplitPos;
    }

    String sub(section.Substring(lastSplitPos, length));

    if (sub != String::EMPTY) {
      split.Push(sub);
    }
  }

  SettingsMap* currentMap(&map_);
  for (const auto& itr : split) {

    // Find section.
    SettingsMap* newMap(nullptr);
    for (const auto& map_itr : *currentMap) {
      if (map_itr.first_ == section) {
        newMap = static_cast<SettingsMap*>(map_itr.second_.GetVoidPtr());

        // Key exists, but is not a SettingsMap.
        if (!newMap) {
          return nullptr;
        }

        // Key exists.
        break;
      }
    }

    // Key does not exist.
    if (!newMap) {
      if (create) {
        currentMap->operator[](section) = new SettingsMap(); //@TODO: Check for a few bytes leak.
        newMap = static_cast<SettingsMap*>((*currentMap)[section].GetVoidPtr());
      }
    }

    if (newMap) {
      currentMap = newMap;
    }
  }

  return currentMap;
}
