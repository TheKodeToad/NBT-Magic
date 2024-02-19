#include "tag_model.hpp"

TagModel::TagModel(std::shared_ptr<nbt::NamedTag> tag, QObject *parent) : tag(std::move(tag)), QAbstractListModel(parent) {

}
