#include "binary.hpp"

#include <QStack>
#include <optional>

namespace nbt {

    constexpr int MAX_DEPTH = 1024;
    const QString TAG_NAMES[] = {
            QStringLiteral("TAG_End"),
            QStringLiteral("TAG_Byte"),
            QStringLiteral("TAG_Short"),
            QStringLiteral("TAG_Int"),
            QStringLiteral("TAG_Long"),
            QStringLiteral("TAG_Float"),
            QStringLiteral("TAG_Double"),
            QStringLiteral("TAG_Byte_Array"),
            QStringLiteral("TAG_String"),
            QStringLiteral("TAG_List"),
            QStringLiteral("TAG_Compound"),
            QStringLiteral("TAG_Int_Array"),
            QStringLiteral("TAG_Long_Array")
    };

    static NamedTag read_named(QIODevice &file, int depth);
    static Tag read_unnamed(QIODevice &file, int depth);
    static Tag read_payload(QIODevice &file, TagType type, int depth);
    static int8_t read_byte(QIODevice &file);
    static int16_t read_short(QIODevice &file);
    static int32_t read_int(QIODevice &file);
    static int64_t read_long(QIODevice &file);
    static float read_float(QIODevice &file);
    static double read_double(QIODevice &file);
    static TagType read_tag_type(QIODevice &file);
    static QByteArray read_bytes(QIODevice &file, int length);
    static QString read_string(QIODevice &file);

    NamedTag read_named_binary(QIODevice &file) {
        return read_named(file, 0);
    }

    Tag read_unnamed_binary(QIODevice &file) {
        return read_unnamed(file, 0);
    }

    static NamedTag read_named(QIODevice &file, int depth) {
        const TagType type = read_tag_type(file);
        if (type == TAG_End)
            return {};

        const QString name = read_string(file);
        return {read_payload(file, type, depth), name};
    }

    static Tag read_unnamed(QIODevice &file, int depth) {
        const TagType type = read_tag_type(file);
        return read_payload(file, type, depth);
    }

    static Tag read_payload(QIODevice &file, TagType type, int depth) {
        if (depth > MAX_DEPTH)
            throw IOError("Max depth reached");

        switch (type) {
            case TAG_End:
                return {};
            case TAG_Byte:
                return {read_byte(file)};
            case TAG_Short:
                return {read_short(file)};
            case TAG_Int:
                return {read_int(file)};
            case TAG_Long:
                return {read_long(file)};
            case TAG_Float:
                return {read_float(file)};
            case TAG_Double:
                return {read_double(file)};
            case TAG_Byte_Array: {
                int32_t length = read_int(file);
                return {read_bytes(file, length)};
            }
            case TAG_String: {
                return {read_string(file)};
            }
            case TAG_List: {
                TagType item_type = read_tag_type(file);
                int32_t length = read_int(file);

                Tag result(List(), item_type);
                result.list_value().reserve(length);
                while (length-- != 0)
                    result.list_value().append(read_payload(file, item_type, depth + 1));

                return result;
            }
            case TAG_Compound: {
                Tag result{Compound()};

                NamedTag item;
                while ((item = read_named(file, depth)).tag.type() != TAG_End)
                    result.compound_value()[item.name] = item.tag;

                return result;
            }
            case TAG_Int_Array: {
                Tag result{IntArray()};

                int32_t length = read_int(file);
                result.int_array_value().reserve(length);
                while (length-- != 0)
                    result.int_array_value().append(read_int(file));

                return result;
            }
            case TAG_Long_Array: {
                Tag result{LongArray()};

                int32_t length = read_int(file);
                result.long_array_value().reserve(length);
                while (length-- != 0)
                    result.long_array_value().append(read_long(file));

                return result;
            }
        }

        throw IOError("Unknown tag ID");
    }

    static int8_t read_byte(QIODevice &file) {
        char result;
        if (file.atEnd())
            throw IOError("EOF");
        if (!file.getChar(&result))
            throw IOError(file.errorString());

        return static_cast<int8_t>(result);
    }

    static int16_t read_short(QIODevice &file) {
        uint8_t bytes[2];
        if (file.read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
            throw IOError("EOF");

        return static_cast<int16_t>(
                (bytes[0] << 8) +
                (bytes[1] << 0)
        );
    }

    static int32_t read_int(QIODevice &file) {
        uint8_t bytes[4];
        if (file.read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
            throw IOError("EOF");

        return static_cast<int32_t>(
                (bytes[0] << 24) +
                (bytes[1] << 16) +
                (bytes[2] << 8) +
                (bytes[3] << 0)
        );
    }

    static int64_t read_long(QIODevice &file) {
        uint8_t bytes[8];
        if (file.read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
            throw IOError("EOF");

        return static_cast<int32_t>(
                (((int64_t) bytes[0] << 56) +
                 ((int64_t) (bytes[1] & 255) << 48) +
                 ((int64_t) (bytes[2] & 255) << 40) +
                 ((int64_t) (bytes[3] & 255) << 32) +
                 ((int64_t) (bytes[4] & 255) << 24) +
                 ((bytes[5] & 255) << 16) +
                 ((bytes[6] & 255) << 8) +
                 ((bytes[7] & 255) << 0))
        );
    }

    static float read_float(QIODevice &file) {
        const int32_t src = read_int(file);
        float dst;
        static_assert(sizeof(src) == sizeof(dst));
        memcpy(&dst, &src, sizeof(src));
        return dst;
    }

    static double read_double(QIODevice &file) {
        const int64_t src = read_long(file);
        double dst;
        static_assert(sizeof(src) == sizeof(dst));
        memcpy(&dst, &src, sizeof(src));
        return dst;
    }

    static TagType read_tag_type(QIODevice &file) {
        int8_t id = read_byte(file);
        if (id < 0 || id >= TAG_ID_COUNT)
            throw IOError(QString("Invalid tag ID: %1").arg(id));

        return static_cast<TagType>(id);
    }

    static QByteArray read_bytes(QIODevice &file, int32_t length) {
        QByteArray result = file.read(length);
        if (result.length() != length)
            throw IOError("EOF");

        return result;
    }

    static QString read_string(QIODevice &file) {
        const uint16_t length = read_short(file);
        return QString::fromUtf8(read_bytes(file, length));
    }

    static void write_named(QIODevice &file, const NamedTag &value, int depth);
    static void write_unnamed(QIODevice &file, const Tag &value, int depth);
    static void write_payload(QIODevice &file, const Tag &value, int depth);
    static void write_byte(QIODevice &file, int8_t value);
    static void write_short(QIODevice &file, int16_t value);
    static void write_int(QIODevice &file, int32_t value);
    static void write_long(QIODevice &file, int64_t value);
    static void write_float(QIODevice &file, float value);
    static void write_double(QIODevice &file, double value);
    static void write_string(QIODevice &file, const QString &value);
    static void write_bytes(QIODevice &file, const QByteArray &value);

    void write_named_binary(QIODevice &file, const NamedTag &tag) {
        write_named(file, tag, 0);
    }

    void write_unnamed_binary(QIODevice &file, const Tag &tag) {
        write_unnamed(file, tag, 0);
    }

    template<typename To, typename From>
    static std::optional<To> numeric_cast(From value) {
        if (!std::numeric_limits<To>::is_signed && value < 0)
            return {};

        if (value > std::numeric_limits<To>().max())
            return {};
        if (value < std::numeric_limits<To>().min())
            return {};

        return static_cast<To>(value);
    }

    void write_named(QIODevice &file, const NamedTag &value, int depth) {
        const auto &[tag, name] = value;
        write_byte(file, tag.type());
        if (tag.type() == TAG_End)
            return;

        write_string(file, name);
        write_payload(file, tag, depth);
    }

    void write_unnamed(QIODevice &file, const Tag &value, int depth) {
        write_byte(file, value.type());
        write_payload(file, value, depth);
    }

    void write_payload(QIODevice &file, const Tag &value, int depth) {
        switch (value.type()) {
            case TAG_End:
                return;
            case TAG_Byte:
                write_byte(file, value.byte_value());
                return;
            case TAG_Short:
                write_short(file, value.short_value());
                return;
            case TAG_Int:
                write_int(file, value.int_value());
                return;
            case TAG_Long:
                write_long(file, value.long_value());
                return;
            case TAG_Float:
                write_float(file, value.float_value());
                return;
            case TAG_Double:
                write_double(file, value.double_value());
                return;
            case TAG_Byte_Array:
                write_int(file, value.byte_array_value().length());
                write_bytes(file, value.byte_array_value());
                return;
            case TAG_String:
                write_string(file, value.string_value());
                return;
            case TAG_List: {
                write_byte(file, value.content_type());
                write_int(file, value.list_value().length());

                for (const Tag &tag : value.list_value())
                    write_payload(file, tag, depth + 1);
                return;
            }
            case TAG_Compound: {
                for (auto iter = value.compound_value().cbegin(); iter != value.compound_value().cend(); ++iter)
                    write_named(file, NamedTag{iter.value(), iter.key()}, depth + 1);

                write_byte(file, TAG_End);
                return;
            }
            case TAG_Int_Array: {
                write_int(file, value.int_array_value().length());

                for (int item : value.int_array_value())
                    write_int(file, item);
                return;
            }
            case TAG_Long_Array: {
                write_int(file, value.long_array_value().length());

                for (long item : value.long_array_value())
                    write_long(file, item);
                return;
            }
        }

        throw IOError("Unknown tag ID");
    }

    void write_byte(QIODevice &file, int8_t value) {
        if (!file.putChar(value))
            throw IOError(file.errorString());
    }

    void write_short(QIODevice &file, int16_t value) {
        QByteArray result(2, Qt::Uninitialized);
        result[0] = static_cast<char>(value >> 8);
        result[1] = static_cast<char>(value);
        file.write(result);
    }

    void write_int(QIODevice &file, int32_t value) {
        QByteArray result(4, Qt::Uninitialized);
        result[0] = static_cast<char>(value >> 24);
        result[1] = static_cast<char>(value >> 16);
        result[2] = static_cast<char>(value >> 8);
        result[3] = static_cast<char>(value);
        file.write(result);
    }

    void write_long(QIODevice &file, int64_t value) {
        QByteArray result(8, Qt::Uninitialized);
        result[0] = static_cast<char>(value >> 56);
        result[1] = static_cast<char>(value >> 48);
        result[2] = static_cast<char>(value >> 40);
        result[3] = static_cast<char>(value >> 32);
        result[4] = static_cast<char>(value >> 24);
        result[5] = static_cast<char>(value >> 16);
        result[6] = static_cast<char>(value >> 8);
        result[7] = static_cast<char>(value);
        file.write(result);
    }

    void write_float(QIODevice &file, float value) {
        int32_t result;
        static_assert(sizeof(value) == sizeof(result));
        memcpy(&result, &value, sizeof(value));
        write_int(file, result);
    }

    void write_double(QIODevice &file, double value) {
        int64_t result;
        static_assert(sizeof(value) == sizeof(result));
        memcpy(&result, &value, sizeof(value));
        write_long(file, result);
    }

    void write_string(QIODevice &file, const QString &value) {
        QByteArray bytes = value.toUtf8();
        const auto length = numeric_cast<int16_t>(bytes.length());
        if (!length.has_value())
            throw IOError("String too long");

        write_short(file, length.value());
        write_bytes(file, bytes);
    }

    void write_bytes(QIODevice &file, const QByteArray &value) {
        if (file.write(value) != value.length())
            throw IOError(file.errorString());
    }

}
