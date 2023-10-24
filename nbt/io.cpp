#include "io.hpp"

#include <QDebug>

namespace nbt {

    bool is_gzipped(QIODevice &file) {
        QByteArray magic = file.peek(2);
        return magic.length() == 2 && magic.at(0) == '\x1f' && magic.at(1) == '\x8b';
    }

    static Tag read_payload(QIODevice &file, TagType type, QString name = {});

    template<typename T>
    T read_primative(QIODevice &file);

    template<>
    int8_t read_primative<int8_t>(QIODevice &file) {
        char byte;
        file.getChar(&byte);
        return static_cast<int8_t>(byte);
    }

    template<>
    int16_t read_primative<int16_t>(QIODevice &file) {
        uint8_t bytes[2];
        file.read(reinterpret_cast<char*>(bytes), sizeof(bytes));
        return static_cast<int16_t>((bytes[0] << 8) + (bytes[1] << 0));
    }

    template<>
    uint16_t read_primative<uint16_t>(QIODevice &file) {
        uint8_t bytes[2];
        file.read(reinterpret_cast<char*>(bytes), sizeof(bytes));
        return static_cast<uint16_t>((bytes[0] << 8) + (bytes[1] << 0));
    }

    template<>
    int32_t read_primative<int32_t>(QIODevice &file) {
        uint8_t bytes[4];
        file.read(reinterpret_cast<char*>(bytes), sizeof(bytes));
        return static_cast<int32_t>((bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + (bytes[3] << 0));
    }

    template<>
    int64_t read_primative<int64_t>(QIODevice &file) {
        uint8_t bytes[8];
        file.read(reinterpret_cast<char*>(bytes), sizeof(bytes));
        return static_cast<int64_t>(
                ((int64_t) bytes[0] << 56) +
                ((int64_t) (bytes[1] & 255) << 48) +
                ((int64_t) (bytes[2] & 255) << 40) +
                ((int64_t) (bytes[3] & 255) << 32) +
                ((int64_t) (bytes[4] & 255) << 24) +
                ((bytes[5] & 255) << 16) +
                ((bytes[6] & 255) << 8) +
                ((bytes[7] & 255) << 0)
        );
    }

    template<>
    Float read_primative<Float>(QIODevice &file) {
        static_assert(sizeof(Int) == sizeof(Float));

        auto i = read_primative<Int>(file);
        Float result;
        memcpy(&result, &i, sizeof(Int));
        return result;
    }

    template<>
    Double read_primative<Double>(QIODevice &file) {
        static_assert(sizeof(Long) == sizeof(Double));

        auto i = read_primative<Long>(file);
        Double result;
        memcpy(&result, &i, sizeof(Long));
        return result;
    }

    static TagType read_tag_type(QIODevice &file) {
        auto result = read_primative<Byte>(file);
        if (result < 0 || result >= TAG_ID_COUNT)
            throw ReadError(QString("Invalid tag ID $1").arg(result));

        return (TagType) result;
    }

    static String read_string(QIODevice &file) {
        auto length = read_primative<uint16_t>(file);
        if (length == 0)
            return {};

        QByteArray bytes = file.read(length);
        return QString::fromUtf8(bytes);
    }

    template<typename type>
    static Array<type> read_array(QIODevice &file) {
        auto length = read_primative<int32_t>(file);
        if (length == 0)
            return {};
        if (length < 0)
            throw ReadError("Length < 0");

        Array<type> result;
        result.reserve(length);
        while (length-- != 0)
            result.append(read_primative<type>(file));

        return result;
    }

    static QPair<List, TagType> read_list(QIODevice &file) {
        const auto type = read_tag_type(file);
        auto length = read_primative<Int>(file);
        if (length == 0)
            return {};
        if (length < 0)
            throw ReadError("Length < 0");

        if (type == TAG_End)
            throw ReadError("List of type End cannot contain any items");

        List result;
        result.reserve(length);
        while (length-- != 0)
            result.append(read_payload(file, type));

        return {result, type};
    }

    static Compound read_compound(QIODevice &file) {
        Compound result;
        Tag tag;
        while (!(tag = read_named(file)).is_end())
            result[tag.name()] = std::move(tag);

        return result;
    }

    static Tag read_payload(QIODevice &file, TagType type, QString name) {
        switch (type) {
            case TAG_End:
                return {};
            case TAG_Byte:
                return {read_primative<Byte>(file), name};
            case TAG_Short:
                return {read_primative<Short>(file), name};
            case TAG_Int:
                return {read_primative<Int>(file), name};
            case TAG_Long:
                return {read_primative<Long>(file), name};
            case TAG_Float:
                return {read_primative<Float>(file), name};
            case TAG_Double:
                return {read_primative<Double>(file), name};
            case TAG_Byte_Array:
                return {read_array<Byte>(file), name};
            case TAG_String:
                return {read_string(file), name};
            case TAG_List: {
                const auto [list, item_type] = read_list(file);
                return {list, item_type, name};
            }
            case TAG_Compound:
                return {read_compound(file), name};
            case TAG_Int_Array:
                return {read_array<Int>(file), name};
            case TAG_Long_Array:
                return {read_array<Long>(file), name};
        }

        return {};
    }

    static Tag read_tag(QIODevice &file, bool named) {
        const auto type = read_tag_type(file);
        if (type == TAG_End)
            return {};

        const QString name = named ? read_string(file) : QString();
        return read_payload(file, type, name);
    }

    Tag read_named(QIODevice &file) {
        return read_tag(file, true);
    }

    Tag read_unnamed(QIODevice &file) {
        return read_tag(file, false);
    }

    const QString TAG_NAMES[] = {
            "TAG_End",
            "TAG_Byte",
            "TAG_Short",
            "TAG_Int",
            "TAG_Long",
            "TAG_Float",
            "TAG_Double",
            "TAG_Byte_Array",
            "TAG_String",
            "TAG_List",
            "TAG_Compound",
            "TAG_Int_Array",
            "TAG_Long_Array"
    };

    QString debug_string(const Tag &tag) {
        QString result = TAG_NAMES[tag.type()];
        if (!tag.name().isNull()) {
            result.append("(\"");
            result.append(tag.name());
            result.append("\")");
        }
        result.append(": ");

        switch (tag.type()) {
            case TAG_End:
                break;
            case TAG_Byte:
                result.append(QString::number(tag.byte_value()));
                break;
            case TAG_Short:
                result.append(QString::number(tag.short_value()));
                break;
            case TAG_Int:
                result.append(QString::number(tag.int_value()));
                break;
            case TAG_Long:
                result.append(QString::number(tag.long_value()));
                break;
            case TAG_Float:
                result.append(QString::number(tag.float_value(), 'g', 16));
                break;
            case TAG_Double:
                result.append(QString::number(tag.double_value(), 'g', 16));
                break;
            case TAG_Byte_Array: {
                const ByteArray &value = tag.byte_array_value();
                result.append(QString::number(value.count()));
                result.append(" entries\n{\n");
                for (const Byte &item: value) {
                    result.append('\t');
                    result.append(QString::number(item));
                    result.append('\n');
                }
                result.append('}');
                break;
            }
            case TAG_String:
                result.append(tag.string_value());
                break;
            case TAG_List: {
                const List &value = tag.list_value();
                result.append(QString::number(value.count()));
                result.append(" entries of type ");
                result.append(TAG_NAMES[tag.content_type()]);
                result.append("\n{\n");
                for (const Tag &item: value) {
                    result.append('\t');
                    result.append(debug_string(item).replace("\n", "\n\t"));
                    result.append('\n');
                }
                result.append('}');
                break;
            }
            case TAG_Compound: {
                const Compound &value = tag.compound_value();
                result.append(QString::number(value.count()));
                result.append(" entries\n{\n");
                for (const Tag &item: value) {
                    result.append('\t');
                    result.append(debug_string(item).replace("\n", "\n\t"));
                    result.append('\n');
                }
                result.append('}');
                break;
            }
            case TAG_Int_Array: {
                const IntArray &value = tag.int_array_value();
                result.append(QString::number(value.count()));
                result.append(" entries\n{\n");
                for (const Int &item: value) {
                    result.append('\t');
                    result.append(QString::number(item));
                    result.append('\n');
                }
                result.append('}');
                break;
            }
            case TAG_Long_Array: {
                const LongArray &value = tag.long_array_value();
                result.append(QString::number(value.count()));
                result.append(" entries\n{\n");
                for (const Long &item: value) {
                    result.append('\t');
                    result.append(QString::number(item));
                    result.append('\n');
                }
                result.append('}');
                break;
            }
        }

        return result;
    }

}