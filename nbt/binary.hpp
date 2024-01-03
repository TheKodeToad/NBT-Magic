#pragma once

#include <QVariant>
#include <QIODevice>
#include <utility>
#include <iostream>
#include "tag.hpp"

namespace nbt {

    NamedTag read_named_binary(QIODevice &file);
    Tag read_unnamed_binary(QIODevice &file);

    void write_named_binary(QIODevice &file, const NamedTag &tag);
    void write_unnamed_binary(QIODevice &file, const Tag &tag);

    class IOError : public std::exception {
    public:
        IOError() = default;
        explicit IOError(const QString& message) : message(message.toLocal8Bit().toStdString()) {}

        const char* what() const noexcept override { return message.c_str(); }

    private:
        // used simply for ownership
        std::string message;
    };

}
