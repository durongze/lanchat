#ifndef __OCTREE_H__
#define __OCTREE_H__
#include <map>

class OctreeNode
{
public:
	OctreeNode();
	OctreeNode(int level, int value);
	OctreeNode(const OctreeNode& node);
	~OctreeNode();

public:
	int Assign(int level, int value);
	int Assign(const OctreeNode& node);
	int InsertChild(int idx, OctreeNode*& node);
	int RemoveChild(int idx);
	int PeekChild(int idx, OctreeNode& node);
	void Dump();
private:
	std::map<int, OctreeNode*> m_childNode;
	int m_val;
	int m_lvl;
};

class Octree
{
public:
	Octree();
	Octree(int depth);
	~Octree();
	void InitChild(OctreeNode *child, int d, int depth);
	void Dump();
private:
	int m_childNum;
	int m_depth;
	OctreeNode *m_root;
};
#endif // !__OCTREE_H__