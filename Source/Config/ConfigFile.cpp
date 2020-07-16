/**
  @brief ConfigFile for Urho3D
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
#include <Urho3D/Container/Str.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/IO/Deserializer.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/IO/Serializer.h>
#include <Urho3D/Math/Color.h>
#include <Urho3D/Math/Matrix3.h>
#include <Urho3D/Math/Matrix3x4.h>
#include <Urho3D/Math/Matrix4.h>
#include <Urho3D/Math/Quaternion.h>
#include <Urho3D/Math/Vector2.h>
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Vector4.h>
#include <Urho3D/Resource/Resource.h>

using namespace Urho3D;

ConfigFile::ConfigFile(Context* context, bool caseSensitive) :
  Resource(context)
  , caseSensitive_(caseSensitive) {
}

void ConfigFile::RegisterObject(Context* context) {
  context->RegisterFactory<ConfigFile>();
}

bool ConfigFile::Has(const String& section, const String& parameter) {
  return GetString(section, parameter) != String::EMPTY;
}

const String ConfigFile::GetString(const String& section, const String& parameter, const String& defaultValue) {
  // Find the correct section.
  ConfigSection* configSection(nullptr);
  for (Vector<ConfigSection>::Iterator itr(map_.Begin()); itr != map_.End(); ++itr) {
    if (itr->Begin() == itr->End()) {
      continue;
    }

    String& header(*(itr->Begin()));
    header = ParseHeader(header);

    if (caseSensitive_) {
      if (section == header) {
        configSection = &(*itr);
      }
    } else {
      if (section.ToLower() == header.ToLower()) {
        configSection = &(*itr);
      }
    }
  }

  // Section does not exist.
  if (!configSection) {
    return defaultValue;
  }

  for (Vector<String>::ConstIterator itr(configSection->Begin()); itr != configSection->End(); ++itr) {
    String property;
    String value;
    ParseProperty(*itr, property, value);

    if (property == String::EMPTY || value == String::EMPTY) {
      continue;
    }

    if (caseSensitive_) {
      if (parameter == property) {
        return value;
      }
    } else {
      if (parameter.ToLower() == property.ToLower()) {
        return value;
      }
    }
  }

  return defaultValue;
}

const int ConfigFile::GetInt(const String& section, const String& parameter, const int defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToInt(property);
}

const unsigned int ConfigFile::GetUInt(const String& section, const String& parameter, const unsigned int defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToUInt(property);
}

const bool ConfigFile::GetBool(const String& section, const String& parameter, const bool defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToBool(property);
}

const float ConfigFile::GetFloat(const String& section, const String& parameter, const float defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToFloat(property);
}

const Vector2 ConfigFile::GetVector2(const String& section, const String& parameter, const Vector2& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToVector2(property);
}

const Vector3 ConfigFile::GetVector3(const String& section, const String& parameter, const Vector3& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToVector3(property);
}

const Vector4 ConfigFile::GetVector4(const String& section, const String& parameter, const Vector4& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToVector4(property);
}

const Quaternion ConfigFile::GetQuaternion(const String& section, const String& parameter, const Quaternion& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToQuaternion(property);
}

const Color ConfigFile::GetColor(const String& section, const String& parameter, const Color& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToColor(property);
}

const IntRect ConfigFile::GetIntRect(const String& section, const String& parameter, const IntRect& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToIntRect(property);
}

const IntVector2 ConfigFile::GetIntVector2(const String& section, const String& parameter, const IntVector2& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToIntVector2(property);
}

const Matrix3 ConfigFile::GetMatrix3(const String& section, const String& parameter, const Matrix3& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToMatrix3(property);
}

const Matrix3x4 ConfigFile::GetMatrix3x4(const String& section, const String& parameter, const Matrix3x4& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToMatrix3x4(property);
}

const Matrix4 ConfigFile::GetMatrix4(const String& section, const String& parameter, const Matrix4& defaultValue) {
  String property(GetString(section, parameter));

  if (property == String::EMPTY) {
    return defaultValue;
  }

  return ToMatrix4(property);
}

void ConfigFile::Set(const String& section, const String& parameter, const String& value) {
  // Find the correct section.
  ConfigSection* configSection(nullptr);
  for (Vector<ConfigSection>::Iterator itr(map_.Begin()); itr != map_.End(); ++itr) {
    if (itr->Begin() == itr->End()) {
      continue;
    }

    String& header(*(itr->Begin()));
    header = ParseHeader(header);

    if (caseSensitive_) {
      if (section == header) {
        configSection = &(*itr);
      }
    } else {
      if (section.ToLower() == header.ToLower()) {
        configSection = &(*itr);
      }
    }
  }

  if (section == String::EMPTY) {
    configSection = &(*map_.Begin());
  }

  // Section does not exist.
  if (!configSection) {
    String sectionName(section);

    // Format header.
    if (ConfigFile::ParseHeader(sectionName) == sectionName) {
      sectionName = "[" + sectionName + "]";
    }

    // Create section.
    map_.Push(ConfigSection());
    configSection = &map_.Back();

    // Add header and blank line.
    configSection->Push(sectionName);
    configSection->Push("");
  }

  String* line(nullptr);
  unsigned int separatorPos(0);
  for (Vector<String>::Iterator itr(configSection->Begin()); itr != configSection->End(); ++itr) {
    // Find property separator.
    separatorPos = itr->Find("=");
    if (separatorPos == String::NPOS) {
      separatorPos = itr->Find(":");
    }

    // Not a property.
    if (separatorPos == String::NPOS) {
      continue;
    }

    String workingLine(ParseComments(*itr));

    String oldParameter(workingLine.Substring(0, separatorPos).Trimmed());
    String oldValue(workingLine.Substring(separatorPos + 1).Trimmed());

    // Not the correct parameter.
    if (caseSensitive_ ? (oldParameter == parameter) : (oldParameter.ToLower() == parameter.ToLower())) {
      // Replace the value.
      itr->Replace(itr->Find(oldValue, separatorPos), oldValue.Length(), value);
      return;
    }
  }

  // Parameter doesn't exist yet.
  // Find a good place to insert the parameter, avoiding lines which are entirely comments or whitespacing.
  int index(configSection->Size() - 1);
  for (int i(index); i >= 0; i--) {
    if (ParseComments((*configSection)[i]) != String::EMPTY) {
      index = i + 1;
      break;
    }
  }
  configSection->Insert(index, parameter + "=" + value);
}

bool ConfigFile::BeginLoad(Deserializer& source) {
  unsigned int dataSize(source.GetSize());
  if (!dataSize && !source.GetName().Empty()) {
    URHO3D_LOGERROR("Zero-sized data in " + source.GetName());
    return false;
  }

  map_.Push(ConfigSection());
  ConfigSection* configSection(&map_.Back());
  while (!source.IsEof()) {
    String line(source.ReadLine());

    // Parse headers.
    if (line.StartsWith("[") && line.EndsWith("]")) {
      map_.Push(ConfigSection());
      configSection = &map_.Back();
    }

    configSection->Push(line);
  }

  return true;
}

bool ConfigFile::Save(Serializer& dest) const {
  dest.WriteLine("# AUTO-GENERATED");

  // Iterate over all sections, printing out the header followed by the properties.
  for (Vector<ConfigSection>::ConstIterator itr(map_.Begin()); itr != map_.End(); ++itr) {
    if (itr->Begin() == itr->End()) {
      continue;
    }

    // Don't print section if there's nothing to print.
    Vector<String>::ConstIterator section_itr(itr->Begin());
    String header(ParseHeader(*section_itr));

    // Don't print header if it's empty.
    if (header != String::EMPTY) {
      dest.WriteLine("[" + header + "]");
    }

    dest.WriteLine("");

    for (; section_itr != itr->End(); ++section_itr) {
      const String line(ParseComments(*section_itr));

      String property;
      String value;

      ParseProperty(line, property, value);

      if (property != String::EMPTY && value != String::EMPTY) {
        dest.WriteLine(property + "=" + value);
      }
    }

    dest.WriteLine("");
  }

  return true;
}

bool ConfigFile::Save(Serializer& dest, bool smartSave) const {
  if (!smartSave) {
    return Save(dest);
  }

  // Iterate over all sections, printing out the header followed by the properties.
  for (Vector<ConfigSection>::ConstIterator itr(map_.Begin()); itr != map_.End(); ++itr) {
    if (itr->Begin() == itr->End()) {
      continue;
    }

    for (Vector<String>::ConstIterator section_itr(itr->Begin()); section_itr != itr->End(); ++section_itr) {
      const String line(*section_itr);

      if (section_itr == itr->Begin()) {
        dest.WriteLine("[" + line + "]");
      } else {
        dest.WriteLine(line);
      }
    }
  }

  return true;
}

bool ConfigFile::FromString(const String& source) {
  if (source.Empty()) {
    return false;
  }

  MemoryBuffer buffer(source.CString(), source.Length());
  return Load(buffer);
}

void ConfigFile::SetCaseSensitive(bool caseSensitive) {
  caseSensitive_ = caseSensitive;
}

const String ConfigFile::ParseHeader(String line) {
  // Only parse comments outside of headers.
  unsigned int commentPos(0);

  while (commentPos != String::NPOS) {
    // Find next comment.
    unsigned int lastCommentPos(commentPos);
    unsigned int commaPos(line.Find("//", commentPos));
    unsigned int hashPos(line.Find("#", commentPos));
    commentPos = (commaPos < hashPos) ? commaPos : hashPos;

    // Header is behind a comment.
    if (line.Find("[", lastCommentPos) > commentPos) {
      // Stop parsing this line.
      break;
    }

    // Header is before a comment.
    if (line.Find("[") < commentPos) {
      unsigned int startPos(line.Find("[") + 1);
      unsigned int l1(line.Find("]"));
      unsigned int length(l1 - startPos);
      line = line.Substring(startPos, length);
      break;
    }
  }

  line = line.Trimmed();

  return line;
}

const void ConfigFile::ParseProperty(String line, String& property, String& value) {
  line = ParseComments(line);

  // Find property separator.
  unsigned int separatorPos(line.Find("="));
  if (separatorPos == String::NPOS) {
    separatorPos = line.Find(":");
  }

  // Not a property.
  if (separatorPos == String::NPOS) {
    property = String::EMPTY;
    value = String::EMPTY;
    return;
  }

  property = line.Substring(0, separatorPos).Trimmed();
  value = line.Substring(separatorPos + 1).Trimmed();
}

const String ConfigFile::ParseComments(String line) {
  // Normalize comment tokens.
  line.Replace("//", "#");

  // Strip comments.
  unsigned int commentPos(line.Find("#"));
  if (commentPos != String::NPOS) {
    line = line.Substring(0, commentPos);
  }

  return line;
}
