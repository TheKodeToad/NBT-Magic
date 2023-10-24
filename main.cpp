#include <iostream>
#include <QFile>
#include <QDebug>

#include "nbt/io.hpp"

int main() {
    QFile file("bigtest.nbt");
    if (!file.open(QFile::ReadOnly))
        return 1;

    auto nbtee = nbt::read_named(file);
    std::cout << nbt::debug_string(nbtee).toStdString() << '\n';
    return 0;
}
