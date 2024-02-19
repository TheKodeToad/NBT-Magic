#pragma once

#include <QTreeWidget>
#include "nbt/tag.hpp"

class TagModel : public QAbstractListModel {
public:
    explicit TagModel(std::shared_ptr<nbt::NamedTag> tag, QObject *parent = nullptr);

private:
    std::shared_ptr<nbt::NamedTag> tag;
};
