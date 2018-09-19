#include <fstream>
#include <time.h>

#include "baseframework.h"
#include "MyGlobalParameters.h"


GameTheoty::GameTheoty(const char * argv[]) : BaseFramework(argv){}


inline float getTransprob(Vertex * _u, Vertex * _v)
{
	float real_total_edgeweight = 0.0f;
	for (auto n : _u->edgetype_outdegree)
	{
		if (n != 0)
			real_total_edgeweight += 1.0f;
	}
	return (g_edgeweight[_u->vertextype][_v->vertextype] / (_u->edgetype_outdegree[_v->vertextype] * real_total_edgeweight));
}


inline float GameTheoty::getResponsecost(int _vertexid, int _clusterid)
{
	// dblp_8w :   9.0741f
	// flickr_9w:  37.4126f
	//m_cn = 9.0741f;    // �����Ҫ�淶�������Ļ����ڴ˴���Ϊ��ֵ

	// 1. assignment cost = ppr; assignment cost only
	// -----------------------------------------
	//float ac = 0.0f;
	//float sc = 1.0f;
	//if (m_GlobalTable[_vertexid][3 + 3 * _clusterid] > 0)
	//	ac = m_cn * g_gamma * m_GlobalTable[_vertexid][3 + 3 * _clusterid + 1] / m_GlobalTable[_vertexid][3 + 3 * _clusterid];
	//float cost_c = sc - ac;
	// -----------------------------------------

	// 2. assignment cost = ppr; assignment cost & social cost
	// -----------------------------------------

	//if (m_GlobalTable[_vertexid][3 + 3 * _clusterid] < 0)
	//	cout << "================" << m_GlobalTable[_vertexid][3 + 3 * _clusterid] << endl;

	float ac = 0.0f;
	if (m_GlobalTable[_vertexid][3 + 3 * _clusterid] > 0)
		ac = m_cn * g_gamma * m_GlobalTable[_vertexid][3 + 3 * _clusterid + 1] / m_GlobalTable[_vertexid][3 + 3 * _clusterid];
	float sc = m_GlobalTable[_vertexid][3 + 3 * _clusterid + 2];
	float cost_c = sc - ac;
	// -----------------------------------------

	// 3. assignment cost = entropy; assignment cost & social cost
	// -----------------------------------------
	//float sc = m_GlobalTable[_vertexid][3 + 2 * _clusterid + 1];
	//float ac = m_cn * g_gamma * m_GlobalTable[_vertexid][3 + 2 * _clusterid];
	//float cost_c = sc + ac;
	// -----------------------------------------

	//if(ac > 0)
	//	cout << ac << "\t" << sc << "\t" << m_GlobalTable[_vertexid][3 + 3 * _clusterid] << endl;
	
	return cost_c;
}


//inline float GameTheoty::getClusterEntropy(set<int> & _cluster)
//{
//	float entropy = 0.0f;
//	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
//	{
//		entropy += getEntropy(_cluster, STRUCTURE, i);
//	}
//	return (entropy / ATTRIBUTE_NUM);
//}


//inline float GameTheoty::AET_getClusterEntropy(int _vertexid, int _clusterid)
//{
//	float total_entropy = 0.0f;
//	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
//	{
//		int total = 0;    // ͳ����������
//		for (auto & pair : AET[_vertexid][_clusterid][i - 1])       // �˴�������Ԥ��
//		{
//			total += pair.second;
//		}
//		 
//		float entropy = 0.0f;
//		for (auto & pair : AET[_vertexid][_clusterid][i - 1])
//		{
//			float prob = pair.second * 1.0f / total;
//			entropy += -1 * prob * log2(prob);
//		}
//
//		total_entropy += entropy;
//	}
//	return (total_entropy / ATTRIBUTE_NUM);
//}


void GameTheoty::gameTheory_ReservePush()
{
	m_pprDistances.clear();  // ��ʼ��

	for (int targetID = g_cluster_startid; targetID <= g_cluster_endid; targetID++)
	{
		vector<float> p(g_vertexnum, 0.0);
		vector<float> r(g_vertexnum, 0.0);

		set<int> pushback_queue;      // ��Ŵ�pushback�Ľڵ�

		r[targetID] = 1.0f;   // target point
		pushback_queue.insert(targetID);

		while (pushback_queue.size() > 0)
		{
			int uID = *pushback_queue.begin();
			pushback_queue.erase(pushback_queue.begin());

			Vertex * u = g_vertices[uID];   // ��ȡ��pushback�ڵ���Ϣ

			p[uID] += g_alpha * r[uID];     // estimated value
			g_pushbackcount++;

			//����u�ܹ�����ĵ㣨reverse push��
			for (auto & wID : u->neighborvertex)
			{
				Vertex * w = g_vertices[wID];
				r[wID] += (1 - g_alpha) * r[uID] * getTransprob(w, u);    // residual value

				if (r[wID] > g_epsilon)
				{
					pushback_queue.insert(wID);
				}
			}

			r[uID] = 0;
		}

		// ��ȡ����
		map<int, float> m_map;
		for (int i = g_cluster_startid; i <= g_cluster_endid; i++)
		{
			if (p[i] > g_epsilon)
			{
				m_map.insert(pair<int, float>(i, p[i]));
			}
		}
		m_pprDistances.insert(pair<int, map<int, float>>(targetID, m_map));
	}
}


void GameTheoty::buildGlobalTable()
{
	// == Initialize
	m_GlobalTable.clear();  
	swap(m_happy_queue, stack<int>());  // ��ն���  
	cost_queue.clear();   

	// == Build glable table
	for (auto v_id : m_valid_cluster_points)
	{
		m_GlobalTable.insert(pair<int, vector<float>>(v_id, vector<float>(3 + 3 * m_clusters.size(), 0)));
	}

	for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
	{
		for (auto vertexid : m_clusters[clusterid])  
		{
			// ��ʼ����ͷ
			m_GlobalTable[vertexid][0] = (float)clusterid;      // ʵ�����                 

			// ����maxSC
			int valid_neighbor_num = 0;
			Vertex * v = g_vertices[vertexid];
			for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
			{
				int f = v->neighborvertex[i];     // f is a friend of v
				if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())    // �ھӵ���ֻѡ�� valid point
				{
					valid_neighbor_num++;
				}
			}
			m_GlobalTable[vertexid][2] = (float)valid_neighbor_num;    // �ڵ�Ķ�
			float maxSC = (1 - g_gamma) * 0.5f * valid_neighbor_num;   // ���߶���ͬ���͵�����ڵ㣬����Ȩ��Ϊ1��������Ҫ���д���
			for (int c_id = 0; c_id < m_clusters.size(); c_id++)       // ��ÿ����
			{
				m_GlobalTable[vertexid][3 + 3 * c_id + 2] = maxSC;
			}
		}
	}

	// == Initialize happy queue
	for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
	{
		for (auto vertexid : m_clusters[clusterid]) 
		{
			// social cost
			Vertex * v = g_vertices[vertexid];
			for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
			{
				int f = v->neighborvertex[i];  // f is friend of v
				if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())  // �ھӵ���ֻѡ�� valid point
				{
					auto iter = m_GlobalTable.find(f);
					int f_clusterid = (int)round(iter->second[0]);   // ��ȡf���ڵ����
					m_GlobalTable[vertexid][3 + 3 * f_clusterid + 2] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];  // �޸�social cost
				}
			}

			// assignment cost
			for (auto & dis_map : m_pprDistances[vertexid])
			{
				int dis_id = dis_map.first;

				if (dis_id == vertexid)  // ���뼯���а�������ȥ��
					continue;

				if (m_valid_cluster_points.find(dis_id) != m_valid_cluster_points.end())
				{
					int dis_clusterid = (int)round(m_GlobalTable[dis_id][0]);               // ���Ҹõ����ڵ����
					m_GlobalTable[vertexid][3 + 3 * dis_clusterid] += 1;                    // num
					m_GlobalTable[vertexid][3 + 3 * dis_clusterid + 1] += dis_map.second;   // sum
				}
			}

			// �洢���������
			set<CostNode> ss;
			for (int c_id = 0; c_id < m_clusters.size(); c_id++)
			{
				ss.insert(CostNode(c_id, getResponsecost(vertexid, c_id)));
			}

			cost_queue.insert(pair<int, set<CostNode>>(vertexid, ss));

			m_GlobalTable[vertexid][1] = (float)(ss.begin()->s_clusterid);       // �洢��С�������

			// �����ǰ������С�������ͬ������Ҫ���е����� ����Ҫ�����ĵ����happy queue
			if ((int)round(m_GlobalTable[vertexid][0]) != (int)round(m_GlobalTable[vertexid][1]))
			{
				m_happy_queue.push(vertexid);
			}
		}
	}

	// == Compute normalization factor
	if (m_cn_flag)
	{
		int sum_degree = 0;
		float sum_max_ppr_dist = 0.0f;
		int ccount = 0;

		for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
		{
			for (auto vertexid : m_clusters[clusterid])  // �������нڵ�
			{
				float max_ac = 1e8f;    // maximum ppr distance 
				for (int c_id = 0; c_id < m_clusters.size(); c_id++)  // ��ÿ����
				{
					if (m_GlobalTable[vertexid][3 + 3 * c_id] > 0)
					{
						float aacc = m_GlobalTable[vertexid][3 + 3 * c_id + 1] / m_GlobalTable[vertexid][3 + 3 * c_id];
						max_ac = min(max_ac, aacc);
					}
				}

				if ((int)round(m_GlobalTable[vertexid][2]) != 0 && max_ac > 0)
				{
					ccount++;                                                 // size of valid points
					sum_degree += (int)round(m_GlobalTable[vertexid][2]);     // degree 
					sum_max_ppr_dist += max_ac;                               // ppr
				}
			}
		}

		//float average_degree = (float)sum_degree / (m_valid_cluster_points.size() - ccount);
		float average_degree = (float)sum_degree / ccount;
		float average_weight = 1.0f;          // ���߶���ͬ���͵�����ڵ㣬����Ȩ��Ϊ1��������Ҫ���д���       
		//float average_max_ppr_dist = sum_max_ppr_dist / m_valid_cluster_points.size();
		float average_max_ppr_dist = sum_max_ppr_dist / ccount;
		//m_cn = average_degree * average_weight / (2.0f * sum_max_ppr_dist * (float)sqrt(m_clusters.size()));
		m_cn = average_degree * average_weight / (2.0f * average_max_ppr_dist * (float)sqrt(m_clusters.size()));

		//cout << "average_degree = " << average_degree << endl;
		//cout << "average_max_ppr_dist = " << average_max_ppr_dist << endl;
		//cout << "m_cn = " << m_cn << endl;
		//getchar();

		m_cn_flag = false;
	}
	
}

 
//void GameTheoty::initializeHappyQueue()
//{
//	m_happy_queue.clear();  // ��ʼ��
//	cost_queue.clear();     // ��ʼ��
//
//	for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
//	{
//		for (auto vertexid : m_clusters[clusterid])  // �������нڵ�
//		{
//			// social cost
//			Vertex * v = g_vertices[vertexid];
//			for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
//			{
//				int f = v->neighborvertex[i];  // f is friend of v
//				if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())  // �ھӵ���ֻѡ�� valid point
//				{
//					auto iter = m_GlobalTable.find(f);
//					int f_clusterid = (int)round(iter->second[0]);   // ��ȡf���ڵ����
//					m_GlobalTable[vertexid][3 + 3 * f_clusterid + 2] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];  // �޸�social cost
//				}
//			}
//
//			// assignment cost
//			for (auto & dis_map : m_pprDistances[vertexid])
//			{
//				int dis_id = dis_map.first;
//
//				if (dis_id == vertexid)  // ���뼯���а�������ȥ��
//					continue;
//
//				if (m_valid_cluster_points.find(dis_id) != m_valid_cluster_points.end())
//				{
//					int dis_clusterid = (int)round(m_GlobalTable[dis_id][0]);               // ���Ҹõ����ڵ����
//					m_GlobalTable[vertexid][3 + 3 * dis_clusterid] += 1;                    // num
//					m_GlobalTable[vertexid][3 + 3 * dis_clusterid + 1] += dis_map.second;   // sum
//				}
//			}
//
//			// �洢���������
//			set<CostNode> ss;
//			for (int c_id = 0; c_id < m_clusters.size(); c_id++)  
//			{
//				ss.insert(CostNode(c_id, getResponsecost(vertexid, c_id)));
//			}
//			
//			cost_queue.insert(pair<int, set<CostNode>>(vertexid, ss));
//				
//			m_GlobalTable[vertexid][1] = (float)(ss.begin()->s_clusterid);       // ��С�������
//
//			// �����ǰ������С�������ͬ������Ҫ���е����� ����Ҫ�����ĵ����happy queue
//			if ((int)round(m_GlobalTable[vertexid][0]) != (int)round(m_GlobalTable[vertexid][1]))
//			{
//				m_happy_queue.insert(vertexid);
//				//m_happy_queue.insert(DegreenNode(vertexid, (int)round(m_GlobalTable[vertexid][2])));
//			}
//		}
//	}
//}


//// ������С���۸��º��жϴ���������������
//void GameTheoty::bestResponseDynamics()
//{
//	m_updatetimes = 0; 
//	while (!m_happy_queue.empty())
//	{
//		// ��ȡ��Ҫ�����Ľڵ�
//		int response_vertexid = *m_happy_queue.begin();
//		m_happy_queue.erase(m_happy_queue.begin());
//
//		m_updatetimes++;  // ͳ�Ƹ��µĵ�Ĵ���
//
//		// ��������������۸���
//		int currentclusterid = (int)round(m_GlobalTable[response_vertexid][0]);
//		int nextclusterid = (int)round(m_GlobalTable[response_vertexid][1]);
//
//		if (currentclusterid == nextclusterid)      // �ڱ�����۽�һ����С�����
//			continue;
//
//		m_GlobalTable[response_vertexid][0] = (float)nextclusterid;  // �������
//		
//		// ���ݵ����ı仯�����������е�� assignment cost
//		for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
//		{
//			for (auto vertexid : m_clusters[clusterid])  // �������нڵ�
//			{
//				if (vertexid == response_vertexid)  // vertexid����� assignment cost ����
//					continue;
//
//				/*
//					response_vertexid����, ����vertexid��currentclusterid����assignment cost���ܷ����仯
//					���vertexid��response_vertexid֮���ppr score > 0, �������assignment cost�ı仯, ���е���;
//					����, �����е���
//				*/
//				// �������
//				if (m_pprDistances[vertexid].find(response_vertexid) != m_pprDistances[vertexid].end())   // vertexid �� response_vertexid ֮��� ppr > 0
//				{
//					m_GlobalTable[vertexid][3 + 3 * currentclusterid] -= 1;
//					m_GlobalTable[vertexid][3 + 3 * currentclusterid + 1] -= m_pprDistances[vertexid][response_vertexid];
//				}
//				float cost_old = getResponsecost(vertexid, currentclusterid);   // ** �������
//
//				// ��������
//				if (m_pprDistances[vertexid].find(response_vertexid) != m_pprDistances[vertexid].end())
//				{
//					m_GlobalTable[vertexid][3 + 3 * nextclusterid] += 1;
//					m_GlobalTable[vertexid][3 + 3 * nextclusterid + 1] += m_pprDistances[vertexid][response_vertexid];
//				}
//				float cost_new = getResponsecost(vertexid, nextclusterid);      // ** �������
//
//				// �������Ŵ���
//				float cost_c = min(cost_old, cost_new);
//				if (cost_c < m_GlobalTable[vertexid][2])
//				{
//					m_GlobalTable[vertexid][2] = cost_c;
//					m_GlobalTable[vertexid][1] = (cost_old < cost_new) ? (float)currentclusterid : (float)nextclusterid;
//					m_happy_queue.insert(vertexid);
//				}
//				
//			}
//		}
//
//		// �����ھӽڵ���� social cost
//		Vertex * v = g_vertices[response_vertexid];
//		for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
//		{
//			int f = v->neighborvertex[i];  // f is friend of v
//			if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())  // �ھӵ���ֻѡ�� valid point
//			{
//				// ���࣬�˴�����ֻ�����󣬲����и�С�Ľ�
//				m_GlobalTable[f][3 + 3 * currentclusterid + 2] += (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
//
//				// ����
//				m_GlobalTable[f][3 + 3 * nextclusterid + 2] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
//
//				// ** �������
//				float cost_c = getResponsecost(f, nextclusterid);
//				if (cost_c < m_GlobalTable[f][2])
//				{
//					m_GlobalTable[f][2] = cost_c;
//					m_GlobalTable[f][1] = (float)nextclusterid;
//					m_happy_queue.insert(f);
//				}
//			}
//		}
//	}
//}


void GameTheoty::bestResponseDynamics()
{
	m_updatetimes = 0;    // ��ʼ��

	//clock_t t_start, t_end;
	//clock_t c_start, c_end;
	//int c_total = 0;

	//t_start = clock();

	while (!m_happy_queue.empty())
	{
		// ��ȡ��Ҫ�����Ľڵ�
		int response_vertexid = m_happy_queue.top();
		m_happy_queue.pop();
		//DegreenNode dn = *m_happy_queue.begin();
		//int response_vertexid = dn.s_vertexid;

		m_updatetimes++;  // ͳ�Ƹ��µĵ�Ĵ���
		//cout << "m_updatetimes = " << m_updatetimes << endl;

		//cout << response_vertexid << "\t";

		// ��������������۸���
		int currentclusterid = (int)round(m_GlobalTable[response_vertexid][0]);
		int nextclusterid = (int)round(m_GlobalTable[response_vertexid][1]);

		if (currentclusterid == nextclusterid)      // �ڱ�����۽�һ����С�����
			continue;

		m_GlobalTable[response_vertexid][0] = m_GlobalTable[response_vertexid][1];  // �������

		/*���еĽڵ�Ĵ������ҽ���currentclusterid��nextclusterid�����仯*/

		// adjust social cost
		Vertex * v = g_vertices[response_vertexid];
		for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
		{
			int f = v->neighborvertex[i];  // f a is friend of v
			if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())  // f is a valid point
			{
				// old cluster
				if ((int)round(m_GlobalTable[f][0]) == currentclusterid)  
				{
					// ���� response_vertexid �� social cost
					m_GlobalTable[response_vertexid][3 + 3 * currentclusterid + 2] += (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
					// �����ھӽڵ�� social cost
					m_GlobalTable[f][3 + 3 * currentclusterid + 2] += (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
				}

				// new cluster
				if ((int)round(m_GlobalTable[f][0]) == nextclusterid)
				{
					// ���� response_vertexid �� social cost
					m_GlobalTable[response_vertexid][3 + 3 * nextclusterid + 2] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
					// �����ھӽڵ�� social cost
					m_GlobalTable[f][3 + 3 * nextclusterid + 2] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
				}	
			}
		}

		// adjust assignment cost
		for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
		{
			for (auto vertexid : m_clusters[clusterid]) 
			{
				float cur_cost = 1e8;
				float next_cost = 1e8;
				
				if (vertexid == response_vertexid)     // response_vertexid ����� assignment cost ����
				{
					for (auto c_n : cost_queue[vertexid]) 
					{
						if (c_n.s_clusterid == currentclusterid)
							cur_cost = c_n.s_cost;
						if (c_n.s_clusterid == nextclusterid)
							next_cost = c_n.s_cost;

						if (cur_cost < 1e8 && next_cost < 1e8)
							break;
					}

					// old cluster
					cost_queue[vertexid].erase(cost_queue[vertexid].find(CostNode(currentclusterid, cur_cost)));                 // ɾ���ɵĴ���
					cost_queue[vertexid].insert(CostNode(currentclusterid, getResponsecost(vertexid, currentclusterid)));        // �����µĴ���
					// new cluster
					cost_queue[vertexid].erase(cost_queue[vertexid].find(CostNode(nextclusterid, next_cost)));                 // ɾ���ɵĴ���
					cost_queue[vertexid].insert(CostNode(nextclusterid, getResponsecost(vertexid, nextclusterid)));            // �����µĴ���

					m_GlobalTable[vertexid][1] = (float)(cost_queue[vertexid].begin())->s_clusterid;
					if ((int)round(m_GlobalTable[vertexid][0]) != (int)round(m_GlobalTable[vertexid][1]))  // ��Ҫ����������
					{
						m_happy_queue.push(vertexid);
						//m_happy_queue.insert(DegreenNode(vertexid, (int)round(m_GlobalTable[vertexid][2])));
					}

					continue;
				}
					
				/*
				response_vertexid����, ����vertexid��currentclusterid����assignment cost���ܷ����仯
				���vertexid��response_vertexid֮���ppr score > 0, �������assignment cost�ı仯, ���е���; ����, �����е���
				*/
				
				if (m_pprDistances[vertexid].find(response_vertexid) != m_pprDistances[vertexid].end())   // vertexid �� response_vertexid ֮��� ppr > 0
				{
					for (auto c_n : cost_queue[vertexid])
					{
						if (c_n.s_clusterid == currentclusterid)
							cur_cost = c_n.s_cost;
						if (c_n.s_clusterid == nextclusterid)
							next_cost = c_n.s_cost;

						if (cur_cost < 1e8 && next_cost < 1e8)
							break;
					}

					// old cluster
					m_GlobalTable[vertexid][3 + 3 * currentclusterid] -= 1;
					m_GlobalTable[vertexid][3 + 3 * currentclusterid + 1] -= m_pprDistances[vertexid][response_vertexid];

					cost_queue[vertexid].erase(cost_queue[vertexid].find(CostNode(currentclusterid, cur_cost)));                 // ɾ���ɵĴ���
					cost_queue[vertexid].insert(CostNode(currentclusterid, getResponsecost(vertexid, currentclusterid)));        // �����µĴ���

					// new cluster
					m_GlobalTable[vertexid][3 + 3 * nextclusterid] += 1;
					m_GlobalTable[vertexid][3 + 3 * nextclusterid + 1] += m_pprDistances[vertexid][response_vertexid];

					cost_queue[vertexid].erase(cost_queue[vertexid].find(CostNode(nextclusterid, next_cost)));                 // ɾ���ɵĴ���
					cost_queue[vertexid].insert(CostNode(nextclusterid, getResponsecost(vertexid, nextclusterid)));            // �����µĴ���
				}

				m_GlobalTable[vertexid][1] = (float)(cost_queue[vertexid].begin())->s_clusterid;
				if ((int)round(m_GlobalTable[vertexid][0]) != (int)round(m_GlobalTable[vertexid][1]))  // ��Ҫ����������
				{
					m_happy_queue.push(vertexid);
					//m_happy_queue.insert(DegreenNode(vertexid, (int)round(m_GlobalTable[vertexid][2])));
				}
			}
		}
	}

	//t_end = clock();
}


//void GameTheoty::E_buildGlobalTable()
//{
//	// ��ʼ��
//	m_GlobalTable.clear();
//	AET.clear();
//	for (auto v_id : m_valid_cluster_points)
//	{
//		m_GlobalTable.insert(pair<int, vector<float>>(v_id, vector<float>(3 + 2 * m_clusters.size(), 0)));
//		AET.insert(pair<int, vector<vector<unordered_map<int, int>>>>(v_id, vector<vector<unordered_map<int, int>>>
//			(m_clusters.size(), vector<unordered_map<int, int>>(ATTRIBUTE_NUM, unordered_map<int, int>()))));
//	}
//
//	for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
//	{
//		for (auto vertexid : m_clusters[clusterid])
//		{
//			// ��ʼ����ͷ
//			m_GlobalTable[vertexid][0] = (float)clusterid;      // ʵ�����
//			m_GlobalTable[vertexid][1] = -1.0f;                 // ��С�������
//			m_GlobalTable[vertexid][2] = 1e8;                   // minCost = �����
//
//			// ����maxSC
//			int valid_neighbor_num = 0;
//			Vertex * v = g_vertices[vertexid];
//			for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
//			{
//				int f = v->neighborvertex[i];     // f is a friend of v
//				if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())    // �ھӵ���ֻѡ�� valid point
//				{
//					valid_neighbor_num++;
//				}
//			}
//			float maxSC = (1 - g_gamma) * 0.5f * valid_neighbor_num;   // ���߶���ͬ���͵�����ڵ㣬����Ȩ��Ϊ1��������Ҫ���д���
//
//			for (int c_id = 0; c_id < m_clusters.size(); c_id++)       // ��ÿ����
//			{
//				// ����maxSC
//				m_GlobalTable[vertexid][3 + 2 * c_id + 1] = maxSC;
//				
//				// ���㵱ǰ�ڵ���䵽�����ص���
//				set<int> temp_cluster(m_clusters[c_id]);   // ����cluster
//				if (c_id != clusterid)
//					temp_cluster.insert(vertexid);         // ������ڵ�����������
//
//				// ������
//				//m_GlobalTable[vertexid][3 + 2 * c_id] = getClusterEntropy(temp_cluster);
//			}
//		}
//	}
//}
//
//
//void GameTheoty::AET_buildGlobalTable()
//{
//	// ��ʼ��
//	m_GlobalTable.clear();
//	AET.clear();
//	for (auto v_id : m_valid_cluster_points)
//	{
//		m_GlobalTable.insert(pair<int, vector<float>>(v_id, vector<float>(3 + 2 * m_clusters.size(), 0)));
//		AET.insert(pair<int, vector<vector<unordered_map<int, int>>>>(v_id, vector<vector<unordered_map<int, int>>>
//			(m_clusters.size(), vector<unordered_map<int, int>>(ATTRIBUTE_NUM, unordered_map<int, int>()))));
//	}
//
//	for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
//	{
//		for (auto vertexid : m_clusters[clusterid])
//		{
//			// ��ʼ����ͷ
//			m_GlobalTable[vertexid][0] = (float)clusterid;      // ʵ�����
//			m_GlobalTable[vertexid][1] = -1.0f;                 // ��С�������
//			m_GlobalTable[vertexid][2] = 1e8;                   // minCost = �����
//
//			// ����maxSC
//			int valid_neighbor_num = 0;
//			Vertex * v = g_vertices[vertexid];
//			for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
//			{
//				int f = v->neighborvertex[i];     // f is a friend of v
//				if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())    // �ھӵ���ֻѡ�� valid point
//				{
//					valid_neighbor_num++;
//				}
//			}
//			float maxSC = (1 - g_gamma) * 0.5f * valid_neighbor_num;   // ���߶���ͬ���͵�����ڵ㣬����Ȩ��Ϊ1��������Ҫ���д���
//
//			for (int c_id = 0; c_id < m_clusters.size(); c_id++)       // ��ÿ����
//			{
//				// ����maxSC
//				m_GlobalTable[vertexid][3 + 2 * c_id + 1] = maxSC;
//
//				// ���㵱ǰ�ڵ���䵽�����ص���
//				set<int> temp_cluster(m_clusters[c_id]);   // ����cluster
//				if (c_id != clusterid)
//					temp_cluster.insert(vertexid);         // ������ڵ�����������
//
//				// ����appearances��
//				// ------------------------------------------------------------
//				for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
//				{
//					int type1 = STRUCTURE;
//					int type2 = i;
//
//					//ö�ٴ������ÿһ������ΪԴ��
//					for (int vid : temp_cluster)
//					{
//						Vertex * v = g_vertices[vid];
//
//						if (v->vertextype == type1)             //����õ�����type1
//						{
//							//ö��������ܵ����������
//							for (auto & uid : v->neighborvertex)
//							{
//								Vertex * u = g_vertices[uid];
//
//								if (u->vertextype == type2)      //����յ�������type2
//								{
//									AET[vertexid][c_id][i - 1][u->vertexid] += 1;  // ͳ��appearances
//								}
//							}
//						}
//					}
//				}
//				// ------------------------------------------------------------
//
//				// ������
//				m_GlobalTable[vertexid][3 + 2 * c_id] = AET_getClusterEntropy(vertexid, c_id);
//			}
//		}
//	}
//}
//
//
//void GameTheoty::E_initializeHappyQueue()
//{
//	m_happy_queue.clear();  // ��ʼ��
//
//	for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
//	{
//		for (auto vertexid : m_clusters[clusterid])  // �������нڵ�
//		{
//			// ����ÿ�����
//			float minCost = 1e8;
//			for (int c_id = 0; c_id < m_clusters.size(); c_id++)  // ��ÿ����
//			{
//				// ** �������
//				float cost_c = getResponsecost(vertexid, c_id);
//
//				if (cost_c < minCost)
//				{
//					minCost = cost_c;
//					m_GlobalTable[vertexid][1] = (float)c_id;
//				}
//			}
//
//			// ������Ч���ھӽڵ�
//			Vertex * v = g_vertices[vertexid];
//			for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
//			{
//				int f = v->neighborvertex[i];  // f is friend of v
//				if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())  // �ھӵ���ֻѡ�� valid point
//				{
//					auto iter = m_GlobalTable.find(f);
//					int f_clusterid = (int)round(iter->second[0]);   // ��ȡf���ڵ����
//					m_GlobalTable[vertexid][3 + 2 * f_clusterid + 1] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];  // �޸�social cost
//
//					// ** �������
//					float cost_c = getResponsecost(vertexid, f_clusterid);
//					if (cost_c < minCost)
//					{
//						minCost = cost_c;
//						m_GlobalTable[vertexid][1] = (float)f_clusterid;
//					}
//				}
//			}
//
//			// ���� minCost 
//			m_GlobalTable[vertexid][2] = minCost;
//
//			// �����ǰ������С�������ͬ������Ҫ���е����� ͳ����Ҫ�����ĵ�
//			if (m_GlobalTable[vertexid][0] != m_GlobalTable[vertexid][1])
//			{
//				m_happy_queue.insert(vertexid);
//			}
//		}
//	}
//}
//
//
//void GameTheoty::E_bestResponseDynamics()
//{
//	m_updatetimes = 0;
//	while (!m_happy_queue.empty())
//	{
//		// ��ȡ��Ҫ�����Ľڵ�
//		int response_vertexid = *m_happy_queue.begin();
//		m_happy_queue.erase(m_happy_queue.begin());
//
//		m_updatetimes++;  // ͳ�Ƹ��µĵ�Ĵ���
//
//		// ��������������۸���
//		int currentclusterid = (int)round(m_GlobalTable[response_vertexid][0]);
//		int nextclusterid = (int)round(m_GlobalTable[response_vertexid][1]);
//
//		if (currentclusterid == nextclusterid)      // �ڱ�����۽�һ����С�����
//			continue;
//
//		m_GlobalTable[response_vertexid][0] = (float)nextclusterid;  // �������
//
//		// �Ӿ������Ƴ�response_vertexid����������
//		m_clusters[currentclusterid].erase(m_clusters[currentclusterid].find(response_vertexid));
//		m_clusters[nextclusterid].insert(response_vertexid);
//
//		// ���ݵ����ı仯�����������е�� assignment cost
//		for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
//		{
//			for (auto vertexid : m_clusters[clusterid])  // �������нڵ�
//			{
//				if (vertexid == response_vertexid)  // vertexid����� assignment cost ����
//					continue;
//
//				/*
//					response_vertexid����������֮��������������currentclusterid��nextclusterid���ض���Ҫ���¼���
//				*/
//				// �������, ������֮ǰ��Ҫ�Ƴ�response_vertexid
//				set<int> temp_cluster(m_clusters[currentclusterid]);   // ����cluster
//				if (clusterid != currentclusterid)
//				{
//					temp_cluster.insert(vertexid);  // vertexid�������currentclusterid�У�����
//				}
//				m_GlobalTable[vertexid][3 + 2 * currentclusterid] = getClusterEntropy(temp_cluster);   // ������
//
//				// ��������, ������֮ǰ��Ҫ���response_vertexid
//				set<int> temp_cluster2(m_clusters[nextclusterid]);   // ����cluster
//				if (clusterid != nextclusterid)
//				{
//					temp_cluster2.insert(vertexid);  // vertexid�������nextclusterid�У�����
//				}
//				m_GlobalTable[vertexid][3 + 2 * nextclusterid] = getClusterEntropy(temp_cluster2);   // ������
//
//				// ** �������
//				float cost_old = getResponsecost(vertexid, currentclusterid);
//				float cost_new = getResponsecost(vertexid, nextclusterid);
//				float cost_c = min(cost_old, cost_new);
//				if (cost_c < m_GlobalTable[vertexid][2])
//				{
//					m_GlobalTable[vertexid][2] = cost_c;
//					m_GlobalTable[vertexid][1] = (cost_old < cost_new) ? (float)currentclusterid : (float)nextclusterid;
//					m_happy_queue.insert(vertexid);
//				}
//			}
//		}
//
//		// �����ھӽڵ���� social cost
//		Vertex * v = g_vertices[response_vertexid];
//		for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
//		{
//			int f = v->neighborvertex[i];  // f is friend of v
//			if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())  // �ھӵ���ֻѡ�� valid point
//			{
//				// ����, �˴�����ֻ�����󣬲����и�С�Ľ�
//				m_GlobalTable[f][3 + 2 * currentclusterid + 1] += (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
//
//				// ����
//				m_GlobalTable[f][3 + 2 * nextclusterid + 1] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
//
//				// ** �������
//				float cost_c = getResponsecost(f, nextclusterid);
//				if (cost_c < m_GlobalTable[f][2])
//				{
//					m_GlobalTable[f][2] = cost_c;
//					m_GlobalTable[f][1] = (float)nextclusterid;
//					m_happy_queue.insert(f);
//				}
//			}
//		}
//	}
//}
//
//
//void GameTheoty::AET_bestResponseDynamics()
//{
//	m_updatetimes = 0;
//	while (!m_happy_queue.empty())
//	{
//		// ��ȡ��Ҫ�����Ľڵ�
//		int response_vertexid = *m_happy_queue.begin();
//		m_happy_queue.erase(m_happy_queue.begin());
//
//		m_updatetimes++;  // ͳ�Ƹ��µĵ�Ĵ���
//
//		// ��������������۸���
//		int currentclusterid = (int)round(m_GlobalTable[response_vertexid][0]);
//		int nextclusterid = (int)round(m_GlobalTable[response_vertexid][1]);
//
//		if (currentclusterid == nextclusterid)      // �ڱ�����۽�һ����С�����
//			continue;
//
//		m_GlobalTable[response_vertexid][0] = (float)nextclusterid;  // �������
//
//		Vertex * v = g_vertices[response_vertexid];
//
//		// ���ݵ����ı仯�����������е�� assignment cost
//		for (int clusterid = 0; clusterid < m_clusters.size(); clusterid++)
//		{
//			for (auto vertexid : m_clusters[clusterid])  // �������нڵ�
//			{
//				if (vertexid == response_vertexid)  // vertexid����� assignment cost ����
//					continue;
//
//				/*
//				����: response_vertexid����������֮��������������currentclusterid��nextclusterid���ض���Ҫ���¼���
//				*/
//				for (int ii = v->edgetype_outdegree[STRUCTURE]; ii < v->neighborvertex.size(); ii++)   // ���������ھӵ�
//				{
//					int uid = v->neighborvertex[ii];  // ȡ���������ھӵ�
//					Vertex * u = g_vertices[uid];
//
//					// �������, ��vertexid��Ӧ��currentclusterid�������ɾ��response_vertexid������appearance
//					if (AET[vertexid][currentclusterid][u->vertextype - 1][uid] < 2)  // ֻ��һ��appearance, ɾ�������ֵ��
//						AET[vertexid][currentclusterid][u->vertextype - 1].erase(AET[vertexid][currentclusterid][u->vertextype - 1].find(uid));
//					else
//						AET[vertexid][currentclusterid][u->vertextype - 1][uid] -= 1;
//
//					// ��������, ��vertexid��Ӧ��nextclusterid����������response_vertexid������appearance
//					AET[vertexid][nextclusterid][u->vertextype - 1][uid] += 1;
//				}
//
//				m_GlobalTable[vertexid][3 + 2 * currentclusterid] = AET_getClusterEntropy(vertexid, currentclusterid);  // ����
//				m_GlobalTable[vertexid][3 + 2 * nextclusterid] = AET_getClusterEntropy(vertexid, nextclusterid);        // ����
//
//				// ** �������
//				float cost_old = getResponsecost(vertexid, currentclusterid);
//				float cost_new = getResponsecost(vertexid, nextclusterid);
//				float cost_c = min(cost_old, cost_new);
//				if (cost_c < m_GlobalTable[vertexid][2])
//				{
//					m_GlobalTable[vertexid][2] = cost_c;
//					m_GlobalTable[vertexid][1] = (cost_old < cost_new) ? (float)currentclusterid : (float)nextclusterid;
//					m_happy_queue.insert(vertexid);
//				}
//			}
//		}
//
//		// �����ھӽڵ���� social cost
//		for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
//		{
//			int f = v->neighborvertex[i];  // f is friend of v
//			if (m_valid_cluster_points.find(f) != m_valid_cluster_points.end())  // �ھӵ���ֻѡ�� valid point
//			{
//				// ����, �˴�����ֻ�����󣬲����и�С�Ľ�
//				m_GlobalTable[f][3 + 2 * currentclusterid + 1] += (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
//
//				// ����
//				m_GlobalTable[f][3 + 2 * nextclusterid + 1] -= (1 - g_gamma) * 0.5f * g_edgeweight[STRUCTURE][STRUCTURE];
//				// ** �������
//				float cost_c = getResponsecost(f, nextclusterid);
//				if (cost_c < m_GlobalTable[f][2])
//				{
//					m_GlobalTable[f][2] = cost_c;
//					m_GlobalTable[f][1] = (float)nextclusterid;
//					m_happy_queue.insert(f);
//				}
//			}
//		}
//	}
//}


void GameTheoty::gatherClusterResult()
{
	vector<set<int>> old_clusters(m_clusters);
	vector<set<int>> new_clusters(m_clusters.size());
	m_clusters.clear();

	for (int clusterid = 0; clusterid < old_clusters.size(); clusterid++)
	{
		for (auto vertexid : old_clusters[clusterid])  // �������нڵ�
		{
			// ����ͳ�Ƹ�����ľ�����
			new_clusters[(int)round(m_GlobalTable[vertexid][0])].insert(vertexid);
		}
	}

	// �Ծ����Ľ������ɸѡ
	for (auto iter = new_clusters.begin(); iter != new_clusters.end();)
	{
		if ((*iter).empty())
		{
			iter = new_clusters.erase(iter);
		}
		else
			iter++;
	}

	m_clusters = new_clusters;
}


void GameTheoty::gameTheoryModulation()
{
	// 1. assignment cost == ppr
	// --------------------------------------------------
	buildGlobalTable();        // ���� Global Table, ���� cN
	//initializeHappyQueue();    // ��ʼ�� happy_queue
	bestResponseDynamics();    // best-response dynamics
	// --------------------------------------------------

	// 2. assignment cost = entropy
	// --------------------------------------------------
	//E_buildGlobalTable();        // ���� Global Table, ���� cN
	//E_initializeHappyQueue();    // ��ʼ�� happy_queue
	//E_bestResponseDynamics();    // best-response dynamics
	// --------------------------------------------------

	// 3. assignment cost = entropy(ʹ��Appearance Entropy Table)
	// --------------------------------------------------
	//AET_buildGlobalTable();        // ���� Global Table, ���� cN
	//E_initializeHappyQueue();      // ��ʼ�� happy_queue(�뷽��2��ͬ)
	//AET_bestResponseDynamics();    // best-response dynamics(AET�Ż�����)
	// --------------------------------------------------

	// ����ͳ�ƾ�����
	gatherClusterResult();
}


void GameTheoty::execute()
{
	string result_output = g_resultpath + "result_" + to_string(g_datasetid) + "_" + to_string(g_delta)
		+ "_" + to_string(m_minPts) + "_" + to_string(g_gamma) + "_" + to_string(g_epsilon) + ".txt";
	string cluster_output = g_resultpath + "cluster_result_" + to_string(g_datasetid) + "_" + to_string(g_schemeid) + "_"
		+ to_string(g_delta) + "_" + to_string(m_minPts) + "_" + to_string(g_gamma) + "_" + to_string(g_epsilon) + ".txt";

	//string cluster_output = "F:\\WorkSpace\\GraphClustering\\GC_ApproximateReversePush\\x64\\Release\\2.txt";

	ofstream log_ou;
	log_ou.open(result_output, ios::app);

	float diff = 1e10;
	int iterTimes = 0;
	long long total_pushback_times = 0;
	int total_Update_times = 0;
	clock_t total_start, total_end;

	log_ou << endl << "********************************" << endl << "GameTheory Approach: " << endl;

	readGraph();
	g_vertexnum = (int)g_vertices.size();

	m_cn_flag = true;   // ֻ����һ��cn

	// ����ʱ���ظ�20��ȡƽ��ֵ
	int runTimes = 1;  // default = 20
	long long total_running_time = 0;
	for (int i = 0; i < runTimes; i++)
	{
		// ��ʼ��
		diff = 1e10;
		iterTimes = 0;
		total_pushback_times = 0;
		total_Update_times = 0;

		total_start = clock();
		while (diff > 1e-2)
		{
			iterTimes++;
			cout << "iterTimes = " << iterTimes << endl;
			if (iterTimes > 30)
			{
				log_ou << "Can not converge!" << endl;
				return;
			}

			// compute ppr score
			g_pushbackcount = 0;
			gameTheory_ReservePush();
			total_pushback_times += g_pushbackcount;
			// ----- �Գƻ� -----
			SD_PPRSymmetrization();
			// ----- dbscan -----
			dbscan();
			// ========================== Game Theory =============================
			gameTheoryModulation();
			total_Update_times += m_updatetimes;
			// ========================== Game Theory =============================

			// ----- Ȩ�ظ��� -----
			diff = weightUpdate_Entropy();        // �أ�ֻʵ�ְ������жϣ�
			//diff = weightUpdate_Vote();         // ͶƱ��ֻʵ�ְ������жϣ�
		}
		total_end = clock();

		total_running_time += total_end - total_start;
	}

	// ====================================== ͳ�ƽ�� ======================================
	log_ou << "Total runningtime: " << total_running_time / runTimes << endl;
	log_ou << "Iteration Times: " << iterTimes << endl;
	log_ou << "Total Pushback Times: " << total_pushback_times << endl;
	log_ou << "Total Update Times: " << total_Update_times << endl;
	log_ou << "Current weight: ";
	for (int i = ATTRIBUTE_1; i <= ATTRIBUTE_NUM; i++)
	{
		log_ou << g_edgeweight[STRUCTURE][i] << "\t";
	}
	log_ou << endl;

	// ----- ������� -----
	// ������Ŀ
	log_ou << "Cluster_Amount: " << m_clusters.size() << endl;
	log_ou << "Valid_Cluster_points: " << m_valid_cluster_points.size() << endl;
	// density
	log_ou << "Cluster_Density: " << clusterEvaluate_Density() << endl;
	// entropy
	log_ou << "Cluster_Entropy1: " << clusterEvaluate_Entropy1() << endl;
	// entropy2
	//log_ou << "Cluster_Entropy2: " << clusterEvaluate_Entropy2() << endl;
	// within cluster average distance
	//log_ou << "Cluster_WithinClusterAveDistance: " << clusterEvaluate_WithinClusterAveDistance() << endl;

	log_ou.close();

	// �洢������
	storeClusterResult(cluster_output);
}