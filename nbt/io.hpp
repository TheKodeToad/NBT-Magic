#pragma once

#include <QVariant>
#include <QIODevice>
#include <utility>
#include <iostream>
#include "tag.hpp"

namespace nbt {

    bool is_gzipped(QIODevice &file);
    nbt::Tag read_named(QIODevice &file);
    nbt::Tag read_unnamed(QIODevice &file);
    QString debug_string(const nbt::Tag &tag);

    class ReadError : public std::exception {
    public:
        ReadError() = default;
        explicit ReadError(const QString& message) : message(message.toLocal8Bit().toStdString()) {}

        const char* what() const noexcept override { return message.c_str(); }

    private:
        // used simply for ownership
        std::string message;
    };

}
