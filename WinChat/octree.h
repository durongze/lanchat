#ifndef __OCTREE_H__
#define __OCTREE_H__
#include <map>
#include <fstream>

class Number
{
public:
	Number(int value = 0);
	~Number();
	void InitBit(int value);
	int TruncBit(int bitNum);
	int SetBit(int bitIdx, int tabIdx, int mapIdx);
	void Dump(std::fstream& fs);
	int GetTabIdx(int bitIdx);
	int GetMapIdx(int bitIdx);
public:
	struct Bit
	{
		int tabIdx;
		int mapIdx;
	public:
		Bit(int t = 0, int m = 0) : tabIdx(t), mapIdx(m) {}
		Bit(const Bit& b) { if (this == &b) return; tabIdx = b.tabIdx, mapIdx = b.mapIdx; }
		Bit& operator=(const Bit& b) { if (this == &b) return *this; tabIdx = b.tabIdx, mapIdx = b.mapIdx; }
		bool operator==(const Bit& b) { return tabIdx == b.tabIdx && mapIdx == b.mapIdx; }
	};
	typedef std::map<int, Bit>::iterator Iterator;
	Iterator begin();
	Iterator end();
private:
	std::map<int, Bit> m_bit;
};

class OctreeNode
{
public:
	OctreeNode(int level = 0, int tabIdx = 0, int mapIdx = 0);
	OctreeNode(const OctreeNode& node);
	~OctreeNode();

public:
	int Assign(int level, int value, int idxNum);
	int Assign(const OctreeNode& node);
	int InsertChild(int idx, OctreeNode*& node);
	int ReplaceChild(int idx, OctreeNode*& node);
	int RemoveChild(int idx);
	int PeekChild(int idx, OctreeNode& node);
	void Dump(std::fstream& fs, unsigned int idxChild);
	OctreeNode *NextChild(int idxChild);
	Number::Bit GetValue();
	int GetLevel();
	int ContainsChild(int idxNum);
private:
	std::map<int, OctreeNode*> m_childNode;
	OctreeNode* m_parent;
	Number::Bit m_val;
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
	int GetNumberType(Number& num);
	void Dump(std::fstream& fs);
private:
	int m_childNum;
	int m_depth;
	OctreeNode *m_root;
};

#endif // !__OCTREE_H__