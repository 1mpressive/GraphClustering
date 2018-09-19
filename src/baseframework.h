#ifndef BASEFRAMEWORK_H
#define BASEFRAMEWORK_H

//#include<Python.h>  // ����Python

#include <string>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <queue>
#include <stack>

#include "Vertex.h"
#include "mmf.h"

using namespace std;


class BaseFramework
{
protected:
	string m_inputpath;         // ��������·��
	string m_stoc_file;
	int m_minPts;               // dbscan ���� minPts

	map<int, map<int, float>> m_pprDistances;		// �洢ppr score
	unordered_set<int> m_valid_cluster_points;		// ͳ����Ч�ľ����
	vector<set<int>> m_clusters;					// ������

	void readGraph();					// ��ͼ
	void PPRSymmetrization();			// ���������ĶԳƻ���ʽ, approximate, partial ����ʹ��
	void SD_PPRSymmetrization();		// �������ĶԳƻ���ʽ, game theory ����ʹ��
	void dbscan();						// DBSCAN
	float getEntropy(set<int> & cluster, int type1, int type2);    // �����ض�����(type1, type2)����
	float weightUpdate_Entropy();                                  // �����ص�Ȩ�ظ��·���
	float weightUpdate_Vote();                                     // ����ͶƱ��Ȩ�ظ����㷨

	void storeClusterResult(string cluster_output);                // �洢������
	void storeClusterResultForComparison(string cluster_output_path);   // ��baseline�Ĵ������������ָ��

	void getClusterResult(std::string _cluster_result_path);

	float clusterEvaluate_Density();                               // ����Ч������ - density
	float clusterEvaluate_Entropy1();                              // ����Ч������ - entropy (ֱ����ƽ��)
	float clusterEvaluate_Entropy2();                              // ����Ч������ - entropy (��Ȩƽ��)
	float clusterEvaluate_WithinClusterAveDistance();              // ����Ч������ - �ؼ�ƽ������
	float clusterEvaluate_NMI();                                   // ����Ч������ - NMI

public:
	explicit BaseFramework(const char * argv[]);
	virtual ~BaseFramework(){}
};


// �������� 
class BaseReversePush : public BaseFramework
{
private:
	void baseReservePush();
	void baseReservePushWithMemory();
public:
	explicit BaseReversePush(const char * argv[]);
	~BaseReversePush(){}
	void execute();
};


// ���Ƽ��㷽��
class ApproximateReversePush : public BaseFramework
{
private:
	void reversePush_Pre_Encode();
	void approximateReversePushMultiThreadUpdate();
public:
	explicit ApproximateReversePush(const char * argv[]);
	~ApproximateReversePush(){}
	void execute();
};


// ���ּ��㷽��
class PartialReversePush : public BaseFramework
{
private:
	void reversePush_Partial_Encode();
	void partialReversePushMultiThreadUpdate();
public:
	explicit PartialReversePush(const char * argv[]);
	~PartialReversePush(){}
	void execute();
};


// �����۷���
class GameTheoty : public BaseFramework
{
private:
	struct DegreenNode    // ����ڵ㼰���Ӧ�Ķ� 
	{
		int s_vertexid;
		int s_degree;

		DegreenNode(int _vertexid, int _degree)
		{
			this->s_vertexid = _vertexid;
			this->s_degree = _degree;
		}

		bool operator < (const DegreenNode & _dn) const  // ����
		{
			if (this->s_degree != _dn.s_degree)
				return (this->s_degree > _dn.s_degree);   
			if (this->s_vertexid != _dn.s_vertexid)
				return (this->s_vertexid < _dn.s_vertexid);
			return false;
		}
	};

	struct CostNode     // ����ڵ㼰���Ӧ�ĵ�������
	{
		int s_clusterid;
		float s_cost;

		CostNode(int _clusterid, float _cost)
		{
			this->s_clusterid = _clusterid;
			this->s_cost = _cost;
		}

		bool operator < (const CostNode & _cn) const  // ����
		{
			if (this->s_cost != _cn.s_cost)
				return (this->s_cost < _cn.s_cost);             // s_cost ��ͬ������ s_cost ��������
			if (this->s_clusterid != _cn.s_clusterid)     
				return this->s_clusterid < _cn.s_clusterid;     // s_cost ��ͬ�� ���� s_clusterid ����������
			return false;                                       // ���߲���ͬʱΪtrue: strict weak ordering
		}
	};

	
	unordered_map<int, vector<float>> m_GlobalTable;   // Global Table, ����best response dynamics���Ż�

	unordered_map<int, set<CostNode>> cost_queue;      // priority queue for cost, ascending order of cost

	//set<DegreenNode> m_happy_queue;                    // store the points to be adjusted, descending order of degree
	//unordered_set<int> m_happy_queue;                  // store the points to be adjusted, no order
	//queue<int> m_happy_queue;
	stack<int> m_happy_queue;
	float m_cn;                                        // ��һ������(�ݲ�����)
	int m_updatetimes;                                 // update points times per iteration
	bool m_cn_flag;                                    // �Ƿ���㲢ʹ�ù�һ������

	unordered_map<int, vector<vector<unordered_map<int, int>>>> AET;   // �洢�����Ե�ĳ��ֵĴ���, �����Ż��صļ���
	/*            ��     ��    ����    ����ִ���                */

	float getResponsecost(int _vertexid, int _clusterid);
	void gameTheory_ReservePush();

	void gameTheory_dbscan();

	void buildGlobalTable();
	void initializeHappyQueue();
	void bestResponseDynamics();

	// �����ã�
	float getClusterEntropy(set<int> & _cluster);   // ������ڵ�ƽ����
	void E_buildGlobalTable();
	void E_initializeHappyQueue();
	void E_bestResponseDynamics();

	// �����ã�
	float AET_getClusterEntropy(int _vertexid, int _clusterid);
	void AET_buildGlobalTable();
	void AET_bestResponseDynamics();
	
	void gatherClusterResult();       // �ռ�ÿ�ֵ���������Ľ��
	void gameTheoryModulation();      // �������Ż��㷨
public:
	explicit GameTheoty(const char * argv[]);
	~GameTheoty(){}
	void execute();
};

#endif