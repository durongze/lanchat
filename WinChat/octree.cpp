#include "stdafx.h"
#include "octree.h"
#include <iostream>
#include <fstream>

Number::Number(int type, int value, unsigned int bitNum)
	:m_val(value), m_bitNum(bitNum), m_type(type)
{
	InitBit(m_val);
}

Number::~Number()
{

}

void Number::InitBit(int value)
{
	m_bitNum = sizeof(value) * 8;
	unsigned int idx;
	for (idx = 0; idx < m_bitNum; idx++) {
		m_bit[idx] = (value >> (m_bitNum - idx - 1)) & 1;
	}
}

int Number::TruncBit(int bitNum)
{
	unsigned int idx;
	std::map<int, int>::iterator iter;
	for (idx = 0, iter = m_bit.begin(); idx < bitNum && iter != m_bit.end(); idx++, ++iter) {
	}
	m_bit.erase(iter, m_bit.end());
	return 0;
}

void Number::Dump(std::fstream& fs)
{
	std::map<int, int>::iterator iter;
	for (iter = m_bit.begin(); iter != m_bit.end(); ++iter) {
		fs << "[" << iter->first << "," << iter->second << "] ";
	}
}

int Number::GetType()
{
	return m_type;
}

Number::Iterator Number::begin()
{
	return m_bit.begin();
}

Number::Iterator Number::end()
{
	return m_bit.end();
}

OctreeNode::OctreeNode()
	:m_val(0), m_childNode(), m_lvl(0)
{
}

OctreeNode::OctreeNode(int level, int value)
	:m_val(value), m_childNode(), m_lvl(level)
{
}

OctreeNode::OctreeNode(const OctreeNode& node)
{
	if (this == &node) {
		return;
	}
	m_val = node.m_val;
	m_lvl = node.m_lvl;
	std::map<int, OctreeNode*>::const_iterator iter;
	for (iter = node.m_childNode.begin(); node.m_childNode.end() != iter; iter++) {
		OctreeNode *child = NULL;
		if (iter->second) {
			child = new OctreeNode(*iter->second);
		}
		m_childNode[iter->first] = child;
	}
}

OctreeNode::~OctreeNode()
{
	std::map<int, OctreeNode*>::iterator iter;
	for (iter = m_childNode.begin(); m_childNode.end() != iter; iter++) {
		if (iter->second) {
			delete iter->second; // 
			iter->second = NULL;
		}
	}
	m_childNode.clear();
}

int OctreeNode::Assign(int level, int value)
{
	m_lvl = level;
	m_val = value;
	return 0;
}

int OctreeNode::Assign(const OctreeNode& node)
{
	if (this == &node) {
		return 0;
	}
	m_val = node.m_val;
	m_lvl = node.m_lvl;
	std::map<int, OctreeNode*>::const_iterator iter;
	for (iter = node.m_childNode.begin(); node.m_childNode.end() != iter; iter++) {
		OctreeNode *child = NULL;
		if (iter->second) {
			child = new OctreeNode(*iter->second);
		}
		m_childNode[iter->first] = child;
	}
	return 0;
}

int OctreeNode::InsertChild(int idx, OctreeNode*& node)
{
	if (m_childNode[idx] != NULL) {
		delete m_childNode[idx];
	}
	m_childNode[idx] = node;
	return 0;
}

int OctreeNode::RemoveChild(int idx)
{
	if (m_childNode[idx] != NULL) {
		delete m_childNode[idx];
		m_childNode[idx] = NULL;
	}
	return 0;
}

int OctreeNode::PeekChild(int idx, OctreeNode& node)
{
	if (m_childNode[idx] != NULL) {
		node.Assign(*m_childNode[idx]);
	}
	return 0;
}

OctreeNode *OctreeNode::NextChild(int idx)
{
	return m_childNode[idx];
}

void OctreeNode::Dump(std::fstream& fs, unsigned int idxChild)
{
	unsigned int idx;
	std::map<int, OctreeNode*>::const_iterator iter;
	fs << "[" << m_lvl << "," << idxChild << ","  << m_val << "] ";
	fs << std::endl;
	for (iter = m_childNode.begin(), idx = 0; m_childNode.end() != iter; iter++, idx++) {
		for (int id = -1; id < m_lvl; id++) {
			fs << "    ";
		}
		if (iter->second) {
			iter->second->Dump(fs, idx);
		}
	}
}

Octree::Octree()
	:m_depth(0), m_root(NULL),m_childNum(8)
{

}

Octree::Octree(int depth)
	: m_depth(depth), m_root(NULL), m_childNum(8)
{
	m_root = new OctreeNode(-1, 65535);
	InitChild(m_root, 0, m_depth);
}

Octree::~Octree()
{
	delete m_root;
}

void Octree::InitChild(OctreeNode *root, int level, int depth)
{
	int base = 100;
	if (root == NULL) {
		return;
	}
	if (++level >= depth) {
		return;
	}
	unsigned int l;
	for (l = 0; l < m_childNum; l++) {
		int value = l;
		OctreeNode *child = new OctreeNode(level, base + value);
		if (child != NULL) {
			root->InsertChild(l, child);
			InitChild(child, level, depth);
		}
	}
	return;
}

int Octree::InsertNumber(Number& num)
{
	int childIdx = num.GetType();
	Number::Iterator iter;
	OctreeNode *root = NULL;
	for (iter = num.begin(), root = m_root;
		iter != num.end() && root != NULL;
		iter++, root = root->NextChild(childIdx)) {
		OctreeNode *node = new OctreeNode(iter->first, iter->second);
		root->InsertChild(childIdx, node);
	}
	return 0;
}

void Octree::Dump(std::fstream& fs)
{
	m_root->Dump(fs, 0);
}

