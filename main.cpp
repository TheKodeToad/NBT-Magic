#include <iostream>
#include <QFile>
#include <QDebug>
#include <QFileInfo>

#include "nbt/binary.hpp"

int main() {
    QFile file("bigtest.nbt");
    if (!file.open(QFile::ReadOnly))
        return 1;

    auto nbt = nbt::read_named_binary(file);
    qInfo() << nbt.name << '\n';

    QFile out("out.nbt");
    if (!out.open(QFile::WriteOnly))
        return 1;

    nbt::write_named_binary(out, nbt);
    return 0;
}
