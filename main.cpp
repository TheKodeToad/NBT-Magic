#include <iostream>
#include <QFile>
#include <QDebug>

#include "nbt/io.hpp"

int main() {
    QFile file("bigtest.nbt");
    if (!file.open(QFile::ReadOnly))
        return 1;

    auto nbtee = nbt::read_named(file);
    qInfo() << nbtee.name << '\n';
    return 0;
}
