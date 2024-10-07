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

#include "io.hpp"

#include <optional>

namespace nbt {

	static constexpr int MAX_DEPTH = 1024;

	static NamedTag read_named(QIODevice *file, int depth);
	static Tag read_unnamed(QIODevice *file, int depth);
	static Tag read_payload(QIODevice *file, TagType type, int depth);
	static int8_t read_byte(QIODevice *file);
	static int16_t read_short(QIODevice *file);
	static int32_t read_int(QIODevice *file);
	static int64_t read_long(QIODevice *file);
	static float read_float(QIODevice *file);
	static double read_double(QIODevice *file);
	static TagType read_tag_type(QIODevice *file);
	static QByteArray read_bytes(QIODevice *file, int length);
	static QString read_string(QIODevice *file);

	NamedTag read_named_binary(QIODevice *file) {
		return read_named(file, 0);
	}

	Tag read_unnamed_binary(QIODevice *file) {
		return read_unnamed(file, 0);
	}

	static NamedTag read_named(QIODevice *file, int depth) {
		const TagType type = read_tag_type(file);
		if (type == TagType::END)
			return {};

		const QString name = read_string(file);
		return {read_payload(file, type, depth), name};
	}

	static Tag read_unnamed(QIODevice *file, int depth) {
		const TagType type = read_tag_type(file);
		return read_payload(file, type, depth);
	}

	static Tag read_payload(QIODevice *file, TagType type, int depth) {
		if (depth > MAX_DEPTH)
			throw IOError("Max depth reached");

		switch (type) {
			case TagType::END:
				return {};
			case TagType::BYTE:
				return Tag::of_byte(read_byte(file));
			case TagType::SHORT:
				return Tag::of_short(read_short(file));
			case TagType::INT:
				return Tag::of_int(read_int(file));
			case TagType::LONG:
				return Tag::of_long(read_long(file));
			case TagType::FLOAT:
				return Tag::of_float(read_float(file));
			case TagType::DOUBLE:
				return Tag::of_double(read_double(file));
			case TagType::BYTE_ARRAY: {
				Tag result = Tag::of_byte_array();

				int32_t length = read_int(file);
				result.list_value().reserve(length);
				while (length-- != 0)
					result.list_value().append(Tag::of_byte(read_byte(file)));

				return result;
			}
			case TagType::STRING: {
				return Tag::of_string(read_string(file));
			}
			case TagType::LIST: {
				TagType item_type = read_tag_type(file);
				int32_t length = read_int(file);

				Tag result = Tag::of_list(item_type);
				result.list_value().reserve(length);
				while (length-- != 0)
					result.list_value().append(read_payload(file, item_type, depth + 1));

				return result;
			}
			case TagType::COMPOUND: {
				Tag result = Tag::of_compound();

				NamedTag item;
				while ((item = read_named(file, depth)).tag.type() != TagType::END)
					result.compound_value().append(item);

				return result;
			}
			case TagType::INT_ARRAY: {
				Tag result = Tag::of_int_array();

				int32_t length = read_int(file);
				result.list_value().reserve(length);
				while (length-- != 0)
					result.list_value().append(Tag::of_int(read_int(file)));

				return result;
			}
			case TagType::LONG_ARRAY: {
				Tag result = Tag::of_long_array();

				int32_t length = read_int(file);
				result.list_value().reserve(length);
				while (length-- != 0)
					result.list_value().append(Tag::of_long(read_long(file)));

				return result;
			}
		}

		throw IOError("Unknown tag ID");
	}

	static int8_t read_byte(QIODevice *file) {
		char result;
		if (file->atEnd())
			throw IOError("EOF");
		if (!file->getChar(&result))
			throw IOError(file->errorString());

		return static_cast<int8_t>(result);
	}

	static int16_t read_short(QIODevice *file) {
		uint8_t bytes[2];
		if (file->read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
			throw IOError("EOF");

		return static_cast<int16_t>((bytes[0] << 8) + (bytes[1] << 0));
	}

	static int32_t read_int(QIODevice *file) {
		uint8_t bytes[4];
		if (file->read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
			throw IOError("EOF");

		return static_cast<int32_t>((bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + (bytes[3] << 0));
	}

	static int64_t read_long(QIODevice *file) {
		uint8_t bytes[8];
		if (file->read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
			throw IOError("EOF");

		return static_cast<int32_t>((((int64_t)bytes[0] << 56) + ((int64_t)(bytes[1] & 255) << 48) +
									 ((int64_t)(bytes[2] & 255) << 40) + ((int64_t)(bytes[3] & 255) << 32) +
									 ((int64_t)(bytes[4] & 255) << 24) + ((bytes[5] & 255) << 16) +
									 ((bytes[6] & 255) << 8) + ((bytes[7] & 255) << 0)));
	}

	static float read_float(QIODevice *file) {
		const int32_t src = read_int(file);
		float dst;
		static_assert(sizeof(src) == sizeof(dst));
		memcpy(&dst, &src, sizeof(src));
		return dst;
	}

	static double read_double(QIODevice *file) {
		const int64_t src = read_long(file);
		double dst;
		static_assert(sizeof(src) == sizeof(dst));
		memcpy(&dst, &src, sizeof(src));
		return dst;
	}

	static TagType read_tag_type(QIODevice *file) {
		int8_t id = read_byte(file);
		if (id < 0 || id >= TAG_ID_COUNT)
			throw IOError(QString("Invalid tag ID: %1").arg(id));

		return static_cast<TagType>(id);
	}

	static QByteArray read_bytes(QIODevice *file, int32_t length) {
		QByteArray result = file->read(length);
		if (result.length() != length)
			throw IOError("EOF");

		return result;
	}

	static QString read_string(QIODevice *file) {
		const uint16_t length = read_short(file);
		return QString::fromUtf8(read_bytes(file, length));
	}

	static void write_named(QIODevice *file, const NamedTag &value, int depth);
	static void write_unnamed(QIODevice *file, const Tag &value, int depth);
	static void write_payload(QIODevice *file, const Tag &value, int depth);
	static void write_byte(QIODevice *file, int8_t value);
	static void write_short(QIODevice *file, int16_t value);
	static void write_int(QIODevice *file, int32_t value);
	static void write_long(QIODevice *file, int64_t value);
	static void write_float(QIODevice *file, float value);
	static void write_double(QIODevice *file, double value);
	static void write_string(QIODevice *file, const QString &value);
	static void write_bytes(QIODevice *file, const QByteArray &value);

	void write_named_binary(QIODevice *file, const NamedTag &tag) {
		write_named(file, tag, 0);
	}

	void write_unnamed_binary(QIODevice *file, const Tag &tag) {
		write_unnamed(file, tag, 0);
	}

	template <typename To, typename From> static std::optional<To> numeric_cast(From value) {
		if (!std::numeric_limits<To>::is_signed && value < 0)
			return {};

		if (value > std::numeric_limits<To>().max())
			return {};
		if (value < std::numeric_limits<To>().min())
			return {};

		return static_cast<To>(value);
	}

	void write_named(QIODevice *file, const NamedTag &value, int depth) {
		const auto &[tag, name] = value;
		write_byte(file, static_cast<int8_t>(tag.type()));
		if (tag.type() == TagType::END)
			return;

		write_string(file, name);
		write_payload(file, tag, depth);
	}

	void write_unnamed(QIODevice *file, const Tag &value, int depth) {
		write_byte(file, static_cast<int8_t>(value.type()));
		write_payload(file, value, depth);
	}

	void write_payload(QIODevice *file, const Tag &value, int depth) {
		switch (value.type()) {
			case TagType::END:
				return;
			case TagType::BYTE:
				write_byte(file, value.byte_value());
				return;
			case TagType::SHORT:
				write_short(file, value.short_value());
				return;
			case TagType::INT:
				write_int(file, value.int_value());
				return;
			case TagType::LONG:
				write_long(file, value.long_value());
				return;
			case TagType::FLOAT:
				write_float(file, value.float_value());
				return;
			case TagType::DOUBLE:
				write_double(file, value.double_value());
				return;
			case TagType::STRING:
				write_string(file, value.string_value());
				return;
			case TagType::LIST:
			case TagType::BYTE_ARRAY:
			case TagType::INT_ARRAY:
			case TagType::LONG_ARRAY: {
				if (value.type() == TagType::LIST)
					write_byte(file, static_cast<int8_t>(value.content_type()));

				write_int(file, value.list_value().length());

				for (const Tag &tag : value.list_value())
					write_payload(file, tag, depth + 1);
				return;
			}
			case TagType::COMPOUND: {
				for (const NamedTag &tag : value.compound_value())
					write_named(file, tag, depth + 1);

				write_byte(file, static_cast<int8_t>(TagType::END));
				return;
			}
		}

		throw IOError("Unknown tag ID");
	}

	void write_byte(QIODevice *file, int8_t value) {
		if (!file->putChar(value))
			throw IOError(file->errorString());
	}

	void write_short(QIODevice *file, int16_t value) {
		QByteArray result(2, Qt::Uninitialized);
		result[0] = static_cast<char>(value >> 8);
		result[1] = static_cast<char>(value);
		file->write(result);
	}

	void write_int(QIODevice *file, int32_t value) {
		QByteArray result(4, Qt::Uninitialized);
		result[0] = static_cast<char>(value >> 24);
		result[1] = static_cast<char>(value >> 16);
		result[2] = static_cast<char>(value >> 8);
		result[3] = static_cast<char>(value);
		file->write(result);
	}

	void write_long(QIODevice *file, int64_t value) {
		QByteArray result(8, Qt::Uninitialized);
		result[0] = static_cast<char>(value >> 56);
		result[1] = static_cast<char>(value >> 48);
		result[2] = static_cast<char>(value >> 40);
		result[3] = static_cast<char>(value >> 32);
		result[4] = static_cast<char>(value >> 24);
		result[5] = static_cast<char>(value >> 16);
		result[6] = static_cast<char>(value >> 8);
		result[7] = static_cast<char>(value);
		file->write(result);
	}

	void write_float(QIODevice *file, float value) {
		int32_t result;
		static_assert(sizeof(value) == sizeof(result));
		memcpy(&result, &value, sizeof(value));
		write_int(file, result);
	}

	void write_double(QIODevice *file, double value) {
		int64_t result;
		static_assert(sizeof(value) == sizeof(result));
		memcpy(&result, &value, sizeof(value));
		write_long(file, result);
	}

	void write_string(QIODevice *file, const QString &value) {
		QByteArray bytes = value.toUtf8();
		const auto length = numeric_cast<int16_t>(bytes.length());
		if (!length.has_value())
			throw IOError("String too long");

		write_short(file, length.value());
		write_bytes(file, bytes);
	}

	void write_bytes(QIODevice *file, const QByteArray &value) {
		if (file->write(value) != value.length())
			throw IOError(file->errorString());
	}

}
