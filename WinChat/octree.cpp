#include "stdafx.h"
#include "octree.h"
#include <iostream>
#include <fstream>

int startLevel = 1;

Number::Number(int value)
{
	InitBit(value);
}

Number::~Number()
{

}

void Number::InitBit(int value)
{
	int startBitIdx = startLevel;
	int m_bitNum = sizeof(value) * 8;
	unsigned int idx;
	for (idx = 0; idx < m_bitNum; idx++) {
		if (value >> (m_bitNum - idx - 1) & 1) {
			break;
		}
	}
	m_bitNum -= idx;

	for (idx = 0 + startBitIdx; idx < m_bitNum + startBitIdx; idx++) {
		m_bit[idx].mapIdx = (value >> (m_bitNum - idx - 1)) & 1;
		m_bit[idx].tabIdx = 1;
	}
}

int Number::TruncBit(int bitNum)
{
	unsigned int idx;
	Iterator iter;
	for (idx = 0, iter = m_bit.begin(); idx < bitNum && iter != m_bit.end(); idx++, ++iter) {
	}
	m_bit.erase(iter, m_bit.end());

	return 0;
}

int Number::SetBit(int bitIdx, int tabIdx, int mapIdx)
{
	m_bit[bitIdx] = Bit(tabIdx, mapIdx);
	return 0;
}

void Number::Dump(std::fstream& fs)
{
	Iterator iter;
	fs << typeid(this).name() << " " << __FUNCTION__ << std::endl;
	for (iter = m_bit.begin();iter != m_bit.end() ;++iter) {
		fs << "[" << iter->first << ":"  << iter->second.tabIdx << "," << iter->second.mapIdx << "]";
	}
	fs << std::endl;
}

int Number::GetTabIdx(int bitIdx)
{
	return m_bit[bitIdx].tabIdx;
}
int Number::GetMapIdx(int bitIdx)
{
	return m_bit[bitIdx].mapIdx;
}

Number::Iterator Number::begin()
{
	return m_bit.begin();
}

Number::Iterator Number::end()
{
	return m_bit.end();
}

OctreeNode::OctreeNode(int level, int tabIdx, int mapIdx)
	:m_val(tabIdx, mapIdx), m_lvl(level), m_childNode(), m_parent(NULL)
{
}

OctreeNode::OctreeNode(const OctreeNode& node)
	:m_val(node.m_val), m_lvl(node.m_lvl), m_childNode(), m_parent(NULL)
{
	if (this == &node) {
		return;
	}
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

int OctreeNode::Assign(int level, int tabIdx, int mapIdx)
{
	m_lvl = level;
	m_val.tabIdx = tabIdx;
	m_val.mapIdx = mapIdx;
	return 0;
}

int OctreeNode::Assign(const OctreeNode& node)
{
	if (this == &node) {
		return 0;
	}
	Assign( node.m_lvl, node.m_val.tabIdx, node.m_val.mapIdx); 
	std::map<int, OctreeNode*>::const_iterator iter;
	for (iter = node.m_childNode.begin(); node.m_childNode.end() != iter; iter++) {
		OctreeNode *child = NULL;
		if (iter->second) {
			child = new OctreeNode(*iter->second);
		}
		if (m_childNode[iter->first] == NULL) {
			m_childNode[iter->first] = child;
		} else {
			if (child != NULL) {
				m_childNode[iter->first]->Assign(*child);
			}
			else {
				delete m_childNode[iter->first];
				m_childNode[iter->first] = NULL;
			}
		}
	}
	return 0;
}

int OctreeNode::InsertChild(int idx, OctreeNode*& node)
{
	// return ReplaceChild(idx, node);
	if (m_childNode[idx] == NULL) {
		m_childNode[idx] = node;
		return 0;
	} 
	if (node != NULL) {
		m_childNode[idx]->Assign(*node);
	}
	else {
		m_childNode[idx] = NULL;
	}
	return 0;
}

int OctreeNode::ReplaceChild(int idx, OctreeNode*& node)
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

OctreeNode *OctreeNode::NextChild(int idxChild)
{
	return m_childNode[idxChild];
}

Number::Bit OctreeNode::GetValue()
{
	return m_val;
}
int OctreeNode::GetLevel()
{
	return m_lvl;
}

int OctreeNode::ContainsChild(int idxNum)
{
	return !(m_childNode.find(idxNum) == m_childNode.end());
}

void OctreeNode::Dump(std::fstream& fs, unsigned int idxChild)
{
	unsigned int idx;
	std::map<int, OctreeNode*>::const_iterator iter;
	fs << "[" << m_lvl << "," << idxChild << "," << m_val.tabIdx << ":" << m_val.mapIdx << "] ";
	fs << std::endl;
	for (iter = m_childNode.begin(), idx = 0; m_childNode.end() != iter; iter++, idx++) {
		for (int id = startLevel; id < m_lvl + startLevel; id++) {
			fs << "    ";
		}
		if (iter->second) {
			iter->second->Dump(fs, iter->first);
		}
	}

}

Octree::Octree(int depth)
	: m_depth(depth), m_root(NULL), m_childNum(8)
{
	m_root = new OctreeNode(0, 65535, 0);
	if (depth > 0) {
		InitChild(m_root, 1, m_depth);
	}
}

Octree::~Octree()
{
	if (m_root) {
		delete m_root;
	}
}

void Octree::InitChild(OctreeNode *root, int level, int depth, int base)
{
	if (root == NULL) {
		return;
	}
	if (level > depth) {
		return;
	}
	unsigned int l;
	for (l = 0; l < m_childNum; l++) {
		int value = base * 100 + level * 10 + l;
		OctreeNode *child = new OctreeNode(level, value, 0);
		if (child != NULL) {
			root->InsertChild(l, child);
			InitChild(child, level + 1, depth, value);
		}
	}
	return;
}

int Octree::InsertNumber(Number& num)
{
	Number::Iterator iter;
	OctreeNode *root = NULL;
	for (iter = num.begin(), root = m_root;	iter != num.end() && root != NULL; iter++) {
		OctreeNode *node = new OctreeNode(iter->first, iter->second.tabIdx, iter->second.mapIdx);
		root->InsertChild(iter->second.mapIdx, node);
		root = root->NextChild(iter->second.mapIdx);
	}
	return 0;
}

int Octree::GetChildIdx(int idxAllChild, int level)
{
	int lvl = m_depth - level;
	int idxChild = idxAllChild / (int)pow(m_childNum, lvl) % m_childNum;
	return idxChild;
}

OctreeNode *Octree::GetChild(int idxAllChild, int level, int& idxChild)
{
	int i;
	OctreeNode *child = m_root;
	for (i = 1;	i <= level && child != NULL; i++) {
		idxChild = GetChildIdx(idxAllChild, i);
		child = child->NextChild(idxChild);
	}

	return child;
}

/*idx 为最大层的编号，该编号到根节点只有一个路径，即为我们要的数字*/
int Octree::PickupNumber(Number& num, int idx, std::fstream& fsOct)
{
	int level = startLevel;
	int depth = m_depth;
	int idxChild = 0;
	int i;
	OctreeNode *child = m_root;
	for (i = 1;	i <= m_depth && child != NULL; i++) {
		idxChild = GetChildIdx(idx, i);
		child = child->NextChild(idxChild);
		Number::Bit bit = child->GetValue();
		num.SetBit(i - 1, bit.tabIdx, bit.mapIdx);
		fsOct << "[" << i << "," << idxChild << "," << bit.tabIdx << ":" << bit.mapIdx << "] ";
	}
	fsOct << std::endl;
	num.TruncBit(m_depth);

	return 0;
}

int Octree::GetNumberType(Number& num)
{
	int bitIdx = startLevel;
	Number::Iterator iter;
	OctreeNode* root = NULL;
	for (iter = num.begin(), root = m_root;	iter != num.end() && root != NULL; iter++) {
		if (root->ContainsChild(iter->second.mapIdx)) {
			root = root->NextChild(iter->second.mapIdx);
		}
		else {
			return root->GetValue().tabIdx;
		}
	}
	return 0;
}

void Octree::Dump(std::fstream& fs)
{
	if (m_root) {
		m_root->Dump(fs, startLevel);
	}
	else {
		fs << "Octree is NULL.";
	}
}

