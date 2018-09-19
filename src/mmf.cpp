#include <iostream>
#include "mmf.h"

using namespace std;


void MMF::mmfCreate(const char * _sharedname, const char * _filename, const DWORD _mmfsizehigh, const DWORD _mmfsizelow, float * & _mmfm_base_address)
{
	
	//��ȡģʽ
	mmf_access_mode access_mode = (GENERIC_READ | GENERIC_WRITE);

	//����ģʽ
	mmf_share_mode share_mode = 0;             // ������
	//mmf_share_mode share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;

	//�ļ�����
	mmf_flags flags = FILE_FLAG_SEQUENTIAL_SCAN;      // ����������ʶ��ļ���������Ż�
	//mmf_flags flags = FILE_FLAG_RANDOM_ACCESS;      // ���������ʶ��ļ���������Ż�

	DWORD error_code;

	//�����ļ�,�����ļ����(HANDLE)
	mmHandle = CreateFile(_filename,		       // Ҫ�򿪵��ļ�������
							     access_mode,	   // GENERIC_READ ��ʾ������豸���ж�����; GENERIC_WRITE ��ʾ������豸����д����(�����ʹ��); ���Ϊ�㣬��ʾֻ�����ȡ��һ���豸�йص���Ϣ
		                         share_mode,	   // 0 ��ʾ������;FILE_SHARE_READ �� / �� FILE_SHARE_WRITE ��ʾ������ļ����й������
		                         NULL,			   // SECURITY_ATTRIBUTES ��ָ��һ�� SECURITY_ATTRIBUTES �ṹ��ָ�룬�������ļ��İ�ȫ���ԣ��������ϵͳ֧�ֵĻ���
								 OPEN_ALWAYS,	   /* CREATE_NEW: �����ļ�, ���ļ�����������; CREATE_ALWAYS: �����ļ������дǰһ���ļ�; OPEN_EXISTING: �ļ������Ѿ�����, ���豸���Ҫ��;
												      OPEN_ALWAYS: ���ļ��������򴴽���; TRUNCATE_EXISTING: �������ļ�����Ϊ�㳤�� */
						         flags,            // �ļ�����
						         NULL);            // �����Ϊ�㣬��ָ��һ���ļ���������ļ���������ļ��и�����չ����

	if (mmHandle == INVALID_HANDLE_VALUE)  // �����ļ�ʧ��
	{
		error_code = GetLastError();
		cout << "����mmfʧ��:" << error_code << endl;
	}
	else                                  // ����mmf�ɹ�
	{
		DWORD high_size;
		DWORD file_size = GetFileSize(mmHandle, &high_size);    // ��ȡ�ļ���С

		if (file_size == BAD_POS && (error_code = GetLastError()) != SUCCESS)   // �����ļ�ʧ��
		{
			CloseHandle(mmHandle);   //�ر��ڴ�ӳ���ļ�
			cout << "error��" << error_code << endl;
		}

		//�����ļ�ӳ�䣬���Ҫ�����ڴ�ҳ���ļ���ӳ�䣬��һ����������ΪINVALID_HANDLE_VALUE
		mmfm = CreateFileMapping(mmHandle,                     /* ָ�򴴽�ӳ�������ļ����. �����Ҫ�������ļ�����, Ҫȷ�������ļ�������ʱ��ķ���ģʽ����flProtect����ָ����"������ʶ"ƥ�䣬
															   ���磺�����ļ�ֻ���� �ڴ�ӳ����Ҫ��д�ͻᷢ�������Ƽ��������ļ�ʹ�ö�ռ��ʽ���� */
										NULL,                  // ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬�����Ƿ��ӽ��̼̳С����ΪNULL,������ܱ��̳У�һ�����ó�NULL
										PAGE_READWRITE,        // ���ļ�ӳ���ʱ����Ҫ�˲������ñ�����ʶ. PAGE_READWRITE: ͬʱ�ж���д��Ȩ��
										_mmfsizehigh,          // dwMaximumSizeHigh���ļ�ӳ���������ֵ�ĸ�λ��DWORD��
										_mmfsizelow,           // dwMaximumSizeLow: �ļ�ӳ���������ֵ�ĵ�λ��DWORD�������dwMaximumSizeHighΪ0���ļ�ӳ���������ֵΪ��hFile��ʶ���ļ��Ĵ�С
										_sharedname);          // �ļ�ӳ����������

		error_code = GetLastError();
		if (SUCCESS != error_code)
		{
			cout << "createFileMapping error: " << error_code << endl;
		}
		else
		{
			if (mmfm == NULL)
			{
				if (mmHandle != INVALID_HANDLE_VALUE)
				{
					CloseHandle(mmHandle);
				}
			}
			else
			{
				DWORD view_access = FILE_MAP_ALL_ACCESS;      // ����д�ķ���Ȩ��

				// ���ӳ����ͼ. ӳ���ļ���ͼ�����ý��̵ĵ�ַ�ռ�. ӳ��һ���ļ�������ָ�����ļ������ڵ��ý��̵ĵ�ַ�ռ�ɼ�
				// ��������ɹ�������ֵ��ӳ����ͼ�Ŀ�ʼλ��.�������ʧ�ܣ�����ֵΪNULL������ͨ������GetLastError���������ϸ�Ĵ�����Ϣ
				_mmfm_base_address = (float*)MapViewOfFile(mmfm,          // hFileMappingObject: һ���򿪵�ӳ���ļ�����ľ����������������CreateFileMapping��OpenFileMapping��������
														 view_access,     // dwDesiredAccess: ָ�������ļ���ͼ������
														 0,               // dwFileOffsetHigh: ָ����ʼӳ���ļ�ƫ�����ĸ�λ
														 0,               // dwFileOffsetLow: ָ����ʼӳ���ļ�ƫ�����ĵ�λ
														 0);              // dwNumberOfBytesToMap: ָ����Ҫӳ����ļ����ֽ����������dwNumberOfBytesToMapΪ0��ӳ���������ļ�

				if (_mmfm_base_address == NULL)
				{
					error_code = GetLastError();
					if (error_code != SUCCESS)
					{
						cout << "error code " << error_code << endl;
					}
				}
			}
		}
	}
}


void MMF::mmfClose(float * & _mmfm_base_address)
{
	//ж��ӳ��
	UnmapViewOfFile(_mmfm_base_address);
	//�ر��ڴ�ӳ���ļ�
	CloseHandle(mmfm);
	//�ر��ļ�
	CloseHandle(mmHandle);
}