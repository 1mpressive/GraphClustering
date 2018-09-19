#ifndef MYGLOBALPARAMETERS_H
#define MYGLOBALPARAMETERS_H
#include <map>
#include <set>
#include <iostream>
#include <fstream>

#include "Vertex.h"
#include "mmf.h"

using namespace std;

                                 // ���ݼ�       DBLP			Flickr		Football
#define STRUCTURE			0    // ���ڵ�		 paper			image		player
#define ATTRIBUTE_1			1    // ���Ե�1		 author			user		list
#define ATTRIBUTE_2			2    // ���Ե�2		 conference		tag			tweet
#define ATTRIBUTE_3			3    // ���Ե�2		 keywords
extern int ATTRIBUTE_NUM;		 // ������Ŀ     3				2

#define ATTRIBUTE_BUFF_SIZE 4

// global parameters
extern int g_clustertype;                            // �������ͣ�Ĭ��ΪSTRUCTURE
extern int g_datasetid;                              // ���ݼ����
extern int g_schemeid;                               // �������
extern float g_epsilon;                              // reverse push ������
extern float g_delta;                                // dbscan range
extern int g_minPts;                                 // dbscan minPts
extern float g_alpha;                                // reverse push ��������
extern float g_gamma;                                // game theory ���ۺ�������
extern int g_cluster_startid;                        // �������Ϳ�ʼ�ڵ�(�����õ�)
extern int g_cluster_endid;                          // �������ͽ����ڵ�(�����õ�)
extern int g_vertexnum;                              // �ܵĽڵ���Ŀ
extern long long g_pushbackcount;                    // pushback ����
extern vector<Vertex*> g_vertices;                   // ��ṹ
extern map<int, set<int>> g_dbscanneighborsets;      // ���ھӽڵ�(��ָ���ڵ�����)

const int g_buff_size = 7;                    // ����������
extern float * g_bufferpool[];                // Ԥ�����ݻ�����
extern MMF * g_mmfpool[];                     // MMF�����
extern float * g_mmfm_address_pool[];         // MMF ��ַ��
extern set<int> g_buffer_queue;               // buffer�ط������
extern float * g_pr_pool[];                   // pr������

extern HANDLE  g_hSemaphoreRunnerNum;      // �ź������趨���Ĳ����߳���
extern HANDLE  g_hThreadEvent;             // �ź����������̷ֲ߳���ͬ��
extern CRITICAL_SECTION g_cs;              // �ؼ���

// ��Ȩ��
extern float g_structure_edgeweight;              // ����ڵ��Ȩ�� { = 1}
extern float g_total_edgeweight;                  // �������ͱߵ�Ȩ�غ�
extern float g_attribute_total_edgeweight;        // ���Խڵ��Ȩ�غ�
extern float g_base_weight[ATTRIBUTE_BUFF_SIZE];                                // ��ʼȨ��
extern float g_former_edgeweight[ATTRIBUTE_BUFF_SIZE][ATTRIBUTE_BUFF_SIZE];     // ��BaseFramework::BaseFramework()�г�ʼ��
extern float g_edgeweight[ATTRIBUTE_BUFF_SIZE][ATTRIBUTE_BUFF_SIZE];            // ��BaseFramework::BaseFramework()�г�ʼ��

extern string g_inputpath;        // ���������ļ�����·��
extern string g_resultpath;       // �������ļ�����·��

extern string g_mmfPath;          // Ԥ�����ݱ������·��
extern int g_preflag;             // �Ƿ�Ԥ������

// ApproximateReversePush
extern DWORD a_mmfsizehigh;                // ��λ�ļ���С��x4G��
extern DWORD a_mmfsizelow;                 // ��λ�ļ���С
extern unsigned int a_mmf_buffer_size;     // ����ӳ���ļ��Ĵ�С
extern int a_THREADNUM;                    // ���Ʒ������߳�������

// PartialReversePush
extern DWORD p_mmfsizehigh;                // ��λ�ļ���С��x4G��
extern DWORD p_mmfsizelow;                 // ��λ�ļ���С
extern unsigned int p_mmf_buffer_size;     // ����ӳ���ļ��Ĵ�С
extern int p_THREADNUM;                    // ���ּ��㷽�����߳�������

#endif