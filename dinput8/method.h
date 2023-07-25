#pragma once
#include <string>
using namespace std;

#define GWL_WNDPROC (-4)
static HANDLE hConsole = nullptr;
static wstring runPath;
struct clientInfo {
    DWORD pid;
    HWND hWnd;
    WNDPROC oldProc;
    HANDLE hProcess;
    DWORD64 startAddress;
    DWORD64 endAddress;
};
//��ʽ���������ݽ����Ĳ������ַ�����ʽ���
void PrintToConsole(const char* format, ...);
void CreateConsole();
namespace method {
    BOOL WriteAddressBin(HANDLE hProcess, DWORD64 lpBaseAddress, DWORD64 lpBuffer);
    //��ȡָ���ڴ��������� �����Ƿ�ɹ�
    //�򿪵Ľ��̾�� ��Ҫ�����ڴ��ַ �洢�������ݵ�ָ�� ���ĵ�ַ��С(�ɿ�) ƫ��...(�ɿ� ��һ��������)
    BOOL ReadAddressBin(HANDLE hProcess, LPCVOID lpBaseAddress, LPCVOID lpBuffer, size_t size, int offset, ...);

    //���뵱ǰĿ¼�µ�R3nzSkin.dll
    void loadR3nz();
    //void findMemory(HANDLE hProcess,char*Code,DWORD64 Start,DWORD64 End);
    //��ȡģ��ķ�Χ
    pair<DWORD64, DWORD64> GetModuleAddressRange(HMODULE hModule);
    //�Ա����������Ƿ���ͬ ֧����0XFF��ʾģ����
    BOOL CompareArrays(const vector<BYTE>& TZM, const vector<BYTE>& NC);
    //�����ո��??��������ת�����ֽ����鲢��FF���??
    vector<unsigned char> ConvertStringToByteArray(const std::string& featureCode);
    //��ָ���ķ�Χ�����������룬������������+ƫ�����ڵ��ڴ�λ��
    DWORD64 LocateSignature(HANDLE hProcess, const string& tezhengma, DWORD64 startAddress, DWORD64 endAddress, int offset);
}
