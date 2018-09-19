#include "baseframework.h"
#include "MyGlobalParameters.h"
#include <algorithm>

using namespace std;

BaseFramework::BaseFramework(const char * argv[])
{
	m_stoc_file = (string)argv[1];
	m_inputpath = g_inputpath + (string)argv[1];   // ���ݼ��ļ�
	// ---------------------------------------------------------------------
	g_epsilon = (float)atof(argv[2]);  // MAIN ����ʱ���
	g_delta = (float)atof(argv[3]);    // MAIN ����ʱ���
	m_minPts = atoi(argv[4]);          // MAIN ����ʱ���
	//m_minPts = g_minPts;             // PRA  ����ʱ���
	// ---------------------------------------------------------------------
	g_datasetid = atoi(argv[5]); 
	g_preflag = atoi(argv[7]);
	g_gamma = (float)atof(argv[8]);

	// ѡ�����ݼ�
	switch (g_datasetid)   
	{
	case 1:{    // dblp_8w
		ATTRIBUTE_NUM = 3;
		
		// ָ�����Ա�Ȩ�غ�
		g_attribute_total_edgeweight = 3.0f;     // weight sum = 2, [3], 4, 5, 6

		g_total_edgeweight = 1.0f + g_attribute_total_edgeweight;
		for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
		{
			g_base_weight[i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			g_former_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_former_edgeweight[i][STRUCTURE] = g_total_edgeweight;
			g_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_edgeweight[i][STRUCTURE] = g_total_edgeweight;
		}
		
		// �������ݼ���g_epsilon���������ڴ�ӳ���ļ��Ĵ�С
		// g_epsilon = 1e-3
		// ---------------------------------------------
		a_mmf_buffer_size = 16 * 1024 * 1024;
		a_mmfsizelow = a_mmf_buffer_size;

		p_mmf_buffer_size = 1024 * 1024;
		p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------

		// g_epsilon = 1e-4
		// ---------------------------------------------
		//a_mmf_buffer_size = 128 * 1024 * 1024;
		//a_mmfsizelow = a_mmf_buffer_size;

		//p_mmf_buffer_size = 4 * 1024 * 1024;
		//p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------

		// g_epsilon = 1e-5
		// ---------------------------------------------
		//a_mmf_buffer_size = 512 * 1024 * 1024;
		//a_mmfsizelow = a_mmf_buffer_size;

		//p_mmf_buffer_size = 8 * 1024 * 1024;
		//p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------
	}
		   break;
	case 2:{    // flicker_9w
		ATTRIBUTE_NUM = 2;

		// ָ�����Ա�Ȩ�غ�
		g_attribute_total_edgeweight = 2.0f;     // weight sum = [2], 3, 4, 5, 6

		g_total_edgeweight = 1.0f + g_attribute_total_edgeweight;
		for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
		{
			g_base_weight[i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			g_former_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_former_edgeweight[i][STRUCTURE] = g_total_edgeweight;
			g_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_edgeweight[i][STRUCTURE] = g_total_edgeweight;
		}

		// �������ݼ���g_epsilon���������ڴ�ӳ���ļ��Ĵ�С
		// g_epsilon = 1e-3
		// ---------------------------------------------
		a_mmf_buffer_size = 32 * 1024 * 1024; 
		a_mmfsizelow = a_mmf_buffer_size;

		p_mmf_buffer_size = 2 * 1024 * 1024;
		p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------

		// g_epsilon = 1e-4
		// ---------------------------------------------
		//a_mmf_buffer_size = 128 * 1024 * 1024;
		//a_mmfsizelow = a_mmf_buffer_size;

		//p_mmf_buffer_size = 4 * 1024 * 1024;
		//p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------

		// g_epsilon = 1e-5
		// ---------------------------------------------
		//a_mmf_buffer_size = 640 * 1024 * 1024;
		//a_mmfsizelow = a_mmf_buffer_size;

		//p_mmf_buffer_size = 8 * 1024 * 1024;
		//p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------
	}
		   break;
	case 3:{     // dblp_3k
		ATTRIBUTE_NUM = 3;

		// ָ�����Ա�Ȩ�غ�
		g_attribute_total_edgeweight = 3.0f;

		g_total_edgeweight = 1.0f + g_attribute_total_edgeweight;
		for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
		{
			g_base_weight[i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			g_former_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_former_edgeweight[i][STRUCTURE] = g_total_edgeweight;
			g_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_edgeweight[i][STRUCTURE] = g_total_edgeweight;
		}

		// �������ݼ���g_epsilon���������ڴ�ӳ���ļ��Ĵ�С
		// g_epsilon = 1e-3
		// ---------------------------------------------
		a_mmf_buffer_size = 256 * 1024;
		a_mmfsizelow = a_mmf_buffer_size;

		p_mmf_buffer_size = 2 * 1024;
		p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------
	}
		   break;
	case 4:{	     // flicker_4k
		ATTRIBUTE_NUM = 2;

		// ָ�����Ա�Ȩ�غ�
		g_attribute_total_edgeweight = 2.0f;

		g_total_edgeweight = 1.0f + g_attribute_total_edgeweight;
		for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
		{
			g_base_weight[i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			g_former_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_former_edgeweight[i][STRUCTURE] = g_total_edgeweight;
			g_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_edgeweight[i][STRUCTURE] = g_total_edgeweight;
		}

		// �������ݼ���g_epsilon���������ڴ�ӳ���ļ��Ĵ�С
		// g_epsilon = 1e-3
		// ---------------------------------------------
		a_mmf_buffer_size = 256 * 1024;
		a_mmfsizelow = a_mmf_buffer_size;

		p_mmf_buffer_size = 32 * 1024;
		p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------
	}
		   break;

	case 5:{	// case study - dblp_cs
		ATTRIBUTE_NUM = 3;

		// ָ�����Ա�Ȩ�غ�
		g_attribute_total_edgeweight = 3.0f;     // weight sum = 2, [3], 4, 5, 6

		g_total_edgeweight = 1.0f + g_attribute_total_edgeweight;
		for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
		{
			g_base_weight[i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			g_former_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_former_edgeweight[i][STRUCTURE] = g_total_edgeweight;
			g_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_edgeweight[i][STRUCTURE] = g_total_edgeweight;
		}

		// �������ݼ���g_epsilon���������ڴ�ӳ���ļ��Ĵ�С
		// g_epsilon = 1e-3
		// ---------------------------------------------
		a_mmf_buffer_size = 256 * 1024;
		a_mmfsizelow = a_mmf_buffer_size;

		p_mmf_buffer_size = 2 * 1024;
		p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------
	}
		   break;

	case 6:{      // politics
		ATTRIBUTE_NUM = 2;

		// ָ�����Ա�Ȩ�غ�
		g_attribute_total_edgeweight = 2.0f;

		g_total_edgeweight = 1.0f + g_attribute_total_edgeweight;
		for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
		{
			g_base_weight[i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			g_former_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_former_edgeweight[i][STRUCTURE] = g_total_edgeweight;
			g_edgeweight[STRUCTURE][i] = g_attribute_total_edgeweight / ATTRIBUTE_NUM;
			//g_edgeweight[i][STRUCTURE] = g_total_edgeweight;
		}

		// �������ݼ���g_epsilon���������ڴ�ӳ���ļ��Ĵ�С
		// g_epsilon = 1e-3
		// ---------------------------------------------
		a_mmf_buffer_size = 4 * 1024 * 1024;
		a_mmfsizelow = a_mmf_buffer_size;

		p_mmf_buffer_size = 1024 * 1024;
		p_mmfsizelow = p_mmf_buffer_size;
		// ---------------------------------------------
	}
		   break;

	default:
		cout << "Get the wrong dataset id!" << endl;
		break;
	}

	g_cluster_startid = 0;
	g_cluster_endid = 0;
}


void BaseFramework::readGraph()
{
	ifstream in;
	in.open(m_inputpath, ios::in);

	int v_id = 0;
	bool c_flag = true;     // ��Ǿ���ڵ㿪ʼ���
	int vertexid;
	int vertextype;
	string vertexname;
	int et_outdegree;
	int neighbor;
	int totalneighbors;

	// experiment evalution: ͳ�����ݼ��������ͽڵ�ͱߵ���Ŀ
	// ---------------------------------------------
	unordered_map<int, int> points_amount;
	unordered_map<int, int> edges_amount;
	bool count_switch = false;
	// ---------------------------------------------

	g_cluster_startid = 0;
	while (in >> vertexid >> vertexname >> vertextype)
	{
		// ͳ�ƾ��࿪ʼ�ͽ����Ľڵ���
		if (vertextype == g_clustertype)
		{
			if (c_flag)
			{
				g_cluster_startid = v_id;
				c_flag = false;
			}
			g_cluster_endid = v_id;
		}

		Vertex * v = new Vertex(vertexid, vertextype, vertexname);

		in >> totalneighbors;

		points_amount[vertextype]++;               // count points amount

		for (int i = 0; i < ATTRIBUTE_NUM + 1; i++)
		{
			in >> et_outdegree;
			v->edgetype_outdegree.push_back(et_outdegree);

			if (vertextype == 0)
				edges_amount[i] += et_outdegree;   // count edges amount
		}

		for (int i = 0; i < totalneighbors; i++)
		{
			in >> neighbor;
			v->neighborvertex.push_back(neighbor);
		}

		g_vertices.push_back(v);
		v_id++;
	}
	in.close();

	// experiment evalution: ͳ�����ݼ��������ͽڵ�ͱߵ���Ŀ
	// ---------------------------------------------
	cout << "Vertex Statistics:" << endl;
	for (auto pa_iter = points_amount.begin(); pa_iter != points_amount.end(); pa_iter++)
	{
		cout << pa_iter->first << "\t" << pa_iter->second << endl;
	}
	cout << endl;
	cout << "Edges Statistics:" << endl;
	for (auto pe_iter = edges_amount.begin(); pe_iter != edges_amount.end(); pe_iter++)
	{
		cout << pe_iter->first << "\t" << pe_iter->second << endl;
	}
	// ---------------------------------------------
}


// ���������ĶԳƻ���ʽ
void BaseFramework::PPRSymmetrization()
{
	for (auto map_itr = g_dbscanneighborsets.begin(); map_itr != g_dbscanneighborsets.end(); map_itr++)
	{
		int target_id = map_itr->first;
		for (auto n_itr = map_itr->second.begin(); n_itr != map_itr->second.end();)
		{
			int check_id = *n_itr;
			if (g_dbscanneighborsets[check_id].find(target_id) == g_dbscanneighborsets[check_id].end())
			{
				n_itr = g_dbscanneighborsets[target_id].erase(n_itr);
			}
			else
				n_itr++;
		}
	}
}


// �������ĶԳƻ���ʽ
void BaseFramework::SD_PPRSymmetrization()
{
	g_dbscanneighborsets.clear();  // ���

	for (auto map_itr = m_pprDistances.begin(); map_itr != m_pprDistances.end(); map_itr++)
	{
		int target_id = map_itr->first;
		set<int> n_set;
		for (auto n_itr = map_itr->second.begin(); n_itr != map_itr->second.end();)
		{
			int check_id = n_itr->first;
			if (m_pprDistances[check_id].find(target_id) == m_pprDistances[check_id].end())  // ��Ӧ�㲻���ڣ��Ƴ�
			{
				n_itr = m_pprDistances[target_id].erase(n_itr);  
			}
			else   // ��Ӧ�����,ȡ��С��ֵ
			{
				float minppr = min(m_pprDistances[target_id][check_id], m_pprDistances[check_id][target_id]);
				m_pprDistances[target_id][check_id] = minppr;
				m_pprDistances[check_id][target_id] = minppr;

				// �ж��ھӵ�
				if (minppr > g_delta)
				{
					n_set.insert(check_id);
				}
				n_itr++;
			}
		}
		g_dbscanneighborsets.insert(pair<int, set<int>>(target_id, n_set));
	}
}


void BaseFramework::dbscan()
{
	unordered_map<int, bool> visited;    // ���dbscan������ĳ������û�б����ʹ�
	unordered_map<int, int> belonging;   // belonging[v]��ʾ��v�����ĸ���
	
	m_clusters.clear();
	m_valid_cluster_points.clear();  // ��ʼ��

	for (auto itr = g_dbscanneighborsets.begin(); itr != g_dbscanneighborsets.end(); itr++)
	{
		visited.insert(pair<int, bool>(itr->first, false));
		belonging.insert(pair<int, int>(itr->first, -1));
	}

	int clusterid = 0;   // �صı��
	for (auto itr = g_dbscanneighborsets.begin(); itr != g_dbscanneighborsets.end(); itr++)
	{
		if (visited[itr->first] == true)   // �ýڵ��Ѿ����ʹ�
			continue;

		// �ж��Ƿ��Ǻ��ĵ㣨�ھ�������ʱ��������
		if (g_dbscanneighborsets[itr->first].size() < m_minPts)    // ���Ǻ��ĵ� 
		{
			visited[itr->first] = true;
		}
		else    // �Ǻ��ĵ�
		{
			set<int> cluster;   // �洢�´�
			unordered_set<int> neighbors;

			neighbors.insert(itr->first);

			//expand cluster
			while (!neighbors.empty())
			{
				int p = *neighbors.begin();
				neighbors.erase(neighbors.begin());

				if (!visited[p])
				{
					visited[p] = true;
					if (g_dbscanneighborsets[p].size() >= m_minPts)    // ����¼ӵĸõ��Ǻ��ĵ㣬��չ���������ھӵ����neighbors������
					{
						for (auto neigh : g_dbscanneighborsets[p])     // Ĭ���ھ��а�������
							neighbors.insert(neigh);
					}
				}

				if (belonging[p] == -1)                // �õ�δ�����䣬���䵽��Ӧ�Ĵ���
				{
					belonging[p] = clusterid;
					cluster.insert(p);
					m_valid_cluster_points.insert(p);  // ͳ����Ч�ľ����
				}
			}
			
			m_clusters.push_back(cluster);
			clusterid++;
		}
	}
}


float BaseFramework::getEntropy(set<int> & _cluster, int _type1, int _type2)
{
	map<int, int> appearances;        // <�ڵ�id, �ýڵ���ֵĴ���>

	//ö�ٴ������ÿһ������ΪԴ��
	for (int vid : _cluster)
	{
		Vertex * v = g_vertices[vid];

		if (v->vertextype == _type1)             //����õ�����type1
		{
			//ö��������ܵ����������
			for (auto & uid : v->neighborvertex)
			{
				Vertex * u = g_vertices[uid];

				if (u->vertextype == _type2)      //����յ�������type2
				{
					appearances[u->vertexid] += 1;
				}
			}
		}
	}

	int total = 0;    // ͳ����������
	for (auto & pair : appearances)
	{
		total += pair.second;
	}

	float entropy = 0.0;

	for (auto & pair : appearances)
	{
		float prob = pair.second * 1.0f / total;
		entropy += -1 * prob * log2(prob);
	}
	return entropy;
}


float BaseFramework::weightUpdate_Entropy()
{
	float entropy[ATTRIBUTE_BUFF_SIZE] = { 0.0f, 0.0f, 0.0f, 0.0f };   // �洢��<����-������>�ߵ���֮�ͣ�����entropy[0]��ʾ������أ��������㣬����

	float totalentropy = 0;

	//�����Ȩ�ز��䣬�������ͱ�Ȩ�ظ����ص������ܺͲ���
	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{
		for (auto cluster : m_clusters)
		{
			if (cluster.size() <= 1)    // �������ֻ��һ���ڵ㣬��Ϊ0��ֱ�ӽ�����һ��
				continue;

			//entropy[i] += getEntropy(cluster, STRUCTURE, i);    // ����Ȩ���㣬ÿ���ص���Ҫ����ͬ
			entropy[i] += (float)cluster.size() / m_valid_cluster_points.size() * getEntropy(cluster, STRUCTURE, i);   // ��Ȩ���㣬��Խ����Ҫ��Խ��
			//entropy[i] += (1 - (float)cluster.size() / m_valid_cluster_points.size()) * getEntropy(cluster, STRUCTURE, i);   // ��Ȩ���㣬��Խ����Ҫ��ԽС
		}
		totalentropy += entropy[i];
	}

	// ����ĳ��������Ϊ0���������(ȡ��ֵ)
	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{
		if (abs(entropy[i]) < 1e-6)
			entropy[i] = (float)totalentropy / ATTRIBUTE_NUM;
	}

	// �����Ӧ�ı���
	float inverse_entropy[ATTRIBUTE_BUFF_SIZE];
	float inverse_totalentropy = 0.0;
	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{
		inverse_entropy[i] = (float) 1.0f / entropy[i];   // �����صķ������¼��� totalentropy
		//inverse_entropy[i] = entropy[i];                    // �����ص��������¼��� totalentropy
		inverse_totalentropy += inverse_entropy[i];
	}

	// ���½���Ȩ�ط���
	float diff = 0.0f;
	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{
		// �����ظ���Ȩ��
		float beforeweight = g_edgeweight[STRUCTURE][i];

		// ����Ȩ�ط����ȫ�ֱ�����
		g_edgeweight[STRUCTURE][i] += (inverse_entropy[i] / inverse_totalentropy) * g_attribute_total_edgeweight;  // ����������Ȩ��
		g_edgeweight[STRUCTURE][i] /= 2.0f;

		// ����Ȩ��ƫ��
		diff += pow(g_edgeweight[STRUCTURE][i] - beforeweight, 2);
	}

	return sqrt(diff);
}


float BaseFramework::weightUpdate_Vote()
{
	set<int> s;
	vector<int> vote(ATTRIBUTE_NUM + 1, 0);   // �洢�����ͶƱ��(vote[0]��ʾ���࣬������)
	int vote_all = 0;     // ��ͶƱ��

	// �ֱ�����������ͶƱ
	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{
		for (auto & cluster : m_clusters)    // �ֱ��������
		{
			//���ڽڵ������Ƚ�
			for (auto v : cluster)
			{
				s.clear();
				int n_startid = 0;
				for (int kk = i; kk > 0; kk--)
				{
					n_startid += g_vertices[v]->edgetype_outdegree[kk - 1];
				}

				for (; n_startid < g_vertices[v]->neighborvertex.size(); n_startid++)
				{
					int nn_v = g_vertices[v]->neighborvertex[n_startid];
					if (g_vertices[nn_v]->vertextype == i)   // ���ھӽڵ����ڵ�ǰ���
						s.insert(nn_v);
					else
						break;
				}

				for (auto u : cluster)
				{
					if (v > u)        // ֻ���ǵ��ߣ������ظ�����
						continue; 

					set<int> ss(s);   // ���� s

					int m_startid = 0;
					for (int kk = i; kk > 0; kk--)
					{
						m_startid += g_vertices[u]->edgetype_outdegree[kk - 1];
					}

					for (; m_startid < g_vertices[u]->neighborvertex.size(); m_startid++)
					{
						int mm_u = g_vertices[u]->neighborvertex[m_startid];
						if (g_vertices[mm_u]->vertextype == i)   // ���ھӽڵ����ڵ�ǰ���
							ss.insert(mm_u);
						else
							break;
					}

					// ���㽻��
					vote[i] += g_vertices[v]->edgetype_outdegree[i] + g_vertices[u]->edgetype_outdegree[i] - (int)ss.size();   // ͬ�����У�������Ŀ��ͶƱ��Խ�࣬����Ч��Խ��
				}
			}

		}
		vote_all += vote[i];
	}

	float diff = 0;

	// ����Ȩ��  vote[i] Խ�󣬷����Ȩ��Խ��
	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{
		float before = g_edgeweight[STRUCTURE][i];   // �ɵ�Ȩ��

		g_edgeweight[STRUCTURE][i] += (vote[i] * ATTRIBUTE_NUM * 1.0f / vote_all);  // �µ�Ȩ��
		g_edgeweight[STRUCTURE][i] /= 2.0;

		diff += pow(before - g_edgeweight[STRUCTURE][i], 2);
	}

	return sqrt(diff);
}


float BaseFramework::clusterEvaluate_Density()
{
	int valid_edge_num = 0;
	int cc_num = 0;
	for (auto & cluster : m_clusters)
	{
		for (auto vid : cluster)
		{
			Vertex *v = g_vertices[vid];
			for (int i = 0; i < v->edgetype_outdegree[STRUCTURE]; i++)
			{
				int v_n = v->neighborvertex[i];

				// ͳ���ܱ���Ŀ
				if (m_valid_cluster_points.find(v_n) != m_valid_cluster_points.end())
				{
					valid_edge_num++;   
				}

				// ͳ�ƴ��ڱ�
				if (cluster.find(v_n) != cluster.end())
				{
					cc_num++;
				}
			}
		}
	}

	valid_edge_num /= 2;
	cc_num /= 2;
	float density = (float)cc_num / valid_edge_num;
	return density;
}

// ��ͨƽ��
float BaseFramework::clusterEvaluate_Entropy1()
{
	float total_entropy = 0.0f;

	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{
		for (auto cluster : m_clusters)
		{
			if (cluster.size() <= 1)    // �������ֻ��һ���ڵ㣬��Ϊ0��ֱ�ӽ�����һ��
				continue;

			// ֱ��ȡƽ��
			total_entropy += (float) cluster.size() / m_valid_cluster_points.size() * getEntropy(cluster, STRUCTURE, i);
		}
	}

	total_entropy /= ATTRIBUTE_NUM;

	return total_entropy;
}


// ��Ȩƽ��
float BaseFramework::clusterEvaluate_Entropy2()
{
	float total_entropy = 0.0f;

	for (int i = ATTRIBUTE_1; i < ATTRIBUTE_1 + ATTRIBUTE_NUM; i++)
	{ 
		for (auto cluster : m_clusters)
		{
			if (cluster.size() <= 1)    // �������ֻ��һ���ڵ㣬��Ϊ0��ֱ�ӽ�����һ��
				continue;

			// ��Ȩƽ��
			total_entropy += (float)cluster.size() / m_valid_cluster_points.size() * getEntropy(cluster, STRUCTURE, i) * g_edgeweight[STRUCTURE][i];
		}
	}

	total_entropy /= ATTRIBUTE_NUM;

	return total_entropy;
}


float BaseFramework::clusterEvaluate_WithinClusterAveDistance()
{
	float sums = 0.0f;
	for (auto & cluster : m_clusters)
	{
		float sum = 0.0f;
		for (auto vid : cluster)
		{
			// ����ͬ������������
			for (auto uid : cluster)
			{
				if (uid > vid && m_pprDistances[vid].find(uid) != m_pprDistances[vid].end())  // ͬ�����������㲢�� ppr score > 0
				{
					sum += m_pprDistances[vid][uid];
				}
			}
		}
		sum /= cluster.size();
		sums += sum;
	}

	return (sums /= m_clusters.size());
}


//float BaseFramework::clusterEvaluate_NMI()
//{
//	// ʵ�ַ����ο��� https://blog.csdn.net/tobacco5648/article/details/50890106
//	Py_Initialize();       // ��ʼ��python������,���߱�����Ҫ�õ�python������
//	PyRun_SimpleString("import Py_NMI");           // ����python�ļ���python�ļ�Ҫ���ں�exeִ���ļ���ͬĿ¼
//	PyRun_SimpleString("Py_NMI.printNMI()");       // ����python�ļ��е�ִ�к���
//	Py_Finalize();         // ����python���������ͷ���Դ
//}


void BaseFramework::storeClusterResult(string _cluster_output)
{
	ofstream cluster_ou;
	cluster_ou.open(_cluster_output, ios::out);

	// Store Results
	// -----------------------------------------------------------
	//int valid_cluster_points = 0;
	//cluster_ou << m_clusters.size() << " " << endl;
	//for (int i = 0; i < m_clusters.size(); i++)
	//{
	//	cluster_ou << m_clusters[i].size() << " ";
	//	for (auto vv : m_clusters[i])
	//	{
	//		cluster_ou << vv << " ";
	//		valid_cluster_points += 1;
	//	}
	//	cluster_ou << endl;
	//}
	//cluster_ou << "valid points = " << valid_cluster_points << endl;
	// -----------------------------------------------------------

	// Case Study
	// -----------------------------------------------------------
	for (auto & cluster : m_clusters)
	{
		for (auto vid : cluster)
		{
			Vertex * v = g_vertices[vid];

			cluster_ou << v->vertexname << "\t";

			//cluster_ou << vid << "\t";

			//if (g_dbscanneighborsets[vid].size() >= m_minPts)
			//{
			//	cluster_ou << "��" << vid << "��" << "\t";
			//}
			//else
			//{
			//	cluster_ou << vid << "\t";
			//}
				

			//cluster_ou << vid << ":" << m_pprDistances[0][vid] << endl;

			//cluster_ou << vid << ":" << g_dbscanneighborsets[vid].size() << "\t";
		}
		cluster_ou << endl;
	}

	//sort(core_point.begin(), core_point.end());
	//for (int i = 0; i < core_point.size(); i++)
	//{
	//	for (int j = 0; j < core_point.size(); j++)
	//	{
	//		if (m_pprDistances[core_point[i]][core_point[j]] > g_delta)
	//			cluster_ou << core_point[i] << "_" << core_point[j] << ": " << m_pprDistances[core_point[i]][core_point[j]] << endl;
	//	}
	//}
	// -----------------------------------------------------------

	// �洢����
	//for (auto p_map : m_pprDistances)
	//{
	//	for (auto dis : p_map.second)
	//	{
	//		cluster_ou << dis.first << ":" << dis.second << "\t";
	//	}
	//	cluster_ou << endl;
	//}

	cluster_ou.close();
}


void BaseFramework::storeClusterResultForComparison(string _cluster_output)
{
	ofstream cluster_ou;
	cluster_ou.open(_cluster_output, ios::out);

	for (auto & cluster : m_clusters)
	{
		cluster_ou << cluster.size() << "\t";
		for (auto vid : cluster)
		{
			cluster_ou << vid << "\t";
		}
	}
	cluster_ou.close();
}

void BaseFramework::getClusterResult(std::string _cluster_result_path)
{
	m_clusters.clear();

	std::ifstream in;
	in.open(_cluster_result_path, std::ios::in);

	int num;
	while (in >> num)
	{
		if (num < 2)
			continue;  //	ȥ��������
		std::set<int> cluster;
		for (int i = 0; i < num; i++)
		{
			int tmp;
			in >> tmp;
			m_valid_cluster_points.insert(tmp);
			cluster.insert(tmp);
		}
		m_clusters.push_back(cluster);
	}
	in.close();
	return;
}