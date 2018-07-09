#include "ConfigFile.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Container/HashSet.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/IO/Log.h>

ConfigFile::ConfigFile(Urho3D::Context* context, bool caseSensitive)
    : Urho3D::Resource(context)
    , _caseSensitive(caseSensitive)
{

}

ConfigFile::~ConfigFile()
{

}

void ConfigFile::RegisterObject(Urho3D::Context* context)
{
    context->RegisterFactory<ConfigFile>();
}

bool ConfigFile::BeginLoad(Urho3D::Deserializer& source)
{
    unsigned dataSize = source.GetSize();
    if (!dataSize && !source.GetName().Empty())
    {
        URHO3D_LOGERROR("Zero sized data in " + source.GetName());
        return false;
    }

    ConfigSection* configSection = new ConfigSection();
    _configMap.Push(*configSection);
    while (!source.IsEof())
    {
        // Reads the next line
        auto line = source.ReadLine();

        // Parse headers
        if (line.StartsWith("[") && line.EndsWith("]"))
        {
            //Urho3D::String sectionName = line.Substring(1, line.Length() - 2);
            //currentMap = &_configMap[sectionName];

            _configMap.Push(ConfigSection());
            configSection = &_configMap.Back();
            
        }

        configSection->Push(line);
    }

    return true;
}

bool ConfigFile::Save(Urho3D::Serializer& dest) const
{
    dest.WriteLine("################");
    dest.WriteLine("# AUTO-GENERATED");
    dest.WriteLine("################");

    // Iterate over all sections, printing out the header followed by the properties
    for (auto itr = _configMap.Begin(); itr != _configMap.End(); itr++)
    {
        if (itr->Begin() == itr->End())
            continue;

        // Don't print section if there's nothing to print
        bool hasHeader = false;
        for (auto section_itr = itr->Begin(); section_itr != itr->End(); section_itr++)
        {
            if (ParseComments(*section_itr) != Urho3D::String::EMPTY)
            {
                hasHeader = true;
                dest.WriteLine("");
                break;
            }
        }

        auto section_itr = itr->Begin();

        // Doesn't print header if it's empty
        if (hasHeader)
        {
            dest.WriteLine("[" + ParseHeader(*section_itr) + "]");
            dest.WriteLine("");
        }

        for (; section_itr != itr->End(); section_itr++)
        {
            auto line = ParseComments(*section_itr);

            Urho3D::String property;
            Urho3D::String value;

            ParseProperty(line, property, value);

            if (property != Urho3D::String::EMPTY && value != Urho3D::String::EMPTY)
                dest.WriteLine(property + "=" + value);
        }
    }

    return true;
}

bool ConfigFile::Save(Urho3D::Serializer& dest, bool smartSave) const
{
    if (!smartSave)
        return Save(dest);

    // Iterate over all sections, printing out the header followed by the properties
    for (auto itr = _configMap.Begin(); itr != _configMap.End(); itr++)
    {
        if (itr->Begin() == itr->End())
            continue;

        for (auto section_itr = itr->Begin(); section_itr != itr->End(); section_itr++)
        {
            auto line = *section_itr;

            dest.WriteLine(line);
        }
    }

    return true;
}

bool ConfigFile::FromString(const Urho3D::String& source)
{
    if (source.Empty())
        return false;

    Urho3D::MemoryBuffer buffer(source.CString(), source.Length());
    return Load(buffer);
}

bool ConfigFile::Has(const Urho3D::String& section, const Urho3D::String& parameter)
{
    return GetString(section, parameter) != Urho3D::String::EMPTY;
}

const Urho3D::String ConfigFile::GetString(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::String& defaultValue)
{
    // Find the correct section
    ConfigSection* configSection = 0;
    for (auto itr = _configMap.Begin(); itr != _configMap.End(); itr++)
    {
        if (itr->Begin() == itr->End())
            continue;

        auto header = *(itr->Begin());
        header = ParseHeader(header);

        if (_caseSensitive)
        {
            if (section == header)
            {
                configSection = &(*itr);
            }
        }
        else
        {
            if (section.ToLower() == header.ToLower())
            {
                configSection = &(*itr);
            }
        }
    }

    // Section doesn't exist
    if (!configSection)
        return defaultValue;

    for (auto itr = configSection->Begin(); itr != configSection->End(); itr++)
    {
        Urho3D::String property;
        Urho3D::String value;
        ParseProperty(*itr, property, value);

        if (property == Urho3D::String::EMPTY || value == Urho3D::String::EMPTY)
            continue;

        if (_caseSensitive)
        {
            if (parameter == property)
                return value;
        }
        else
        {
            if (parameter.ToLower() == property.ToLower())
                return value;
        }
    }

    return defaultValue;
}

const int ConfigFile::GetInt(const Urho3D::String& section, const Urho3D::String& parameter, const int defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToInt(property);
}

const bool ConfigFile::GetBool(const Urho3D::String& section, const Urho3D::String& parameter, const bool defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToBool(property);
}

const float ConfigFile::GetFloat(const Urho3D::String& section, const Urho3D::String& parameter, const float defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToFloat(property);
}

const Urho3D::Vector2 ConfigFile::GetVector2(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Vector2& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToVector2(property);
}

const Urho3D::Vector3 ConfigFile::GetVector3(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Vector3& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToVector3(property);
}

const Urho3D::Vector4 ConfigFile::GetVector4(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Vector4& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToVector4(property);
}

const Urho3D::Quaternion ConfigFile::GetQuaternion(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Quaternion& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToQuaternion(property);
}

const Urho3D::Color ConfigFile::GetColor(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Color& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToColor(property);
}

const Urho3D::IntRect ConfigFile::GetIntRect(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::IntRect& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToIntRect(property);
}

const Urho3D::IntVector2 ConfigFile::GetIntVector2(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::IntVector2& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToIntVector2(property);
}

const Urho3D::Matrix3 ConfigFile::GetMatrix3(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Matrix3& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToMatrix3(property);
}

const Urho3D::Matrix3x4 ConfigFile::GetMatrix3x4(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Matrix3x4& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToMatrix3x4(property);
}

const Urho3D::Matrix4 ConfigFile::GetMatrix4(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::Matrix4& defaultValue)
{
    auto property = GetString(section, parameter);

    if (property == Urho3D::String::EMPTY)
        return defaultValue;

    return Urho3D::ToMatrix4(property);
}

void ConfigFile::Set(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::String& value)
{
    // Find the correct section
    ConfigSection* configSection = 0;
    for (auto itr = _configMap.Begin(); itr != _configMap.End(); itr++)
    {
        if (itr->Begin() == itr->End())
            continue;

        auto header = *(itr->Begin());
        header = ParseHeader(header);

        if (_caseSensitive)
        {
            if (section == header)
            {
                configSection = &(*itr);
            }
        }
        else
        {
            if (section.ToLower() == header.ToLower())
            {
                configSection = &(*itr);
            }
        }
    }

    // Section doesn't exist
    if (!configSection)
    {
        // Create section
        _configMap.Push(ConfigSection());
        configSection = &_configMap.Back();
    }

    Urho3D::String* line = 0;
    unsigned separatorPos = 0;
    for (auto itr = configSection->Begin(); itr != configSection->End(); itr++)
    {
        // Find property seperator
        separatorPos = itr->Find("=");
        if (separatorPos == Urho3D::String::NPOS)
        {
            separatorPos = itr->Find(":");
        }

        // Not a property
        if (separatorPos == Urho3D::String::NPOS)
            continue;

        Urho3D::String workingLine = ParseComments(*itr);

        auto oldParameter = workingLine.Substring(0, separatorPos).Trimmed();
        auto oldValue = workingLine.Substring(separatorPos + 1).Trimmed();
        
        // Not the correct parameter
        if (_caseSensitive ? (oldParameter == parameter) : (oldParameter.ToLower() == parameter.ToLower()))
        {
            // Replace the value

            itr->Replace(itr->Find(oldValue, separatorPos), oldValue.Length(), value);
            return;
        }
    }

    // Parameter doesn't exist yet
    // We need to find a good place to insert the parameter
    // Avoiding lines which are entirely comments or whitespacing
    int index = configSection->Size() - 1;
    for (int i = index; i >= 0; i--)
    {
        if (ParseComments((*configSection)[i]) != Urho3D::String::EMPTY)
        {
            index = i + 1;
            break;
        }
    }
    configSection->Insert(index, parameter + "=" + value);
}

// Returns header without bracket
Urho3D::String ConfigFile::ParseHeader(Urho3D::String line) const
{
    // Only parse comments outside of headers
    unsigned commentPos = 0;

    while (commentPos != Urho3D::String::NPOS)
    {
        // Find next comment
        auto lastCommentPos = commentPos;
        auto commaPos = line.Find("//", commentPos);
        auto hashPos = line.Find("#", commentPos);
        commentPos = (commaPos < hashPos) ? commaPos : hashPos;

        // Header is behind a comment
        if (line.Find("[", lastCommentPos) > commentPos)
        {
            // Stop parsing this line
            break;
        }

        // Header is before the comment
        if (line.Find("[") < commentPos)
        {
            int startPos = line.Find("[") + 1;
            int l1 = line.Find("]");
            int length = l1 - startPos;
            line = line.Substring(startPos, length);
            break;
        }
    }

    line = line.Trimmed();

    return line;
}

//  property or Empty if no property
void ConfigFile::ParseProperty(Urho3D::String line, Urho3D::String& property, Urho3D::String& value) const
{
    line = ParseComments(line);

    // Find property seperator
    auto separatorPos = line.Find("=");
    if (separatorPos == Urho3D::String::NPOS)
    {
        separatorPos = line.Find(":");
    }

    // Not a property
    if (separatorPos == Urho3D::String::NPOS)
    {
        property = Urho3D::String::EMPTY;
        value = Urho3D::String::EMPTY;
        return;
    }

    property = line.Substring(0, separatorPos).Trimmed();
    value = line.Substring(separatorPos + 1).Trimmed();
}

// strips comments and whitespaces
Urho3D::String ConfigFile::ParseComments(Urho3D::String line) const
{
    // Comments are normalized
    line.Replace("//", "#");

    // Ignore comments
    unsigned commentPos = line.Find("#");
    if (commentPos != Urho3D::String::NPOS)
    {
        line = line.Substring(0, commentPos);
    }

    return line;
}