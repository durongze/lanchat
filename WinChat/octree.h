#ifndef __OCTREE_H__
#define __OCTREE_H__
#include <map>
#include <fstream>
class Number
{
public:
	Number(int value);
	~Number();
	void InitBit(int value);
	int TruncBit(int bitNum);
	int SetBit(int bitIdx, int typeIdx, int bitVal);
	void Dump(std::fstream& fs);
	int GetType(int bitIdx);
public:
	typedef std::map<int, int>::iterator Iterator;
	Iterator begin();
	Iterator end();
private:
	std::map<int, int> m_type;
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
	int ReplaceChild(int idx, OctreeNode*& node);
	int RemoveChild(int idx);
	int PeekChild(int idx, OctreeNode& node);
	void Dump(std::fstream& fs, unsigned int idxChild);
	OctreeNode *NextChild(int idxChild);
	int GetValue();
	int GetLevel();
private:
	std::map<int, OctreeNode*> m_childNode;
	int m_val;
	int m_lvl;
};

class Octree
{
public:
	Octree(int depth = 0);
	~Octree();
	void InitChild(OctreeNode *child, int level, int depth, int base = 0);
	int InsertNumber(Number& num);
	int GetChildIdx(int idxAllChild, int level);
	OctreeNode *GetChild(int idxAllChild, int level, int& idxChild);
	int PickupNumber(Number& num, int idx, std::fstream& fsOct);
	void Dump(std::fstream& fs);
private:
	int m_childNum;
	int m_depth;
	OctreeNode *m_root;
};

#endif // !__OCTREE_H__