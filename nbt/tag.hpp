#pragma once

#include <cstdint>
#include <memory>
#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

// Very ugly definitions for NBT tags
// Even though this is not machine generated you might as well consider it as such if it makes you feel better :)

namespace nbt {

    using Byte = int8_t;
    using Short = int16_t;
    using Int = int32_t;
    using Long = int64_t;
    using Float = float;
    using Double = double;
    template<typename T> using Array = QList<T>;
    using ByteArray = Array<Byte>;
    using String = QString;
    using List = Array<class Tag>;
    using Compound = QMap<QString, class Tag>;
    using IntArray = Array<Int>;
    using LongArray = Array<Long>;

    enum TagType : Byte {
        TAG_End = 0,
        TAG_Byte = 1,
        TAG_Short = 2,
        TAG_Int = 3,
        TAG_Long = 4,
        TAG_Float = 5,
        TAG_Double = 6,
        TAG_Byte_Array = 7,
        TAG_String = 8,
        TAG_List = 9,
        TAG_Compound = 10,
        TAG_Int_Array = 11,
        TAG_Long_Array = 12
    };

    constexpr Byte TAG_ID_COUNT = TAG_Long_Array + 1;

    class Tag {
    public:
        Tag() : m_type(TagType::TAG_End) {}

        Tag(Byte value, QString name = {}) : m_type(TagType::TAG_Byte), m_name(std::move(name)), value(value) {}

        Tag(Short value, QString name = {}) : m_type(TagType::TAG_Short), m_name(std::move(name)), value(value) {}

        Tag(Int value, QString name = {}) : m_type(TagType::TAG_Int), m_name(std::move(name)), value(value) {}

        Tag(Long value, QString name = {}) : m_type(TagType::TAG_Long), m_name(std::move(name)), value(value) {}

        Tag(Float value, QString name = {}) : m_type(TagType::TAG_Float), m_name(std::move(name)), value(value) {}

        Tag(Double value, QString name = {}) : m_type(TagType::TAG_Double), m_name(std::move(name)), value(value) {}

        Tag(ByteArray value, QString name = {}) : m_type(TagType::TAG_Byte_Array), m_name(std::move(name)),
                                                  value(std::move(value)) {}

        Tag(String value, QString name = {}) : m_type(TagType::TAG_String), m_name(std::move(name)),
                                               value(std::move(value)) {}

        Tag(List value, TagType content_type, QString name = {}) : m_type(TagType::TAG_List),
                                                                            m_content_type(content_type),
                                                                            m_name(std::move(name)),
                                                                            value(std::move(value)) {}

        Tag(Compound value, QString name = {}) : m_type(TagType::TAG_Compound), m_name(std::move(name)),
                                                 value(std::move(value)) {}

        Tag(IntArray value, QString name = {}) : m_type(TagType::TAG_Int_Array), m_name(std::move(name)),
                                                 value(std::move(value)) {}

        Tag(LongArray value, QString name = {}) : m_type(TagType::TAG_Long_Array), m_name(std::move(name)),
                                                  value(std::move(value)) {}

        const QString &name() const { return m_name; }

        TagType type() const { return m_type; }

        TagType content_type() const { return m_content_type; }

        bool is_end() const { return m_type == TagType::TAG_End; }

        bool is_byte() const { return m_type == TagType::TAG_Byte; }

        bool is_short() const { return m_type == TagType::TAG_Short; }

        bool is_int() const { return m_type == TagType::TAG_Int; }

        bool is_long() const { return m_type == TagType::TAG_Long; }

        bool is_float() const { return m_type == TagType::TAG_Float; }

        bool is_double() const { return m_type == TagType::TAG_Double; }

        bool is_byte_array() const { return m_type == TagType::TAG_Byte_Array; }

        bool is_string() const { return m_type == TagType::TAG_String; }

        bool is_list() const { return m_type == TagType::TAG_List; }

        bool is_compound() const { return m_type == TagType::TAG_Compound; }

        bool is_int_array() const { return m_type == TagType::TAG_Int_Array; }

        bool is_long_array() const { return m_type == TagType::TAG_Long_Array; }

        Byte &byte_value() { return std::get<Byte>(value); }

        Short &short_value() { return std::get<Short>(value); }

        Int &int_value() { return std::get<Int>(value); }

        Long &long_value() { return std::get<Long>(value); }

        Float &float_value() { return std::get<Float>(value); }

        Double &double_value() { return std::get<Double>(value); }

        ByteArray &byte_array_value() { return std::get<ByteArray>(value); }

        String &string_value() { return std::get<String>(value); }

        List &list_value() { return std::get<List>(value); }

        Compound &compound_value() { return std::get<Compound>(value); }

        IntArray &int_array_value() { return std::get<IntArray>(value); }

        LongArray &long_array_value() { return std::get<LongArray>(value); }

        const Byte &byte_value() const { return std::get<Byte>(value); }

        const Short &short_value() const { return std::get<Short>(value); }

        const Int &int_value() const { return std::get<Int>(value); }

        const Long &long_value() const { return std::get<Long>(value); }

        const Float &float_value() const { return std::get<Float>(value); }

        const Double &double_value() const { return std::get<Double>(value); }

        const ByteArray &byte_array_value() const { return std::get<ByteArray>(value); }

        const String &string_value() const { return std::get<String>(value); }

        const List &list_value() const { return std::get<List>(value); }

        const Compound &compound_value() const { return std::get<Compound>(value); }

        const IntArray &int_array_value() const { return std::get<IntArray>(value); }

        const LongArray &long_array_value() const { return std::get<LongArray>(value); }

    private:
        TagType m_type = TAG_End;
        TagType m_content_type = TAG_End;
        QString m_name;
        std::variant<Byte, Short, Int, Long, Float, Double, ByteArray, String, List, Compound, IntArray, LongArray> value;
    };

}
