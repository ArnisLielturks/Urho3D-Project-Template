#pragma once

#include <Urho3D/Urho3D.h>
#include <Urho3D/Resource/Resource.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Core/StringUtils.h>

namespace Urho3D
{
    class File;
    class Variant;
}

typedef Urho3D::Vector<Urho3D::String> ConfigSection;
typedef Urho3D::Vector<ConfigSection> ConfigMap;

class ConfigFile : public Urho3D::Resource
{
public:
    ConfigFile(Urho3D::Context* context, bool caseSensitive = false);
    ~ConfigFile();

    static void RegisterObject(Urho3D::Context* context);

    void SetCaseSensitive(bool caseSensitive) { _caseSensitive = caseSensitive; }

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    virtual bool BeginLoad(Urho3D::Deserializer& source);
    /// Save resource
    virtual bool Save(Urho3D::Serializer& dest) const;
    /// Smart Save resource, replacing only the values, keeping whitespacing and comments
    virtual bool Save(Urho3D::Serializer& dest, bool smartSave) const;

    /// Deserialize from a string. Return true if successful.
    bool FromString(const Urho3D::String& source);

    const ConfigMap* GetMap() { return &_configMap; }

    bool Has(const Urho3D::String& section, const Urho3D::String& parameter);

    const Urho3D::String GetString(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::String& defaultValue = Urho3D::String::EMPTY);
    const int GetInt(const Urho3D::String& section, const Urho3D::String& parameter, const int defaultValue = 0);
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
    
    void Set(const Urho3D::String& section, const Urho3D::String& parameter, const Urho3D::String& value);

protected:

    // Returns header without bracket
    Urho3D::String ParseHeader(Urho3D::String line) const;
    //  property or Empty if no property
    void ParseProperty(Urho3D::String line, Urho3D::String& property, Urho3D::String& value) const;
    // strips comments and whitespaces
    Urho3D::String ParseComments(Urho3D::String line) const;

protected:

    bool _caseSensitive;
    ConfigMap _configMap;
};