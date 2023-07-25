#include <Windows.h>
#include <winnt.h>
#include <vector>
#include <Psapi.h>
#include "method.h"

using namespace std;

void PrintToConsole(const char* format, ...)
{
    // ����һ���ɱ�����б�
    va_list args;
    va_start(args, format);

    // �����ʽ���ַ�����ĳ���
    int length = _vscprintf(format, args) + 5 + 1; // ��� 5 ���ַ��ĳ������ڴ洢���

    // ���仺�����������ʽ������ַ���
    char* buffer = new char[length];

    // ��ӱ�ǵ���ʽ������ַ���
    snprintf(buffer, length, "[101] %s", format);

    // ��ʽ���ַ���
    vsnprintf(buffer + 5, length - 5, format, args);

    // ����ʽ������ַ������������̨
    printf("%s\n", buffer);

    // �ͷŻ������Ϳɱ�����б�
    delete[] buffer;
    va_end(args);
}

// ��������ʼ������̨
void CreateConsole()
{
    // �������̨
    AllocConsole();
    // ��ȡ��׼��������
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    // ����׼���������ض��򵽿���̨
    hConsole = CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hConsole != INVALID_HANDLE_VALUE)
    {
        SetStdHandle(STD_OUTPUT_HANDLE, hConsole);

        // ʹ�� freopen_s ��� freopen
        FILE* pConsole;
        freopen_s(&pConsole, "CONOUT$", "w", stdout);
        freopen_s(&pConsole, "CONOUT$", "w", stderr);
    }
}

BOOL method::WriteAddressBin(HANDLE hProcess, DWORD64 lpBaseAddress,DWORD64 lpBuffer){
   //dwProcessId ����ID , lpBaseAddress �ڴ��ַ, lpBuffer д�������
    return  WriteProcessMemory(hProcess,(LPVOID)lpBaseAddress, &lpBuffer, sizeof(lpBuffer),NULL);
}
BOOL method::ReadAddressBin(HANDLE hProcess,LPCVOID lpBaseAddress, LPCVOID lpBuffer,size_t size,int offset, ...) {
    if (size == NULL)
    {
        size = sizeof(&lpBuffer);
    }
    if (offset == NULL)//û��ƫ�� ֱ�Ӷ�ȡ
    {
        return  ReadProcessMemory(hProcess, (LPCVOID)lpBaseAddress, &lpBuffer, size, NULL);
    }
    BOOL RET;//����ֵ
    int tp_offsets = 0;//���ܿɱ��������ʱ����
    va_list VA_OFFSETS;//�洢�ɱ�������б�
    LPCVOID address =  lpBaseAddress, temp = lpBaseAddress;//��һ�Σ�����ַ���� ֮�󣺽���ȡ���ĵ�ַ�ŵ�����
    DWORD64 ofs;//�洢���ݵ�ַ�����ĵ�ַ
    va_start(VA_OFFSETS, offset);//��ʼ���б�
    while (tp_offsets != NULL)
    {
        //�״Σ���ȡ��ַ+0ƫ�� ֮�� ��ȡ���ĵ�ַ+ƫ�� ѭ��
        RET = ReadProcessMemory(hProcess, address, &ofs, size, NULL);
        if (!RET)
        {
            //���ܼ�����address�л�ȡ�����ˣ�����ǰ��address���ظ�������
            lpBuffer = &address;
            return FALSE;
        }
        temp = address;
        tp_offsets = va_arg(VA_OFFSETS, int);
        if (tp_offsets != NULL)
        {
            address = LPCVOID(ofs + tp_offsets);//�������ĵ�ַ+ƫ�Ƹ�ֵ�� address ������ȡ
        }
        else
        {
            break;
        }
    }
    va_end(VA_OFFSETS);//����
    //���һ��ƫ��=temp ��Ҫֱ�Ӷ���ַ
    return  ReadProcessMemory(hProcess, temp, &lpBuffer, size, NULL);
}

pair<DWORD64, DWORD64> method::GetModuleAddressRange(HMODULE hModule) {
    MODULEINFO moduleInfo;
    if (GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
        DWORD_PTR baseAddress = reinterpret_cast<DWORD_PTR>(moduleInfo.lpBaseOfDll);
        DWORD moduleSize = moduleInfo.SizeOfImage;

        DWORD64 startAddress = static_cast<DWORD64>(baseAddress);
        DWORD64 endAddress = startAddress + static_cast<DWORD64>(moduleSize);

        return make_pair(startAddress, endAddress);
    }

    return make_pair(0, 0);
}

BOOL method::CompareArrays(const vector<BYTE>& TZM, const vector<BYTE>& NC) {
    DWORD size = static_cast<DWORD>(TZM.size());
    for (DWORD i = 0; i < size; ++i) {
        // ����������Ŀ������Ķ�ӦԪ���Ƿ����
        if (TZM[i] != NC[i] && TZM[i] != 0xFF) {
            // �������ȣ�����ģ��ƥ����ж�
            // ��������벻��ģ��ƥ���ռλ��(0xFF)����Ŀ�����鲻ƥ�䣬�򷵻�FALSE
            return FALSE;
        }
    }
    return TRUE;
}

vector<unsigned char> method::ConvertStringToByteArray(const std::string& featureCode) {
    std::vector<unsigned char> byteArray;
    std::string byteStr;

    for (size_t i = 0; i < featureCode.length(); ++i) {
        if (featureCode[i] == ' ') {
            continue;  // Ignore spaces
        }
        else if (featureCode[i] == '?' && featureCode[i + 1] == '?') {
            byteArray.push_back(0xFF);
            i++;  // Skip the second '?'
        }
        else {
            byteStr = featureCode.substr(i, 2);
            unsigned char byte = std::stoi(byteStr, nullptr, 16);
            byteArray.push_back(byte);
            i++;  // Skip the second character of the byte
        }
    }

    return byteArray;
}

DWORD64 method::LocateSignature(HANDLE hProcess, const string& tezhengma, DWORD64 start, DWORD64 end, int offset) {
    vector<BYTE> tempFeature(4096, 0);
    vector<unsigned char> byteArray = ConvertStringToByteArray(tezhengma);
    DWORD size = static_cast<DWORD>(byteArray.size());
    while (start < end) {
        ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(start), tempFeature.data(), 4096, NULL);
        for (DWORD i = 0; i < 4096 - size; ++i) {
            if (CompareArrays(byteArray, vector<BYTE>(tempFeature.begin() + i, tempFeature.begin() + i + size))) {
                return  start + i + offset; // �ҵ�ƥ��������룬�������������ڵ��ڴ��ַ+ƫ��
            }
        }
        start += 4096 - size; // ����startAddress
    }
    return 0; // ���δ�ҵ�ƥ��������룬�򷵻�0���ʵ��Ĵ������
}


HMODULE hModule = NULL;
void method::loadR3nz(){
        if (hModule == NULL)
        {
            if (GetFileAttributesW(L"R3nzSkin.dll") != INVALID_FILE_ATTRIBUTES)
            {
                hModule = LoadLibraryW(L"R3nzSkin.dll");
                PrintToConsole("R3nzSkin����·���ɹ� hModule = %d",  hModule);
                return;
            }
            PrintToConsole("û�а취����R3nzSkin.dll");
        }
}