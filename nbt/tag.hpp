/*
 * This project is licensed under the MIT license:
 *
 * Copyright (c) 2023-2024 TheKodeToad and project contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    using ByteArray = QByteArray;
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

        Tag(Byte value) : m_type(TagType::TAG_Byte), value(value) {}

        Tag(Short value) : m_type(TagType::TAG_Short), value(value) {}

        Tag(Int value) : m_type(TagType::TAG_Int), value(value) {}

        Tag(Long value) : m_type(TagType::TAG_Long), value(value) {}

        Tag(Float value) : m_type(TagType::TAG_Float), value(value) {}

        Tag(Double value) : m_type(TagType::TAG_Double), value(value) {}

        Tag(ByteArray value) : m_type(TagType::TAG_Byte_Array),
                               value(std::move(value)) {}

        Tag(String value) : m_type(TagType::TAG_String), value(std::move(value)) {}

        Tag(List value, TagType content_type) : m_type(TagType::TAG_List), m_content_type(content_type),
                                                value(std::move(value)) {}

        Tag(Compound value) : m_type(TagType::TAG_Compound), value(std::move(value)) {}

        Tag(IntArray value) : m_type(TagType::TAG_Int_Array), value(std::move(value)) {}

        Tag(LongArray value) : m_type(TagType::TAG_Long_Array), value(std::move(value)) {}

        TagType type() const {
            return m_type;
        }

        TagType content_type() const {
            return m_content_type;
        }

        Byte &byte_value() {
            return std::get<Byte>(value);
        }

        Short &short_value() {
            return std::get<Short>(value);
        }

        Int &int_value() {
            return std::get<Int>(value);
        }

        Long &long_value() {
            return std::get<Long>(value);
        }

        Float &float_value() {
            return std::get<Float>(value);
        }

        Double &double_value() {
            return std::get<Double>(value);
        }

        ByteArray &byte_array_value() {
            return std::get<ByteArray>(value);
        }

        String &string_value() {
            return std::get<String>(value);
        }

        List &list_value() {
            return std::get<List>(value);
        }

        Compound &compound_value() {
            return std::get<Compound>(value);
        }

        IntArray &int_array_value() {
            return std::get<IntArray>(value);
        }

        LongArray &long_array_value() {
            return std::get<LongArray>(value);
        }

        const Byte &byte_value() const {
            return std::get<Byte>(value);
        }

        const Short &short_value() const {
            return std::get<Short>(value);
        }

        const Int &int_value() const {
            return std::get<Int>(value);
        }

        const Long &long_value() const {
            return std::get<Long>(value);
        }

        const Float &float_value() const {
            return std::get<Float>(value);
        }

        const Double &double_value() const {
            return std::get<Double>(value);
        }

        const ByteArray &byte_array_value() const {
            return std::get<ByteArray>(value);
        }

        const String &string_value() const {
            return std::get<String>(value);
        }

        const List &list_value() const {
            return std::get<List>(value);
        }

        const Compound &compound_value() const {
            return std::get<Compound>(value);
        }

        const IntArray &int_array_value() const {
            return std::get<IntArray>(value);
        }

        const LongArray &long_array_value() const {
            return std::get<LongArray>(value);
        }

    private:
        TagType m_type = TAG_End;
        TagType m_content_type = TAG_End;
        std::variant<Byte, Short, Int, Long, Float, Double, ByteArray, String, List, Compound, IntArray, LongArray> value;
    };

    struct NamedTag {
        Tag tag;
        QString name;
    };

}
