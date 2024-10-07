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

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <cstdint>
#include <memory>

// Very ugly definitions for NBT tags
// Even though this is not machine generated you might as well consider it as such if it makes you feel better :)

namespace nbt {

	using Byte = int8_t;
	using Short = int16_t;
	using Int = int32_t;
	using Long = int64_t;
	using Float = float;
	using Double = double;
	using String = QString;
	using List = QList<class Tag>;
	using Compound = QList<class NamedTag>;

	enum class TagType : Byte {
		END = 0,
		BYTE = 1,
		SHORT = 2,
		INT = 3,
		LONG = 4,
		FLOAT = 5,
		DOUBLE = 6,
		BYTE_ARRAY = 7,
		STRING = 8,
		LIST = 9,
		COMPOUND = 10,
		INT_ARRAY = 11,
		LONG_ARRAY = 12
	};

	constexpr Byte TAG_ID_COUNT = static_cast<Byte>(TagType::LONG_ARRAY) + 1;

	class Tag {
	public:
		static Tag of_byte(Byte value = 0) {
			return {TagType::BYTE, value};
		}

		static Tag of_short(Short value = 0) {
			return {TagType::SHORT, value};
		}

		static Tag of_int(Int value = 0) {
			return {TagType::INT, value};
		}

		static Tag of_long(Long value = 0) {
			return {TagType::LONG, value};
		}

		static Tag of_float(Float value = 0) {
			return {TagType::FLOAT, value};
		}

		static Tag of_double(Double value = 0) {
			return {TagType::DOUBLE, value};
		}

		static Tag of_byte_array(List value = {}) {
			return {TagType::BYTE_ARRAY, std::move(value)};
		}

		static Tag of_string(String value = "") {
			return {TagType::STRING, std::move(value)};
		}

		static Tag of_list(TagType content_type, List value = {}) {
			return {TagType::LIST, content_type, std::move(value)};
		}

		static Tag of_compound(Compound value = {}) {
			return {TagType::COMPOUND, std::move(value)};
		}

		static Tag of_int_array(List value = {}) {
			return {TagType::INT_ARRAY, std::move(value)};
		}

		static Tag of_long_array(List value = {}) {
			return {TagType::LONG_ARRAY, std::move(value)};
		}

		Tag() = default;

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

		String &string_value() {
			return std::get<String>(value);
		}

		List &list_value() {
			return std::get<List>(value);
		}

		Compound &compound_value() {
			return std::get<Compound>(value);
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

		const String &string_value() const {
			return std::get<String>(value);
		}

		const List &list_value() const {
			return std::get<List>(value);
		}

		const Compound &compound_value() const {
			return std::get<Compound>(value);
		}

	private:
		TagType m_type = TagType::END;
		TagType m_content_type = TagType::END;
		std::variant<std::monostate, Byte, Short, Int, Long, Float, Double, String, List, Compound> value;

		Tag(TagType type, decltype(value) value) : m_type(type), value(std::move(value)) {}

		Tag(TagType type, TagType content_type, decltype(value) value)
			: m_type(type), m_content_type(content_type), value(std::move(value)) {}
	};

	struct NamedTag {
		Tag tag;
		QString name;
	};

}
