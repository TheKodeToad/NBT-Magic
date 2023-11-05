#include "io.hpp"

#include <QDebug>
#include <QStack>

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
    static Tag read_payload(TagType type, QIODevice &file, int depth);
    static int8_t read_byte(QIODevice &file);
    static int16_t read_short(QIODevice &file);
    static int32_t read_int(QIODevice &file);
    static int64_t read_long(QIODevice &file);
    static float read_float(QIODevice &file);
    static double read_double(QIODevice &file);
    static TagType read_tag_type(QIODevice &file);
    static QByteArray read_bytes(QIODevice &file, int length);
    static QString   read_string(QIODevice &file);

    NamedTag read_named(QIODevice &file) {
        return read_named(file, 0);
    }

    Tag read_unnamed(QIODevice &file) {
        return read_unnamed(file, 0);
    }

    static NamedTag read_named(QIODevice &file, int depth) {
        const TagType type = read_tag_type(file);
        if (type == TAG_End)
            return {};

        const QString name = read_string(file);
        return {read_payload(type, file, depth), name};
    }

    static Tag read_unnamed(QIODevice &file, int depth) {
        const TagType type = read_tag_type(file);
        return read_payload(type, file, depth);
    }

    static Tag read_payload(TagType type, QIODevice &file, int depth) {
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
                uint16_t length = read_short(file);
                return {QString::fromUtf8(file.read(length))};
            }
            case TAG_List: {
                TagType item_type = read_tag_type(file);
                int32_t length = read_int(file);

                Tag result(List(), item_type);
                result.list_value().reserve(length);
                while (length-- != 0)
                    result.list_value().append(read_payload(item_type, file, depth + 1));

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

        throw ReadError("Unknown tag ID");
    }

    static int8_t read_byte(QIODevice &file) {
        char result;
        if (file.atEnd())
            throw ReadError("EOF");
        if (!file.getChar(&result))
            throw ReadError(file.errorString());

        return static_cast<int8_t>(result);
    }

    static int16_t read_short(QIODevice &file) {
        uint8_t bytes[2];
        if (file.read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
            throw ReadError("EOF");

        return static_cast<int16_t>(
                (bytes[0] << 8) +
                (bytes[1] << 0)
        );
    }

    static int32_t read_int(QIODevice &file) {
        uint8_t bytes[4];
        if (file.read(reinterpret_cast<char *>(bytes), sizeof(bytes)) != sizeof(bytes))
            throw ReadError("EOF");

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
            throw ReadError("EOF");

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
            throw ReadError(QString("Invalid tag ID: %1").arg(id));

        return static_cast<TagType>(id);
    }

    static QByteArray read_bytes(QIODevice &file, int32_t length) {
        QByteArray result = file.read(length);
        if (result.length() != length)
            throw ReadError("EOF");

        return result;
    }

    static QString read_string(QIODevice &file) {
        const uint16_t length = read_short(file);
        return QString::fromUtf8(read_bytes(file, length));
    }

}