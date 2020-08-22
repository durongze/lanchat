#ifndef __OCTREE_H__
#define __OCTREE_H__
#include <map>

class Number
{
public:
	Number(int type, int value, unsigned int bitNum = sizeof(int) * 8);
	~Number();
	void InitBit(int value);
	int TruncBit(int bitNum);
	void Dump(std::fstream& fs);
	int GetType();
public:
	typedef std::map<int, int>::iterator Iterator;
	Iterator begin();
	Iterator end();
private:
	int m_val;
	int m_type;
	unsigned int m_bitNum;
	std::map<int, int> m_bit;
};

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
	void Dump(std::fstream& fs, unsigned int idxChild);
	OctreeNode *NextChild(int idx);
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
	void InitChild(OctreeNode *child, int level, int depth);
	int InsertNumber(Number& num);
	void Dump(std::fstream& fs);
private:
	int m_childNum;
	int m_depth;
	OctreeNode *m_root;
};

#endif // !__OCTREE_H__