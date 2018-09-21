#include <fstream>
#include <time.h>
#include <process.h>

#include "baseframework.h"
#include "MyGlobalParameters.h"

using namespace std;


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


void ApproximateReservePush::reservePush_Pre_Encode()
{
	static float * p = new float[g_vertexnum];
	static float * r = new float[g_vertexnum];
	static float * ur = new float[g_vertexnum * ATTRIBUTE_NUM];

	const int encode_buffer_size = 5 + (5 + ATTRIBUTE_NUM) * g_vertexnum;     // data block �����ռ�ÿռ�(�����������ж�λ)
	float * encode_buffer = new float[encode_buffer_size];

	unsigned int write_buffer_size = 0;   // ʵ�� data block ��С
	float flag[1];                        // ��Ǹÿ�����Ƿ������ݿ�

	// �ڴ�ӳ�����
	MMF * mmf_ptr = NULL;
	float * mmfm_base_address = NULL;
	float * write_dst_ptr;
	int mapfileid = 0;

	// ��һ��mmf�ļ�
	mmf_ptr = new MMF();
	char sharedname[1024] = "\0";
	char filename[1024] = "\0";
	sprintf_s(sharedname, "%d%s%d%s%d%s%d", g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", mapfileid);
	sprintf_s(filename, "%s%d%s%d%s%d%s%d", g_mmfPath.data(), g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", mapfileid, ".mmf");
	mmf_ptr->mmfCreate(sharedname, filename, a_mmfsizehigh, a_mmfsizelow, mmfm_base_address);
	write_dst_ptr = mmfm_base_address;
	mapfileid++;

	for (int targetID = g_cluster_startid; targetID <= g_cluster_endid; targetID++)
	{
		if (targetID % 1000 == 0)
			cout << targetID << endl;

		memset(p, 0, g_vertexnum * sizeof(float));
		memset(r, 0, g_vertexnum * sizeof(float));
		memset(ur, 0, g_vertexnum * ATTRIBUTE_NUM * sizeof(float));

		memset(encode_buffer, 0, encode_buffer_size * sizeof(float));

		// ---------- pushback ----------
		set<int> pushback_queue;

		r[targetID] = 1.0f;  // target point

		pushback_queue.insert(targetID);

		while (pushback_queue.size() > 0)
		{
			int uID = *pushback_queue.begin();
			pushback_queue.erase(pushback_queue.begin());

			Vertex * u = g_vertices[uID];    // ��ȡ��pushback�ڵ���Ϣ

			p[uID] += g_alpha * r[uID];      // estimated value

			//����u�ܹ�����ĵ㣨reserve push��
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
		pushback_queue.clear();

		// ---------- ����ur ----------
		int u_ptr = 0;
		for (int i = 0; i < g_vertexnum; i++)
		{
			Vertex * u = g_vertices[i];
			int checkhop = u->edgetype_outdegree[STRUCTURE];       // ������Խڵ㿪ʼλ��

			for (int j = ATTRIBUTE_1; j <= ATTRIBUTE_NUM; j++)   // ��Ӧ���Ա�ţ���1��ʼ
			{
				float sum_atribute_p = 0.0;
				for (int k = 0; k < u->edgetype_outdegree[j]; k++)
				{
					sum_atribute_p += p[u->neighborvertex[checkhop]];
					checkhop++;
				}

				float r_per_attribute;
				if (u->edgetype_outdegree[j] == 0)
					r_per_attribute = 0.0;
				else
					r_per_attribute = (1 - g_alpha) / (g_alpha * g_total_edgeweight * u->edgetype_outdegree[j]) * sum_atribute_p;

				ur[u_ptr] = r_per_attribute;
				u_ptr++;
			}
		}

		// ---------- ����ѹ����д���ļ� ----------
		int buffer_index = 4;    // [0] - [3] Ϊ���ݿ�ͷ 
		int b_p_count = 0;
		int b_r_count = 0;
		int b_ur_count = 0;

		// estimated value
		for (int i = 0; i < g_vertexnum; i++)
		{
			if (p[i] > 0)
			{
				b_p_count++;
				encode_buffer[buffer_index++] = (float)i;
				encode_buffer[buffer_index++] = p[i];
			}
		}
		// residual value
		for (int i = 0; i < g_vertexnum; i++)
		{
			if (r[i] > 0)
			{
				b_r_count++;
				encode_buffer[buffer_index++] = (float)i;
				encode_buffer[buffer_index++] = r[i];
			}
		}
		// update residual value
		for (int i = 0; i < g_vertexnum; i++)
		{
			bool c_flag = true;
			for (int tt = 0; tt < ATTRIBUTE_NUM; tt++)    // �������Զ�ӦֵȫΪ0�Ļ���ȥ��
			{
				if (ur[i * ATTRIBUTE_NUM + tt] > 0)
				{
					c_flag = false;
					break;
				}
			}
			if (c_flag)
				continue;

			b_ur_count++;
			encode_buffer[buffer_index++] = (float)i;
			for (int j = i * ATTRIBUTE_NUM; j < i * ATTRIBUTE_NUM + ATTRIBUTE_NUM; j++)
			{
				encode_buffer[buffer_index++] = ur[j];
			}
		}

		unsigned int block_size = 4 + b_p_count * 2 + b_r_count * 2 + b_ur_count * (1 + ATTRIBUTE_NUM);   // ���ݿ��С

		// buffer��ͷ��ͳ��
		encode_buffer[0] = (float)targetID;     // �ڵ�id
		encode_buffer[1] = (float)b_p_count;
		encode_buffer[2] = (float)b_r_count;
		encode_buffer[3] = (float)b_ur_count;

		write_buffer_size += (block_size + 1) * sizeof(float);   // �����������ж�λ

		if (write_buffer_size < a_mmf_buffer_size)
		{
			if (targetID > g_cluster_startid)  // ��һ�����ݿ鲻��Ҫ��ǰ��ӱ�־λ
			{
				flag[0] = 1.0f;     // ���滹�����ݿ�
				memcpy(write_dst_ptr, flag, sizeof(float));
				write_dst_ptr = write_dst_ptr + 1;
			}

			// ����д���ڴ�ӳ���ļ�
			memcpy(write_dst_ptr, encode_buffer, block_size * sizeof(float));
			write_dst_ptr = write_dst_ptr + block_size;
		}
		else     // ��һ���ļ���С�Ѿ��ﵽ����
		{
			flag[0] = -1.0f;
			memcpy(write_dst_ptr, flag, sizeof(float));
			write_dst_ptr = write_dst_ptr + 1;

			mmf_ptr->mmfClose(mmfm_base_address);
			delete mmf_ptr;

			write_buffer_size = block_size * sizeof(float);    // ����ͳ�ƿ�

			// ����һ���µ��ļ�
			mmf_ptr = new MMF();
			char sharedname[1024] = "\0";
			char filename[1024] = "\0";
			sprintf_s(sharedname, "%d%s%d%s%d%s%d", g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", mapfileid);
			sprintf_s(filename, "%s%d%s%d%s%d%s%d", g_mmfPath.data(), g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", mapfileid, ".mmf");
			mmf_ptr->mmfCreate(sharedname, filename, a_mmfsizehigh, a_mmfsizelow, mmfm_base_address);
			write_dst_ptr = mmfm_base_address;
			mapfileid++;

			// ����д���ڴ�ӳ���ļ�
			memcpy(write_dst_ptr, encode_buffer, block_size * sizeof(float));
			write_dst_ptr = write_dst_ptr + block_size;
		}
	}
	// �ر����һ���ڴ�ӳ���ļ�
	flag[0] = -1.0f;
	memcpy(write_dst_ptr, flag, sizeof(float));
	mmf_ptr->mmfClose(mmfm_base_address);
	delete mmf_ptr;

	// ����Ԥ���ļ�������
	int pre_file_num = mapfileid;
	ofstream pre_file_ou;
	string pre_file_outputpath = g_mmfPath + to_string(g_datasetid) + "_" + to_string(g_clustertype) + "_" + to_string(g_schemeid) + ".num";
	pre_file_ou.open(pre_file_outputpath, ios::out);
	pre_file_ou << pre_file_num << endl;
	pre_file_ou.close();

	// �ͷŻ�����
	delete p;
	delete r;
	delete ur;
	delete encode_buffer;
}


// ���̣߳����ڲ���
void approximateReservePushSingleThreadUpdate(ofstream & log_ou)
{
	int taskid = 0;  // ��Ϊָ���ļ����Ʊ��

	int pr_id = 0;
	int ud_id = 0;
	int checkID = -1;

	g_mmfpool[taskid] = new MMF();
	char sharedname[1024] = "\0";
	char filename[1024] = "\0";
	sprintf_s(sharedname, "%d%s%d%s%d%s%d", g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", taskid);
	sprintf_s(filename, "%s%d%s%d%s%d%s%d", g_mmfPath.data(), g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", taskid, ".mmf");
	g_mmfpool[taskid]->mmfCreate(sharedname, filename, a_mmfsizehigh, a_mmfsizelow, g_mmfm_address_pool[taskid]);
	memset(g_bufferpool[taskid], 0, a_mmf_buffer_size);
	memcpy(g_bufferpool[taskid], g_mmfm_address_pool[taskid], a_mmf_buffer_size);
	g_mmfpool[taskid]->mmfClose(g_mmfm_address_pool[taskid]);
	delete g_mmfpool[taskid];
	// --------------------------------------------------------------------------------------

	float * buffersection = g_bufferpool[taskid];
	int b_index = 0;
	int b_p_count;
	int b_r_count;
	int b_ur_count;
	float continue_flag;

	int targetID;

	while (true)
	{
		memset(g_pr_pool[pr_id], 0, 2 * g_vertexnum * sizeof(float));   // ���һλ����targetid

		targetID = (int)round(buffersection[b_index++]);                // _pr ���һλ���� target_id
		b_p_count = (int)round(buffersection[b_index++]);
		b_r_count = (int)round(buffersection[b_index++]);
		b_ur_count = (int)round(buffersection[b_index++]);

		cout << targetID << endl;

		// decode estimate value
		for (int i = 0; i < b_p_count; i++)
		{
			int p_id = (int)round(buffersection[b_index++]);
			g_pr_pool[pr_id][p_id] = buffersection[b_index++];
		}

		// decode residual value
		for (int i = 0; i < b_r_count; i++)
		{
			int r_id = g_vertexnum + (int)round(buffersection[b_index++]);
			g_pr_pool[pr_id][r_id] = buffersection[b_index++];
		}

		// decode update residual value
		for (int i = 0; i < b_ur_count; i++)
		{
			int ur_id = g_vertexnum + (int)round(buffersection[b_index++]);
			for (int j = 1; j <= ATTRIBUTE_NUM; j++)
			{
				g_pr_pool[pr_id][ur_id] += (g_edgeweight[STRUCTURE][j] - g_base_weight[j]) * buffersection[b_index++];
			}
		}

		// ����
		set<int> pushback_queue;

		for (int i = g_cluster_startid; i <= g_cluster_endid; i++)
		{
			if (g_pr_pool[ud_id][g_vertexnum + i] > g_epsilon)
			{
				pushback_queue.insert(i);
			}
		}

		// ���ݸ��½���ٴν��� pushback
		while (pushback_queue.size() > 0)
		{
			//push_back u
			int uID = *pushback_queue.begin();
			pushback_queue.erase(pushback_queue.begin());

			Vertex * u = g_vertices[uID];                                            // ��ȡ��pushback�ڵ���Ϣ

			g_pr_pool[ud_id][uID] += g_alpha * g_pr_pool[ud_id][g_vertexnum + uID];      // estimated value

			//����u�ܹ�����ĵ㣨reserve push��
			for (auto & wID : u->neighborvertex)
			{
				Vertex * w = g_vertices[wID];
				g_pr_pool[ud_id][g_vertexnum + wID] += (1 - g_alpha) * g_pr_pool[ud_id][g_vertexnum + uID] * getTransprob(w, u);    // residual value

				if (g_pr_pool[ud_id][g_vertexnum + wID] > g_epsilon)
				{
					pushback_queue.insert(wID);
				}
			}
			g_pr_pool[ud_id][g_vertexnum + uID] = 0;
		}
		pushback_queue.clear();

		// ��ȡ�ھ�
		set<int> n_set;
		for (int i = 0; i < g_vertexnum; i++)
		{
			if (g_pr_pool[ud_id][i] > g_delta)
			{
				n_set.insert(i);
			}
		}

		g_dbscanneighborsets.insert(pair<int, set<int>>(targetID, n_set));
		n_set.clear();

		// ������־
		continue_flag = buffersection[b_index++];
		if (continue_flag < 0)
		{
			break;
		}
	}
}


unsigned int __stdcall approximateReservePushMultiThreadUpdateFunc(PVOID pM)
{
	int taskid = *(int*)pM;
	SetEvent(g_hThreadEvent);      //�����¼�

	//�����ȡ������
	EnterCriticalSection(&g_cs);

	if (g_buffer_queue.size() == 0)
	{
		cout << "Error: the buffer queue is empty!" << endl;
	}
	int buffer_id = *g_buffer_queue.begin();
	g_buffer_queue.erase(g_buffer_queue.begin());
	g_mmfpool[buffer_id] = new MMF();
	char sharedname[1024] = "\0";
	char filename[1024] = "\0";
	sprintf_s(sharedname, "%d%s%d%s%d%s%d", g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", taskid);
	sprintf_s(filename, "%s%d%s%d%s%d%s%d", g_mmfPath.data(), g_datasetid, "_", g_clustertype, "_", g_schemeid, "_", taskid, ".mmf");
	g_mmfpool[buffer_id]->mmfCreate(sharedname, filename, a_mmfsizehigh, a_mmfsizelow, g_mmfm_address_pool[buffer_id]);
	memset(g_bufferpool[buffer_id], 0, a_mmf_buffer_size);
	memcpy(g_bufferpool[buffer_id], g_mmfm_address_pool[buffer_id], a_mmf_buffer_size);
	g_mmfpool[buffer_id]->mmfClose(g_mmfm_address_pool[buffer_id]);
	delete g_mmfpool[buffer_id];
	LeaveCriticalSection(&g_cs);
	// --------------------------------------------------------------------------------------
	float * buffersection = g_bufferpool[buffer_id];
	int b_index = 0;
	int b_p_count;
	int b_r_count;
	int b_ur_count;
	float continue_flag;

	int pushback_c = 0;
	int targetID;

	vector<pair<int, set<int>>> set_queue;

	while (true)
	{
		memset(g_pr_pool[buffer_id], 0, 2 * g_vertexnum * sizeof(float));

		// �������ݿ�
		targetID = (int)round(buffersection[b_index++]);
		b_p_count = (int)round(buffersection[b_index++]);
		b_r_count = (int)round(buffersection[b_index++]);
		b_ur_count = (int)round(buffersection[b_index++]);
		
		// decode estimate value
		for (int i = 0; i < b_p_count; i++)
		{
			int p_id = (int)round(buffersection[b_index++]);
			g_pr_pool[buffer_id][p_id] = buffersection[b_index++];
		}

		// decode residual value
		for (int i = 0; i < b_r_count; i++)
		{
			int r_id = g_vertexnum + (int)round(buffersection[b_index++]);
			g_pr_pool[buffer_id][r_id] = buffersection[b_index++];
		}

		// decode update residual value
		for (int i = 0; i < b_ur_count; i++)
		{
			int ur_id = g_vertexnum + (int)round(buffersection[b_index++]);
			for (int j = ATTRIBUTE_1; j <= ATTRIBUTE_NUM; j++)
			{
				g_pr_pool[buffer_id][ur_id] += (g_edgeweight[STRUCTURE][j] - g_base_weight[j]) * buffersection[b_index++];
			}
		}

		// ����
		set<int> pushback_queue;

		for (int i = 0; i < g_vertexnum; i++)
		{
			if (g_pr_pool[buffer_id][g_vertexnum + i] > g_epsilon)
			{
				pushback_queue.insert(i);
			}
		}

		// ���ݸ��½���ٴν��� pushback
		while (pushback_queue.size() > 0)
		{
			//push_back u
			int uID = *pushback_queue.begin();
			pushback_queue.erase(pushback_queue.begin());

			Vertex * u = g_vertices[uID];

			g_pr_pool[buffer_id][uID] += g_alpha * g_pr_pool[buffer_id][g_vertexnum + uID];      // estimated value
			pushback_c++;

			//����u�ܹ�����ĵ㣨reserve push��
			for (auto & wID : u->neighborvertex)
			{
				Vertex * w = g_vertices[wID];
				g_pr_pool[buffer_id][g_vertexnum + wID] += (1 - g_alpha) * g_pr_pool[buffer_id][g_vertexnum + uID] * getTransprob(w, u);  // residual value

				if (g_pr_pool[buffer_id][g_vertexnum + wID] > g_epsilon)
				{
					pushback_queue.insert(wID);
				}
			}
			g_pr_pool[buffer_id][g_vertexnum + uID] = 0;
		}
		pushback_queue.clear();

		// ��ȡ�ھ�
		set<int> n_set;
		for (int i = g_cluster_startid; i <= g_cluster_endid; i++)
		{
			if (g_pr_pool[buffer_id][i] > g_delta)
			{
				n_set.insert(i);
			}
		}
		set_queue.push_back(pair<int, set<int>>(targetID, n_set));
		
		// ������־
		continue_flag = buffersection[b_index++];
		if (continue_flag < 0)
		{
			break;
		}
	}

	//�����ͷŻ�����
	EnterCriticalSection(&g_cs);

	for (auto npair : set_queue)
		g_dbscanneighborsets.insert(npair);

	g_buffer_queue.insert(buffer_id);

	g_pushbackcount += pushback_c;
	LeaveCriticalSection(&g_cs);

	// ֪ͨ�ɼ����µ��߳�
	ReleaseSemaphore(g_hSemaphoreRunnerNum, 1, NULL);
	return 0;
}


void ApproximateReservePush::approximateReservePushMultiThreadUpdate()
{
	g_dbscanneighborsets.clear();   // ����

	InitializeCriticalSection(&g_cs);        
	g_hSemaphoreRunnerNum = CreateSemaphore(NULL, 7, 7, NULL);      // ������ʾ����ͬʱ���е��߳���(�뻺������Ŀ��Ӧ)
	g_hThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);         // ������֤�������ݵ�ͬ��

	// ָ���߳�����(��������)
	HANDLE * hThread = new HANDLE[a_THREADNUM];

	// ִ���߳�
	for (int i = 0; i < a_THREADNUM; i++)
	{
		// �ȴ��пյĻ���������
		WaitForSingleObject(g_hSemaphoreRunnerNum, INFINITE);
		hThread[i] = (HANDLE)_beginthreadex(NULL, 0, approximateReservePushMultiThreadUpdateFunc, &i, 0, NULL);
		WaitForSingleObject(g_hThreadEvent, INFINITE);             // �ȴ��¼�������  
	}

	WaitForMultipleObjects(a_THREADNUM, hThread, TRUE, INFINITE);

	for (int i = 0; i < a_THREADNUM; i++)
		CloseHandle(hThread[i]);
	delete hThread;
	CloseHandle(g_hSemaphoreRunnerNum);
	DeleteCriticalSection(&g_cs);
}


void ApproximateReservePush::execute()
{
	string result_output = g_resultpath + "result_" + to_string(g_datasetid) + "_" + to_string(g_delta)
		+ "_" + to_string(m_minPts) + "_" + to_string(g_gamma) + "_" + to_string(g_epsilon) + ".txt";
	string cluster_output = g_resultpath + "cluster_result_" + to_string(g_datasetid) + "_" + to_string(g_schemeid) + "_"
		+ to_string(g_delta) + "_" + to_string(m_minPts) + "_" + to_string(g_gamma) + "_" + to_string(g_epsilon) + ".txt";

	ofstream log_ou;
	log_ou.open(result_output, ios::app);
	 
	float diff = 1e10;
	int iterTimes = 0;
	long long total_pushback_times = 0;
	clock_t total_start, total_end;

	log_ou << endl << "********************************" << endl << "Approximate Approach: " << endl;
	// ====================================== ��ͼ ======================================
	readGraph();
	g_vertexnum = (int)g_vertices.size();
	// ====================================== Ԥ���� ====================================== 
	if (g_preflag)
	{
		reservePush_Pre_Encode();   // ѹ��
		cout << "PreProcess is Completed!" << endl;
		log_ou << "PreProcess is Completed!" << endl;
		return;
	}
	// ��ȡԤ���ļ�������
	ifstream pre_file_in;
	string pre_file_inputpath = g_mmfPath + to_string(g_datasetid) + "_" + to_string(g_clustertype) + "_" + to_string(g_schemeid) + ".num";
	pre_file_in.open(pre_file_inputpath, ios::in);
	pre_file_in >> a_THREADNUM;
	pre_file_in.close();
	// ====================================== �������� ======================================
	// ���仺�����ռ�
	g_buffer_queue.clear();
	for (int i = 0; i < g_buff_size; i++)
	{
		g_bufferpool[i] = new float[a_mmf_buffer_size / sizeof(float)];
		g_pr_pool[i] = new float[2 * g_vertexnum];
		g_buffer_queue.insert(i);
	}

	// ����ʱ���ظ�20��ȡƽ��ֵ
	int runTimes = 1;
	long long total_running_time = 0;
	for (int i = 0; i < runTimes; i++)
	{
		// Initialize
		diff = 1e10;
		iterTimes = 0;
		total_pushback_times = 0;

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

			// Conpute ppr score
			g_pushbackcount = 0;
			// 1. ���߳�
			//approximateReservePushSingleThreadUpdate(log_ou);
			// 2. ���߳�
			approximateReservePushMultiThreadUpdate();
			total_pushback_times += g_pushbackcount;

			// Symmetrization
			symmetrization();

			// DBSCAN
			dbscan();

			// Weight update
			log_ou << "Current weight: ";
			for (int i = ATTRIBUTE_1; i <= ATTRIBUTE_NUM; i++)
			{
				log_ou << g_edgeweight[STRUCTURE][i] << "\t";
			}
			log_ou << endl;
			diff = weightUpdate_Entropy();         // �أ�ֻʵ�ְ������жϣ�
			//diff = weightUpdate_Vote();          // ͶƱ��ֻʵ�ְ������жϣ�
		}
		total_end = clock();

		total_running_time += total_end - total_start;
	}

	// ���ջ�����
	for (int i = 0; i < g_buff_size; i++)
	{
		delete g_bufferpool[i];
		delete g_pr_pool[i];
	}

	// ====================================== ͳ�ƽ�� ======================================
	log_ou << "Total runningtime: " << total_running_time / runTimes << endl;
	log_ou << "Iteration Times: " << iterTimes << endl;
	log_ou << "Total Pushback Times: " << total_pushback_times << endl;
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
	log_ou << "Cluster_Entropy: " << clusterEvaluate_NormalEntropy() << endl;

	log_ou.close();

	// �洢������
	storeClusterResult(cluster_output);
}