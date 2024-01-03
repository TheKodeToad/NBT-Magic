/**
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
