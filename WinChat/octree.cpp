#include "stdafx.h"
#include "octree.h"
#include <iostream>
#include <fstream>

std::fstream g_fs;

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

void OctreeNode::Dump()
{
	std::map<int, OctreeNode*>::const_iterator iter;
	g_fs << "[" << m_lvl << ","  << m_val << "] ";
	g_fs << std::endl;
	for (iter = m_childNode.begin(); m_childNode.end() != iter; iter++) {
		for (int id = 0; id < m_lvl; id++) {
			g_fs << "    ";
		}
		if (iter->second) {
			iter->second->Dump();
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
	m_root = new OctreeNode();
	InitChild(m_root, 0, m_depth);
}

Octree::~Octree()
{

}

void Octree::InitChild(OctreeNode *root, int d, int depth)
{
	if (root == NULL) {
		return;
	}
	if (++d >= depth) {
		return;
	}
	unsigned int l;
	for (l = 0; l < m_childNum; l++) {
		int value = l;
		OctreeNode *child = new OctreeNode(d, value);
		if (child != NULL) {
			root->InsertChild(l, child);
			InitChild(child, d, depth);
		}
	}
	return;
}

void Octree::Dump()
{
	g_fs.open("octree_log.txt", std::ios::trunc | std::ios::out | std::ios::in);
	m_root->Dump();
	g_fs.close();
}