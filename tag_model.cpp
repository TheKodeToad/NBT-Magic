/*
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

#include "tag_model.hpp"

enum : int {
	COLUMN_KEY,
	COLUMN_VALUE,
	COLUMN_COUNT
};

struct TagModelNode {
	TagModelNode *parent;
	nbt::Tag *tag;
	nbt::NamedTag *named_tag;
};

template<typename T>
static int index_of_ptr(const QList<T> &list, const T *item) {
	int index = 0;

	for (const T &check_item : list) {
		if (&check_item == item)
			return index;

		++index;
	}

	return -1;
}

TagModel::TagModel(std::shared_ptr<nbt::NamedTag> tag, QObject *parent)
	: root_tag(std::move(tag)), root_node(std::make_unique<TagModelNode>(nullptr, &root_tag->tag, root_tag.get())), QAbstractItemModel(parent) {}

QModelIndex TagModel::index(int row, int column, const QModelIndex &parent) const {
	TagModelNode *parent_node = node(parent);

		switch (parent_node->tag->type()) {
		case nbt::TagType::BYTE_ARRAY:
		case nbt::TagType::LIST:
		case nbt::TagType::INT_ARRAY:
		case nbt::TagType::LONG_ARRAY:
			return createIndex(row, column, new TagModelNode(parent_node, &parent_node->tag->list_value()[row], nullptr));
		case nbt::TagType::COMPOUND: {
			nbt::NamedTag &tag = parent_node->tag->compound_value()[row];
			return createIndex(row, column, new TagModelNode(parent_node, &tag.tag, &tag));
		}
		default:
			return {};
	}
}

int TagModel::rowCount(const QModelIndex &parent) const {
	TagModelNode *parent_node = node(parent);

	switch (parent_node->tag->type()) {
		case nbt::TagType::BYTE_ARRAY:
		case nbt::TagType::LIST:
		case nbt::TagType::INT_ARRAY:
		case nbt::TagType::LONG_ARRAY:
			return parent_node->tag->list_value().length();
		case nbt::TagType::COMPOUND:
			return parent_node->tag->compound_value().length();
		default:
			return 0;
	}
}

int TagModel::columnCount(const QModelIndex &parent) const {
	return COLUMN_COUNT;
}

QVariant TagModel::data(const QModelIndex &index, int role) const {
	if (role != Qt::DisplayRole)
		return {};

	TagModelNode *index_node = node(index);

	switch (index.column()) {
		case COLUMN_KEY:
			if (index_node->named_tag != nullptr)
				return index_node->named_tag->name;

			return QString::number(index.row());
		case COLUMN_VALUE:
			switch (index_node->tag->type()) {
				case nbt::TagType::BYTE:
					return QString::number(index_node->tag->byte_value());
				case nbt::TagType::SHORT:
					return QString::number(index_node->tag->short_value());
				case nbt::TagType::INT:
					return QString::number(index_node->tag->int_value());
				case nbt::TagType::LONG:
					return QString::number(index_node->tag->long_value());
				case nbt::TagType::FLOAT:
					return QString::number(index_node->tag->float_value());
				case nbt::TagType::DOUBLE:
					return QString::number(index_node->tag->double_value());
				case nbt::TagType::STRING:
					return index_node->tag->string_value();
				case nbt::TagType::BYTE_ARRAY:
				case nbt::TagType::LIST:
				case nbt::TagType::INT_ARRAY:
				case nbt::TagType::LONG_ARRAY:
					return tr("[%1 tags]").arg(index_node->tag->list_value().length());
				case nbt::TagType::COMPOUND:
					return tr("[%1 tags]").arg(index_node->tag->compound_value().length());
				default:
					return "???";
			}
	}
}

QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (!(role == Qt::DisplayRole && orientation == Qt::Horizontal))
		return {};

	switch (section) {
		case COLUMN_KEY:
			return tr("Key");
		case COLUMN_VALUE:
			return tr("Value");
		default:
			return {};
	}
}

QModelIndex TagModel::parent(const QModelIndex &child) const {
	TagModelNode *child_node = node(child);
	TagModelNode *parent_node = child_node->parent;

	if (parent_node == nullptr)
		return {};

	TagModelNode *grandparent_node = parent_node->parent;

	if (grandparent_node == nullptr)
		return createIndex(0, 0, root_node.get());

	switch (grandparent_node->tag->type()) {
		case nbt::TagType::BYTE_ARRAY:
		case nbt::TagType::LIST:
		case nbt::TagType::INT_ARRAY:
		case nbt::TagType::LONG_ARRAY:
			return createIndex(index_of_ptr(grandparent_node->tag->list_value(), parent_node->tag), 0, parent_node);
		case nbt::TagType::COMPOUND:
			return createIndex(index_of_ptr(grandparent_node->tag->compound_value(), parent_node->named_tag), 0, parent_node);
		default:
			return {};
	}
}

TagModelNode *TagModel::node(const QModelIndex &index) const {
	void *ptr = index.internalPointer();

	if (ptr == nullptr)
		return root_node.get();

	return static_cast<TagModelNode *>(ptr);
}
