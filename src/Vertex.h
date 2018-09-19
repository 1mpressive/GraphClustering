#ifndef Clustering_Vertex_h
#define Clustering_Vertex_h

#include <vector>
#include <string>

using std::string;
using std::vector;

class Vertex
{
public:
	int vertexid;                             // �ڵ�Id
	int vertextype;                           // �ڵ�����
	string vertexname;                        // �ڵ����ƣ�ͳһ32λ��ţ�
	vector<int> edgetype_outdegree;           // ÿ�����͵��ڱ���Ŀ���±�������. DBLP��˳���� p a c k; Flickr��˳����image, user, tag
	vector<int> neighborvertex;               // �ھӽڵ��ţ�����outDegrees����˳�����У�

	Vertex(int _vertexid, int _vertextype, string vertexname) :vertexid(_vertexid), vertextype(_vertextype), vertexname(vertexname){}
};

#endif
